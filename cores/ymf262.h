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

#ifndef BEEVGM_YMF262
#define BEEVGM_YMF262

#include <ymf262.h>
using namespace beenuked;

class BeeVGM_YMF262
{
    public:
	BeeVGM_YMF262()
	{
	    chip.init();
	}

	~BeeVGM_YMF262()
	{

	}

	uint32_t get_sample_rate(uint32_t clock_rate)
	{
	    return chip.get_sample_rate(clock_rate);
	}

	void save_config(uint32_t flags)
	{
	    return;
	}

	void writeIO(int port, uint8_t data)
	{
	    chip.writeIO(port, data);
	}

	void writeROM(int type, size_t rom_size, size_t data_start, size_t data_len, vector<uint8_t> rom_data)
	{
	    return;
	}

	void writeRAM(int data_start, int data_len, vector<uint8_t> ram_data)
	{
	    return;
	}

	void clock()
	{
	    chip.clockchip();
	}

	array<int32_t, 2> get_sample()
	{
	    auto samples = chip.get_samples();

	    // Outputs A and C get routed to left channel...
	    int32_t sample_left = mix_sample(samples[0], samples[2]);
	    // ...while outputs B and D get routed to right channel...
	    int32_t sample_right = mix_sample(samples[1], samples[3]);

	    array<int32_t, 2> final_samples = {sample_left, sample_right};
	    return final_samples;
	}

    private:
	YMF262 chip;

	int32_t mix_sample(int32_t sample1, int32_t sample2)
	{
	    int32_t old_sample = sample1;
	    int32_t new_sample = clamp<int32_t>(sample2, -32768, 32767);

	    int32_t result = (old_sample + new_sample);
	    return clamp<int32_t>(result, -32768, 32767);
	}
};

#endif // BEEVGM_YMF262