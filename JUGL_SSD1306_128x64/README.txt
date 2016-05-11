This is an open source driver supported by Jaycon Systems

Supported Screens:
	SSD1306 0.96" (128x32) OLED http://www.jayconsystems.com/0-96-inch-128x64-oled-display-white.html

Supported Architectures:
	AVR
	
Default Constructor:
	SSD1306_128x32(
	int8_t SID = -1, 
	int8_t SCLK = -1, 
	int8_t DC = -1, 
	int8_t CS = -1, 
	int8_t RST = 4);
	
Additional Details:
	Uses initialization and flush functions from Adafruit's SSD1306 library.
	Note that several of the pins are initialized to invalid pins. The When those are used, driver detects that and uses Arduino's
	built-in Wire library. See here for wiring specific avr devices http://arduino.cc/en/reference/wire. It is recommended not to
	stray from this since the other method involved direct port manipulation and is not tested.