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

#ifndef STRUCTS_H
#define STRUCTS_H

/**
* A structure that stores an X and Y component.
*/	
struct Point {
	// default ctor
	Point () : X(0), Y(0) { }

	//copy ctor
	Point (const Point& p) : X(p.X), Y(p.Y) { }

	// initializer
	Point (int16_t x, int16_t y) : X(x), Y(y) { }

	int16_t X;
	int16_t Y;
};

/**
*	Size is semantically identical to Point, but the name is rebranded to be more meaningful in its own context
*/
typedef Point Size;

/**
* A structure that stores the coordinates and size of a rectangle
*/
struct Rectangle {
	Rectangle(Point TLCorner, uint16_t width, uint16_t height)
		: TLCorner(TLCorner), Width(Width), Height(Height){}

	Rectangle(Point TLCorner, Point BRCorner)
		: TLCorner(TLCorner){
		Width = BRCorner.X - TLCorner.X;
		Height = BRCorner.X - TLCorner.Y;
	}
	Point TLCorner;
	int16_t Width;
	int16_t Height;
};

#endif // STRUCTS_H