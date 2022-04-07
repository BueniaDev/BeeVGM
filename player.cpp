/*
    This file is part of the BeeVGM engine.
    Copyright (C) 2022 BueniaDev.

    BeeVGM is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    BeeVGM is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with BeeVGM.  If not, see <https://www.gnu.org/licenses/>.
*/

// BeeVGM's official VGM player frontend
//
// Leverages em_inflate tiny inflater from https://github.com/emmanuel-marty/em_inflate

#include <iostream>
#include <functional>
#include <signal.h>
#include <utfcpp/utf8.h>
#include <SDL2/SDL.h>
#include "em_inflate.h"
#include "beevgm.h"
using namespace beevgm;
using namespace std;
using namespace utf8;
using namespace std::placeholders;

vector<int16_t> audiobuffer;

bool is_exit = false;

void signal_callback(int signum)
{
    cout << "Exiting..." << endl;
    is_exit = true;
}

vector<uint8_t> loadFile(string filename)
{
    vector<uint8_t> result;
    ifstream file(filename.c_str(), ios::in | ios::binary | ios::ate);

    if (!file.is_open())
    {
	return result;
    }

    streampos size = file.tellg();
    result.resize(size, 0);
    file.seekg(0, ios::beg);
    file.read((char*)result.data(), result.size());
    file.close();
    return result;
}

bool decompressVGM(vector<uint8_t> vgm_memory, vector<uint8_t> &vgm_data)
{
    uint8_t *end = &vgm_memory[vgm_memory.size()];
    uint32_t uncompressed = end[-4] | (end[-3] << 8) | (end[-2] << 16) | (end[-1] << 24);
    vgm_data.resize(uncompressed, 0);

    int result = em_inflate(vgm_memory.data(), vgm_memory.size(), vgm_data.data(), vgm_data.size());

    if (result == -1)
    {
	cout << "Error decompressing data from file" << endl;
	return false;
    }

    return true;
}

string gd3_vec_to_utf8(BeeGD3_Vec vec)
{
    string utf8_tag;
    utf16to8(vec.begin(), vec.end(), back_inserter(utf8_tag));
    return utf8_tag;
}


void printGD3Tag(BeeVGM &vgm)
{
    BeeGD3 tag = vgm.getGD3Tag();

    if (!tag.is_found())
    {
	cout << "No GD3 tag found" << endl;
	return;
    }

    cout << "Gd3 info: " << endl;
    cout << "Track name (English): " << gd3_vec_to_utf8(tag.get_track_name_en()) << endl;
    cout << "Track name (Japanese): " << gd3_vec_to_utf8(tag.get_track_name_jp()) << endl;
    cout << "Game name (English): " << gd3_vec_to_utf8(tag.get_game_name_en()) << endl;
    cout << "Game name (Japanese): " << gd3_vec_to_utf8(tag.get_game_name_jp()) << endl;
    cout << "System name (English): " << gd3_vec_to_utf8(tag.get_sys_name_en()) << endl;
    cout << "System name (Japanese): " << gd3_vec_to_utf8(tag.get_sys_name_jp()) << endl;
    cout << "Track author (English): " << gd3_vec_to_utf8(tag.get_track_author_en()) << endl;
    cout << "Track author (Japanese): " << gd3_vec_to_utf8(tag.get_track_author_jp()) << endl;
    cout << "Game release date: " << gd3_vec_to_utf8(tag.get_game_release_date()) << endl;
    cout << "Converted by: " << gd3_vec_to_utf8(tag.get_name_of_converter()) << endl;
    cout << "Notes: " << gd3_vec_to_utf8(tag.get_notes()) << endl;
    cout << endl;
}

vector<uint8_t> loadVGM(string filename)
{
    vector<uint8_t> emptyvec; // Empty vector (return value if file loading fails)
    vector<uint8_t> data = loadFile(filename);

    if (data.empty())
    {
	return emptyvec;
    }

    if (data.size() >= 10 && (data[0] == 0x1F && data[1] == 0x8B && data[2] == 0x08))
    {
	vector<uint8_t> compressed_data = data;

	vector<uint8_t> uncompressed_data;

	if (!decompressVGM(compressed_data, uncompressed_data))
	{
	    return emptyvec;
	}

	return uncompressed_data;
    }

    return data;
}

void outputsample(array<int16_t, 2> sample)
{
    audiobuffer.push_back(sample[0]);
    audiobuffer.push_back(sample[1]);

    if (audiobuffer.size() >= 4096)
    {
	audiobuffer.clear();

	while (SDL_GetQueuedAudioSize(1) > (4096 * sizeof(int16_t)))
	{
	    SDL_Delay(1);
	}

	SDL_QueueAudio(1, audiobuffer.data(), (4096 * sizeof(int16_t)));
    }
}

int main(int argc, char* argv[])
{
    cout << "Welcome to the Blythie VGM Player." << endl;

    if (argc < 2)
    {
	cout << "Usage: vgmplayer [VGM file]" << endl;
	return 1;
    }

    signal(SIGINT, signal_callback);

    vector<uint8_t> vgm_data = loadVGM(argv[1]);

    if (vgm_data.empty())
    {
	cout << "Could not load VGM file." << endl;
	return 1;
    }

    BeeVGM vgmcore;

    if (!vgmcore.load(vgm_data))
    {
	cout << "Could not parse VGM file." << endl;
	return 1;
    }

    printGD3Tag(vgmcore);

    bool is_loop_around = false;

    SDL_Init(SDL_INIT_AUDIO);

    SDL_AudioSpec audiospec;
    audiospec.freq = 44100;
    audiospec.format = AUDIO_S16SYS;
    audiospec.channels = 2;
    audiospec.samples = 4096;
    audiospec.callback = NULL;

    SDL_OpenAudio(&audiospec, NULL);
    SDL_PauseAudio(0);

    while (!is_exit)
    {
	uint32_t num_samples = vgmcore.decodeFrame();

	if (num_samples > 0)
	{
	    for (uint32_t i = 0; i < num_samples; i++)
	    {
		array<int16_t, 2> audiosample = vgmcore.generateSample();
		outputsample(audiosample);
	    }
	}

	// End of stream
	if (vgmcore.isEndofStream())
	{
	    // If the VGM file has a loop offset, then loop around once
	    uint32_t loop_offs = vgmcore.getLoopOffset();

	    if ((loop_offs != 0) && !is_loop_around)
	    {
		vgmcore.seekLoop(loop_offs);
		is_loop_around = true;
	    }
	    else
	    {
		// Drain any remaining queued audio
		while (SDL_GetQueuedAudioSize(1) > 0)
		{
		    SDL_Delay(1);
		}

		break;
	    }
	}
    }
	
    SDL_PauseAudio(1);
    SDL_CloseAudio();
    SDL_Quit();
    return 0;
}