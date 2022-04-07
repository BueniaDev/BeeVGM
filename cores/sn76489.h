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

#ifndef BEEVGM_SN76489
#define BEEVGM_SN76489

#include <sn76489.h>
using namespace beepsg;

class BeeVGM_SN76489
{
    public:
	BeeVGM_SN76489()
	{
	    chip.reset();
	}

	~BeeVGM_SN76489()
	{

	}

	uint32_t get_sample_rate(uint32_t clock_rate)
	{
	    return chip.get_sample_rate(clock_rate);
	}

	void save_config(uint32_t flags)
	{
	    int noisefb = (flags >> 16);
	    int lfsrbitwidth = ((flags >> 8) & 0xFF);
	    chip.config(noisefb, lfsrbitwidth);
	}

	void writeIO(int port, uint8_t data)
	{
	    if ((port & 1) == 0)
	    {
		chip.writeIO(data);
	    }
	    else
	    {
		chip.writestereo(data);
	    }
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
	    array<int32_t, 2> final_samples = {samples[0], samples[1]};
	    return final_samples;
	}

    private:
	SN76489 chip;
};

#endif // BEEVGM_SN76489