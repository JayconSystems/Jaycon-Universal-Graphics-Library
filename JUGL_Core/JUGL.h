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

#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>
#include <Arduino.h>
#include "headers/config.h"
#include "headers/colors.h"
#include "headers/structs.h"
#include "headers/font.h"
#include "headers/progmemfix.h"
#include "headers/sprite.h"

namespace JUGL
{

/**
* An abstract interface all JUGL drivers derive from.
*/
#ifdef JUGL_TEXT_PRINTING_SUPPORT
class IScreen : public Print
#else
class IScreen
#endif
{
public:
	virtual void Begin() = 0;

	// Rendering functions
	virtual void DrawPoint(Point p) = 0;
	virtual void DrawLine(Point p1, Point p2);
	virtual void DrawHorizontal( Point p1, int16_t len);
	virtual void DrawPolygon(Point* points, uint8_t size);
	virtual void DrawRectangle(Point TLCorner, uint16_t w, uint16_t h);
	virtual void FillRectangle(Point TLCorner, uint16_t w, uint16_t h);
	virtual void End() {}
	virtual void Clear(Color c = BLACK);
	virtual void Flush() {} // necessary for Epaper and other devices that use buffers

#ifdef JUGL_CIRCLE_SUPPORT
	virtual void FillCircle(Point p, uint16_t radius);
	virtual void DrawCircle(Point p, uint16_t radius);
#endif

#ifdef JUGL_TRIANGLE_SUPPORT
	virtual void DrawTriangle(Point p1, Point p2, Point p3);
	virtual void FillTriangle(Point p1, Point p2, Point p3);
#endif

	// Printing Text
#ifdef JUGL_TEXT_PRINTING_SUPPORT
	virtual size_t write(uint8_t);
	inline void SetCursor(Point p)
	{
		_cursor = p;
	}
	inline void SetFont(Font* f)
	{
		_font = f;
	}
	inline void SetTextWrap(bool tWrap)
	{
		_tWrap = tWrap;
	}
#endif

#ifdef SPRITE_SUPPORT
	virtual void DrawSprite(Point loc, Sprite& sprite);
#endif

	virtual void SetColor(Color c)
	{
		_drawColor = c;
	}
	virtual Color GetColor()
	{
		return _drawColor;
	}
	virtual void SetColor(DefinedColor c)
	{
		_drawColor = Color(c);
	}

	// Screen_ properties
	inline int GetHeight()
	{
		return _h;
	}
	inline int GetWidth()
	{
		return _w;
	}

	// Constructor
	IScreen(int w, int h);

protected:
#ifdef JUGL_TEXT_PRINTING_SUPPORT
	Point _cursor;
	Font* _font;
	bool _tWrap;
#endif

	Color _drawColor;
	int _w;
	int _h;

#ifdef JUGL_TRIANGLE_SUPPORT
	void RasterizeTriangle(Point p1, Point p2, Point p3);
#endif
};
}

#endif //SCREEN_H
