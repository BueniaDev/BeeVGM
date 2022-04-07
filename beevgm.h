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

// BeeVGM - core VGM player logic
//
// To-do list:
// Finish implementing dual-chip support
// Finish up existing sound chip emulators
// Ensure full compliance with VGM v1.60 specification

#ifndef BEEVGM_H
#define BEEVGM_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <vector>
#include <array>
#include <functional>
#include <bitset>
#include <cores/sn76489.h>
#include <cores/ym2413.h>
#include <cores/ym2612.h>
#include <cores/ym2151.h>
#include <cores/ym2203.h>
#include <cores/ym2610.h>
#include <cores/ym3526.h>
#include <cores/y8950.h>
#include <cores/ym3812.h>
#include <cores/ymf262.h>
#include <cores/segapcm.h>
#include <cores/ymz280b.h>
#include <cores/rf5c68.h>
#include <cores/multipcm.h>
using namespace std;

namespace beevgm
{
    typedef vector<uint16_t> BeeGD3_Vec;

    class BeeGD3
    {
	public:
	    BeeGD3()
	    {

	    }

	    bool open(vector<uint8_t> memory, size_t gd3_pos)
	    {
		if (is_parsed)
		{
		    return is_gd3_found;
		}

		is_parsed = true;

		if (gd3_pos == 0)
		{
		    return false;
		}

		char id_str[4] = {'G', 'd', '3', ' '};

		bool is_gd3_id_match = true;

		for (int i = 0; i < 4; i++)
		{
		    char gd3_char = readByte(memory, (gd3_pos + i));

		    if (gd3_char != id_str[i])
		    {
			is_gd3_id_match = false;
			break;
		    }
		}

		if (!is_gd3_id_match)
		{
		    cout << "GD3 ID mismatch" << endl;
		    return false;
		}

		uint32_t gd3_ver = readLong(memory, (gd3_pos + 4));

		if (gd3_ver != 0x100)
		{
		    cout << "GD3 version mismatch" << endl;
		    return false;
		}

		tag_offset = (gd3_pos + 12);
		is_gd3_found = true;

		createGD3Vec(memory, track_name_en);
		createGD3Vec(memory, track_name_jp);
		createGD3Vec(memory, game_name_en);
		createGD3Vec(memory, game_name_jp);
		createGD3Vec(memory, sys_name_en);
		createGD3Vec(memory, sys_name_jp);
		createGD3Vec(memory, track_author_en);
		createGD3Vec(memory, track_author_jp);
		createGD3Vec(memory, game_release_date);
		createGD3Vec(memory, name_of_converter);
		createGD3Vec(memory, notes);
		return true;
	    }

	    bool is_found()
	    {
		return is_gd3_found;
	    }

	    BeeGD3_Vec get_track_name_en()
	    {
		return track_name_en;
	    }

	    BeeGD3_Vec get_track_name_jp()
	    {
		return track_name_jp;
	    }

	    BeeGD3_Vec get_game_name_en()
	    {
		return game_name_en;
	    }

	    BeeGD3_Vec get_game_name_jp()
	    {
		return game_name_jp;
	    }

	    BeeGD3_Vec get_sys_name_en()
	    {
		return sys_name_en;
	    }

	    BeeGD3_Vec get_sys_name_jp()
	    {
		return sys_name_jp;
	    }

	    BeeGD3_Vec get_track_author_en()
	    {
		return track_author_en;
	    }

	    BeeGD3_Vec get_track_author_jp()
	    {
		return track_author_jp;
	    }

	    BeeGD3_Vec get_game_release_date()
	    {
		return game_release_date;
	    }

	    BeeGD3_Vec get_name_of_converter()
	    {
		return name_of_converter;
	    }

	    BeeGD3_Vec get_notes()
	    {
		return notes;
	    }

	private:
	    bool is_gd3_found = false;
	    bool is_parsed = false;

	    BeeGD3_Vec track_name_en;
	    BeeGD3_Vec track_name_jp;
	    BeeGD3_Vec game_name_en;
	    BeeGD3_Vec game_name_jp;
	    BeeGD3_Vec sys_name_en;
	    BeeGD3_Vec sys_name_jp;
	    BeeGD3_Vec track_author_en;
	    BeeGD3_Vec track_author_jp;
	    BeeGD3_Vec game_release_date;
	    BeeGD3_Vec name_of_converter;
	    BeeGD3_Vec notes;

	    uint8_t readByte(vector<uint8_t> memory, uint32_t addr)
	    {
		return memory.at(addr);
	    }

	    uint16_t readWord(vector<uint8_t> memory, uint32_t addr)
	    {
		return (readByte(memory, (addr + 1)) << 8) | (readByte(memory, addr));
	    }

	    uint32_t readLong(vector<uint8_t> memory, uint32_t addr)
	    {
		return (readWord(memory, (addr + 2)) << 16) | (readWord(memory, addr));
	    }

	    void createGD3Vec(vector<uint8_t> memory, BeeGD3_Vec &vec)
	    {
		uint16_t track_char = 0;

		do
		{
		    track_char = readWord(memory, tag_offset);
		    tag_offset += 2;
		    vec.push_back(track_char);
		} while (track_char != 0);
	    }

	    uint32_t tag_offset = 0;
    };

    template<class T>
    class BeeVGMChip
    {
	public:
	    BeeVGMChip()
	    {

	    }

	    ~BeeVGMChip()
	    {

	    }

	    void init(uint32_t clockrate, uint32_t samplerate = 44100)
	    {
		clock_rate = (clockrate & 0x3FFFFFFF);
		out_step = chip.get_sample_rate(clock_rate);
		in_step = samplerate;
		out_time = 0.0f;
		is_enabled = true;
	    }

	    bool isChipEnabled()
	    {
		return is_enabled;
	    }

	    void setEnable(bool enable_val)
	    {
		is_enabled = enable_val;
	    }

	    void enableOutput(bool enable_val)
	    {
		is_output = enable_val;
	    }

	    void config(uint32_t flags)
	    {
		chip.save_config(flags);
	    }

	    void writeYM(uint8_t reg, uint8_t data)
	    {
		writeYM(0, reg, data);
	    }

	    void writeYM(int port, uint8_t reg, uint8_t data)
	    {
		int port_val = (port << 1);
		writeIO(port_val, reg);
		writeIO((port_val | 1), data);
	    }

	    void writeIO(int port, uint8_t val)
	    {
		if (!isChipEnabled())
		{
		    return;
		}

		chip.writeIO(port, val);
	    }

	    void writeROM(size_t rom_size, size_t data_start, size_t data_len, vector<uint8_t> rom_data)
	    {
		if (!isChipEnabled())
		{
		    return;
		}

		writeROM(0, rom_size, data_start, data_len, rom_data);
	    }

	    void writeROM(int type, size_t rom_size, size_t data_start, size_t data_len, vector<uint8_t> rom_data)
	    {
		if (!isChipEnabled())
		{
		    return;
		}

		chip.writeROM(type, rom_size, data_start, data_len, rom_data);
	    }

	    void writeRAM(int data_start, int data_len, vector<uint8_t> ram_data)
	    {
		if (!isChipEnabled())
		{
		    return;
		}

		chip.writeRAM(data_start, data_len, ram_data);
	    }

	    void writeBank(uint8_t channel, uint16_t bank_offs)
	    {
		if (!isChipEnabled())
		{
		    return;
		}

		chip.writeIO(3, channel);
		chip.writeIO(4, (bank_offs >> 8));
		chip.writeIO(5, (bank_offs & 0xFF));
	    }

	    void writeMem(uint16_t addr, uint8_t data)
	    {
		if (!isChipEnabled())
		{
		    return;
		}

		chip.writeIO(0, (addr >> 8));
		chip.writeIO(1, (addr & 0xFF));
		chip.writeIO(2, data);
	    }

	    void writeReg(uint8_t reg, uint8_t data)
	    {
		if (!isChipEnabled())
		{
		    return;
		}

		chip.writeIO(3, reg);
		chip.writeIO(4, data);
	    }

	    void add_samples(array<int32_t, 2> &old_samples)
	    {
		if (!isChipEnabled() || !is_output)
		{
		    return;
		}

		auto new_samples = chipclock();
		for (int i = 0; i < 2; i++)
		{
		    old_samples[i] = mix_sample(old_samples[i], new_samples[i]);
		}
	    }

	private:
	    T chip;

	    float out_step = 0.0f;
	    float in_step = 0.0f;
	    float out_time = 0.0f;
	    bool is_enabled = false;
	    bool is_output = true;

	    uint32_t clock_rate = 0;

	    array<int32_t, 2> chipclock()
	    {
		while (out_step > out_time)
		{
		    chip.clock();
		    out_time += in_step;
		}

		out_time -= out_step;

		array<int32_t, 2> sample = chip.get_sample();
		return sample;
	    }

	    int32_t mix_sample(int32_t old_sample, int32_t new_sample)
	    {
		int32_t sample1 = int32_t(old_sample);
		int32_t sample2 = clamp<int32_t>(new_sample, -32768, 32767);

		int32_t result = (sample1 + sample2);
		return clamp<int32_t>(result, -32768, 32767);
	    }
    };

    template<class T>
    class BeeVGMDualChip
    {
	public:
	    BeeVGMDualChip()
	    {

	    }

	    ~BeeVGMDualChip()
	    {

	    }

	    void init(uint32_t clock_rate)
	    {
		bool is_dual_chip = ((clock_rate >> 30) & 1);
		clock_rate &= 0x3FFFFFFF;

		sound_chips[0].init(clock_rate);

		if (is_dual_chip)
		{
		    cout << "Dual chips detected" << endl;
		    sound_chips[1].init(clock_rate);
		}
	    }

	    void writeYM(bool is_chip2, uint8_t reg, uint8_t data)
	    {
		writeYM(is_chip2, 0, reg, data);
	    }

	    void writeYM(bool is_chip2, uint8_t port, uint8_t reg, uint8_t data)
	    {
		getChip(is_chip2).writeYM(port, reg, data);
	    }

	    void writeIO(bool is_chip2, int port, uint8_t data)
	    {
		getChip(is_chip2).writeIO(port, data);
	    }

	    T& getChip(bool is_chip2)
	    {
		int chip_num = (is_chip2) ? 1 : 0;
		return sound_chips.at(chip_num);
	    }

	    T& at(int index)
	    {
		if ((index < 0) || (index >= 2))
		{
		    throw out_of_range("Invalid chip number");
		}

		return sound_chips.at(index);
	    }

	    T& operator [](int index)
	    {
		return at(index);
	    }

	    void add_samples(array<int32_t, 2> &old_samples)
	    {
		for (auto &chip : sound_chips)
		{
		    chip.add_samples(old_samples);
		}
	    }

	private:
	    array<T, 2> sound_chips;
    };

    using SNPSG = BeeVGMChip<BeeVGM_SN76489>;
    using OPL = BeeVGMChip<BeeVGM_YM3526>;
    // using OPL = BeeVGMChip<BeeVGM_YM3526_OPL3>;
    using OPL_MSX = BeeVGMChip<BeeVGM_Y8950>;
    using OPL2 = BeeVGMChip<BeeVGM_YM3812>;
    using OPL3 = BeeVGMChip<BeeVGM_YMF262>;
    using OPLL = BeeVGMChip<BeeVGM_YM2413>;
    using OPN2Type = BeeVGMChip<BeeVGM_YM2612>;
    using OPN2 = BeeVGMDualChip<OPN2Type>;
    using OPM = BeeVGMChip<BeeVGM_YM2151>;
    using OPN = BeeVGMChip<BeeVGM_YM2203>;
    using OPNB = BeeVGMChip<BeeVGM_YM2610>;
    using SegaPCM = BeeVGMChip<BeeVGM_SegaPCM>;
    using YMZ280B = BeeVGMChip<BeeVGM_YMZ280B>;
    using RF5C68 = BeeVGMChip<BeeVGM_RF5C68>;
    using MultiPCMType = BeeVGMChip<BeeVGM_MultiPCM>;
    using MultiPCM = BeeVGMDualChip<MultiPCMType>;

    class BeeVGM
    {
	public:
	    BeeVGM();
	    ~BeeVGM();

	    bool load(vector<uint8_t> memory);
	    uint32_t decodeFrame();
	    array<int16_t, 2> generateSample();
	    bool isEndofStream();
	    uint32_t getLoopOffset();
	    void seekLoop(uint32_t offset);
	    BeeGD3 getGD3Tag();

	private:
	    bool parseheader();

	    void unrecognized_instr(uint8_t vgm_instr);
	    uint32_t fetch_start();
	    bool is_at_least(uint8_t major, uint8_t minor);
	    string fetch_version_str(bool is_wip);
	    void detect_standard_features();
	    void detect_extra_features();
	    void detect_v151_features();
	    void detect_v161_features();

	    vector<uint8_t> vgm_data;
	    uint32_t vgm_pos = 0;
	    uint32_t vgm_version = 0;
	    uint32_t vgm_loop_offset = 0;
	    bool end_of_stream = false;

	    uint8_t readByte(uint32_t addr);
	    uint16_t readWord(uint32_t addr);
	    uint32_t readHLong(uint32_t addr);
	    uint32_t readLong(uint32_t addr);
	    uint32_t readLongHeader(uint32_t addr);

	    uint8_t getimmByte();
	    uint16_t getimmWord();
	    uint32_t getimmHLong();
	    uint32_t getimmLong();

	    void init_sn76489();
	    void init_ym2413();
	    void init_ym2612();
	    void init_ym2151();

	    void init_segapcm();
	    void init_ym2203();
	    void init_ym2610();
	    void init_ymz280b();
	    void init_ym3526();
	    void init_y8950();
	    void init_ym3812();
	    void init_ymf262();
	    void init_rf5c68();

	    void init_multipcm();

	    uint32_t pcm_pos = 0;

	    bool is_ymfm_auto = false;

	    SNPSG snpsg_chip;
	    OPLL opll_chip;
	    OPN2 opn2_chips;
	    OPM opm_chip;
	    OPN opn_chip;
	    OPNB opnb_chip;
	    OPL opl_chip;
	    OPL_MSX opl_msx_chip;
	    OPL2 opl2_chip;
	    OPL3 opl3_chip;
	    SegaPCM segapcm_chip;
	    YMZ280B ymz280b_chip;
	    RF5C68 rf5c68_chip;
	    MultiPCM multipcm_chips;

	    array<vector<uint8_t>, 0x40> pcm_data;

	    BeeGD3 vgm_tag;

	    uint32_t gd3_pos = 0;
	    void parseGD3();
    };
};


#endif // BEEVGM_H