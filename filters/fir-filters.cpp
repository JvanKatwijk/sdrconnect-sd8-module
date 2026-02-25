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

#include	"ft8-constants.h"
#include	"fir-filters.h"

		basicFIR::basicFIR (int16_t size):
	                                   filterKernel (size),
	                                   Buffer       (size) {
	int16_t		i;
	filterSize	= size;
	ip		= 0;

	for (i = 0; i < filterSize; i ++) {
	   filterKernel [i]	= 0;
	   Buffer [i]		= 0;
	}
}

		basicFIR::~basicFIR (void) {
}
//
std::complex<float>	basicFIR::Pass (std::complex<float> z) {
int16_t	i;
std::complex<float>	tmp	= 0;

	Buffer [ip]	= z;
	for (i = 0; i < filterSize; i ++) {
	   int16_t index = ip - i;
	   if (index < 0)
	      index += filterSize;
	   tmp		+= Buffer [index] * filterKernel [i];
	}

	ip = (ip + 1) % filterSize;
	return tmp;
}

float	basicFIR::Pass (float v) {
int16_t		i;
float	tmp	= 0;

	Buffer [ip] = std::complex<float> (v, 0);
	for (i = 0; i < filterSize; i ++) {
	   int16_t index = ip - i;
	   if (index < 0)
	      index += filterSize;
	   tmp += real (Buffer [index]) * real (filterKernel [i]);
	}

	ip = (ip + 1) % filterSize;
	return tmp;
}

void	basicFIR::setTaps (int16_t len, float *itaps, float *qtaps) {
int16_t	i;

	if (len != filterSize)
	   fprintf (stderr, "H E L P\n");

	for (i = 0; i < len; i ++) {
	   float re = 0;
	   float im = 0;

	   if (itaps != NULL)
	      re = itaps [i];
	   if (qtaps != NULL)
	      im = qtaps [i];
	   filterKernel [i] = std::complex<float> (re, im);
	}
}
	      

	lowpassFIR::lowpassFIR (int16_t firsize,
	                        int32_t Fc, int32_t fs):
	                                     basicFIR (firsize) {
	sampleRate	= fs;
	newKernel (Fc);
}

void	lowpassFIR::newKernel (int32_t Fc) {
int16_t	i;
float	*tmp	= dynVec (float, filterSize);
float	f	= (float)Fc / sampleRate;
float	sum	= 0.0;

	for (i = 0; i < filterSize; i ++) {
	   if (i == filterSize / 2)
	      tmp [i] = 2 * M_PI * f;
	   else 
	      tmp [i] = sin (2 * M_PI * f * (i - filterSize/2))/ (i - filterSize/2);
//
//	Blackman window
	   tmp [i]  *= (0.42 -
		    0.5 * cos (2 * M_PI * (float)i / filterSize) +
		    0.08 * cos (4 * M_PI * (float)i / filterSize));

	   sum += tmp [i];
	}

	for (i = 0; i < filterSize; i ++)
	   filterKernel [i] = std::complex<float> (tmp [i] / sum, 0);
}

	lowpassFIR::~lowpassFIR () {
}

std::complex<float>	*lowpassFIR::getKernel (void) {
	return filterKernel. data ();
}

//=====================================================================

	highpassFIR::highpassFIR (int16_t firsize,
	                          int32_t Fc, int32_t fs):
	                                          basicFIR (firsize) {
	sampleRate	= fs;
	newKernel (Fc);
}

void	highpassFIR::newKernel (int32_t Fc) {
int16_t	i;
float 	*tmp	= dynVec (float, filterSize);
float f	= (float)Fc / sampleRate;
float sum	= 0.0;

	for (i = 0; i < filterSize; i ++) {
	   if (i == filterSize / 2)
	      tmp [i] = 2 * M_PI * f;
	   else 
	      tmp [i] = sin (2 * M_PI * f * (i - filterSize/2))/ (i - filterSize/2);

	   tmp [i]  *= (0.42 -
		    0.5 * cos (2 * M_PI * (float)i / (float)filterSize) +
		    0.08 * cos (4 * M_PI * (float)i / (float)filterSize));

	   sum += tmp [i];
	}

	for (i = 0; i < filterSize; i ++)
	   if (i == filterSize / 2)
	      filterKernel [i] = std::complex<float> (1.0 - tmp [i] / sum, 0);
	   else  
	      filterKernel [i] = std::complex<float> (- tmp [i] / sum, 0);
}

	highpassFIR::~highpassFIR () {
}

//===============================================================

static
float	Blackman (float *v, int fsize, float f) {
int	i;
float		sum = 0;

	for (i = 0; i < fsize; i ++) {
	   if (i == fsize / 2)
	      v [i] = 2 * M_PI * f;
	   else 
	      v [i] = sin (2 * M_PI * f * (i - fsize/2))/ (i - fsize/2);

	   v [i] *= (0.42 -
		        0.5 * cos (2 * M_PI * (float)i / (float)fsize) +
		        0.08 * cos (4 * M_PI * (float)i / (float)fsize));

	   sum += v [i];
	}

	return sum;
}

static
float	*bandpassKernel (float *v, int16_t fsize, float Fcl, float Fch) {
float	sumA	= 0.0;
float	sumB	= 0.0;
int16_t	i;
float	*tmp1	= dynVec (float, fsize);
float	*tmp2	= dynVec (float, fsize);

	if ((Fcl > 0.5) || (Fch <= Fcl) || (Fch > 0.5)) {
            fprintf (stderr, "bandpasskernel ??? (%f, %f) %d\n",
	                  (float)Fcl, (float)Fch, fsize);
	   Fcl	= 0.2;
	   Fch	= 0.4;
	}
	   
	sumA	= Blackman (tmp1, fsize, Fcl);
	sumB	= Blackman (tmp2, fsize, Fch);
	/* normalize	*/
	for (i = 0; i < fsize; i ++) {
	   tmp1 [i] =   tmp1 [i] / sumA;
	   tmp2 [i] = - tmp2 [i] / sumB;
	   v [i] = - (tmp1 [i] + tmp2 [i]);
	}

	return v;
}

	basicbandPass::basicbandPass (int16_t firsize,
	                              int32_t low, int32_t high,
	                              int32_t rate):
	                                      basicFIR (firsize) {
float	*t1	= dynVec (float, firsize);
int16_t		i;

	sampleRate	= rate;
	(void) bandpassKernel (t1, firsize, (float) low / rate,
	                                     (float) high / rate);
	for (i = 0; i < filterSize; i ++)  
	   filterKernel [i] = std::complex<float> (t1 [i], t1 [i]);
}

	basicbandPass::~basicbandPass () {
}

std::complex<float>	*basicbandPass::getKernel () {
	return filterKernel. data ();
}

/*
 *	The bandfilter is for the complex domain. 
 *	We create a lowpass filter, which stretches over the
 *	positive and negative half, then shift this filter
 *	to the right position to form a nice bandfilter.
 *	For the real domain, we use the Simple BandPass version.
 */
	bandpassFIR::bandpassFIR (int16_t firSize,
	                          int32_t low, int32_t high,
	                          int32_t fs):basicFIR (firSize) {
	sampleRate	= fs;
	newKernel (low, high);
}
//
//	For the kernel we compute the size of the "equivalent"
//	low pass filter (lo), and the amount of the shift
void	bandpassFIR::newKernel (int32_t low, int32_t high) {
float	*tmp	= dynVec (float, filterSize);
float	lo	= (float)((high - low) / 2) / sampleRate;
float	shift	= (float) ((high + low) / 2) / sampleRate;
float	sum	= 0.0;
int16_t	i;

	for (i = 0; i < filterSize; i ++) {
	   if (i == filterSize / 2)
	      tmp [i] = 2 * M_PI * lo;
	   else 
	      tmp [i] = sin (2 * M_PI * lo * (i - filterSize /2)) / (i - filterSize/2);
//
//	windowing, according to Blackman
	   tmp [i]  *= (0.42 -
		    0.5 * cos (2 * M_PI * (float)i / (float)filterSize) +
		    0.08 * cos (4 * M_PI * (float)i / (float)filterSize));

	   sum += tmp [i];
	}

	for (i = 0; i < filterSize; i ++) {	// shifting
	   float v = (i - filterSize / 2) * (2 * M_PI * shift);
	   filterKernel [i] = std::complex<float> (tmp [i] * cos (v) / sum, 
	                                           tmp [i] * sin (v) / sum);
	}
}

	bandpassFIR::~bandpassFIR () {
}

std::complex<float>	*bandpassFIR::getKernel () {
	return filterKernel. data ();
}

//
//====================================================================
//
//	decimationFilter.
//	Useful for e.g. reduction of the samplerate with dabsticks
//	Two tastes are within the same class: the lowpass and
//	the bandpass variant.

	decimatingFIR::decimatingFIR (int16_t firSize,
	                              int32_t low,
	                              int32_t fs,
	                              int16_t Dm):
	                                   basicFIR (firSize) {

	sampleRate		= fs;
	decimationFactor	= Dm;
	decimationCounter	= 0;
	newKernel (low);
}

void	decimatingFIR::newKernel (int32_t low) {
int16_t	i;
float	*tmp	= (float *)alloca (filterSize * sizeof (float));
float	f	= (float)low / sampleRate;
float	sum	= 0.0;

	for (i = 0; i < filterSize; i ++) {
	   if (i == filterSize / 2)
	      tmp [i] = 2 * M_PI * f;
	   else 
	      tmp [i] = sin (2 * M_PI * f * (i - filterSize/2))/ (i - filterSize/ 2);

	   tmp [i]  *= (0.42 -
		    0.5 * cos (2 * M_PI * (float)i / (float)filterSize) +
		    0.08 * cos (4 * M_PI * (float)i / (float)filterSize));

	   sum += tmp [i];
	}

	for (i = 0; i < filterSize; i ++)
	   filterKernel [i] = std::complex<float> (tmp [i] / sum, 0);
}

	decimatingFIR::decimatingFIR (int16_t firSize,
	                              int32_t low,
	                              int32_t high,
	                              int32_t fs,
	                              int16_t Dm):
	                                   basicFIR (firSize) {

	sampleRate		= fs;
	decimationFactor	= Dm;
	decimationCounter	= 0;
	newKernel (low, high);
}

void	decimatingFIR::newKernel (int32_t low, int32_t high) {
float	*tmp	= (float *)alloca (filterSize * sizeof (float));
float	lo	= (float)((high - low) / 2) / sampleRate;
float	shift	= (float) ((high + low) / 2) / sampleRate;
float	sum	= 0.0;
int16_t	i;

	for (i = 0; i < filterSize; i ++) {
	   if (i == filterSize / 2)
	      tmp [i] = 2 * M_PI * lo;
	   else 
	      tmp [i] = sin (2 * M_PI * lo * (i - filterSize /2)) / (i - filterSize/2);
//
//	windowing
	   tmp [i]  *= (0.42 -
		        0.5 * cos (2 * M_PI * (float)i / (float)filterSize) +
		        0.08 * cos (4 * M_PI * (float)i / (float)filterSize));

	   sum += tmp [i];
	}
//
//	and modulate the kernel
	for (i = 0; i < filterSize; i ++) {	// shifting
	   float v = (i - filterSize / 2) * (2 * M_PI * shift);
	   filterKernel [i] = std::complex<float> (tmp [i] * cos (v) / sum, 
	                                           tmp [i] * sin (v) / sum);
	}
}

	decimatingFIR::~decimatingFIR () {
}
//
//	The real cpu killer: this function is called once for every
//	sample that comes from the dongle. So, it really should be
//	optimized.
bool	decimatingFIR::Pass (std::complex<float> z,
	                                 std::complex<float> *z_out) {
int16_t		i;
std::complex<float>	tmp	= 0;
int16_t		index;

	Buffer [ip] = z;
	if (++decimationCounter < decimationFactor) {
	   ip =  (ip + 1) % filterSize;
	   return false;
	}

	decimationCounter = 0;
//
//	we are working with a circular buffer, we take two steps
//	we move from ip - 1 .. 0 with i going from 0 .. ip -1
	for (i = 0; i <= ip; i ++) {
	   index =  ip - i;
	   tmp 	+= Buffer [index] * filterKernel [i];
	}
//	and then we take the rest
	for (i = ip + 1; i < filterSize; i ++) {
	   index =  filterSize + ip - i;
	   tmp 	+= Buffer [index] * filterKernel [i];
	}

	ip = (ip + 1) % filterSize;
	*z_out = tmp;
	return true;
}

bool	decimatingFIR::Pass (float z, float *z_out) {
	if (++decimationCounter < decimationFactor) {
	   Buffer [ip] = std::complex<float> (z, 0);
	   ip = (ip + 1) % filterSize;
	   return false;
	}

	decimationCounter = 0;
	*z_out = basicFIR::Pass (z);
	return true;
}


//====================================================================
/*
 *	The Hilbertfilter is derived from QEX Mar/April 1998
 *	to perform a 90 degree phase shift needed for (a.o)
 *	USB and LSB detection.
 */
	HilbertFilter::HilbertFilter (int16_t fsize, float f, int32_t rate) {
	firsize		= fsize;
	this	-> rate	= rate;
	cosKernel	= new float [fsize];
	sinKernel	= new float [fsize];
	Buffer		= new std::complex<float> [fsize];
	adjustFilter (f);
}

	HilbertFilter::~HilbertFilter () {
	delete[]	cosKernel;
	delete[]	sinKernel;
	delete[]	Buffer;
}
/*
 * 	the validity of the hilbertshift was validated
 * 	by computing (and displaying) 
 * 	arg (res - DSPCOMPLEX (real (res), real (res)))
 * 	where res = ... -> Pass (s1, s1)
 */
void	HilbertFilter::adjustFilter (float centre) {
float	*v1	= (float *)alloca (firsize * sizeof (float));
float	sum = Blackman (v1, firsize, centre);
int16_t	i;

	for (i = 0; i < firsize; i ++)
	   v1 [i] = v1 [i] / sum;

	for (i = 0; i < firsize; i ++) {
	   float omega = 2.0 * M_PI * centre;
	   cosKernel [i] = v1 [i] * 
	             cos (omega * (i - ((float)firsize - 1) / (2.0 * rate)));
	   sinKernel [i] = v1 [i] * 
	             sin (omega * (i - ((float)firsize - 1) / (2.0 * rate)));
	   Buffer [i] = 0;
	}

	ip = 0;
}

std::complex<float>	HilbertFilter::Pass (float a, float b) {
	return Pass (std::complex<float> (a, b));
}

std::complex<float>	HilbertFilter::Pass (std::complex<float> z) {
std::complex<float>	tmp = 0;
float	re, im;
int	i;

	Buffer [ip] = z;
	ip = (ip + 1) % firsize;

	for (i = 0; i < firsize; i ++) {
	   int16_t index = ip - i;
	   if (index < 0)
	      index += firsize;
	   re = real (Buffer [index]);
	   im = imag (Buffer [index]);
	   tmp += std::complex<float> (re * cosKernel [i], im * sinKernel [i]);
	}

	return tmp;
}

