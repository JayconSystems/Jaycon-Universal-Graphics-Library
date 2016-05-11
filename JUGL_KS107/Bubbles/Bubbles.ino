#include <JUGL.h>
#include <KS0107_driver.h>

using namespace JUGL;

int width;
int height;

// This is the interface screen interface
IScreen* scr;

void setup()
{
	// Creating an instance of the screen driver in dynamic memory.
	// It is normally better practice to statically assign
	scr = new KS0107_Driver;

	// starting the screen
	scr->Begin();

	// setting up some global variables
	width = scr->GetWidth();
	height = scr->GetHeight();
}

void loop()
{
	// generating random numbers
	int randx = random(1, width);
	int randy = random(1, height);
	int randr = random(1, width/2);

	scr->FillCircle(Point(randx, randy), randr); // creating a random circle
	scr->SetColor((DefinedColor)random()); // picking a random color
}

