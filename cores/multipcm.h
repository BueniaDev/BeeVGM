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

#ifndef BEEVGM_MULTIPCM
#define BEEVGM_MULTIPCM

#include <multipcm.h>
using namespace beepcm;

class BeeVGM_MultiPCM
{
    public:
	BeeVGM_MultiPCM()
	{
	    chip.init();
	}

	~BeeVGM_MultiPCM()
	{

	}

	uint32_t get_sample_rate(uint32_t clock_rate)
	{
	    // Fix VGM clock rate, which is based on the old /180 clock divider
	    clock_rate = ((clock_rate * 224) / 180);
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
		case 0:
		case 1:
		case 2: chip.writeIO(port, data); break;
		case 3: channel = data; break;
		case 4: bank_offs = (data << 8); break;
		case 5:
		{
		    bank_offs |= data;
		    chip.writeBankVGM(channel, bank_offs);
		}
		break;
	    }
	}

	void writeROM(int type, size_t rom_size, size_t data_start, size_t data_len, vector<uint8_t> rom_data)
	{
	    if (type == 0)
	    {
		chip.writeROM(rom_size, data_start, data_len, rom_data);
	    }
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
	MultiPCM chip;

	uint8_t channel = 0;
	uint16_t bank_offs = 0;
};

#endif // BEEVGM_MULTIPCM