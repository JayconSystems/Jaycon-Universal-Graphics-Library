#include <SPI.h>
#include <DataFlash.h>
#include <JUGL.h>
#include <EPaperDriver.h>

using namespace JUGL;

void setup()
{
  EPaperDriver screen(EPD_2_7); // initialize with EDP_1_44, EPD_2_0 or EPD_2_7

  screen.Clear();  
  screen.DrawRectangle(Point(0, 0), screen.GetWidth() - 1, screen.GetHeight() - 1);  
  screen.Flush();  
  
  delay(1000);
  
  screen.FillRectangle(Point(2, 2), screen.GetWidth() - 5, screen.GetHeight() - 4);
  screen.Flush();
  
  delay(500);
  
  screen.SetColor(WHITE);
  screen.SetCursor(Point(5, 5));
  screen.print("This is a rastered string");
  screen.Flush();
}

void loop()
{
  
}