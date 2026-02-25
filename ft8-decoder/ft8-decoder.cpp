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
#
#include	"ft8-decoder.h"
#include	"radio.h"
#include	"pack-handler.h"
#include	<QStandardItemModel>

#define	LOG_BASE	240

#define	WORKING_RATE	12000
#define	TONE_LENGTH	((int)(WORKING_RATE / 6.25))
	ft8_Decoder::ft8_Decoder (RadioInterface *mr,
	                          int32_t	inRate,
	                          QSettings		*settings) :
	                              theProcessor (mr, 20),
	                              the_fftHandler ((int)(2 * TONE_LENGTH)) {
	(void)inRate;
	ft8Settings	= settings;
	this	-> mr	= mr;
	for (int i = 0; i < nrBUFFERS; i ++)
	   theBuffer [i] = new float [2 * TONE_LENGTH + 1];
	window			= new float [TONE_LENGTH];
	inputBuffer		= new std::complex<float> [TONE_LENGTH];
	inBuffer		= new std::complex<float> [TONE_LENGTH / FRAMES_PER_TONE];
	fftVector		= new std::complex<float> [2 * TONE_LENGTH];

	for (int i = 0; i < TONE_LENGTH; i ++)  {
	   window [i] =  (0.42 -
	            0.5 * cos (2 * M_PI * (float)i / TONE_LENGTH) +
	            0.08 * cos (4 * M_PI * (float)i / TONE_LENGTH));
	}

	memset (is_syncTable, 0, 79);
	for (int i = 0; i < 7; i ++) {
	   is_syncTable [i] = true;
	   is_syncTable [36 + i] = true;
	   is_syncTable [72 + i] = true;
	}
	fillIndex	= 0;
	readIndex	= 0;
	inPointer	= 0;
	lineCounter	= 0;

//	filePointer. store (nullptr);
	ft8Settings	-> beginGroup ("ft8Settings");
	int val		= ft8Settings -> value ("maxIterations", 20). toInt ();
	maxIterations. store (val);
	theProcessor. set_maxIterations (val);
	val		= ft8Settings -> value ("width", 4000). toInt ();
	spectrumWidth. store (val);
	ft8Settings	-> endGroup ();
	
	connect (this, &ft8_Decoder::show_pskStatus,
	         mr, &RadioInterface::show_pskStatus);
	connect (this, &ft8_Decoder::sendMessages,
	         mr, &RadioInterface::sendMessages);
	show_pskStatus (false);
	teller		= 0;
}

	ft8_Decoder::~ft8_Decoder	() {
	for (int i = 0; i < nrBUFFERS; i ++)
	   delete [] theBuffer [i];
	delete [] window;
	delete [] fftVector;
}

void	ft8_Decoder::processBuffer	(std::complex<float> *buffer,
	                                                 int32_t amount) {
	for (int i = 0; i < amount; i ++) 
	   process (buffer [i]);
}

//
void	ft8_Decoder::process		(std::complex<float> z) {
	inBuffer [inPointer ++] = z;
	if (inPointer < TONE_LENGTH / FRAMES_PER_TONE)
	   return;

	teller += inPointer;
	inPointer = 0;

//	locker. lock ();
	if (teller > WORKING_RATE * 10) {
	   teller = 0;
	   sendMessages	();
	}
//	locker. unlock ();
	int content = (FRAMES_PER_TONE - 1) * TONE_LENGTH / FRAMES_PER_TONE;
	int newAmount = TONE_LENGTH / FRAMES_PER_TONE;

//
//	shift the inputBuffer to left
	memmove (inputBuffer, &inputBuffer [newAmount],
	                      content * sizeof (std::complex<float>));
//
//	copy the data that is read, into the buffer
	memcpy (&inputBuffer [content], inBuffer, 
	                    newAmount * sizeof (std::complex<float>));
//
//	prepare for fft
	for (int i = 0; i < TONE_LENGTH; i ++) {
	   fftVector [i] = inputBuffer [i] * window [i];
	   fftVector [TONE_LENGTH + i] = std::complex<float> (0, 0);
	}

	the_fftHandler. do_FFT (fftVector);
//
//	copy the spectrum into the buffer, negative frequencies first
	for (int i = 0; i < TONE_LENGTH; i ++) {
	   float x = abs (fftVector [i]);
	   if ((x < 0) || (x > 10000000)) {
//	      fprintf (stderr, "ellende\n");
	      x = 0;
	   }
	   float x_p =  x;

	   float y = abs (fftVector [TONE_LENGTH + i]);
	   if ((y < 0) || (y > 10000000)) {
//	      fprintf (stderr, "ellende\n");
	      y = 0;
	   }
	   float x_n = y;

	   if (x_p < 0) x_p = 0;
	   if (x_n < 0) x_n = 0;
	   theBuffer [fillIndex][TONE_LENGTH + i] = x_p;
	   theBuffer [fillIndex][i] = x_n;
	}

	float avg = 0;
	for (int i = 0; i < 2 * TONE_LENGTH; i ++)
	   avg += theBuffer [fillIndex][i];
	avg /= (2 * TONE_LENGTH);
	float min = 0;
	for (int i = 0; i < 2 * TONE_LENGTH; i ++) {
	   if (theBuffer [fillIndex][i] < avg) {
	      min += theBuffer [fillIndex][i];
	      teller ++;
	   }
	}
	min /= teller;
	for (int i = 0; i < 2 * TONE_LENGTH; i ++)
	   theBuffer [fillIndex][i] /= avg;
	
	theBuffer [fillIndex] [0] = avg;
	theBuffer [fillIndex] [2 * TONE_LENGTH] = min;

	fillIndex = (fillIndex + 1) % nrBUFFERS;
	lineCounter ++;
	if (lineCounter < FRAMES_PER_TONE * nrTONES) {
	   return;
	}
//
//	there is a constant relationship between the fill- and read-index
	readIndex	= fillIndex - FRAMES_PER_TONE * nrTONES;
	if (readIndex < 0)
	   readIndex += nrBUFFERS;
	processLine (readIndex);
}
//
//	we apply a costas test on selected elements in the line
//	and if we are (more or less) convinced the data is recorded
//	in the cache
void	ft8_Decoder::processLine (int lineno) {
std::vector<costasValue> cache;
int	lowBin	= - spectrumWidth. load () / 2 / BINWIDTH + TONE_LENGTH;
int	highBin	=   spectrumWidth. load () / 2 / BINWIDTH + TONE_LENGTH;

float xxx [2 * TONE_LENGTH];

	for (int bin = lowBin; bin < highBin; bin ++) {
	   float tmp = testCostas (lineno, bin);
	   xxx [bin] = tmp;
	}

	peakFinder (xxx, lowBin, highBin, cache);

	if (cache. size () == 0)
	   return;

//	float	max	= 0;
//	int	maxIndex	= 0;
//	for (int i = 0; i < cache. size (); i ++) {
//	   if (cache. at (i). relative > max) {
//	      max = cache. at (i). relative;
//	      maxIndex = i;
//	   }
//	}
//	costasValue it = cache. at (maxIndex);
	costasValue it = cache. at (0);

	float log174 [FT8_LDPC_BITS];
	it. value = decodeTones (lineno, it. index, log174);
	theProcessor . PassOn (lineCounter, it.value, it. strength,
	                 (int)(it. index - TONE_LENGTH) * BINWIDTH, log174);
}

//      with an FFT of 3840 over een band of 12 K, the binwidth = 3.125,
//      i.e. one tone fits takes 2 bins
static
int costasPattern [] = {3, 1, 4, 0, 6, 5, 2};

static
float	getScore	(float *p8, int bin, int tone) {
int index = bin + 2 * costasPattern [tone];
float res =  8 * (p8 [index] + p8 [index + 1]);

	for (int i = -3; i < +3; i ++)
	   res -= p8 [index + 2 * i] + p8 [index + 2 * i];
	return res < 0 ? 0 : res;
}

//float	ft8_Decoder::testCostas (int row, int bin) {
//float	score	= 0;
//
//	for (int tone = 0; tone < 7; tone ++) {
//	   float subScore;
//	   float *buf =  
//	        theBuffer [(row  +  FRAMES_PER_TONE * tone) % nrBUFFERS];
//	   subScore = buf [bin + 2 * costasPattern [tone]] +
//	                          buf [bin + 1 + 2 * costasPattern [tone]];
//	   buf		=
//	        theBuffer [(row + FRAMES_PER_TONE * (36 + tone)) % nrBUFFERS];
//	   subScore *= buf [bin + 2 * costasPattern [tone]] +
//	                         buf [bin + 1 + 2 * costasPattern [tone]];
//	   buf		=
//	        theBuffer [(row + FRAMES_PER_TONE * (72 + tone)) % nrBUFFERS];
//	   subScore *= buf [bin + 2 * costasPattern [tone]] +
//	                         buf [bin + 1 + 2 * costasPattern [tone]];
//	   score += subScore;
//	}
//	return score;
//}
	
float	ft8_Decoder::testCostas (int row, int bin) {
float	*p8;
float	score = 0;

	for (int tone = 0; tone < 7; tone ++) {
	   p8	= theBuffer [(row  +  FRAMES_PER_TONE * tone) % nrBUFFERS];
	   score += getScore (p8, bin, tone);
	}

	for (int tone = 0; tone < 7; tone ++) {
	   p8	= theBuffer [(row + FRAMES_PER_TONE * (36 +  tone)) % nrBUFFERS];
	   score += getScore (p8, bin, tone);
	}

	for (int tone = 0; tone < 7; tone ++) {
	   p8	= theBuffer [(row + FRAMES_PER_TONE * (72 + tone)) % nrBUFFERS];
	   score += getScore (p8, bin, tone);
	}

	return score;
}
//
//	we compute the "strength" of the tone bins as relation beetween
//	the bin with the most energy compared to the average of the
//	bins for each tone, and take the average over the tones.
float	ft8_Decoder::decodeTones (int row, int bin, float *log174) {
int	filler	= 0;
float	strength = 0;

	for (int i = 0; i < nrTONES; i ++) {
	   if (is_syncTable [i])
	      continue;

	   int theRow = (row + FRAMES_PER_TONE * i) % nrBUFFERS;
	   float f =  decodeTone (theRow, bin, &log174 [filler]);
//	   strength += f - 10 * log10 (8192);
	   strength += f -  10 * log (theBuffer [theRow][0]);
	   filler += 3;
	}
	normalize_logl (log174);
	return strength / (79 - 21);
}

static inline
float max2 (float a, float b) {
	return (a >= b) ? a : b;
}

static inline
float max4 (float a, float b, float c, float d) {
	return max2 (max2 (a, b), max2 (c, d));
}

static
const uint8_t FT8_Gray_map [8] = {0, 1, 3, 2, 5, 6, 4, 7};
float	ft8_Decoder::decodeTone (int row, int bin, float *logl) {
float s1 [8];
float s2 [8];
float strength	= -100;

	for (int i = 0; i < 8; i ++) {
	   float a1 = 10 * log (theBuffer [row][bin + 2 * i] + 0.0001);
	   float a2 = 10 * log (theBuffer [row][bin + 2 * i + 1] + 0.0001);
	   if ((a1 + a2) / 2 > strength)
	      strength = (a1 + a2) / 2;
	   s1 [i] =  (a1 + a2) / 2;
	}

	for (int i = 0; i < 8; i ++)
	   s2 [i] = s1 [FT8_Gray_map [i]];

	logl [0] = max4 (s2 [4], s2 [5], s2 [6], s2 [7]) -
	                         max4 (s2 [0], s2 [1], s2 [2], s2 [3]);
	logl [1] = max4 (s2 [2], s2 [3], s2 [6], s2 [7]) -
	                         max4 (s2 [0], s2 [1], s2 [4], s2 [5]);
	logl [2] = max4 (s2 [1], s2 [3], s2 [5], s2 [7]) -
	                        max4 (s2 [0], s2 [2], s2 [4], s2 [6]);

	return strength;
	return strength - LOG_BASE;
}

void	ft8_Decoder::normalize_logl (float *log174) {
float sum = 0;
float sum2 = 0;
//	Compute the variance of log174

	for (int i = 0; i < 174; ++i) {
	   sum  += log174 [i];
	   sum2 += log174 [i] * log174 [i];
	}

	float inv_n = 1.0f / FT8_LDPC_BITS;
	float variance = (sum2 - (sum * sum * inv_n)) * inv_n;

//	Normalize log174 distribution and scale it with
//	experimentally found coefficient
	float norm_factor = sqrtf (72.0f / variance);
//	float norm_factor = sqrtf(24.0f / variance);
	for (int i = 0; i < FT8_LDPC_BITS; ++i) {
	   log174 [i] *= norm_factor;
	}
}

//
void	ft8_Decoder::set_maxIterations (int n) {
	maxIterations. store (n);
	theProcessor. set_maxIterations (n);
}

void	ft8_Decoder::showText (const QStringList &resList) {
	(void)resList;
}

void	ft8_Decoder::set_spectrumWidth (int n) {
	spectrumWidth. store (n);
}

//void	ft8_Decoder::printLine (const QString &s) {
//	printLine_2 (s);
//}

#define	KK	3
static inline
float	sum (float *V, int index) {
//float result = 0;
//	for (int i = 0; i < 2 * KK; i ++)
//	   result  += V [index - KK + i];
//	return result / KK;
	return V [index];
}

void	ft8_Decoder::peakFinder (float *V, int begin, int end,
	                                    std::vector<costasValue> &cache) {
costasValue	E;
//float workVector [2 * TONE_LENGTH];
float	avg	= 0;
//float	max	= 0;
bool	flag	= false;

	for (int index = begin; index < end; index ++)
	   avg += V [index];
	avg /= (end - begin + 1);

	E. value = 0;
	E. relative	= 0;

	for (int index = begin + 5; index < end - 5; index ++) {
	   if (V [index] < 1.10 * avg)	// 
	      continue;

	   if (!(1.05 * V [index - 5] < V [index]) &&
	        (1.05 * V [index + 5] < V [index]))
	      continue;

	   float max = 0;
	   int maxIndex = 0;
	   for (int i = -KK; i < KK; i ++) {
	      if (V [index + i] > max) {
	         max = V [index + i];
	         maxIndex = index + i;
	      }
	   }

	   if (V [maxIndex] / V [maxIndex + 5] > E. relative) {
	      E. index = maxIndex;
	      E. value = theBuffer [readIndex][maxIndex];
	      E. relative = V [maxIndex] / V [maxIndex + 5];
	      float tt	= theBuffer [readIndex][maxIndex] * theBuffer [readIndex][0];
	      E. strength = 10 * log10 (tt / theBuffer [readIndex][2 * TONE_LENGTH]);
	      flag = true;
	   }
	   index += 5;
	}
	if (flag)
	   cache. push_back (E);
}

bool	ft8_Decoder::pskReporterReady () {
	return pskReady;
}

void	ft8_Decoder::print_statistics () {
}
//
//	This is not a slot anymore
void	ft8_Decoder::handle_cq_selector	(const QString &s) {
	theProcessor. set_cqSelector (s == "cq only");
}

