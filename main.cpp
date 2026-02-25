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
 *
 *	Main program
 */
#include	<QApplication>
#include	<QSettings>
#include	<QDir>
#include	<unistd.h>
#include	"radio.h"
#include	"constants.h"

static
QString fullPathfor (QString v, QString type) {
QString fileName;

	if (v == QString (""))
	   return QString ("/tmp/xxx");

	if (v. at (0) == QChar ('/'))           // full path specified
	   return v;

	fileName = QDir::homePath ();
	fileName. append ("/");
	fileName. append (v);
	fileName = QDir::toNativeSeparators (fileName);

	if (!fileName. endsWith (type))
	   fileName. append (type);

	return fileName;
}

#define	DEFAULT_INI	".qt-ft8.ini"
#define	FT8_PRESETS	".ft8-presets.xml"

int	main (int argc, char **argv) {
int32_t		opt;
/*
 *	The default values
 */
RadioInterface	*MyRadioInterface;
QString iniFile		= QDir::homePath ();
QString presetFile	= QDir::homePath ();

	QCoreApplication::setOrganizationName ("Lazy Chair Computing");
        QCoreApplication::setOrganizationDomain ("Lazy Chair Computing");
        QCoreApplication::setApplicationName ("ft8-module");
        QCoreApplication::setApplicationVersion (QString (CURRENT_VERSION) + " Git: " + GITHASH);

        iniFile. append ("/");
        iniFile. append (DEFAULT_INI);
        iniFile = QDir::toNativeSeparators (iniFile);

	presetFile. append ("/");
	presetFile. append (FT8_PRESETS);
	presetFile = QDir::toNativeSeparators (presetFile);
        while ((opt = getopt (argc, argv, "i:B:")) != -1) {
           switch (opt) {
              case 'i':
                 iniFile	= fullPathfor (QString (optarg), ".ini");
                 break;

              case 'B':
                 break;

             default:
                 break;
           }
        }

	QApplication a (argc, argv);

        QFile file (":res/globstyle.qss");
        if (file .open (QFile::ReadOnly | QFile::Text)) {
           a. setStyleSheet (file.readAll ());
           file.close ();
        }

	QSettings ISettings (iniFile, QSettings::IniFormat);
/*
 *	Before we connect control to the gui, we have to
 *	instantiate
 */
        MyRadioInterface = new RadioInterface (&ISettings, presetFile);
	MyRadioInterface -> show ();
        a. exec ();
/*
 *	done:
 */
	fflush (stdout);
	fflush (stderr);
	qDebug ("It is done\n");
	ISettings. sync ();
//	fprintf (stderr, "we gaan deleten\n");
	delete MyRadioInterface;
	return 1;
}
//
