#
/*
 *    Copyright (C)  2025
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
#include	"bandplan.h"

	bandPlan::bandPlan (const QString &fileName) {
	labelTable. resize (0);
	loadPlan (fileName);
}

	bandPlan::~bandPlan () {
}

const
QString	bandPlan::getFrequencyLabel (uint32_t Frequency) {
int	freq	= Frequency / 1000;

	for (uint16_t i = 0; i < labelTable. size (); i ++)
	   if ((labelTable. at (i). low <= freq) &&
	       (freq <= labelTable. at (i). high))
	      return QString (labelTable. at (i). label);

	return QString ("");
}


bool	bandPlan::loadPlan (const QString  &fileName) {
QDomDocument xmlBOM;
QFile f (fileName);

	this	-> fileName = fileName;
	if (!f. open (QIODevice::ReadOnly))  
	   return false;

	xmlBOM. setContent (&f);
	f. close ();
	QDomElement root	= xmlBOM. documentElement ();
	QDomElement component	= root. firstChild (). toElement ();
	while (!component. isNull ()) {
	   if (component. tagName () == "Band") {
	      bandElement bd;
	      bd. low		= component. attribute ("Low", "0"). toInt ();
	      bd. high		= component. attribute ("High", "0"). toInt ();
	      bd. label		= component. attribute ("Label", "");
	      labelTable. push_back (bd);
	   }
	   component = component. nextSibling (). toElement ();
	}
	return true;
}
