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

Support us at http://www.jayconsystems.com/
*/

/*
commenting out these preprocessors will strip features from the library core 
and improve code size
*/

// enable DrawTriangle and FillTriangle
#define JUGL_TRIANGLE_SUPPORT

// enable DrawCircle and FillCircle
#define JUGL_CIRCLE_SUPPORT

// Screen extends Print interface and supports character drawing functions
#define JUGL_TEXT_PRINTING_SUPPORT

// Stores a default font in program memory
#define JUGL_DEFAULT_FONTS

// Enables the Sprite object, SetSprite and RemoveSprite functions
#define SPRITE_SUPPORT