#
/*
 *    Copyright (C) 2025
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the ft8 module
 *
 *    ft8 module is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    ft8 module is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with ft8 module; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#pragma once

//
//	Simple wrapper around fftw
#include	"constants.h"
#include	<complex>
#include	<stdint.h>
//
#define	FFTW_MALLOC		fftwf_malloc
#define	FFTW_PLAN_DFT_1D	fftwf_plan_dft_1d
#define FFTW_DESTROY_PLAN	fftwf_destroy_plan
#define	FFTW_FREE		fftwf_free
#define	FFTW_PLAN		fftwf_plan
#define	FFTW_EXECUTE		fftwf_execute
#include	<fftw3.h>
/*
 *	a simple wrapper
 */

class	fft_handler {
public:
			fft_handler	(int32_t);
			~fft_handler	();
	void		do_FFT		(Complex *);
private:
	int32_t		fftSize;
	FFTW_PLAN	plan;
	Complex		*vector;
};

class	ifft_handler {
public:
			ifft_handler	(int32_t);
			~ifft_handler	();
	void		do_IFFT		(Complex *);
private:
	int		fftSize;
	FFTW_PLAN	plan;
	Complex	*vector;
	void		Scale		(Complex *);
};
	
