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

Originally based on an adaptation of Adafruit's SSD1306 library
*/

#include "JUGL_SSD1306_128x32.h"

#include <SPI.h>
#include <avr/pgmspace.h>
#ifndef __SAM3X8E__
#include <util/delay.h>
#endif
#include <stdlib.h>
#include <Wire.h>

#define SSD1306_SETCONTRAST 0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF
#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA
#define SSD1306_SETVCOMDETECT 0xDB
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9
#define SSD1306_SETMULTIPLEX 0xA8
#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10
#define SSD1306_SETSTARTLINE 0x40
#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COLUMNADDR 0x21
#define SSD1306_PAGEADDR   0x22
#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8
#define SSD1306_SEGREMAP 0xA0
#define SSD1306_CHARGEPUMP 0x8D
#define SSD1306_EXTERNALVCC 0x1
#define SSD1306_SWITCHCAPVCC 0x2

#define WIDTH 128
#define HEIGHT 32

//128x32 takes 512 bytes
static uint8_t buffer[HEIGHT * WIDTH / 8];

// Library functions
namespace JUGL
{

void SSD1306_128x32::SetColor(DefinedColor c)
{
	if(c == BLACK)
		_drawColor = BLACK;
	else
		_drawColor = WHITE;
}

void SSD1306_128x32::ssd1306_command(uint8_t c)
{
	if (sid != -1)
	{
		// SPI
		//digitalWrite(cs, HIGH);
		*csport |= cspinmask;
		//digitalWrite(dc, LOW);
		*dcport &= ~dcpinmask;
		//digitalWrite(cs, LOW);
		*csport &= ~cspinmask;
		fastSPIwrite(c);
		//digitalWrite(cs, HIGH);
		*csport |= cspinmask;
	}
	else
	{
		// I2C
		uint8_t control = 0x00;   // Co = 0, D/C = 0
		Wire.beginTransmission(_i2caddr);
		Wire.write(control);
		Wire.write(c);
		Wire.endTransmission();
	}
}

SSD1306_128x32::SSD1306_128x32(int8_t SID, int8_t SCLK, int8_t DC, int8_t CS, int8_t RST) : IScreen(WIDTH, HEIGHT),  _i2caddr(0x3C), _vccstate(0x2)
{
	cs = CS;
	rst = RST;
	dc = DC;
	sclk = SCLK;
	sid = SID;
	hwSPI = false;
	_drawColor = WHITE;
}

void SSD1306_128x32::Begin()
{
	// set pin directions
	if (sid != -1)
	{
		// SPI
		pinMode(sid, OUTPUT);
		pinMode(sclk, OUTPUT);
		pinMode(dc, OUTPUT);
		pinMode(cs, OUTPUT);
		clkport     = portOutputRegister(digitalPinToPort(sclk));
		clkpinmask  = digitalPinToBitMask(sclk);
		mosiport    = portOutputRegister(digitalPinToPort(sid));
		mosipinmask = digitalPinToBitMask(sid);
		csport      = portOutputRegister(digitalPinToPort(cs));
		cspinmask   = digitalPinToBitMask(cs);
		dcport      = portOutputRegister(digitalPinToPort(dc));
		dcpinmask   = digitalPinToBitMask(dc);
	}
	else
	{
		// I2C Init
		Wire.begin(); // Is this the right place for this?
	}

	// Setup reset pin direction (used by both SPI and I2C)
	pinMode(rst, OUTPUT);
	digitalWrite(rst, HIGH);
	// VDD (3.3V) goes high at start, lets just chill for a ms
	delay(1);
	// bring reset low
	digitalWrite(rst, LOW);
	// wait 10ms
	delay(10);
	// bring out of reset
	digitalWrite(rst, HIGH);
	// turn on VCC (9V?)

	// Init sequence for 128x32 OLED module
	ssd1306_command(SSD1306_DISPLAYOFF);                    // 0xAE
	ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV);            // 0xD5
	ssd1306_command(0x80);                                  // the suggested ratio 0x80
	ssd1306_command(SSD1306_SETMULTIPLEX);                  // 0xA8
	ssd1306_command(0x1F);
	ssd1306_command(SSD1306_SETDISPLAYOFFSET);              // 0xD3
	ssd1306_command(0x0);                                   // no offset
	ssd1306_command(SSD1306_SETSTARTLINE | 0x0);            // line #0
	ssd1306_command(SSD1306_CHARGEPUMP);                    // 0x8D
	if (_vccstate == SSD1306_EXTERNALVCC)
		ssd1306_command(0x10);
	else
		ssd1306_command(0x14);
	ssd1306_command(SSD1306_MEMORYMODE);                    // 0x20
	ssd1306_command(0x00);                                  // 0x0 act like ks0108
	ssd1306_command(SSD1306_SEGREMAP | 0x1);
	ssd1306_command(SSD1306_COMSCANDEC);
	ssd1306_command(SSD1306_SETCOMPINS);                    // 0xDA
	ssd1306_command(0x02);
	ssd1306_command(SSD1306_SETCONTRAST);                   // 0x81
	ssd1306_command(0x8F);
	ssd1306_command(SSD1306_SETPRECHARGE);                  // 0xd9
	if (_vccstate == SSD1306_EXTERNALVCC)
		ssd1306_command(0x22);
	else
		ssd1306_command(0xF1);
	ssd1306_command(SSD1306_SETVCOMDETECT);                 // 0xDB
	ssd1306_command(0x40);
	ssd1306_command(SSD1306_DISPLAYALLON_RESUME);           // 0xA4
	ssd1306_command(SSD1306_NORMALDISPLAY);                 // 0xA6

	ssd1306_command(SSD1306_DISPLAYON);

	// clearing the internal frame buffer
	memset(buffer, 0, sizeof(buffer));
}

void SSD1306_128x32::DrawPoint(Point p)
{
	int x = p.X;
	int y = p.Y;

	if ((x < 0) || (x >= WIDTH) || (y < 0) || (y >= HEIGHT))
		return;

	if (_drawColor.Value == WHITE)
		buffer[x+ (y/8)*WIDTH] |= _BV((y%8));
	else
		buffer[x+ (y/8)*WIDTH] &= ~_BV((y%8));
}

void SSD1306_128x32::Flush()
{
	ssd1306_command(SSD1306_SETLOWCOLUMN | 0x0);  // low col = 0
	ssd1306_command(SSD1306_SETHIGHCOLUMN | 0x0);  // hi col = 0
	ssd1306_command(SSD1306_SETSTARTLINE | 0x0); // line #0

	// save I2C bitrate
	uint8_t twbrbackup = TWBR;
	TWBR = 12; // upgrade to 400KHz!

	// I2C
	for (uint16_t i=0; i<(WIDTH*HEIGHT/8); i++)
	{
		// send a bunch of data in one xmission
		Wire.beginTransmission(_i2caddr);
		Wire.write(0x40);
		for (uint8_t x=0; x<16; x++)
		{
			Wire.write(buffer[i]);
			i++;
		}
		i--;
		Wire.endTransmission();
	}
	// i wonder why we have to do this (check datasheet)
	if (HEIGHT == 32)
	{
		for (uint16_t i=0; i<(WIDTH*HEIGHT/8); i++)
		{
			// send a bunch of data in one xmission
			Wire.beginTransmission(_i2caddr);
			Wire.write(0x40);
			for (uint8_t x=0; x<16; x++)
			{
				Wire.write((uint8_t)0x00);
				i++;
			}
			i--;
			Wire.endTransmission();
		}
	}
	TWBR = twbrbackup;
}
}