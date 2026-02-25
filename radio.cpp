#
/*
 *    Copyright (C)  2026
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
#include	<unistd.h>
#include	<QSettings>
#include	<QDebug>
#include	<QDateTime>
#include	<QObject>
#include	<QDir>
#include	<QColor>
#include	<QMessageBox>
#include	<QFileDialog>
#include	<QHeaderView>
#include        <QDesktopServices>
#include	"radio.h"
#include	"fft-filters.h"
#include        "popup-keypad.h"
#include	"identity-dialog.h"
#include	"ft8-decoder.h"
#include	"psk-writer.h"
//
#include	"ft8-decoder.h"

#define browserAddress  "https://pskreporter.info/pskmap.html"

static
QString	FrequencytoString (int32_t freq) {
	if (freq < 10)
	   return QString (QChar ('0' + (uint8_t)(freq % 10)));
	return 
	   FrequencytoString (freq / 10). append (QChar ('0' + (uint8_t)(freq % 10)));
}

uint32_t freqTable [] =
	{1840, 3753, 5357, 7056, 7071, 7074, 10136,
	14075, 18104, 21075, 24915, 28074, 0};
//
//	reactie op samplerate change vanuit SDRconnect

	RadioInterface::RadioInterface (QSettings	*sI,
	                                const QString	&presetFile,
	                                QWidget		*parent):
	                                    QMainWindow (parent),
	                                    inputData	(1024 * 1024),
	                                    hfFilter	(2048, 377),
	                                    my_bandPlan (":res/sw-bandplan.xml"),
	                                    thePresets (this, presetFile),
	                                    theDecimator (48000, 12000) {
	this	-> settings	= sI;
	this	-> inputRate	= 48000;
	setupUi (this);

	running. store (false);
	tableWidget	= new QTableWidget (0, 5);
	tableWidget	-> setColumnWidth (0, 100);	// date
	tableWidget	-> setColumnWidth (1, 20);
	tableWidget	-> setColumnWidth (2, 100);
	tableWidget	-> setColumnWidth (3, 300);
	tableWidget	-> setColumnWidth (4, 50);
	QHeaderView * headerView = tableWidget -> horizontalHeader ();
	headerView	-> setSectionResizeMode (1, QHeaderView::Stretch);
	tableWidget	-> setHorizontalHeaderLabels (
	       QStringList () << tr ("date") << tr ("???") << tr ("freq") <<
	                      tr ("message") << tr ("strength"));
	displayArea	-> setWidget (tableWidget);
//      and some buffers

	selectedFrequency	= KHz (14070);
	
//	settings
	settings	-> beginGroup ("ft8Settings");
	int val		= settings -> value ("width", 4000). toInt ();
	spectrumWidth_selector -> setValue (val);
	theBand. currentOffset  = 0;
        theBand. lowF           = -val / 2;
        theBand. highF          = val / 2;
	hfFilter. setBand (-val / 2, val / 2, inputRate);
	val         = settings -> value ("maxIterations", 20). toInt ();
        iterationSelector -> setValue (val);
	settings	-> sync ();
        mykeyPad                = new keyPad (this);
	QString homeCall        =
                 settings       -> value ("homeCall", "your call"). toString ();
        homecall_label          -> setText (homeCall);

	this	-> setWindowTitle (QString ("FT8 module for SDRconnect (") + GITHASH + ")");
	copyright_label		-> setToolTip (QString ("build ") + GITHASH);
	settings	-> endGroup ();
	connect (presetButton, &QPushButton::clicked,
	         this, &RadioInterface::handle_presetButton);
        connect (freqButton, &QPushButton::clicked,
                 this, &RadioInterface::handle_freqButton);
	connect (freqSave, &QPushButton::clicked,
                 this, &RadioInterface::set_freqSave);
	connect (cq_selector, &QComboBox::textActivated,
                 this, &RadioInterface::handle_cq_selector);
	connect (identityButton, &QPushButton::clicked,
	         this, &RadioInterface::handle_identityButton);
	connect (spectrumWidth_selector, &QSpinBox::valueChanged,
                 this, &RadioInterface::set_spectrumWidth);
	connect (iterationSelector, &QSpinBox::valueChanged,
                 this, &RadioInterface::set_maxIterations);
	connect	(pskReporterButton, &QPushButton::clicked,
	         this, &RadioInterface::handle_pskReporterButton);
	connect (filesaveButton, &QPushButton::clicked,
	         this, &RadioInterface::handle_filesaveButton);
	theWriter		= nullptr;
	filePointer. store (nullptr);
	delayCount		= 0;

	secondsTimer. setInterval (1000);
	connect (&secondsTimer, SIGNAL (timeout (void)),
                 this, SLOT (updateTime (void)));
        secondsTimer. start (1000);

//	and off we go
	theDecoder	= new ft8_Decoder (this, 12000, settings);
	inputHandler	= nullptr;

	
	settings	-> beginGroup ("ft8Settings");
	QString serverAddress
	       = settings -> value ("serverAddress", "127.0.0.1"). toString ();
	settings	-> endGroup ();
	hostNameLabel	-> setInputMask ("000.000.000.000");
	hostNameLabel	-> setText (serverAddress);
	connect (hostNameLabel, &QLineEdit::returnPressed,
	         this, &RadioInterface::handle_hostName);
	disableButtons	();
}

//      The end of all
        RadioInterface::~RadioInterface () {
	handle_quitButton ();
}
//
//	handle hostname is called whenever the user acknowledges the
//	hostname, no guarantee tht there is a connection
void	RadioInterface::handle_hostName		() {
QString	hostName	= hostNameLabel -> text ();
int	portNumber	= portLabel	-> value ();
	if (inputHandler != nullptr)
	   return;

	inputHandler	= new messageHandler (hostName, portNumber,
	                                              48000, 10136000,
	                                              &inputData);
	settings	-> beginGroup ("ft8Settings");
	settings	-> setValue ("serverAddress", hostName);
	settings	-> endGroup ();
	fprintf (stderr, "we have an input handler\n");
	connect (inputHandler, &messageHandler::connection_success,
	         this, &RadioInterface::handle_connect);
	connect (inputHandler, &messageHandler::connection_failed,
	         this, &RadioInterface::handle_disconnect);
}
//
//	at this point, the underlying handler has set everything
//	ready for processing
void	RadioInterface::handle_connect	() {
	disconnect (inputHandler, &messageHandler::connection_success,
	            this, &RadioInterface::handle_connect);
	connect (inputHandler, &messageHandler::dataAvailable,
	         this, &RadioInterface::sampleHandler);
	connect (inputHandler, &messageHandler::frequency_changed,
	         this, &RadioInterface::displayFrequency);
	connect (inputHandler, &messageHandler::signalPower,
	         this, &RadioInterface::newSNR);
	connect	(inputHandler, &messageHandler::connection_failed,
	         this, &RadioInterface::reset);
	connectionLabel	-> setText ("connected");
	settings        -> beginGroup ("ft8Settings");
        int val         = settings -> value ("freq", 10136). toInt ();
	settings	-> endGroup	();
	settings	-> sync ();
	enableButtons ();
	setFrequency (KHz (val));
}
//
//	disconnect is called when - at start up - no connection
//	could be made
void	RadioInterface::handle_disconnect () {
	disconnect (inputHandler, &messageHandler::connection_success,
	            this, &RadioInterface::handle_connect);
	delete inputHandler;
	inputHandler = nullptr;
	QMessageBox::warning (this, tr ("Warning"),
                                       tr ("Connection lost"));
}
//
//	reset is called when - during processing - the connection fails
//	for whatever reason, so we completely reset
void	RadioInterface::reset	() {
	delete inputHandler;
	inputHandler	= nullptr;
	hostNameLabel	-> setInputMask ("000.000.000.000");
	hostNameLabel	-> setText ("127.0.0.1");
	connect (hostNameLabel, &QLineEdit::returnPressed,
	         this, &RadioInterface::handle_hostName);
	disableButtons	();
	int	rows	= tableWidget -> rowCount ();
	for (int i = rows - 1; i >= 1; i --)
	   tableWidget -> removeRow (i);
}
//
//	If the user quits before selecting a device ....
void	RadioInterface::handle_quitButton	() {
	settings	-> sync ();
	thePresets. hide ();
	if (inputHandler != nullptr)
	   delete inputHandler;
	delete		theDecoder;
	secondsTimer. stop ();
	mykeyPad	-> hidePad ();
        delete		mykeyPad;
	if (filePointer. load () != nullptr)
	   fclose (filePointer. load ());
	hide ();
}
//
//	
void    RadioInterface::handle_freqButton () {
        if (mykeyPad -> isVisible ())
           mykeyPad -> hidePad ();
        else
           mykeyPad     -> showPad ();
}

//	setFrequency is called from the frequency panel,
//	from inside to change VFO and offset,
//	and from the decoder
void	RadioInterface::setFrequency (int32_t frequency) {
	selectedFrequency	= frequency;
	settings        -> beginGroup ("ft8Settings");
        settings	-> setValue ("freq", frequency / KHz (1));
	settings	-> endGroup ();
	settings	-> sync ();
	inputHandler -> setFrequency (selectedFrequency);
	displayFrequency	(selectedFrequency);
}

void	RadioInterface::displayFrequency (int selected) {
	hfFilter. setBand (theBand. currentOffset + theBand. lowF,
                           theBand. currentOffset + theBand. highF,
                                                  inputRate);
	QString ff	= FrequencytoString (selected);
	frequencyDisplay	-> display (ff);
	bandLabel	-> setText (my_bandPlan. getFrequencyLabel (selected));        
	settings	-> beginGroup ("ft8Settings");
	settings	-> setValue ("frequency", selected / 1000);
	settings	-> endGroup ();
	settings	-> sync ();
}
//
int32_t	RadioInterface::get_selectedFrequency	() {
	return selectedFrequency;
}

//
void    RadioInterface::set_freqSave    () {
	int frequency	= inputHandler ->  getFrequency ();
	thePresets. addElement (frequency);
}

//////////////////////////////////////////////////////////////////
//
void	RadioInterface::sampleHandler (int amount) {
std::complex<float> *buffer = dynVec (std::complex<float>, 
                                            theDecimator. inSize ()); 
std::complex<float> *ifBuffer = dynVec (std::complex<float>,
	                                    theDecimator. outSize ());
	(void)amount;
	while (inputData. GetRingBufferReadAvailable () > 512) {
	   inputData. getDataFromBuffer (buffer, 512);
	   bandLocker. lock ();
	   for (int i = 0; i < 512; i ++) {
	      std::complex<float> temp = hfFilter. Pass (buffer [i]);
              if (theDecimator. add (temp, ifBuffer)) {
                 for (int j = 0; j < theDecimator. outSize (); j ++) {
	            theDecoder -> process (ifBuffer [j]);
                 }
              }
           }
	   bandLocker. unlock ();
        }
}
//
//
void	RadioInterface::updateTime		(void) {
QDateTime currentTime = QDateTime::currentDateTime ();

	timeDisplay     -> setText (currentTime.
                                    toString (QString ("dd.MM.yy:hh:mm:ss")));
}

#include <QCloseEvent>
void RadioInterface::closeEvent (QCloseEvent *event) {

        QMessageBox::StandardButton resultButton =
                        QMessageBox::question (this, "ft8-module",
                                               tr("Are you sure?\n"),
                                               QMessageBox::No | QMessageBox::Yes,
                                               QMessageBox::Yes);
        if (resultButton != QMessageBox::Yes) {
           event -> ignore();
        } else {
//	   handle_quitButton ();
           event -> accept ();
//	   exit (1);
        }
}

void	RadioInterface::handle_preset	(const QString &freqText) {
bool	ok;
int32_t frequency = freqText. toInt (&ok);
	if (!ok)
	   return;
	setFrequency (KHz (frequency));
}

void	RadioInterface::show_pskStatus	(bool b) {
	if (b) {
	   pskStatus_label -> setStyleSheet ("QLabel {background-color : green}");
	   pskStatus_label -> setText ("sending to pskServer");
	}
	else {
	   pskStatus_label -> setStyleSheet ("QLabel {background-color : red}");
	   pskStatus_label -> setText ("");
	}
}

void	RadioInterface::handle_cq_selector	(const QString &s) {
	if (theDecoder != nullptr)
	   theDecoder -> handle_cq_selector (s);
}

void	RadioInterface::handle_identityButton () {
	identityDialog Identity (settings);
        Identity. QDialog::exec ();
	settings     -> beginGroup ("ft8Settings");
	QString homeCall = settings     -> value ("homeCall", "your call").
	                                                      toString ();
	settings     -> endGroup ();
	homecall_label	-> setText (homeCall);
}

void	RadioInterface::set_maxIterations	(int n) {
	if (theDecoder != nullptr) {
	   theDecoder -> set_maxIterations	(n);
	   settings	-> beginGroup ("ft8Settings");
	   settings	-> setValue ("maxIterations", n);
	   settings	-> endGroup ();
	   settings	-> sync ();
	}
}

void	RadioInterface::addMessage	(const QString  &call,
	                         const QString  &grid,
	                         int frequency,
	                         int snr) {
	int freq	= inputHandler -> getFrequency () + frequency;
	if (theWriter != nullptr) 
	   theWriter ->  addMessage (call. toStdString (),
	                             grid. toStdString (),
		                     freq, snr);
}


void	RadioInterface::printLine	(const QString &s,
	                                 int val, int freq, 
	                                 const QString &res, int strength) {

int row	= tableWidget -> rowCount ();
	freq		= freq + inputHandler -> getFrequency ();
	tableWidget -> insertRow (row);
	QTableWidgetItem *item0	= new QTableWidgetItem;
	item0		-> setTextAlignment (Qt::AlignLeft);
	tableWidget	-> setItem (row, 0, item0);

	QTableWidgetItem *item1	= new QTableWidgetItem;
	item1		-> setTextAlignment (Qt::AlignLeft);
	tableWidget	-> setItem (row, 1, item1);

	QTableWidgetItem *item2	= new QTableWidgetItem;
	item2		-> setTextAlignment (Qt::AlignLeft);
	tableWidget	-> setItem (row, 2, item2);

	QTableWidgetItem *item3	= new QTableWidgetItem;
	item3		-> setTextAlignment (Qt::AlignLeft);
	tableWidget	-> setItem (row, 3, item3);

	QTableWidgetItem *item4	= new QTableWidgetItem;
	item4		-> setTextAlignment (Qt::AlignLeft);
	tableWidget	-> setItem (row, 4, item4);

	tableWidget	-> item (row, 0) -> setText (s);
	tableWidget	-> item (row, 1) -> setText (QString::number (val));
	tableWidget	-> item (row, 2) -> setText (QString::number (freq));
	tableWidget	-> item (row, 3) -> setText (res);
	tableWidget	-> item (row, 4) -> setText (QString::number (strength));
QString result	= s + ";" + QString::number (val) + ";" +
	          QString::number (freq + inputHandler -> getFrequency ()) + 
	          ";" + res + ";" + QString::number (strength);
	if (theResults. size () >= 50)
           theResults. pop_front ();
        theResults += result;
//	if (filePointer. load () != nullptr)
//	   fprintf (filePointer. load (), "%s\n",
//	                           result. toUtf8 (). data ());
//	fprintf (stderr, "%s\n", result. toLatin1 (). data ());
}

void	RadioInterface::handle_filesaveButton	() {
	if (filePointer. load () != nullptr) {
	   fclose (filePointer);
	   filesaveButton -> setText ("save file");
	   filePointer. store (nullptr);
	   return;
	}

	QString saveName = 
		 QFileDialog::getSaveFileName (nullptr,
                                                 tr ("save file as .."),
                                                 QDir::homePath (),
                                                 tr ("Images (*.txt)"));
	if (saveName == "")
	   return;

	filePointer. store (fopen (saveName. toUtf8 (). data (), "w"));
	if (filePointer. load () == nullptr)
	   return;

	filesaveButton -> setText ("saving");
}

void	RadioInterface::handle_pskReporterButton	() {
	locker. lock ();
	if (theWriter != nullptr) {
	   delete theWriter;
	   pskReady	= false;
	   theWriter	= nullptr;
	   locker. unlock ();
	   show_pskStatus (false);
	   return;
	}

	try {
	   theWriter	= new reporterWriter (settings);
	   pskReady = true;
	} catch (int e) {
	   pskReady	= false;
	}
	locker. unlock ();
	if (pskReady) {
	   if (!QDesktopServices::openUrl(QUrl (browserAddress))) {
	      QMessageBox::warning (this, tr ("Warning"),
	                            tr ("cannot show server"));
	      return;
	   }
//#ifdef	__MINGW32__
//	   std::wstring browserAddress = 
//	                  std::wstring (L"https://pskreporter.info/pskmap.html");
//	   fprintf (stderr, "browser address %s\n", browserAddress. c_str ());
//	   ShellExecute (nullptr, L"open", browserAddress. c_str (),
//	                                   nullptr, nullptr, SW_SHOWNORMAL);
//#else
//	   std::string browserAddress = 
//	                  std::string ("https://pskreporter.info/pskmap.html");
//	   std::string x = std::string ("xdg-open ") + browserAddress;
//	   (void)system (x. c_str ());
//#endif
	}
	show_pskStatus (pskReady);
}

void	RadioInterface::sendMessages () {
	if (theWriter != nullptr) {
	   theWriter -> sendMessages ();
	}
}

void	RadioInterface::handle_presetButton	() {
	if (thePresets. isVisible ())
	   thePresets. hide ();
	else
	   thePresets. show ();
}

void	RadioInterface::set_spectrumWidth	(int n) {
	settings	-> beginGroup ("ft8Settings");
	settings	-> setValue ("width", n);
	settings	-> endGroup ();
	settings	-> sync ();
	theBand. lowF        = -n / 2;
	theBand. highF       = n / 2;
	bandLocker. lock ();
	hfFilter. setBand (theBand. currentOffset + theBand. lowF,
                           theBand. currentOffset + theBand. highF,
	                   inputRate);
	bandLocker. unlock ();
}

void	RadioInterface::newSNR	(double v) {
	this	-> snrDisplay	-> display (v);
}

void	RadioInterface::disableButtons	() {
	freqButton	-> setEnabled (false);
	cq_selector	-> setEnabled (false);
	iterationSelector	-> setEnabled (false);
	identityButton	-> setEnabled (false);
	spectrumWidth_selector	-> setEnabled (false);
	presetButton	-> setEnabled (false);
	filesaveButton	-> setEnabled (false);
}

void	RadioInterface::enableButtons	() {
	freqButton	-> setEnabled (true);
	cq_selector	-> setEnabled (true);
	iterationSelector	-> setEnabled (true);
	identityButton	-> setEnabled (true);
	spectrumWidth_selector	-> setEnabled (true);
	presetButton	-> setEnabled (true);
	filesaveButton	-> setEnabled (true);
}

