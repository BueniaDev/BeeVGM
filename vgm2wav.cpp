/*
    This file is part of the BeeVGM engine.
    Copyright (C) 2021 BueniaDev.

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

// BeeVGM's official VGM-to-WAV converter
//
// Leverages em_inflate tiny inflater from https://github.com/emmanuel-marty/em_inflate

#include <iostream>
#include <functional>
#include "em_inflate.h"
#include "beevgm.h"
using namespace beevgm;
using namespace std;
using namespace std::placeholders;

vector<int16_t> audiobuffer;

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

typedef struct WAV_HEADER {
  /* RIFF Chunk Descriptor */
  uint8_t RIFF[4] = {'R', 'I', 'F', 'F'}; // RIFF Header Magic header
  uint32_t ChunkSize;                     // RIFF Chunk Size
  uint8_t WAVE[4] = {'W', 'A', 'V', 'E'}; // WAVE Header
  /* "fmt" sub-chunk */
  uint8_t fmt[4] = {'f', 'm', 't', ' '}; // FMT header
  uint32_t Subchunk1Size = 16;           // Size of the fmt chunk
  uint16_t AudioFormat = 1; // Audio format 1=PCM,6=mulaw,7=alaw,     257=IBM
                            // Mu-Law, 258=IBM A-Law, 259=ADPCM
  uint16_t NumOfChan = 2;   // Number of channels 1=Mono 2=Sterio
  uint32_t SamplesPerSec = 44100;   // Sampling Frequency in Hz
  uint32_t bytesPerSec = 176400; // bytes per second
  uint16_t blockAlign = 4;          // 2=16-bit mono, 4=16-bit stereo
  uint16_t bitsPerSample = 16;      // Number of bits per sample
  /* "data" sub-chunk */
  uint8_t Subchunk2ID[4] = {'d', 'a', 't', 'a'}; // "data"  string
  uint32_t Subchunk2Size;                        // Sampled data length
} wav_hdr;

void outputsample(array<int16_t, 2> sample)
{
    audiobuffer.push_back(sample[0]);
    audiobuffer.push_back(sample[1]);
}

int main(int argc, char *argv[])
{
    cout << "Welcome to the Blythie VGM-to-WAV Converter." << endl;

    if (argc < 3)
    {
	cout << "Usage: vgm2wav [VGM file] [output file]" << endl;
	return 1;
    }

    bool is_loop_around = false;
    vector<uint8_t> vgm_data = loadVGM(argv[1]);

    if (vgm_data.empty())
    {
	return 1;
    }

    BeeVGM vgmcore;

    if (!vgmcore.load(vgm_data))
    {
	cout << "Could not parse VGM file." << endl;
	return 1;
    }

    while (true)
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
		break;
	    }
	}
    }

    wav_hdr wav;
    wav.ChunkSize = ((audiobuffer.size() * 2) + sizeof(wav_hdr) - 8);
    wav.Subchunk2Size = ((audiobuffer.size() * 2) + sizeof(wav_hdr) - 44);

    ofstream out(argv[2], ios::binary);
    out.write(reinterpret_cast<const char*>(&wav), sizeof(wav));

    for (size_t i = 0; i < audiobuffer.size(); ++i)
    {
	out.write(reinterpret_cast<char*>(&audiobuffer[i]), sizeof(int16_t));
    }

    cout << "WAV succesfully generated." << endl;
    out.close();
    return 0;
}