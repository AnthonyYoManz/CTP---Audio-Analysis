#pragma once
#include "Aliases.h"

/*
Sub-bass	20 to 60 Hz
Bass	60 to 250 Hz
Low midrange	250 to 500 Hz
Midrange	500 Hz to 2 kHz
Upper midrange	2 to 4 kHz
Presence	4 to 6 kHz
Brilliance	6 to 20 kHz
*/

struct AudioSpectrumInstant
{
	real m_subBass;			//20Hz to 60Hz
	real m_bass;			//60Hz to 250Hz
	real m_lowMidrange;		//250Hz to 500Hz
	real m_midrange;		//500Hz to 2000Hz
	real m_upperMidrange;	//2000Hz to 4000Hz
	real m_presence;		//4000Hz to 6000Hz
	real m_brilliance;		//6000Hz to 20000Hz
};