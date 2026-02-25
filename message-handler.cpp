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

#include	<QJsonDocument>
#include	<QJsonObject>
#include	"ft8-constants.h"
#include	"message-handler.h"
#include	"downconverter.h"

	messageHandler::messageHandler (const QString &hostAddress,
	                                int	portNumber,
	                                int	outputRate,
	                                int	startFreq,
	                                RingBuffer<std::complex<float>> *b):
	                                    _I_Buffer (32 * 32768),
	                                    theFilter (1024),
	                                    socketHandler (hostAddress,
	                                                   portNumber,
	                                                   &_I_Buffer) {
	this	-> outputRate	= outputRate;
	_O_Buffer		= b;
	theConverter		= nullptr;
	connect (this, &socketHandler::reportConnect,
	         this, &messageHandler::connection_set);
	connect (this, &socketHandler::reportDisconnect,
	         this, &messageHandler::no_connection);
	vfo_frequency		= startFreq;
}

	messageHandler::~messageHandler	() {
}

void	messageHandler::setFrequency	(int32_t freq) {
QString message	= "{ \"event_type\":\"set_property\",\"property\":\"device_vfo_frequency\",\"value\":\"%1\" }";
	sendMessage (message. arg (QString::number (freq)));
	this ->vfo_frequency	= freq;
}

int	messageHandler::getFrequency	() {
	return vfo_frequency;
}

void	messageHandler::connection_set	() {
	disconnect (this, &socketHandler::reportConnect,
	            this, &messageHandler::connection_set);
	connect (this, &socketHandler::dispatchMessage,
                 this, &messageHandler::dispatchMessage);
	connect (this, &socketHandler::binDataAvailable,
                 this, &messageHandler::binDataAvailable);
	setFrequency (vfo_frequency);
	set_filterBW	(12000);
	QString message	= "{ \"event_type\":\"set_property\",\"property\":\"demodulator\",\"value\":\"%1\" }";
	sendMessage (message. arg ("USB"));
	audioStreamEnable (true);
	emit connection_success	();
}

void	messageHandler::no_connection	() {
	emit connection_failed	();
}

void	messageHandler::audioStreamEnable	(bool b) {
QString message	= "{ \"event_type\":\"audio_stream_enable\",\"property\":\"\",\"value\":\"%1\" }";
	sendMessage (message. arg (b ? "true" : "false"));
}

void	messageHandler::set_filterBW	(uint32_t bw) {
QString message	= "{ \"event_type\":\"set_property\",\"property\":\"filter_bandwidth\",\"value\":\"%1\" }";
	sendMessage (message. arg (QString::number (bw)));
}

void	messageHandler::binDataAvailable () {
	std::complex<int16_t>  *inBuffer = dynVec (std::complex<int16_t>,
	                                            outputRate / 50);
	while (_I_Buffer. GetRingBufferReadAvailable () >=
	                                 (uint32_t)outputRate / 50) {
	   _I_Buffer. getDataFromBuffer (inBuffer, outputRate / 50);
	   std::complex<float> *outBuffer = dynVec (std::complex<float>,
	                                              outputRate / 50);
	   for (int i = 0; i < outputRate / 50; i ++)
	       outBuffer [i] = theFilter. Pass (real (inBuffer [i]) / 2048.0);
	   _O_Buffer -> putDataIntoBuffer (outBuffer, outputRate / 50);
	   emit dataAvailable (_O_Buffer -> GetRingBufferReadAvailable ());
	}
}

void	messageHandler::dispatchMessage	(const QString &m) {
	QJsonObject obj;
	QJsonDocument doc = QJsonDocument::fromJson (m. toUtf8 ());
	if (doc. isNull ())
	   return;	// cannot handle
	obj	= doc. object ();

	QString eventType	= obj ["event_type"]. toString ();
	if (eventType == "property_changed") {
	   QString property = obj ["property"]. toString ();
	   if (property == "device_vfo_frequency") {
	      QString vfoString = obj ["value"]. toString ();
	      bool b;
	      int vfo	= vfoString. toInt (&b);
	      if (!b)
	         return;
	      vfo_frequency	= vfo;
	      emit frequency_changed (vfo);
	   }
	   if (property == "signal_power") {
	      QString snrString = obj ["value"]. toString ();
	      QString res;
	      for (int i = 0; i < snrString. size (); i ++)
                  if (snrString. at (i) == QChar (','))
                     res. push_back (QChar ('.'));
                  else
                     res. push_back (snrString. at (i));
	      bool b;
	      double snr = res. toDouble (&b);
	      if (!b)
	         return;
	      emit signalPower (snr);
	   }
	}
}

