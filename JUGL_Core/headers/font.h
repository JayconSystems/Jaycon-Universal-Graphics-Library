/* 
Copyright (c) 2014, Samuel Knight

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE. 
*/

#ifndef JUGL_TEXT_H
#define JUGL_TEXT_H

#include "config.h"
#include <avr/pgmspace.h>

namespace JUGL {
#ifdef JUGL_TEXT_PRINTING_SUPPORT
	/*
	A generic interface for fetching bitfields to raster fonts
	The caller selects the character to read then copies to a buffer 
	*/
	class Font {
	public:
		virtual void SelectChar(char c) = 0;
		virtual int GetBytes(char* buff, int max) = 0;
		virtual int BytesRemaining() = 0;
		virtual int GetWidth(char c) = 0;
		virtual int GetHeight(char c) = 0;
	};

#ifdef JUGL_DEFAULT_FONTS
	/*
	A default font that comes with the library
	This font stores 
	*/
	class _DefaultFont : public Font {
	public:
		_DefaultFont()
			: _currentChar(0), _selected(0), _remaining(0) {}
		void SelectChar(char c);
		int GetBytes(char* buff, int max);
		int BytesRemaining();
		int GetWidth(char c);
		int GetHeight(char c);
	private:
		unsigned char* _currentChar;
		char _selected;
		int _remaining;
	};

	//declaring a global instance
	//extern _DefaultFont DefaultFont;
#endif //JUGL_TEXT_PRINTING_SUPPORT
#endif //JUGL_DEFAULT_FONTS
}

#endif // JUGL_TEXT_H