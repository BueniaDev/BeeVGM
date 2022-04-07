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

#ifndef BEEVGM_YM2203
#define BEEVGM_YM2203

#include <ym2203.h>
#include <ay8910.h>
using namespace beenuked;
using namespace beepsg;

class BeeNuked_OPNSSG : public OPNSSGInterface
{
    public:
	BeeNuked_OPNSSG()
	{
	    psg_core.init(YM2149_Chip);
	}

	~BeeNuked_OPNSSG()
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

class BeeVGM_YM2203
{
    public:
	BeeVGM_YM2203()
	{
	    chip.set_ssg_interface(new BeeNuked_OPNSSG(ssg));
	    chip.init();
	}

	~BeeVGM_YM2203()
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

	    int32_t final_sample = 0;

	    for (auto &sample : samples)
	    {
		final_sample += sample;
	    }

	    array<int32_t, 2> final_samples = {final_sample, final_sample};

	    return final_samples;
	}

    private:
	YM2203 chip;
	BeeNuked_OPNSSG ssg;
};

#endif // BEEVGM_YM2203