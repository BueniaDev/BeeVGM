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

#include "beevgm.h"
using namespace beevgm;
using namespace std;

BeeVGM::BeeVGM()
{

}

BeeVGM::~BeeVGM()
{

}

bool BeeVGM::load(vector<uint8_t> memory)
{
    vgm_data = memory;
    return parseheader();
}

bool BeeVGM::parseheader()
{
    if (vgm_data.size() < 64)
    {
	return false;
    }

    if (vgm_data[0] != 'V' || vgm_data[1] != 'g' || vgm_data[2] != 'm' || vgm_data[3] != ' ')
    {
	cout << "Data does not appear to be valid VGM data." << endl;
	return false;
    }

    vgm_loop_offset = readLong(0x1C);
    vgm_version = readLong(0x8);
    vgm_pos = fetch_start();

    uint32_t gd3_offs = readLong(0x14);

    if (gd3_offs != 0)
    {
	gd3_pos = (0x14 + gd3_offs);
    }

    parseGD3();

    if (is_at_least(1, 70))
    {
	cout << "Warning: version > 1.61 detected, some things may not work properly!" << endl;
    }
    else
    {
	cout << fetch_version_str(is_at_least(1, 51)) << endl;
    }

    detect_standard_features();
    detect_extra_features();

    return true;
}

void BeeVGM::parseGD3()
{
    if (vgm_tag.open(vgm_data, gd3_pos))
    {
	cout << "GD3 found" << endl;
    }
}

BeeGD3 BeeVGM::getGD3Tag()
{
    return vgm_tag;
}

uint32_t BeeVGM::getLoopOffset()
{
    if (vgm_loop_offset == 0)
    {
	return 0;
    }
    else
    {
	return (0x1C + vgm_loop_offset);
    }
}

void BeeVGM::seekLoop(uint32_t offset)
{
    uint32_t prev_vgm_pos = vgm_pos;
    vgm_pos = offset;

    if (end_of_stream && (vgm_pos < prev_vgm_pos))
    {
	end_of_stream = false;
    }
}

void BeeVGM::detect_standard_features()
{
    uint32_t snpsg_clk = readLong(0xC);

    if (snpsg_clk != 0)
    {
	init_sn76489();
    }

    uint32_t ym2413_clk = readLong(0x10);

    if (is_at_least(1, 10))
    {
	if (ym2413_clk != 0)
	{
	    init_ym2413();
	}

	uint32_t ym2612_clk = readLong(0x2C);

	if (ym2612_clk != 0)
	{
	    init_ym2612();
	}

	uint32_t ym2151_clk = readLong(0x30);

	if (ym2151_clk != 0)
	{
	    init_ym2151();
	}
    }
    else
    {
	if (ym2413_clk != 0)
	{
	    cout << "Auto-detecting FM sound chip..." << endl;
	    is_ymfm_auto = true;
	}
    }
}

void BeeVGM::detect_extra_features()
{
    detect_v151_features();
    detect_v161_features();
}

void BeeVGM::detect_v151_features()
{
    if (!is_at_least(1, 51))
    {
	return;
    }

    uint32_t segapcm_clk = readLongHeader(0x38);

    if (segapcm_clk != 0)
    {
	init_segapcm();
    }

    uint32_t rf5c68_clk = readLongHeader(0x40);

    if (rf5c68_clk != 0)
    {
	init_rf5c68();
    }

    uint32_t ym2203_clk = readLongHeader(0x44);

    if (ym2203_clk != 0)
    {
	init_ym2203();
    }

    uint32_t ym2610_clk = readLongHeader(0x4C);

    if (ym2610_clk != 0)
    {
	init_ym2610();
    }

    uint32_t ym3812_clk = readLongHeader(0x50);

    if (ym3812_clk != 0)
    {
	init_ym3812();
    }

    uint32_t ym3526_clk = readLongHeader(0x54);

    if (ym3526_clk != 0)
    {
	init_ym3526();
    }

    uint32_t y8950_clk = readLongHeader(0x58);

    if (y8950_clk != 0)
    {
	init_y8950();
    }

    uint32_t ymf262_clk = readLongHeader(0x5C);

    if (ymf262_clk != 0)
    {
	init_ymf262();
    }

    uint32_t ymz280b_clk = readLongHeader(0x68);

    if (ymz280b_clk != 0)
    {
	init_ymz280b();
    }
}

void BeeVGM::detect_v161_features()
{
    if (!is_at_least(1, 61))
    {
	return;
    }

    uint32_t multipcm_clk = readLongHeader(0x88);

    if (multipcm_clk != 0)
    {
	init_multipcm();
    }
}

void BeeVGM::init_sn76489()
{
    cout << "SN76489 detected" << endl;
    uint32_t snpsg_clk = readLong(0xC);
    snpsg_clk &= 0x3FFFFFFF;
    int noisefb = vgm_version < 0x110 ? 9 : readWord(0x28);
    int lfsrbitwidth = vgm_version < 0x110 ? 16 : readByte(0x2A);
    uint32_t flags = ((noisefb << 16) | (lfsrbitwidth << 8));
    cout << "Setting SN76489 clock rate to " << dec << (int)snpsg_clk << " Hz" << endl;
    snpsg_chip.init(snpsg_clk);
    snpsg_chip.config(flags);
}

void BeeVGM::init_ym2413()
{
    cout << "YM2413 detected" << endl;
    uint32_t ym2413_clk = readLong(0x10);
    ym2413_clk &= 0x3FFFFFFF;
    cout << "Setting YM2413 clock rate to " << dec << (int)ym2413_clk << " Hz" << endl;
    opll_chip.init(ym2413_clk);
    is_ymfm_auto = false;
}

void BeeVGM::init_ym2612()
{
    cout << "YM2612 detected" << endl;
    uint32_t ym2612_clk = is_at_least(1, 10) ? readLong(0x2C) : readLong(0x10);
    uint32_t clk_masked = (ym2612_clk & 0x3FFFFFFF);
    cout << "Setting YM2612 clock rate to " << dec << int(clk_masked) << " Hz" << endl;
    opn2_chips.init(ym2612_clk);
    is_ymfm_auto = false;
}

void BeeVGM::init_ym2151()
{
    cout << "YM2151 detected" << endl;
    uint32_t ym2151_clk = is_at_least(1, 10) ? readLong(0x30) : readLong(0x10);
    ym2151_clk &= 0x3FFFFFFF;
    cout << "Setting YM2151 clock rate to " << dec << (int)ym2151_clk << " Hz" << endl;
    opm_chip.init(ym2151_clk);
    is_ymfm_auto = false;
}

void BeeVGM::init_segapcm()
{
    cout << "SegaPCM detected" << endl;
    uint32_t segapcm_clk = readLong(0x38);
    segapcm_clk &= 0x3FFFFFFF;
    cout << "Setting SegaPCM clock rate to " << dec << int(segapcm_clk) << " Hz" << endl;
    uint32_t inter_reg = readLong(0x3C);
    cout << "Setting SegaPCM interface register to " << hex << int(inter_reg) << endl;
    segapcm_chip.init(segapcm_clk);
    segapcm_chip.config(inter_reg);
}

void BeeVGM::init_ym2203()
{
    cout << "YM2203 detected" << endl;
    uint32_t ym2203_clk = readLong(0x44);
    ym2203_clk &= 0x3FFFFFFF;
    cout << "Setting YM2203 clock to " << dec << ym2203_clk << " Hz" << endl;
    opn_chip.init(ym2203_clk);
}

void BeeVGM::init_ym2610()
{
    cout << "YM2610 detected" << endl;
    uint32_t ym2610_clk = readLong(0x4C);
    ym2610_clk &= 0x3FFFFFFF;
    cout << "Setting YM2610 clock to " << dec << ym2610_clk << " Hz" << endl;
    opnb_chip.init(ym2610_clk);
}

void BeeVGM::init_ym3812()
{
    cout << "YM3812 detected" << endl;
    uint32_t ym3812_clk = readLong(0x50);
    ym3812_clk &= 0x3FFFFFFF;
    cout << "Setting YM3812 clock to " << dec << ym3812_clk << " Hz" << endl;
    opl2_chip.init(ym3812_clk);
}

void BeeVGM::init_ym3526()
{
    cout << "YM3526 detected" << endl;
    uint32_t ym3526_clk = readLong(0x54);
    ym3526_clk &= 0x3FFFFFFF;
    cout << "Setting YM3526 clock to " << dec << ym3526_clk << " Hz" << endl;
    opl_chip.init(ym3526_clk);
}

void BeeVGM::init_y8950()
{
    cout << "Y8950 detected" << endl;
    uint32_t y8950_clk = readLong(0x58);
    y8950_clk &= 0x3FFFFFFF;
    cout << "Setting Y8950 clock to " << dec << y8950_clk << " Hz" << endl;
    opl_msx_chip.init(y8950_clk);
}

void BeeVGM::init_ymf262()
{
    cout << "YMF262 detected" << endl;
    uint32_t ymf262_clk = readLong(0x5C);
    ymf262_clk &= 0x3FFFFFFF;
    cout << "Setting YMF262 clock to " << dec << ymf262_clk << " Hz" << endl;
    opl3_chip.init(ymf262_clk);
}

void BeeVGM::init_ymz280b()
{
    cout << "YMZ280B detected" << endl;
    uint32_t ymz280b_clk = readLong(0x68);
    ymz280b_clk &= 0x3FFFFFFF;
    cout << "Setting YMZ280B clock to " << dec << ymz280b_clk << " Hz" << endl;
    ymz280b_chip.init(ymz280b_clk);
}

void BeeVGM::init_rf5c68()
{
    cout << "Ricoh RF5C68 detected" << endl;

    uint32_t rf5c68_clk = readLong(0x40);
    rf5c68_clk &= 0x3FFFFFFF;
    cout << "Setting RF5C68 clock to " << dec << rf5c68_clk << " Hz" << endl;
    rf5c68_chip.init(rf5c68_clk);
}

void BeeVGM::init_multipcm()
{
    cout << "Sega MultiPCM detected" << endl;
    uint32_t multipcm_clk = readLong(0x88);
    uint32_t clk_masked = (multipcm_clk & 0x3FFFFFFF);
    cout << "Setting MultiPCM clock to " << dec << clk_masked << " Hz" << endl;
    multipcm_chips.init(multipcm_clk);
}

bool BeeVGM::is_at_least(uint8_t major, uint8_t minor)
{
    auto toBCD = [&](uint8_t val) -> uint8_t {
	return ((val / 10) << 4) + (val % 10);
    };

    uint8_t ver_major = toBCD(major);
    uint8_t ver_minor = toBCD(minor);

    uint16_t ver_number = ((ver_major << 8) | ver_minor);
    return (vgm_version >= ver_number);
}

string BeeVGM::fetch_version_str(bool is_wip)
{
    uint8_t ver_major = (vgm_version >> 8);
    uint8_t ver_minor = (vgm_version & 0xFF);
    stringstream verstr;
    verstr << "VGM v" << hex << (int)ver_major << "." << hex << setw(2) << setfill('0') << (int)ver_minor;

    if (is_wip)
    {
	verstr << " (WIP)";
    }

    return verstr.str();
}

uint8_t BeeVGM::readByte(uint32_t addr)
{
    return vgm_data.at(addr);
}

uint16_t BeeVGM::readWord(uint32_t addr)
{
    return (readByte((addr + 1)) << 8) | (readByte(addr));
}

uint32_t BeeVGM::readHLong(uint32_t addr)
{
    return (readByte((addr + 2)) << 16) | (readWord(addr));
}

uint32_t BeeVGM::readLong(uint32_t addr)
{
    return (readWord((addr + 2)) << 16) | (readWord(addr));
}

uint32_t BeeVGM::readLongHeader(uint32_t addr)
{
    return (fetch_start() >= addr) ? readLong(addr) : 0;
}

uint8_t BeeVGM::getimmByte()
{
    return readByte(vgm_pos++);
}

uint16_t BeeVGM::getimmWord()
{
    uint16_t value = readWord(vgm_pos);
    vgm_pos += 2;
    return value;
}

uint32_t BeeVGM::getimmHLong()
{
    uint32_t value = readHLong(vgm_pos);
    vgm_pos += 3;
    return value;
}

uint32_t BeeVGM::getimmLong()
{
    uint32_t value = readLong(vgm_pos);
    vgm_pos += 4;
    return value;
}

uint32_t BeeVGM::fetch_start()
{
    if (is_at_least(1, 50))
    {
	return (0x34 + readLong(0x34));
    }
    else
    {
	return 0x40;
    }
}

void BeeVGM::unrecognized_instr(uint8_t vgm_instr)
{
    cout << "Unrecognized VGM instruction of " << hex << (int)vgm_instr << endl;
    exit(1);
}

bool BeeVGM::isEndofStream()
{
    return end_of_stream;
}

uint32_t BeeVGM::decodeFrame()
{
    if (end_of_stream)
    {
	return 0;
    }

    uint32_t num_samples = 0;
    uint8_t vgm_instr = getimmByte();

    switch (vgm_instr)
    {
	// Game Gear port 0x06 write
	case 0x4F:
	{
	    uint8_t data = getimmByte();
	    snpsg_chip.writeIO(1, data);
	}
	break;
	// SN76489 write
	case 0x50:
	{
	    uint8_t data = getimmByte();
	    snpsg_chip.writeIO(0, data);
	}
	break;
	// YM2413 write
	case 0x51:
	{
	    if (is_ymfm_auto)
	    {
		init_ym2413();
	    }

	    uint8_t addr = getimmByte();
	    uint8_t data = getimmByte();
	    
	    opll_chip.writeYM(addr, data);
	}
	break;
	// YM2612 chip 0, port 0 write
	case 0x52:
	{
	    if (is_ymfm_auto)
	    {
		init_ym2612();
	    }

	    uint8_t addr = getimmByte();
	    uint8_t data = getimmByte();

	    opn2_chips.writeYM(false, 0, addr, data);
	}
	break;
	// YM2612 chip 0, port 1 write
	case 0x53:
	{
	    if (is_ymfm_auto)
	    {
		init_ym2612();
	    }

	    uint8_t addr = getimmByte();
	    uint8_t data = getimmByte();

	    opn2_chips.writeYM(false, 1, addr, data);
	}
	break;
	// YM2151 write
	case 0x54:
	{
	    if (is_ymfm_auto)
	    {
		init_ym2151();
	    }

	    uint8_t addr = getimmByte();
	    uint8_t data = getimmByte();
	    
	    opm_chip.writeYM(addr, data);
	}
	break;
	// YM2203 write
	case 0x55:
	{
	    uint8_t addr = getimmByte();
	    uint8_t data = getimmByte();

	    opn_chip.writeYM(addr, data);
	}
	break;
	// YM2610 port 0 write
	case 0x58:
	{
	    uint8_t addr = getimmByte();
	    uint8_t data = getimmByte();

	    opnb_chip.writeYM(0, addr, data);
	}
	break;
	// YM2610 port 1 write
	case 0x59:
	{
	    uint8_t addr = getimmByte();
	    uint8_t data = getimmByte();

	    opnb_chip.writeYM(1, addr, data);
	}
	break;
	// YM3812 write
	case 0x5A:
	{
	    uint8_t addr = getimmByte();
	    uint8_t data = getimmByte();

	    opl2_chip.writeYM(addr, data);
	}
	break;
	// YM3526 write
	case 0x5B:
	{
	    uint8_t addr = getimmByte();
	    uint8_t data = getimmByte();

	    opl_chip.writeYM(addr, data);
	}
	break;
	// Y8950 write
	case 0x5C:
	{
	    uint8_t addr = getimmByte();
	    uint8_t data = getimmByte();

	    opl_msx_chip.writeYM(addr, data);
	}
	break;
	// YMZ280B write
	case 0x5D:
	{
	    uint8_t addr = getimmByte();
	    uint8_t data = getimmByte();

	    ymz280b_chip.writeYM(addr, data);
	}
	break;
	// YMF262 port 0 write
	case 0x5E:
	{
	    uint8_t addr = getimmByte();
	    uint8_t data = getimmByte();

	    opl3_chip.writeYM(0, addr, data);
	}
	break;
	// YMF262 port 1 write
	case 0x5F:
	{
	    uint8_t addr = getimmByte();
	    uint8_t data = getimmByte();

	    opl3_chip.writeYM(1, addr, data);
	}
	break;
	// Wait nn samples
	case 0x61:
	{
	    num_samples = getimmWord();
	}
	break;
	// Wait 735 samples
	case 0x62:
	{
	    num_samples = 735;
	}
	break;
	// Wait 882 samples
	case 0x63:
	{
	    num_samples = 882;
	}
	break;
	// End of stream
	case 0x66:
	{
	    end_of_stream = true;
	}
	break;
	// Data block
	case 0x67:
	{
	    vgm_pos += 1;
	    uint8_t data_type = getimmByte();
	    uint32_t data_size = getimmLong();
	    uint8_t chip_type = (data_type & 0x3F);
	    uint8_t data_group = (data_type & 0xC0);

	    bool is_second_chip = ((data_size >> 31) != 0);

	    data_size &= 0x7FFFFFFF;

	    switch (data_group)
	    {
		// Uncompressed data streams
		case 0x00:
		{
		    vector<uint8_t> chip_data = pcm_data.at(chip_type);
		    uint32_t old_size = chip_data.size();
		    chip_data.resize((old_size + data_size));
		    auto begin = (vgm_data.begin() + vgm_pos);
		    auto end = (begin + data_size);
		    copy(begin, end, (chip_data.begin() + old_size));
		    pcm_data.at(chip_type) = chip_data;
		}
		break;
		// Compressed data streams (WIP)
		case 0x40:
		{
		    uint8_t comp_type = readByte(vgm_pos);
		    uint32_t uncomp_size = readLong(vgm_pos + 1);

		    cout << "Reading compression header with compression type of " << hex << int(comp_type) << endl;
		}
		break;
		// ROM/RAM image dumps
		case 0x80:
		{
		    uint32_t rom_size = readLong(vgm_pos);
		    uint32_t data_start = readLong((vgm_pos + 4));
		    uint32_t data_len = (data_size - 8);
		    uint32_t vgm_data_pos = (vgm_pos + 8);

		    auto begin = (vgm_data.begin() + vgm_data_pos);
		    auto end = (begin + data_len);
		    vector<uint8_t> rom_data(begin, end);

		    switch (data_type)
		    {
			// SegaPCM ROM data
			case 0x80: segapcm_chip.writeROM(rom_size, data_start, data_len, rom_data); break;
			// YM2610 ADPCM ROM data
			case 0x82: opnb_chip.writeROM(0, rom_size, data_start, data_len, rom_data); break;
			// YM2610 Delta-T ROM data
			case 0x83: opnb_chip.writeROM(1, rom_size, data_start, data_len, rom_data); break;
			// YMZ280B ROM data
			case 0x86: ymz280b_chip.writeROM(rom_size, data_start, data_len, rom_data); break;
			// Y8950 Delta-T ROM data
			case 0x88: opl_msx_chip.writeROM(rom_size, data_start, data_len, rom_data); break;
			// MultiPCM ROM data
			case 0x89: multipcm_chips.getChip(is_second_chip).writeROM(rom_size, data_start, data_len, rom_data); break;
			default: cout << "Skipping unrecognized PCM ROM type of " << hex << (int)data_type << endl; break;
		    }
		}
		break;
		// RAM writes
		case 0xC0:
		{
		    uint32_t data_start = readWord(vgm_pos);
		    uint32_t data_len = (data_size - 2);
		    uint32_t vgm_data_pos = (vgm_pos + 2);

		    auto begin = (vgm_data.begin() + vgm_data_pos);
		    auto end = (begin + data_len);

		    vector<uint8_t> ram_data(begin, end);

		    

		    switch (data_type)
		    {
			case 0xC0:
			{
			    rf5c68_chip.writeRAM(data_start, data_len, ram_data);
			}
			break;
			default: cout << "Skipping unrecognized RAM data type of " << hex << int(data_type) << endl;
		    }
		}
		break;
		default: cout << "Skipping unrecognized data type of " << hex << int(data_type) << endl; break;
	    }

	    vgm_pos += data_size;
	}
	break;
	// PCM RAM write
	case 0x68:
	{
	    vgm_pos += 1;

	    uint8_t chip_type = getimmByte();
	    uint32_t read_offs = getimmHLong();
	    uint32_t write_offs = getimmHLong();
	    uint32_t data_size = getimmHLong();

	    chip_type &= 0x3F;

	    if (data_size == 0)
	    {
		data_size = 0x1000000;
	    }

	    auto &ram_data = pcm_data.at(chip_type);

	    if (read_offs >= ram_data.size())
	    {
		break;
	    }

	    uint32_t data_length = data_size;
	    uint32_t data_end = (read_offs + data_size);

	    if (data_end > ram_data.size())
	    {
		cout << "Overflow" << endl;
		data_length = (ram_data.size() - read_offs);
	    }

	    auto begin = (ram_data.begin() + read_offs);
	    auto end = (begin + data_length);

	    vector<uint8_t> pcm_ram(begin, end);

	    switch (chip_type)
	    {
		case 0x01:
		{
		    rf5c68_chip.writeRAM(write_offs, data_length, pcm_ram);
		}
		break;
		default: cout << "Skipping unrecognized PCM RAM write data type of " << hex << int(chip_type) << endl; break;
	    }

	    pcm_ram.clear();
	}
	break;
	// YM2612 chip 1, port 0 write
	case 0xA2:
	{
	    uint8_t addr = getimmByte();
	    uint8_t data = getimmByte();

	    opn2_chips.writeYM(true, 0, addr, data);
	}
	break;
	// YM2612 chip 1, port 1 write
	case 0xA3:
	{
	    uint8_t addr = getimmByte();
	    uint8_t data = getimmByte();

	    opn2_chips.writeYM(true, 1, addr, data);
	}
	break;
	// RF5C68 register write
	case 0xB0:
	{
	    uint8_t addr = getimmByte();
	    uint8_t data = getimmByte();

	    rf5c68_chip.writeReg(addr, data);
	}
	break;
	// PWM register write
	case 0xB2:
	{
	    uint8_t addr_byte = getimmByte();
	    uint8_t data_byte = getimmByte();

	    int reg = (addr_byte >> 4);
	    uint16_t data = (((addr_byte & 0xF) << 8) | data_byte);

	    cout << "Writing value of " << hex << int(data) << " to PWM register of " << dec << int(reg) << endl;
	}
	break;
	// MultiPCM register write
	case 0xB5:
	{
	    uint8_t addr = getimmByte();
	    uint8_t data = getimmByte();

	    bool is_second_chip = ((addr >> 7) != 0);

	    addr &= 0x7F;
	    multipcm_chips.writeIO(is_second_chip, addr, data);
	}
	break;
	// Sega PCM RAM write
	case 0xC0:
	{
	    uint16_t addr = getimmWord();
	    uint8_t data = getimmByte();

	    segapcm_chip.writeMem(addr, data);
	}
	break;
	// RF5C68 RAM write
	case 0xC1:
	{
	    uint16_t addr = getimmWord();
	    uint8_t data = getimmByte();

	    rf5c68_chip.writeMem(addr, data);
	}
	break;
	// MultiPCM bank offset set
	case 0xC3:
	{
	    uint8_t channel = getimmByte();
	    uint16_t bank_offs = getimmWord();

	    bool is_second_chip = ((channel >> 7) != 0);

	    channel &= 0x7F;
	    multipcm_chips.getChip(is_second_chip).writeBank(channel, bank_offs);
	}
	break;
	// YMF278B register write
	case 0xD0:
	{
	    uint8_t port = getimmByte();
	    uint8_t addr = getimmByte();
	    uint8_t data = getimmByte();

	    bool is_second_chip = ((port >> 7) != 0);

	    string chip_str = (is_second_chip) ? "second" : "first";

	    port &= 0x7F;

	    cout << "Writing value of " << hex << int(data) << " to " << chip_str << " YMF278B port " << dec << int(port) << " register of " << hex << int(addr) << endl;
	}
	break;
	// YMF271 register write
	case 0xD1:
	{
	    uint8_t port = getimmByte();
	    uint8_t addr = getimmByte();
	    uint8_t data = getimmByte();

	    bool is_second_chip = ((port >> 7) != 0);

	    string chip_str = (is_second_chip) ? "second" : "first";

	    port &= 0x7F;

	    cout << "Writing value of " << hex << int(data) << " to " << chip_str << " YMF271 port " << dec << int(port) << " register of " << hex << int(addr) << endl;
	}
	break;
	// PCM offset
	case 0xE0: pcm_pos = getimmLong(); break;
	default:
	{
	    int vgm_nibble = (vgm_instr & 0xF);
	    switch ((vgm_instr & 0xF0))
	    {
		// Wait n+1 samples
		case 0x70: num_samples = (vgm_nibble + 1); break;
		// Write to YM2612 chip 0 DAC, then wait n samples
		case 0x80:
		{
		    if (is_ymfm_auto)
		    {
			init_ym2612();
		    }

		    auto &chip = opn2_chips.getChip(false);

		    if (chip.isChipEnabled())
		    {
			uint8_t data = 0x80;

			auto ym2612_dac = pcm_data.at(0x00);

			if (pcm_pos < ym2612_dac.size())
			{
			    data = ym2612_dac.at(pcm_pos++);
			}

			chip.writeYM(0, 0x2A, data);
		    }

		    num_samples = vgm_nibble;
		}
		break;
		default: unrecognized_instr(vgm_instr); break;
	    }
	}
	break;
    }

    return num_samples;
}

array<int16_t, 2> BeeVGM::generateSample()
{
    array<int32_t, 2> samples = {0, 0};

    snpsg_chip.add_samples(samples);
    opll_chip.add_samples(samples);
    opn2_chips.add_samples(samples);
    opm_chip.add_samples(samples);

    segapcm_chip.add_samples(samples);
    opn_chip.add_samples(samples);
    opnb_chip.add_samples(samples);
    opl2_chip.add_samples(samples);
    opl_chip.add_samples(samples);
    // opl_msx_chip.add_samples(samples);
    ymz280b_chip.add_samples(samples);
    rf5c68_chip.add_samples(samples);

    multipcm_chips.add_samples(samples);

    array<int16_t, 2> final_samples = {0, 0};
    final_samples[0] = clamp<int16_t>(samples[0], -32768, 32767);
    final_samples[1] = clamp<int16_t>(samples[1], -32768, 32767);

    return final_samples;
}