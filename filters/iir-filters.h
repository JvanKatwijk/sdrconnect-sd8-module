#
/*
 *    Copyright (C)  2025
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
#include	"fft.h"

#define	S_CHEBYSHEV	0100
#define	S_BUTTERWORTH	0101
#define	S_INV_CHEBYSHEV	0102
#define	S_ELLIPTIC	0103

struct	biquad {
	float	A0, A1, A2;
	float	B0, B1, B2;
};

typedef	struct biquad element;

/*
 *	Processing an element, i.e. passing data, is the same for all
 *	recursive filters.
 *	We write it out for the components of the complex data
 *	profiling shows that that is much, much faster
 */
class	Basic_IIR {
public:
	int16_t		numofQuads;
	element		*Quads;
	float		gain;
	Complex		*m1;
	Complex		*m2;
			Basic_IIR	(int16_t n) {
	int16_t i;
		numofQuads	= n;
	        
		Quads		= new element		[2 * numofQuads];
		m1		= new Complex		[2 * numofQuads];
		m2		= new Complex		[2 * numofQuads];
		for (i = 0; i < numofQuads; i ++) {
	           m1 [i]	= 0;
	           m2 [i]	= 0;
	        }
	}
			~Basic_IIR	() {
		delete[]	Quads;
		delete[]	m1;
		delete[]	m2;
	}

	Complex	Pass		(Complex z) {
	Complex	o, w;
	int16_t	i;

		o = cmul (z, gain);
		for (i = 0; i < numofQuads; i ++) {
		   element *MyQuad = &Quads [i];
		   w = o - cmul (m1 [i], MyQuad -> B1) -
	                            cmul (m2 [i], MyQuad -> B2);
		   o = w + cmul (m1 [i], MyQuad -> A1) +
	                            cmul (m2 [i], MyQuad -> A2);
		   m2 [i] = m1 [i];
		   m1 [i] = w;
		}

		return o;
	}

	float	Pass (float v) {
	float	o, w;
	int16_t	i;

		o = v * gain;
		for (i = 0; i < numofQuads; i ++) {
		   element *MyQuad = &Quads [i];
	           float rm1 = real (m1 [i]);
	           float rm2 = real (m2 [i]);
		   w = o - rm1 * MyQuad -> B1 - rm2 * MyQuad -> B2;
	   	   o = w + rm1 * MyQuad -> A1 + rm2 * MyQuad -> A2;
		   m2 [i] = m1 [i];
		   m1 [i] = w;
		}

		return o;
	}
};


class	LowPassIIR : public Basic_IIR {
public:
			LowPassIIR	(int16_t, int32_t, int32_t, int16_t);
	                LowPassIIR	(int16_t, int16_t, int32_t, int32_t, int32_t);
			~LowPassIIR	(void);
};

class	HighPassIIR : public Basic_IIR {
public:
			HighPassIIR	(int16_t, int32_t, int32_t, int16_t);
	                ~HighPassIIR	(void);
};

class	BandPassIIR : public Basic_IIR {
public:
			BandPassIIR	(int16_t,
	                                 int32_t, int32_t, int32_t,
	                                 int16_t);
			~BandPassIIR	(void);
};


