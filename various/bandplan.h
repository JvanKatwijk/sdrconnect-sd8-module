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
#
#pragma once

#include	<stdint.h>
#include        <QString>
#include        <QtXml>
#include        <QFile>
#include        <vector>
#include        <QStringList>

typedef struct {
	int     low;
	int     high;
	QString	label;
} bandElement;

class	bandPlan {
public:
	bandPlan	(const QString &);
	~bandPlan	();
const
QString	getFrequencyLabel	(uint32_t);
private:
	QString		fileName;
	std::vector<bandElement> labelTable;
	bool	loadPlan	(const QString &);
};

