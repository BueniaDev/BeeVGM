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

#ifndef BEEVGM_RF5C68
#define BEEVGM_RF5C68

#include <rf5c68.h>
using namespace beepcm;

class BeeVGM_RF5C68
{
    public:
	BeeVGM_RF5C68()
	{
	    chip.init();
	    chip.enable_vgm_hack();
	}

	~BeeVGM_RF5C68()
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
	    switch (port)
	    {
		case 0: chip_addr = (data << 8); break;
		case 1: chip_addr |= data; break;
		case 2: chip.writemem(chip_addr, data); break;
		case 3: chip_reg = data; break;
		case 4: chip.writereg(chip_reg, data); break;
	    }
	}

	void writeROM(int type, size_t rom_size, size_t data_start, size_t data_len, vector<uint8_t> rom_data)
	{
	    return;
	}

	void writeRAM(int data_start, int data_len, vector<uint8_t> ram_data)
	{
	    chip.writeRAM(data_start, data_len, ram_data);
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
	RF5C68 chip;

	uint16_t chip_addr = 0;

	uint8_t chip_reg = 0;
};

#endif // BEEVGM_RF5C68