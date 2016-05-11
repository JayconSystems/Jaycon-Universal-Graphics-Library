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

Addapted from Adafruit's SSD1306 library
*/

#ifndef JUGL_SSD1206_96x16_H
#define JUGL_SSD1206_96x16_H

#include  <stdint.h>
#include <JUGL.h>
#include <SPI.h>

#ifdef __SAM3X8E__
typedef volatile RwReg PortReg;
typedef uint32_t PortMask;
#else
typedef volatile uint8_t PortReg;
typedef uint8_t PortMask;
#endif


namespace JUGL
{
class SSD1306_96x16 : public IScreen {

public:
	SSD1306_96x16(int8_t SID = -1, int8_t SCLK = -1, int8_t DC = -1, int8_t CS = -1, int8_t RST = 4);
	void Begin();
	void DrawPoint(Point p);
	//void DrawHorizontal(Point p, int16_t len);
	void SetColor(DefinedColor c);
	void Flush();

private:
	int8_t _i2caddr, _vccstate, sid, sclk, dc, rst, cs;
	boolean hwSPI;
	PortReg *mosiport, *clkport, *csport, *dcport;
	PortMask mosipinmask, clkpinmask, cspinmask, dcpinmask;
	
	void ssd1306_command(uint8_t c);
	
	inline void fastSPIwrite(uint8_t d)
	{
		if(hwSPI)
			(void)SPI.transfer(d);
		else
		{
			for(uint8_t bit = 0x80; bit; bit >>= 1)
			{
				*clkport &= ~clkpinmask;
				if(d & bit) *mosiport |=  mosipinmask;
				else        *mosiport &= ~mosipinmask;
				*clkport |=  clkpinmask;
			}
		}
		//*csport |= cspinmask;
	}
};
}

#endif //JUGL_SSD1206_96x16_H