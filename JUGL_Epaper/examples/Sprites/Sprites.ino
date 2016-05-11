#include <JUGL.h>
#include <SPI.h>
#include <DataFlash.h>
#include <EPaperDriver.h>

using namespace JUGL;

EPaperDriver screen(EPD_2_7); // can initialize with EPD_1_44,  EPD_2_0 or EPD_2_7

// 8 bytes
byte Alien1[] = {
  B00011000,
  B00111100,
  B01111110,
  B11011011,
  B11111111,
  B01011010,
  B10000001,
  B01000010,
};

// 16 bytes
byte Alien2[] = {
  B00100000, B10000000,
  B00010001, B0,
  B00111111, B10000000,
  B01101110, B11000000,
  B11111111, B11100000,
  B10111111, B10100000,
  B10100000, B10100000,
  B00011011, B0
};

// 16 bytes
byte Alien3[] = {
  B00100000, B10000000,
  B10010001, B00100000,
  B10111111, B10100000,
  B11101110, B11100000,
  B01111111, B11000000,
  B00111111, B10000000,
  B00100000, B10000000,
  B01000000, B01000000
};

void setup()
{
  // these sprite come from space invaders
  Color drawColor(BLACK);
  BinarySprite bs(Size(8, 8), drawColor, Alien1);
  BinarySprite bs2(Size(11, 8), drawColor,  Alien2);
  BinarySprite bs3(Size(11, 8), drawColor, Alien3); 
  screen.Clear();
  
  // Writing text
  screen.SetCursor(Point(0,0));
  screen.print("SPACE INVADER SPRITES");
  
  // Pushing the sprites onto the screen
  screen.DrawSprite(Point(30, 30), bs);
  screen.DrawSprite(Point(10, 10), bs2);
  screen.DrawSprite(Point(50, 50), bs3);
  screen.Flush();  
}

void loop()
{

}