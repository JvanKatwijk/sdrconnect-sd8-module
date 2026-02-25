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
#include	"downconverter.h"
#include	"fft-filters.h"

#define FILTER_SIZE     1024
#define FILTER_STRENGTH  255

	downConverter::downConverter (int32_t inRate, int32_t outRate) {
int	err;
	this	-> inRate	= inRate;
	this	-> outRate	= outRate;
	filtering_needed	= inRate > outRate;
	if (filtering_needed) {
	   this	-> hfFilter	= new fftFilter (FILTER_SIZE,
                                                 FILTER_STRENGTH);
           hfFilter-> setBand (-outRate / 2, outRate / 2, inRate);
	}
	else
	   this	-> hfFilter	= nullptr;
	inputLimit              = 2048;
        ratio                   = double(outRate) / inRate;
        outputLimit             = inputLimit * ratio;
//	src_converter		= src_new (SRC_SINC_BEST_QUALITY, 2, &err);
	src_converter		= src_new (SRC_LINEAR, 2, &err);
//	src_converter		= src_new (SRC_SINC_MEDIUM_QUALITY, 2, &err);
        src_data                = new SRC_DATA;
        inBuffer                = new float [2 * inputLimit + 20];
        outBuffer               = new float [2 * outputLimit + 20];
        src_data-> data_in      = inBuffer;
        src_data-> data_out     = outBuffer;
        src_data-> src_ratio    = ratio;
        src_data-> end_of_input = 0;
        inp                     = 0;
	dataBuffer		= new RingBuffer<std::complex<float>> (1024 * 1024);
}

	downConverter::~downConverter	() {
	if (hfFilter != NULL)
	   delete hfFilter;
	delete[]	inBuffer;
	delete[]	outBuffer;
	delete		dataBuffer;
	src_delete (src_converter);
	delete		src_data;
}

int32_t	downConverter::bufferSize_in	() {
	return inputLimit;
}

int32_t	downConverter::bufferSize_out	() {
	return outputLimit;
}

void	downConverter::convert_in	(std::complex<float> *buffer) {
int32_t	i;

	for (i = 0; i < inputLimit; i ++) {
	   if (filtering_needed) {
	      buffer [i]		= hfFilter -> Pass (buffer [i]);
	      inBuffer [2 * inp]      = real (buffer [i]);
	      inBuffer [2 * inp + 1]  = imag (buffer [i]);
	   }
	   else {
	      inBuffer [2 * inp]      = real (buffer [i]) / 2048.0;
	      inBuffer [2 * inp + 1]  = imag (buffer [i]) / 2048.0;
	   }
	   inp ++;
	   if (inp >= inputLimit) {
	      src_data	-> input_frames         = inp;
	      src_data  -> output_frames        = outputLimit + 10;
	      int res       = src_process (src_converter, src_data);
	      (void)res;
	      inp       = 0;
	      int framesOut       = src_data -> output_frames_gen;
	      std::complex<float> *b2  = dynVec (std::complex<float>,
	                                              framesOut);
	      for (int j = 0; j < framesOut; j ++)
	         b2 [j] = std::complex<float> (outBuffer [2 * j] * 10,
	                                       outBuffer [2 * j + 1]* 10);
	      dataBuffer -> putDataIntoBuffer (b2, framesOut);
	   }
	}
}

uint32_t downConverter::hasData	() {
	return dataBuffer -> GetRingBufferReadAvailable ();
}

int32_t	downConverter::dataOut	(std::complex<float> *buffer, int amount) {
	return dataBuffer -> getDataFromBuffer (buffer, amount);
}

