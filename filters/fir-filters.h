#
/*
 *    Copyright (C) 2025
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the qt-ft8 decoder
 *
 *    qt-ft8 decoder is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    qt-ft8 decoder is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with the qt-ft8 decoder; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#pragma once

#include	"constants.h"
#include	<vector>

class	HilbertFilter;

class	basicFIR {
public:
		basicFIR	(int16_t);
		~basicFIR	();
Complex		Pass		(Complex);
float		Pass		(float);
void		setTaps		(int16_t len,
	                         float *itaps, float *qtaps);
	std::vector<Complex> filterKernel;
	std::vector<Complex> Buffer;
	int	filterSize;
	int	ip;
	int	sampleRate;
};

class	lowpassFIR : public basicFIR {
public:
			lowpassFIR (int16_t,	// order
	                            int32_t, 	// cutoff frequency
	                            int32_t	// samplerate
	                           );
			~lowpassFIR	();
	Complex		*getKernel	();
	void		newKernel	(int32_t);	// cutoff
};
//
//	Both for lowpass band bandpass, we provide:
class	decimatingFIR: public basicFIR {
public:
		         decimatingFIR	(int16_t, int32_t, int32_t, int16_t);
	                 decimatingFIR	(int16_t, int32_t, int32_t,
	                                                   int32_t, int16_t);
			~decimatingFIR	();
	void		newKernel	(int32_t);
	void		newKernel	(int32_t, int32_t);
	bool		Pass		(Complex, Complex *);
	bool		Pass	(float, float *);
private:
	int16_t	decimationFactor;
	int16_t	decimationCounter;
};

class	highpassFIR: public basicFIR {
public:
			highpassFIR	(int16_t, int32_t, int32_t);
			~highpassFIR	();
	void		newKernel	(int32_t);
};

class	bandpassFIR: public basicFIR {
public:
			bandpassFIR	(int16_t, int32_t, int32_t, int32_t);
			~bandpassFIR	();
	Complex		*getKernel	();
	void		newKernel	(int32_t, int32_t);
private:
};

class	basicbandPass: public basicFIR {
public:
			basicbandPass	(int16_t, int32_t, int32_t, int32_t);
			~basicbandPass	();
	Complex	*getKernel		();
private:
};


class HilbertFilter {
public:
		HilbertFilter	(int16_t, float, int32_t);
		~HilbertFilter	();
	std::complex<float>	Pass		(Complex);
	std::complex<float>	Pass		(float, float);
private:
	int16_t		ip;
	int16_t		firsize;
	int32_t		rate;
	float		*cosKernel;
	float		*sinKernel;
	Complex		*Buffer;
	void		adjustFilter	(float);
};


