#include <JUGL.h>
#include <pcf8833.h>

using namespace JUGL;

#define WAIT 750

// Fills screen with red, green, then blue at regular intervals
void setup()
{
  // Creating an instance of the screen being used.
  PCF8833_Shield screenInst;
  
  // Assigning it to its parent (not necessary, just demonstrating 
  // showing that the drivers have an interface)
  IScreen* scr = &screenInst;
  scr->Begin();
  
  int width = scr->GetWidth();
  int height = scr->GetHeight();
  
  while(true)
  {
    scr->SetColor(RED);
    scr->FillRectangle(Point(0, 0), width, height);
    delay(WAIT);
    
    scr->SetColor(GREEN);
    scr->FillRectangle(Point(0, 0), width, height);
    delay(WAIT);
    
    scr->SetColor(BLUE);
    scr->FillRectangle(Point(0, 0), width, height);
    delay(WAIT);
  }
}

void loop() {}