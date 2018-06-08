/*
 *  Copyright (C) 2002-2017  The DOSBox Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <fcntl.h>
#include <midi.h>
#include <cerrno>
#include <cstring>
#define SEQ_MIDIPUTC    5

class MidiHandler_oss: public MidiHandler {
private:
	int  device;
	Bit8u device_num;
	bool isOpen;
public:
  MidiHandler_oss() : MidiHandler(), isOpen(false) {}
  const char * GetName(void) { return "oss"; }
	bool Open(const char * conf) {
		char devname[512];
		if (conf && conf[0]) safe_strncpy(devname,conf,512);
		else strcpy(devname,"/dev/sequencer");
		char * devfind=(strrchr(devname,','));
		if (devfind) {
			*devfind++=0;
			device_num=atoi(devfind);
		} else device_num=0;
		if (isOpen) return false;
		device=open(devname, O_WRONLY, 0);
    if (device < 0) {
      LOG_MSG("OSS: can't open device %s (%s)", devname, std::strerror(errno));
      return false;
    }
    isOpen = true;
		return true;
	};
	void Close(void) {
		if (!isOpen) return;
		close(device);
    isOpen = false;
	}
	void PlayMsg(Bit8u * msg) {
		Bit8u buf[128];Bitu pos=0;
		Bitu len=MIDI_evt_len[*msg];
		for (;len>0;len--) {
			buf[pos++] = SEQ_MIDIPUTC;
			buf[pos++] = *msg;
			buf[pos++] = device_num;
			buf[pos++] = 0;
			msg++;
		}
    if (0 > write(device, buf, pos)) {
      LOG_MSG("OSS: in %s write() returns error: %s", __func__, std::strerror(errno));
    }
	}
	void PlaySysex(Bit8u * sysex,Bitu len) {
		Bit8u buf[SYSEX_SIZE*4];Bitu pos=0;
		for (;len>0;len--) {
			buf[pos++] = SEQ_MIDIPUTC;
			buf[pos++] = *sysex++;
			buf[pos++] = device_num;
			buf[pos++] = 0;
		}
    if (0 > write(device, buf, pos)) {
      LOG_MSG("OSS: in %s write() returns error: %s", __func__, std::strerror(errno));
    }
	}
  ~MidiHandler_oss() {
    Close();
  }
};

MidiHandler_oss Midi_oss;
