#
/*
 *    Copyright (C)  2026
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the ft8-module
 *
 *    ft8-module is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    ft8-module decoder is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with ft8-module decoder; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#pragma once

#include        <QMainWindow>
#include        <QTimer>
#include        <QLineEdit>
#include	<QTableWidget>
#include	<QTimer>
#include        <QStandardItemModel>
#include        "ui_ft8-module.h"
#include        "constants.h"
#include	"fft-filters.h"
#include        "ringbuffer.h"
#include	"decimator.h"
#include	"bandplan.h"
#include	<mutex>
#include	"preset-handler.h"

#include	"message-handler.h"
class		ft8_Decoder;
class           QSettings;
class           keyPad;
class		reporterWriter;

class	RadioInterface:public QMainWindow,
	               private Ui_MainWindow {
Q_OBJECT
public:
		RadioInterface (QSettings	*sI,
	                        const QString	&presetFile,
	                        QWidget		*parent = nullptr);
		~RadioInterface	();

	int32_t		get_selectedFrequency	();
private:
        RingBuffer<std::complex<float> > inputData;

	struct band {
           int32_t      lowF;
           int32_t      highF;
           int32_t      currentOffset;
        }theBand;

	messageHandler	*inputHandler;
	QTableWidget	*tableWidget;
	fftFilter	hfFilter;
	bandPlan	my_bandPlan;
	presetHandler	thePresets;
	decimator	theDecimator;
	QTimer		secondsTimer;

	int32_t		selectedFrequency;
	QSettings       *settings;
        int32_t         inputRate;
	ft8_Decoder	*theDecoder;
	int16_t		delayCount;
//
        keyPad          *mykeyPad;

	QStandardItemModel	model;
	QStringList	theResults;
	int		teller;
	std::atomic<int>	spectrumWidth;
	std::atomic<bool>	running;

	reporterWriter	*theWriter;
	bool		pskReady;
	std::mutex	locker;
	std::mutex	bandLocker;
	std::atomic<FILE *> filePointer;
	void		enableButtons		();
	void		disableButtons		();
public	slots:
	void		printLine		(const QString &,
	                                         int, int,
	                                         const QString &, int);
	void		addMessage		(const QString &,
	                                         const QString &,
	                                         int, int);
	void		handle_hostName		();
	void		handle_disconnect	();
	void		handle_connect		();
	void		reset			();
private slots:
        void            handle_freqButton       ();
	void		set_freqSave		();
	void		handle_quitButton	();
	void		updateTime		();
	void		closeEvent		(QCloseEvent *event);
	void		handle_cq_selector	(const QString &);
	void		handle_identityButton	();
	void		set_maxIterations	(int);
	void		handle_filesaveButton	();
	void		handle_pskReporterButton	();
	void		set_spectrumWidth	(int);
	void		handle_presetButton	();
public slots:
	void		setFrequency		(int32_t);
	void		sampleHandler		(int amount);
	void		show_pskStatus		(bool);
	void		sendMessages		();
	void		handle_preset		(const QString &);
	void		displayFrequency	(int);
	void		newSNR			(double);
};

