#include <JUGL.h>
#include <SPI.h>
#include <DataFlash.h>
#include <EPaperDriver.h>

#include <string.h>

using namespace JUGL;

int width;
int height;

// This is the interface screen interface
IScreen* scr;

void setup()
{
	Serial.begin(9600);

	// initialiing and starting the screen
        scr = new EPaperDriver(EPD_2_7);
        scr->Begin();	

	// setting up some global variables
	width = scr->GetWidth();
	height = scr->GetHeight();
}

void loop()
{
	Point focus(width/2, height/2);
	
        Serial.println(focus.X, DEC);
	for( int i = 2; i < width/2; i+=2) {
		//long aproxArea = PI*i*i;		
		long time = millis();
                Serial.println(aproxArea);
		
		scr->FillCircle(focus, i);
		scr->Flush();

		time = millis() - time;
		
		char strBuff[128];
		sprintf(strBuff, "For a circle with %i px radius:\n\tTime: %i\n\n", i, time);		
	        Serial.write(strBuff);
        }
        
	Serial.println("Test complete");
}
