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

#include	<QObject>
#include	<QFrame>
#include	<QSettings>
#include	<QStringList>
#include        <QStandardItemModel>
#include	<atomic>
#include	<mutex>
#include	"constants.h"
#include	"ft8-constants.h"
#include	"ldpc.h"
#include	"ft8-processor.h"
#include	"fft.h"

typedef struct {
	int	index;
	float	value;
	float 	relative;
	float	strength;
} costasValue;

class	RadioInterface;

class	ft8_Decoder: public QObject {
Q_OBJECT
public:
		ft8_Decoder	(RadioInterface *,
	                         int32_t,
	                         QSettings	*);
		~ft8_Decoder	();
	void	processBuffer		(std::complex<float> *, int32_t);
	void    process			(std::complex<float> z);
	bool	pskReporterReady 	();
	void	handle_cq_selector	(const QString &);
	void	set_spectrumWidth	(int);
	void	set_maxIterations	(int);

private:	
	ldpc		ldpcHandler;
	ft8_processor	theProcessor;
	fft_handler	the_fftHandler;
	reporterWriter	*theWriter;
	QSettings	*ft8Settings;
	RadioInterface	*mr;
	int32_t		samplesperSymbol;
	bool		pskReady;
	void		peakFinder (float *V, int begin, int end,
                                            std::vector<costasValue> &cache);

	void		processLine	(int);
	float		testCostas	(int, int);
	float		decodeTones	(int row, int bin,
	                                                  float *log174);
	float		decodeTone	(int row, int bin, float *logl);
	void		normalize_logl	(float *);
	int		addCache	(float,
	                                 int, std::vector<costasValue> &);

	std::complex<float> *fftVector;
	float		*window;

	int		messageLength;
	int		costasLength;
	float		*theBuffer [nrBUFFERS];
	std::complex<float> *inputBuffer;
	std::complex<float> *inBuffer;
	int		fillIndex;
	int		readIndex;
	int		inPointer;
	int		lineCounter;
	std::atomic<int>	maxIterations;
	std::atomic<int>	cacheDepth;
	std::atomic<int>	spectrumWidth;
	bool		is_syncTable [nrTONES];

	QStandardItemModel	model;
	void		showText	(const QStringList &);
	QStringList	theResults;
	int		teller;
	void		print_statistics	();
signals:
	void		sendMessages		();
	void		show_pskStatus		(bool);
};


