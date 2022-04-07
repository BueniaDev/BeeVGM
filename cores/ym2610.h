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

#ifndef BEEVGM_YM2610
#define BEEVGM_YM2610

#include <ym2610.h>
#include <ay8910.h>
using namespace beenuked;
using namespace beepsg;

class BeeNuked_OPNBSSG : public OPNBSSGInterface
{
    public:
	BeeNuked_OPNBSSG()
	{
	    psg_core.init(YM2149_Chip);
	}

	~BeeNuked_OPNBSSG()
	{

	}

	void writeIO(int port, uint8_t data)
	{
	    psg_core.writeIO(port, data);
	}

	void clockSSG()
	{
	    psg_core.clock_chip();
	}

	array<int32_t, 3> getSamples()
	{
	    auto samples = psg_core.get_samples();

	    array<int32_t, 3> final_samples = {samples[0], samples[1], samples[2]};
	    return final_samples;
	}

    private:
	AY8910 psg_core;
};

class BeeVGM_YM2610
{
    public:
	BeeVGM_YM2610()
	{
	    chip.set_ssg_interface(new BeeNuked_OPNBSSG(ssg));
	}

	~BeeVGM_YM2610()
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
	    if (type == 0)
	    {
		chip.writeADPCM_ROM(rom_size, data_start, data_len, rom_data);
	    }
	    else if (type == 1)
	    {
		chip.writeDelta_ROM(rom_size, data_start, data_len, rom_data);
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

	    array<int32_t, 2> final_samples = {0, 0};

	    for (int i = 0; i < 2; i++)
	    {
		final_samples[i] = ((samples[0] * 3) + samples[1 + i]);
	    }

	    return final_samples;
	}

    private:
	YM2610 chip;
	BeeNuked_OPNBSSG ssg;
};

#endif // BEEVGM_YM2610