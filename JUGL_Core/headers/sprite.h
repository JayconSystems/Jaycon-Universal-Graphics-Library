#ifndef SPRITE_H
#define SPRITE_H

#include <stdint.h>
#include <math.h>
#include "colors.h"
#include "structs.h"

namespace JUGL
{
/**
* An abstract object for representing a sprite
*/
class Sprite
{
public:

	/**
	* Constructs a new sprite object
	*/
	Sprite(Size s) : _width(s.X), _height(s.Y)
	{}
	
	/**
	* Gets the sprit's width
	*/
	int16_t GetWidth() { return _width; }
	
	/**
	* Gets the sprit's height
	*/
	int16_t GetHeight() { return _height; }

	/**
	* Gets the color at a particular point in the sprite
	* @param p the location to read
	* @param color A Color parameter passed by reference to avoid additional copy overhead
	* @return color will contain the color at the point
	*/
	virtual void GetPixel(Point p, Color& color) = 0;
	

protected:
	int16_t _width;
	int16_t _height;
};

/**
* An implementation of sprite that uses a binary buffer to generate a black and white sprite.
* The sprite carries a refernece to data. The data's integrity is assumed to be consistant for the life of the object
*/
class BinarySprite : public Sprite
{
public:

	/**
	* Constructs an new binary sprite
	* @param size The width and height of the new sprite
	* @param drawColor The color assigned to the high bits
	* @param data An array containing the binary data. Every row must start with a new byte; 
	*  which means resolutions that are not divisible by 8 are padded on the end of each row.
	*/
	BinarySprite(Size size, Color drawColor, uint8_t* data) : Sprite(size), _data(data) 
	{
		_scrWidth = (size.X >> 3) + ((size.X & 0x07) > 0);
		
		if(drawColor.Value == WHITE)
		{
			_highColor.Value = WHITE;
			_lowColor.Value = BLACK;
		}
		else
		{
			_highColor.Value = BLACK;
			_lowColor.Value = WHITE;
		}
	}

	virtual void GetPixel(Point p, Color& color)
	{
		// No out of bounds check. 
		// Reading an incorrect value results in no corruption except in the user's incorrect code.

		uint8_t unmaskedByte = _data[(p.Y * _scrWidth) + (p.X >> 3)];
		
		if((unmaskedByte & (128 >> (p.X & 7))))
			color = _highColor;
		else
			color = _lowColor;
	}

private:
	uint8_t* _data;
	uint16_t _scrWidth; // the size of a row in bytes instead of pixels
	Color _highColor;
	Color _lowColor;
};

/**
 * A sprite implementation that supports up to 24-bit color
 * Note that this implementation requires a lot of memory for a small microcontroller, Length*Width*4 bytes.
 * Since it carries a reference to the data, duplicate instances take a negligable amount of memory
 */
class ColorSprite : public Sprite
{
public:

	/**
	* Constructs an new binary sprite
	* @param size The width and height of the new sprite
	* @param colors An array containing the colors.
	*/
	ColorSprite(Size size, Color* colors) : Sprite(size), _colors(colors)
	{}

	virtual void GetPixel(Point p, Color& color)
	{
		// No out of bounds check. 
		// Reading an incorrect value results in no corruption except in the user's incorrect code.

		color.Value = _colors[(_width*p.Y) + p.X].Value;
	}

private:
	Color* _colors;
};

}

#endif // SPRITE_H