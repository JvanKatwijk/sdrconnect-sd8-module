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
//
#include	<QDomDocument>
#include	<QFile>
#include	"ft8-constants.h"
#include	"preset-handler.h"
#include	"radio.h"

static

uint32_t defaultFreqs [] =
        {1840, 3753, 5357, 7056, 7071, 7074, 10136,
        14075, 18104, 21075, 24915, 28074, 0};
 
	presetHandler::presetHandler	(RadioInterface *radio,
	                                         QString fileName):
	                                             QListView (nullptr) {
	this	-> radio	= radio;
	this	-> fileName	= fileName;
	QDomDocument xmlBOM;
//
//	start with an empty list, waiting ...
	presetList. clear ();
	displayList. setStringList (presetList);
	this	-> setModel (&displayList);
	connect (this, &QListView::clicked,
	         this, &presetHandler::selectElement);
//	connect (this, &QListView::doubleClicked,
//	         this, &presetHandler::removeElement);
	connect (this, &presetHandler::handle_preset,
	         radio, &RadioInterface::handle_preset);
	QFile f (fileName);
	if (f. open (QIODevice::ReadOnly)) {
	   xmlBOM. setContent (&f);
	   f. close ();
	   QDomElement root	= xmlBOM. documentElement ();
	   QDomElement component	= root. firstChild (). toElement ();
	   while (!component. isNull ()) {
	      if (component. tagName () == "PRESET_ELEMENT") {
	         QString frequency =
	                component. attribute ("FREQUENCY", "???");
	         presetList. append (frequency);
	      }
	      component = component. nextSibling (). toElement ();
	   }
	}
	else	// no filename found,
	   for (int i = 0; defaultFreqs [i] != 0; i ++)
	      presetList. append (QString::number (defaultFreqs [i]));
	displayList. setStringList (presetList);
	this	-> setModel (&displayList);
}

	presetHandler::~presetHandler   () {
QDomDocument theScanList;
QDomElement root = theScanList. createElement ("preset_db");

	theScanList. appendChild (root);

	for (int i = 0; i < presetList. size (); i ++) {
           QString Freq = presetList. at (i);
	   QDomElement presetListElement = theScanList.
	                            createElement ("PRESET_ELEMENT");
	   presetListElement. setAttribute ("FREQUENCY", Freq);
	   root. appendChild (presetListElement);
	}

	QFile file (this -> fileName);
	if (!file. open (QIODevice::WriteOnly | QIODevice::Text))
	   return;

	QTextStream stream (&file);
	stream << theScanList. toString ();
	file. close ();
}

static
QStringList	sortList (QStringList &list) {
int 	*inpval  = dynVec (int, list. size ());
QStringList outList;
bool ok;
	for (int i = 0; i < list. size (); i ++) {
	   int val = list. at (i). toInt (&ok);
	   if (ok)
	      inpval [i] = val;
	   else
	      inpval [i] = 0;
	}

	std::sort (inpval, inpval + list. size ());
	for (int i = 0; i < list. size (); i ++)
	   outList << QString::number (inpval [i]);
	return outList;
}
	
void	presetHandler::addElement (int frequency) {
const QString presetListElement = QString::number (frequency / 1000);

	for (int i = 0; i < presetList. size (); i ++)
	   if (presetList. at (i) == presetListElement)
	      return;
	presetList. append (presetListElement);
	presetList = sortList (presetList);
	displayList. setStringList (presetList);
	this	-> setModel (&displayList);
}

void	presetHandler::clearScanList () {
	presetList. clear ();
	displayList. setStringList (presetList);
	this	-> setModel (&displayList);
}
//
//	selecting a preset
//
void	presetHandler::selectElement (QModelIndex ind) {
QString selectedElement = displayList. data (ind, Qt::DisplayRole). toString ();
	handle_preset (selectedElement);
}

void	presetHandler::removeElement (QModelIndex ind) {
QString selectedElement = displayList. data (ind, Qt::DisplayRole). toString ();
	for (int i = 0; i < presetList. size (); i ++) {
	   if (presetList. at (i) == selectedElement) {
	      presetList. erase (presetList. begin () + i);
	      displayList. setStringList (presetList);
	      this	-> setModel (&displayList);
	      return;
	   }
	}
}
