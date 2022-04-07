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

#ifndef BEEVGM_YM3526
#define BEEVGM_YM3526

#include <ym3526.h>
#include <ymf262.h>
using namespace beenuked;

template<class T, int scale = 1>
class BeeVGM_OPLx
{
    public:
	BeeVGM_OPLx()
	{
	    chip.init();
	}

	~BeeVGM_OPLx()
	{

	}

	uint32_t get_sample_rate(uint32_t clock_rate)
	{
	    return chip.get_sample_rate(clock_rate * scale);
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
	    // Convert mono sample to stereo
	    array<int32_t, 2> final_samples = {samples[0], samples[0]};
	    return final_samples;
	}

    private:
	T chip;
};

using BeeVGM_YM3526 = BeeVGM_OPLx<YM3526>;
using BeeVGM_YM3526_OPL3 = BeeVGM_OPLx<YMF262, 4>;

#endif // BEEVGM_YM3526