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
 *    along with qt-ft8 decoder; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#pragma once

#include	"constants.h"
#include	"fir-filters.h"
#include	"fft.h"

class	fftFilter {
public:
			fftFilter	(int32_t, int16_t);
			~fftFilter	(void);
	void		setBand		(int32_t, int32_t, int32_t);
	void		setLowPass	(int32_t, int32_t);
	Complex		Pass		(Complex);
	float		Pass		(float);

private:
	int32_t		fftSize;
	int16_t		filterDegree;
	int16_t		OverlapSize;
	int16_t		NumofSamples;
	fft_handler	*MyFFT;
	Complex		*FFT_A;
	ifft_handler	*MyIFFT;
	Complex		*FFT_C;
	fft_handler	*FilterFFT;
	Complex		*filterVector;
	float		*RfilterVector;
	Complex		*Overloop;
	int32_t		inp;
	float		*blackman;
};


