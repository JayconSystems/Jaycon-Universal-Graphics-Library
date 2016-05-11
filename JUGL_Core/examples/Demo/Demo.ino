#include <JUGL.h>

// uncomment one of the following drivers to use that screen

/*
#include <SPI.h>
#include <DataFlash.h>
#include <EPaperDriver.h>
using namespace JUGL;
EPaperDriver driver(EPD_2_0); // can also initialize with EPD_1_44, EPD_2_0 and EPD_2_7
Color drawColor(BLACK);
*/

/*
#include <SPI.h>
#include <Wire.h>
#include <JUGL_SSD1306_128x32.h>
using namespace JUGL;
SSD1306_128x32 driver;
Color drawColor(WHITE);
*/

/*
// Nokia
#include <pcf8833.h>
using namespace JUGL;
PCF8833_Shield driver;
Color drawColor(WHITE);
*/

///*
#include <KS0107_driver.h>
using namespace JUGL;
KS0107_Driver driver;
Color drawColor(WHITE);
//*/

// forward declaring the demo functions
void RectangleDemo(IScreen& scr);
void CircleDemo(IScreen& scr);
void TriangleDemo(IScreen& scr);
void LineDemo(IScreen& scr);
void TextDemo(IScreen& scr);
void SpriteDemo(IScreen& scr);

int width, height;

void setup()
{
  //Serial.begin(115200);
  // make a reference to the driver
  IScreen& screen = driver; 
  
  // starting up the screen
  screen.Begin(); 
  
  // caching width and height
  width = screen.GetWidth();
  height = screen.GetHeight();

  // running the demos
  RectangleDemo(screen);
  CircleDemo(screen);
  TriangleDemo(screen);
  LineDemo(screen);
  TextDemo(screen);
  SpriteDemo(screen);
}

void loop() { 
} // unused

void RectangleDemo(IScreen& scr)
{
  // Setting up for this demo
  scr.Clear();
  scr.SetColor(drawColor);

  // Draw a rectangle the inscribe with a filled rectangle
  scr.DrawRectangle(Point(0, 0), width - 1, height - 1);  
  scr.FillRectangle(Point(2, 2), width - 5, height - 4);

  scr.Flush(); // for displays with screen buffers
  delay(5000);
}

void CircleDemo(IScreen& scr)
{
  // Setting up for this demo
  scr.Clear();
  scr.SetColor(drawColor);

  // selecting the smaller of the two as radius
  int radius = ((width>height)?height:width)/2; 

  scr.DrawCircle(Point(width/2, height/2), radius - 1);
  scr.FillCircle(Point(width/2, height/2), radius - 4);

  scr.Flush(); // for displays with screen buffers 
  delay(5000);

}

void TriangleDemo(IScreen& scr)
{
  // Setting up for this demo
  scr.Clear();
  scr.SetColor(drawColor);

  scr.DrawTriangle(Point(0, 0), Point(0, height - 1), Point(width - 1, height/2));
  scr.FillTriangle(Point(2, 4), Point(2, height - 5), Point(width - 7, height/2 ));

  scr.Flush(); // for displays with screen buffers   
  delay(5000);
}

void LineDemo(IScreen& scr)
{
  // Setting up for this demo
  scr.Clear();
  scr.SetColor(drawColor);

  // draw a line that crosses to opposing sides of the screen
  scr.DrawLine(Point(0, 0), Point(width - 1, height - 1));
  scr.DrawLine(Point(width - 1, 0), Point(0, height - 1));

  scr.Flush(); // for displays with screen buffers  
  delay(5000);
}

void TextDemo(IScreen& scr)
{
  // The line should automatically wrap when the end of the screen is reached
  
  // Setting up for this demo
  scr.Clear();
  scr.SetColor(drawColor);

  scr.print("Jaycon Universal Graphics Library (JUGL) Text Demo");

  scr.Flush(); // for displays with screen buffers 
  delay(5000);
}

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

void SpriteDemo(IScreen& scr)
{
  scr.Clear();
  scr.SetColor(drawColor);

  // these sprite come from space invaders
  BinarySprite bs(Size(8, 8), drawColor, Alien1);
  BinarySprite bs2(Size(11, 8), drawColor,  Alien2);
  BinarySprite bs3(Size(11, 8), drawColor, Alien3); 

  scr.DrawSprite(Point(0, 0), bs);
  scr.DrawSprite(Point(15, 0), bs2);
  scr.DrawSprite(Point(30, 0), bs3);

  scr.Flush(); // for displays with screen buffers 
  delay(5000);
}

