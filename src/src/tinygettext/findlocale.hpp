/*
 findlocale-0.46.tar.gz from http://icculus.org/~aspirin/findlocale/

Copyright (C) 2004 Adam D. Moss (the "Author").  All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is fur-
nished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FIT-
NESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CON-
NECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the Author of the
Software shall not be used in advertising or otherwise to promote the sale,
use or other dealings in this Software without prior written authorization
from the Author.

*/

#ifndef __findlocale_h_
#define __findlocale_h_

typedef const char* FL_Lang;
typedef const char* FL_Country;
typedef const char* FL_Variant;

typedef struct {
  FL_Lang    lang;
  FL_Country country;
  FL_Variant variant;
} FL_Locale;

typedef enum {
  /* for some reason we failed to even guess: this should never happen */
  FL_FAILED        = 0,
  /* couldn't query locale -- returning a guess (almost always English) */
  FL_DEFAULT_GUESS = 1,
  /* the returned locale type was found by successfully asking the system */
  FL_CONFIDENT     = 2
} FL_Success;

typedef enum {
  FL_MESSAGES = 0
} FL_Domain;

/* This allocates/fills in a FL_Locale structure with pointers to
   strings (which should be treated as static), or NULL for inappropriate /
   undetected fields. */
FL_Success FL_FindLocale(FL_Locale **locale, FL_Domain domain);
/* This should be used to free the struct written by FL_FindLocale */
void FL_FreeLocale(FL_Locale **locale);

#endif /*__findlocale_h_*/
