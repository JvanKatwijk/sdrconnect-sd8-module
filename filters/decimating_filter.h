#
/*
 *    Copyright (C)  2023
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the ft8 module for SDRconnect
 *
 *    ft8 module is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    ft8 module decoder is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with ft8 module; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#pragma once

#include	"fft.h"
#include	"constants.h"
#include	<vector>

class	decimating_filter {
public:
	decimating_filter	(int, int, int);
	~decimating_filter	();
bool	Pass			(Complex, Complex *);
private:
	int		inrate;
	int		outrate;
	fft_handler	fft_in;
	ifft_handler	fft_out;
	std::vector<float>	inwindow;
	std::vector<Complex>	freqWindow;	
	std::complex<float>	*hulpbuffer;
	int		inp;
	int		outp;
	Complex		*inbuffer;
	Complex		*outbuffer;
};


