#include <JUGL.h>
#include <SPI.h>
#include <Wire.h>
#include <JUGL_SSD1306_128x32.h>

using namespace JUGL;

int width;
int height;
Point location;

// This is the interface screen interface
IScreen* scr;

void setup()
{
  Serial.begin(115200);

  // Creating an instance of the screen driver in dynamic memory.
  // It is normally better practice to statically assign
  scr = new SSD1306_128x32();

  // starting the screen
  scr->Begin();
  
  
  width = scr->GetWidth();
  height = scr->GetHeight();
  
  scr->Clear(WHITE);
  scr->SetColor(BLACK);
  scr->SetCursor(Point(4, 1));
  scr->print("CRAZY ANT DEMO");
  
  scr->Flush();
  delay(2000);
}

void loop()
{  
  // get an new point and make sure it is within bounds
  Point newLoc(location.X + random(0, 3) - 1, location.Y + (random(0, 3) - 1));
  if(newLoc.X >= width || newLoc.X < 0 || newLoc.Y >= height || newLoc.Y < 0)
    return;
    
  // erase the current point
  scr->SetColor(BLACK);
  scr->DrawPoint(location);
    
  // assign the new point
  location = newLoc;
  
  // drawing it the the buffer
  scr->SetColor(WHITE);
  scr->DrawPoint(location);
  
  // flushing the buffer to the screen
  scr->Flush();
}
