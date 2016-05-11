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

#include "JUGL.h"

#define signum(a) ((a)>0?1:(a)<0?-1:0)

#if defined(JUGL_TEXT_PRINTING_SUPPORT) && defined(JUGL_DEFAULT_FONTS)
JUGL::_DefaultFont DefaultFont;
#endif

namespace JUGL {

	// a special subset of the line drawing algorithm that runs more efficiently. Len can be negative
	void IScreen::DrawHorizontal(Point p, int16_t len)
	{
		int16_t startX = p.X;
		int16_t endX = p.X + len;

		// ensure the loop runs in the right direction by swapping points
		if(startX > endX) {
			int16_t tmp = startX;
			startX = endX;
			endX = tmp;
		}
		
		// bounds checking
		if(startX > _w || p.Y > _h || p.Y < 0 || endX < 0)
			return;

		if(endX > _w) endX = _w;
		if(startX < 0) startX = 0;

		for(int16_t x = startX; x <= endX; x++)
			DrawPoint(Point(x, p.Y));
	}

	// Written almost semantically verbetim to 
	// http://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
	//see http://en.wikipedia.org/wiki/Xiaolin_Wu%27s_line_algorithm for AA
	void IScreen::DrawLine(Point p1, Point p2)
	{
		int16_t x0 = p1.X;
		int16_t y0 = p1.Y;
		int16_t x1 = p2.X;
		int16_t y1 = p2.Y;

		int16_t dx = abs(x1-x0);
		int16_t dy = abs(y1-y0);
		int16_t sx = (x0 < x1)?1:-1;
		int16_t sy = (y0 < y1)?1:-1;
		int16_t err = dx - dy;
		int16_t err2;

		while(true)
		{
			DrawPoint(Point(x0, y0));
			if(x0 == x1 && y0 == y1) return;
			err2 = 2*err; // should be optimized into a bit shift

			if(err2 > -dy) {
				err -= dy;
				x0 += sx;
			}

			if(x0 == x1 && y0 == y1) {
				DrawPoint(Point(x0, y0));
				return;
			}

			if(err2 < dx) {
				err += dx;
				y0 += sy;
			}
		}
	}

	void IScreen::DrawRectangle(Point TLCorner, uint16_t w, uint16_t h)
	{
		uint16_t xOffset = TLCorner.X + w;
		uint16_t yOffset = TLCorner.Y + h;

		DrawLine(TLCorner, Point(TLCorner.X, yOffset));
		DrawHorizontal(TLCorner, w);
		DrawHorizontal(Point(TLCorner.X, yOffset), w);
		DrawLine(Point(xOffset, TLCorner.Y), Point(xOffset, yOffset));
	}
	void IScreen::DrawPolygon(Point* points, uint8_t size) 
	{
		// connect each point with a line
		for(int i = 0; i < size - 1; i++)
			DrawLine(points[i], points[i + 1]);

		// connect the last point with the first
		DrawLine(points[0], points[size - 1]);

	}

#ifdef JUGL_TEXT_PRINTING_SUPPORT
	size_t IScreen::write(uint8_t val)
	{
		// get the char
		char buff[64]; // a buffer to store

		_font->SelectChar(val);
		_font->GetBytes(buff, 64); // revisist this

		int width = _font->GetWidth(val);
		int height = _font->GetHeight(val);

		// Text Wrap
		if(_cursor.X + width >= _w)
		{
			_cursor.Y += height + 1;
			_cursor.X = 1;
		}

		// This method prints pixel by pixel; NOT OPTIMAL
		for (int8_t i=0; i < width; i++ ) {
			uint8_t line;
			line = *(buff + i);
			for (int8_t j = 0; j< height; j++) {
				if (line & 0x1) {
					DrawPoint(Point(_cursor.X+i, _cursor.Y+j));
				}
				line >>= 1;
			}
		}

		// move cursor over by the width of the character and space
		_cursor.X = (_cursor.X + _font->GetWidth((char)val) + 1) % _w;

		return 1;		
	}
#endif

#ifdef JUGL_CIRCLE_SUPPORT
	// derived from the C implementation of Bresenham's circle algorithm from http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
	void IScreen::DrawCircle(Point p, uint16_t radius)
	{
		int16_t x0 = p.X;
		int16_t y0 = p.Y;

		int16_t error = 1 - radius;
		int16_t errorY = 1;
		int16_t errorX = -2 * radius;
		int16_t x = radius, y = 0;

		DrawPoint(Point(x0, y0 + radius));
		DrawPoint(Point(x0, y0 - radius));
		DrawPoint(Point(x0 + radius, y0));
		DrawPoint(Point(x0 - radius, y0));

		while(y < x)
		{
			if(error > 0)
			{
				// the rarer change
				x--;
				errorX += 2;
				error += errorX;
			}
			y++;
			errorY += 2;
			error += errorY;    

			// after the change in x and y is made, the change is mirrored across the octants
			DrawPoint(Point(x0 + x, y0 + y));
			DrawPoint(Point(x0 - x, y0 + y));
			DrawPoint(Point(x0 + x, y0 - y));
			DrawPoint(Point(x0 - x, y0 - y));
			DrawPoint(Point(x0 + y, y0 + x));
			DrawPoint(Point(x0 - y, y0 + x));
			DrawPoint(Point(x0 + y, y0 - x));
			DrawPoint(Point(x0 - y, y0 - x));
		}
	}

	// a modified version of circle drawing algorigm above
	void IScreen::FillCircle(Point p, uint16_t radius)
	{
		int16_t x0 = p.X;
		int16_t y0 = p.Y;

		int16_t error = 1 - radius;
		int16_t errorY = 1;
		int16_t errorX = -2 * radius;
		int16_t x = radius, y = 0;

		Point origin(x0, y0);

		// a horizontal line across the circle's diameter
		DrawHorizontal(Point(x0 - radius, y0), radius << 1);
		while(y < x)
		{
			if(error > 0)
			{
				x--;
				errorX += 2;
				error += errorX;

				// Saves the algorithm from redundant line drawing by skipping the draw 
				// step when the value of y does not change. Calls to draw point reduced 
				// ~14% (when drawing lines was implemented point by point)

				y++;
				errorY += 2;
				error += errorY; 

				DrawHorizontal(Point(x0 - y, y0 + x + 1), y << 1); // bottom quad
				DrawHorizontal(Point(x0 - y, y0 - x - 1), y << 1); // top quad
			} 
			else {
				y++;
				errorY += 2;
				error += errorY;
			}

			DrawHorizontal(Point(x0 - x, y0 + y), x << 1);
			DrawHorizontal(Point(x0 - x, y0 - y), x << 1);
		}
	}

#endif

#ifdef JUGL_TRIANGLE_SUPPORT

	// Draws lines to connect three points
	void IScreen::DrawTriangle(Point p1, Point p2, Point p3)
	{
		DrawLine(p1, p2);
		DrawLine(p2, p3);
		DrawLine(p1, p3);
	}

	// created from source code example at
	// http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html
	void IScreen::FillTriangle(Point p1, Point p2, Point p3)
	{
		// Need to sort points in assending order in the Y dimension.
		// Since there are only 3 elements, a brute force method is the simplest
		Point tmp;

		if(p1.Y > p2.Y) {
			tmp = p1;
			p1 = p2;
			p2 = tmp;
		}

		if(p1.Y > p3.Y) {
			tmp = p1;
			p1 = p3;
			p3 = tmp;
		}

		if (p2.Y > p3.Y) {
			tmp = p2;
			p2 = p3;
			p3 = tmp;
		}

		// The rasterization method assumes the triangle has a flat (horizontal) edge.
		// The arguments will have to be jumbled to ensure the triangle meets this rule
		if(p2.Y == p3.Y) {
			RasterizeTriangle(p1, p2, p3); // the triangle is flat on the bottom
		} 
		else if( p1.Y == p2.Y) {
			RasterizeTriangle(p3, p1, p2); // the triangle is flat on the top
		} 
		else {
			// a general case that finds a fourth point to split the triangle into two horizontal-edged parts
			// NOT OPTIMAL: floating point math
			tmp = Point((p1.X + ((float)(p2.Y - p1.Y) / (float)(p3.Y - p1.Y)) * (p3.X - p1.X)), p2.Y);
			RasterizeTriangle(p1, p2, tmp);
			RasterizeTriangle(p3, p2, tmp);
		}
	}

	void IScreen::RasterizeTriangle(Point p1, Point p2, Point p3)
	{
		Point p1Tmp = p1;

		bool changed1 = false;
		bool changed2 = false;

		int16_t dx1 = abs(p2.X - p1.X);
		int16_t dy1 = abs(p2.Y - p1.Y);

		int16_t dx2 = abs(p3.X - p1.X);
		int16_t dy2 = abs(p3.Y - p1.Y);

		int16_t signx1 = signum(p2.X - p1.X);
		int16_t signx2 = signum(p3.X - p1.X);
		int16_t signy1 = signum(p2.Y - p1.Y);
		int16_t signy2 = signum(p3.Y - p1.Y);


		if(dy1 > dx1)
		{
			int16_t tmp = dx1;
			dx1 = dy1;
			dy1 = tmp;
			changed1 = true;
		}

		if(dy2 > dx2)
		{
			int16_t tmp = dx2;
			dx2 = dy2;
			dy2 = tmp;
			changed2 = true;
		}

		int16_t e1 = 2*dy1 - dx1;
		int16_t e2 = 2*dy2 - dx2;

		for (int i = 0; i <= dx1; i++)
		{
			DrawHorizontal(p1, p1Tmp.X - p1.X);

			while (e1 >= 0) // looping until a change is generated in
			{
				if (changed1)
					p1.X += signx1;
				else
					p1.Y += signy1;
				e1 = e1 - 2 * dx1;
			}

			if (changed1)
				p1.Y += signy1;
			else
				p1.X += signx1;  

			e1 = e1 + 2 * dy1;

			while (p1Tmp.Y != p1.Y)
			{
				while (e2 >= 0)
				{
					if (changed2)
						p1Tmp.X += signx2;
					else
						p1Tmp.Y += signy2;
					e2 = e2 - 2 * dx2;
				}

				if (changed2)
					p1Tmp.Y += signy2;
				else
					p1Tmp.X += signx2;

				e2 = e2 + 2 * dy2;
			}
		}
	}
#endif

#ifdef SPRITE_SUPPORT
	void IScreen::DrawSprite(Point loc, Sprite& sprite)
	{
		Color tmp = _drawColor; // caching the color
		
		uint16_t width = sprite.GetWidth();
		uint16_t height = sprite.GetHeight();
		
		Point curPoint(0,0);
		for(uint16_t i = 0; i < height; i++)
			for(uint16_t j = 0; j < width; j++)
			{
				sprite.GetPixel(Point(j, i), _drawColor);
				SetColor(_drawColor);
				DrawPoint(Point(loc.X + j, loc.Y + i));
			}
				
		_drawColor = tmp; // returning color to its initial value
	}
#endif
	
	void IScreen::FillRectangle(Point TLCorner, uint16_t w, uint16_t h)
	{
		int xFar = TLCorner.Y + h;
		for(int row = TLCorner.Y; row < xFar; row++)
			DrawHorizontal(Point(TLCorner.X, row), w );
	}

	// some screens have a special command for this, this represents niave implementation
	void IScreen::Clear(Color c)
	{
		// caching the color
		Color tmp = GetColor();
		SetColor(c);

		// setting the display to white
		FillRectangle(Point(0, 0), _w, _h);

		// restoring the draw color
		SetColor(tmp);		
	}

IScreen::IScreen(int w, int h) : 
#ifdef JUGL_TEXT_PRINTING_SUPPORT	
	_cursor(0,0), _font(&DefaultFont), _tWrap(false), _drawColor(BLACK), _w(w), _h(h) {	}
#else
	_drawColor(WHITE), _w(w), _h(h) {	}
#endif // JUGL_TEXT_PRINTING_SUPPORT
}