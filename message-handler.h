#
/*
 *    Copyright (C)   2026
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

#include	<QString>
#include	<complex>
#include	<atomic>
#include	"constants.h"
#include	"ringbuffer.h"
#include	"socket-handler.h"
#include	"hilbertfilter.h"

class		downConverter;

class messageHandler: public socketHandler {
Q_OBJECT
public:
		messageHandler (const QString &hostAddress,
	                        int	portNumber,
	                        int	outputRate,
	                        int	startFreq,
	                        RingBuffer<std::complex<float>> *);
		~messageHandler	();
	void	setFrequency	(int32_t);
	int	getFrequency	();
	void	set_filterBW	(uint32_t);
private:
	RingBuffer<std::complex<int16_t>> _I_Buffer;
	hilbertFilter		theFilter;
	RingBuffer<std::complex<float>> *_O_Buffer;
	void	audioStreamEnable	(bool);
	void	setFrequency	(int32_t, int32_t);
	downConverter	*theConverter;
	int	outputRate;
	int	vfo_frequency;
private slots:
	void	connection_set		();
	void	no_connection		();
	void	binDataAvailable	();
	void	dispatchMessage		(const QString &);
//	for local use
signals:
	void	connection_success	();
	void	connection_failed	();
	void	frequency_changed	(int);
	void	signalPower		(double);
	void	dataAvailable		(int);
};

