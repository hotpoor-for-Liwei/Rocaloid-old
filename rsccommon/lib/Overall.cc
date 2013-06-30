// Overall.cc
//
// Copyright (C) 2013 - rgwan
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#include "../SPKit/spkit.h"
#include "Overall.h"

inline char *Overall::WaveHead()
{
static char WaveHead[]={
			0x52,0x49,0x46,0x46,0x24,0x10,0x1F,0x00,
			0x57,0x41,0x56,0x45,0x66,0x6D,0x74,0x20,
			0x10,0x00,0x00,0x00,0x01,0x00,0x01,0x00,
			0x00,0x77,0x01,0x00,0x00,0xEE,0x02,0x00,
			0x02,0x00,0x10,0x00,0x64,0x61,0x74,0x61,
			0x00,0x10,0x1F,0x00};
	return WaveHead ;
}
inline char *Overall::PitchList(char Pitch)
{
	static char *PitchList[]={
		"C1","C#1","D1","D#1","E1","F1","F#1","G1","G#1","A1","A#1","B1",
		"C2","C#2","D2","D#2","E2","F2","F#2","G2","G#2","A2","A#2","B2", 
		"C3","C#3","D3","D#3","E3","F3","F#3","G3","G#3","A3","A#3","B3",
		"C4","C#4","D4","D#4","E4","F4","F#4","G4","G#4","A4","A#4","B4",
		"C5","C#5","D5","D#5","E5","F5","F#5","G5","G#5","A5","A#5","B5"
	};
	return PitchList[Pitch];
}