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

#include "pcf8833.h"
#include "Arduino.h"
#include <avr/pgmspace.h>

#define DELAY_MS 0x200
#define nop()  __asm__ __volatile__("nop")

//===============================
//	SPI Commands
//===============================

#define MOSI 11
#define MISO 12
#define SCK 13
#define SSEL 9
#define BLIGHT 10
#define RESET 8

#define MOSI0 digitalWrite(MOSI,0)
#define MOSI1 digitalWrite(MOSI,1)

#define SCK0 digitalWrite(SCK,0)
#define SCK1 digitalWrite(SCK,1)

#define SSEL0 digitalWrite(SSEL,0)
#define SSEL1 digitalWrite(SSEL,1)

#define RESET0 digitalWrite(RESET,0)
#define RESET1 digitalWrite(RESET,1)

#define SPI_HW_OFF   SPCR = 0;
#define SPI_HW_ON    SPCR = _BV(MSTR) | _BV(SPE);
#define SPI_HW_WAIT  while (!(SPSR & _BV(SPIF)));

static const short _initPCF8833[] PROGMEM =
{
	0x11,	// SLEEPOUT
	0x21,
	0x13,

	0x01,				// SWRESET
	DELAY_MS,10,
	0x11,				// SLEEPOUT
	DELAY_MS,10,

	0x36,0x100,			// MADCTL memory access control MY MX V LAO RGB X X X

	0x25,0x13f,			// SETCON set contrast 13f
	DELAY_MS,10,

	0x29,				// DISPON display on

	0xBA, 0x107, 0x115,	// Data Order
	0x25, 0x13f,		// Contrast // 0x13f
	0x11,				// Sleep Out
	0x13,				// Display Normal mode

	0x37,0x100,			// VSCROLL ADDR
	0x3A,0x105,			// COLMOD pixel format 4=12,5=16,6=18
};


void SpiCmd(u8 d)
{
	MOSI0;
	SCK1;
	SCK0;
	SPI_HW_ON;
	SPDR = d;
	SPI_HW_WAIT;
	SPI_HW_OFF;
}

void SpiData(u8 d)
{
	MOSI1;
	SCK1;
	SCK0;
	SPI_HW_ON;
	SPDR = d;
	SPI_HW_WAIT;
	SPI_HW_OFF;
}

void Cmd88(u8 cmd, u8 a, u8 b)
{
	SpiCmd(cmd);
	SpiData(a);
	SpiData(b);
}

//====================================
//	SPI wrapper Functions
//====================================

// previously called SetBoundsPCF8833
void SetBounds(int x, int y, int width, int height)
{
	Cmd88(0x2A,x,x+width);	// column start/end
	Cmd88(0x2B,y,y+height);	// page start/end
	SpiCmd(0x2C);				// RAMWR
}
//	Fill a 16 bit color
//	Blit 16 bit pixels
//	Blit 8 bit indexed pixels from palette
//	400,000 pixels/sec on a 16Mhz AVR for Solid fills and 16bpp blits
//	380,000 pixels/sec for indexed blits
//	The code looks crazy 'cause it is using shadow of the spi data clocks
//	do work for the next round. It is within 10% of a hardware 9 bit spi
void FillBlit(u32 color, u32 count, const u8* data = 0, const u8* palette = 0)
{
	PORTB |= (1<<3);			// MOSI = 1 = Data
	u8 sck0 = PORTB & ~(1<<5);	// SCK
	u8 sck1 = PORTB | (1<<5);	// out is 1 clock vs sbi at 2
	SPSR |= 1;	// 2x clock

	u8 a,b = 0;

	//	Handle 16 bit blits and index mode
	signed char x = 0;
	u8 y;
	const u8* p = 0;
	if (data)
	{
		a = *data++;
		x = 1;
		if (palette)
		{
			p = palette + ((*data++) << 1);
			a = p[0];
			b = p[1];
			x = -1;
			y = 0;
		}
	}
	else
	{
		a = color >> 8;
		b = color;
	}

	count <<= 1;
	int loop = count >> 8;
	u8 c = count;
	if (c)
		loop++;
	while (loop)
	{
		for (;;)
		{
			SPCR = 0;						// Turn off SPI HW
			PORTB = sck1;
			PORTB = sck0;					// Clock MOSI 1 bit to signify data on 9 bit spi
			SPCR = _BV(MSTR) | _BV(SPE);	// SPI Master
			SPDR = a;						// Now we have 16 clocks to kill

			//	16 clocks to do fill, blit or indexed blit

			//	Indexed blit - 380k
			if (x < 0)
			{
				if ((y^=1))
				{
					a = b;
					p = palette + ((*data++) << 1);
					if (--c)
						continue;	// Early out
				} else {
					a = p[0];
					b = p[1];
					if (--c)
						continue;	// Early out
				}
				break;
			}


			if (x)
			{
				//	Blit
				a = *data++;
				nop();
				nop();		// 400k
			}
			else
			{
				// Solid Fill
				u8 t = a;	// swap pixel halves
				a = b;
				b = t;
				nop();
				nop();		// 400k
			}
			if (!--c)
				break;	// 5 
		};				// loop takes 3 clocks
		c = 0;
		loop--;
		while (!(SPSR & _BV(SPIF)));	// wait for last pixel
	}
	SPCR = 0;
}

void SetBoundsPCF8833(int x, int y, int width, int height)
{
	Cmd88(0x2A,x+1,x+width);	// column start/end
	Cmd88(0x2B,y+1,y+height);	// page start/end
	SpiCmd(0x2C);			// RAMWR
}

//====================================
//	JUGL Implementation
//====================================

namespace JUGL {

	PCF8833_Shield::PCF8833_Shield()
		: IScreen(130, 130)//, _drawColorImpl(WHITE)
	{}

	void PCF8833_Shield::Begin()
	{
		SSEL1;
		RESET1;
		MOSI1;
		SCK0;

		pinMode(MOSI, OUTPUT);
		pinMode(SCK, OUTPUT);
		pinMode(SSEL, OUTPUT);
		pinMode(RESET, OUTPUT);
		pinMode(BLIGHT, OUTPUT);

		delay(120);						// Too long?

		SSEL0;
		SpiCmd(0x11);					// SLEEPOUT
		delay(120);						// Too long?
		SSEL1;

		RESET0;
		delay(1);
		RESET1;
		delay(100);	// too short?

		SCK0;
		SSEL0;

		const short* data = _initPCF8833; 
		int count = sizeof(_initPCF8833)/2;

		while (count--)
		{
			int b = pgm_read_word(data++);
			if (b == DELAY_MS)
			{
				b = pgm_read_word(data++);
				delay(b);
				count--;
			}
			else
			{
				if (b > 0xFF)
					SpiData(b);
				else
					SpiCmd(b);
			}
		}

		SSEL0;
	}


	void PCF8833_Shield::SetColor(Color c) 
	{ 
		// for this screen, color is implemented as a 16 bit value; 0xRGBB
		_drawColorImpl = (c.Red & 0xF0) << 8 | (c.Green & 0xF0) << 4 | c.Blue;
	}

	Color PCF8833_Shield::GetColor() 
	{ 
		Color c;
		c.Red = (_drawColorImpl >> 8) & 0xF0;
		c.Green = (_drawColorImpl >> 4) & 0xF0;
		c.Blue = _drawColorImpl & 0xFF;
		
		return c;
	}


	void PCF8833_Shield::SetColor(DefinedColor c) 
	{ 
		SetColor(Color(c));
	}

	void PCF8833_Shield::DrawPoint(Point p)
	{
		if(p.X >= _w || p.Y >= _h || p.X < 0 || p.Y < 0)
			return;

		SetBoundsPCF8833(p.X, p.Y, p.X, p.Y);
		FillBlit(_drawColorImpl, 1);
	}

	void PCF8833_Shield::DrawHorizontal(Point p, int16_t len)
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
		if(startX >= _w || p.Y >= _h || endX < 0)
			return;

		if(endX >= _w) endX = _w - 1;
		if(startX < 0) startX = 0;

		// printing the line
		SetBoundsPCF8833(startX, p.Y, len, p.Y);
		FillBlit(_drawColorImpl, endX - startX);
	}

	void PCF8833_Shield::FillRectangle(Point TLCorner, uint16_t w, uint16_t h)
	{
		Point BRCorner(TLCorner.X + w, TLCorner.Y + h);

		// bounds checking
		if (TLCorner.X >= _w || TLCorner.Y >= _h || BRCorner.X < 0 || BRCorner.Y < 0)
			return;

		SetBoundsPCF8833(TLCorner.X, TLCorner.Y, w, h);
		FillBlit(_drawColorImpl, w*h);
	}

	PCF8833_Shield::~PCF8833_Shield()
	{
		SSEL1; // release the LCD
	}
}