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

This source is an adaptation of a screen driver written by Peter Barrett
in 2010
*/

#ifndef PCF833_DRIVER_H
#define PCF833_DRIVER_H

#include "JUGL.h"

#ifndef u8
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
#endif

namespace JUGL
{
	/**
	* A driver implementation for Nokia PCF8833 shield.
	*/
	class PCF8833_Shield : public IScreen
	{
	public:
		PCF8833_Shield();
		void Begin();
		void SetColor(Color c);
		Color GetColor();
		void SetColor(DefinedColor c);
		void DrawPoint(Point p);
		void DrawHorizontal( Point p, int16_t len);
		void FillRectangle(Point TLCorner, uint16_t w, uint16_t h);

		~PCF8833_Shield();
		uint16_t _drawColorImpl;

	private:
	};
}
#endif // PCF833_DRIVER_H