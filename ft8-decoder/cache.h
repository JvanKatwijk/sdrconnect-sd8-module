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
//
//	to be included by the main program of the decoder

#include	<stdint.h>
#include	"constants.h"

class	Cache {
private:
	int	nrows;
	int	ncolums;
	Complex **data;

public:
       Cache (int16_t ncolums, int16_t nrows) {
int16_t i;

        this	-> ncolums	= ncolums;
	this	-> nrows	= nrows;
        data            = new Complex *[nrows];
        for (i = 0; i < nrows; i++)
           data [i] = new Complex [ncolums];
	fprintf (stderr, "new cache with %d rows and %d colums\n",
	                            nrows, ncolums);
}

        ~Cache () {
        for (int i = 0; i < nrows; i ++)
           delete[] data [i];

        delete[] data;
}

Complex	*cacheLine (int16_t n) {
        return data [n];
}

Complex	cacheElement (int16_t line, int16_t element) {
	return data [line] [element];
}
};

