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

#ifndef KS0107_DRIVER_H
#define KS0107_DRIVER_H

#include "JUGL.h"
#include "headers/glcd.h"

/*
 * This implementation wraps a stripped down version of glcd v3
 * Most of these methods should be inlined so they will be defined in the header
 */

namespace JUGL
{
class KS0107_Driver : public IScreen {

public:
	KS0107_Driver()
		: IScreen(256, 256), _glcd_inst()
	{
		_w = _glcd_inst.Width;
		_h = _glcd_inst.Height;
	};

	inline void Begin()
	{
		_glcd_inst.Init();
		_color = GLCD_BLACK;
	}

	inline void SetColor(Color c)
	{
		SetColor((DefinedColor)c.Value);
	}

	inline void SetColor(DefinedColor c)
	{
		if(c == BLACK)
			_color = GLCD_WHITE;
		else
			_color = GLCD_BLACK;

	}

	inline Color GetColor()
	{
		if(_color == GLCD_BLACK)
			return Color(WHITE);

		else
			return Color(BLACK);
	}

	inline void DrawPoint(Point p)
	{
		_glcd_inst.SetDot(p.X, p.Y, _color);
	}

	void DrawHorizontal(Point p, int16_t len)
	{
		// apperently GLCD does not do bounds checking		
		if(len < 0)
		{
			p.X += len;
			len = -len;
		}
		
		if(p.X < 0) 
		{
			len += p.X;
			p.X = 0;
		}
		
		if(p.X > _w || p.Y >= _h || p.Y < 0 || len < 0)
			return;
		
		_glcd_inst.DrawHLine(p.X, p.Y, len, _color);
	}

	inline void FillRectangle(Point TLCorner, uint16_t w, uint16_t h)
	{	
		_glcd_inst.SetPixels(TLCorner.X, TLCorner.Y, TLCorner.X + w, TLCorner.Y + h, _color);
	}

	inline void Clear(Color c = BLACK)
	{
		uint8_t color = (c.Value == BLACK)?GLCD_BLACK:GLCD_WHITE;
			
		_glcd_inst.ClearScreen();
	}

	~KS0107_Driver() {};

private:
	int _color;
	glcd _glcd_inst;
};
}

#endif //KS0107_DRIVER_H
