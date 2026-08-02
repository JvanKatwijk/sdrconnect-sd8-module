#
/*
 *	Copyright (C) 2025
 *	Jan van Katwijk (J.vanKatwijk@gmail.com)
 *	Lazy Chair Computing
 *
 *	This file is part of the ft8 module
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

#include	<QByteArray>

#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<sys/types.h>
#include	<cstring>

#include	"socket-handler.h"

	socketHandler::
	            socketHandler (const QString &hostAddress,
	                          int	portNumber, 
	                          RingBuffer<std::complex<int16_t>> *b) {
	this	-> hostAddress	= hostAddress;
	this	-> portNumber	= portNumber;
	this	-> _I_Buffer	= b;
	this	-> socket	= new QWebSocket;
	connect (socket, &QWebSocket::connected,
	        this, &socketHandler::onConnected);
	connect (socket, &QWebSocket::disconnected,
	         this, &socketHandler::onDisconnect);
#if QT_VERSION >= QT_VERSION_CHECK (6, 7, 0)
        connect (socket, &QWebSocket::errorOccurred,
                        this, &socketHandler::onSocketError);
#endif
	connected	= false;
	QString urlString = "ws://%1:%2";
	socket -> open (QUrl (urlString. arg (hostAddress). arg(QString::number (portNumber))));
}

	socketHandler::~socketHandler	() {
}

void	socketHandler::onConnected	() {
	connected	= true;
	connect (socket, &QWebSocket::textFrameReceived,
                 this, &socketHandler::textMessageReceived);
	connect (socket, &QWebSocket::textMessageReceived,
                 this, &socketHandler::textMessageReceived);
	connect (socket, &QWebSocket::binaryFrameReceived,
                 this, &socketHandler::binaryMessageReceived);
	emit reportConnect ();
}

void	socketHandler::onDisconnect	() {
	fprintf (stderr, "in onDisconnect\n");
	if (socket != 0)
	   socket	-> deleteLater ();
	emit reportDisconnect ();
//	socket	= nullptr;
}
	
void	socketHandler::onSocketError	(QAbstractSocket::SocketError error) {
	(void)error;
	onDisconnect ();
	emit reportDisconnect ();
}

void	socketHandler::sendMessage	(const QString &m) {
	if (connected)
	   socket -> sendTextMessage (m);
}

void	socketHandler::binaryMessageReceived	(const QByteArray &m) {
int16_t *p	= (int16_t *)(m. data ());
	if (p [0] != 1)
	   return;
	if (_I_Buffer -> GetRingBufferWriteAvailable () < m. size () / 4)
	   return;
	_I_Buffer -> putDataIntoBuffer (&(p [1]), (m. size ()  - 1) / 4);
	emit binDataAvailable ();
}

void	socketHandler::textMessageReceived	(const QString &m) {
	dispatchMessage (m);
}

