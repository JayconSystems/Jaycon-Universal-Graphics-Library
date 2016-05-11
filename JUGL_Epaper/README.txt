This is an open source driver supported by Jaycon Systems

Supported Screens:
	http://www.jayconsystems.com/epaper-development-board-v1-1-44.html
	http://www.jayconsystems.com/epaper-development-board-v1-2-0.html
	http://www.jayconsystems.com/epaper-development-board-v1-2-7.html	

Supported Architectures:
	AVR

Default Constructor:
EPaperDevBoard_Driver(EPD_size size,
		  int panel_on_pin = 2,
		  int border_pin = 3,
		  int discharge_pin = 4,
		  int pwm_pin = 5,
		  int reset_pin = 6,
		  int busy_pin = 7,
		  int chip_select_pin = 8,
		  int temp_pin = A0,
		  int flash_cs_pin = 9,
		  int flash_reset = 10,
		  int flash_sw2 = 12);

Additional Details:		  
This driver supports dynamic changes to the screen with an external frame buffer.
This buffer is stored in the built in SRAM module on the located on the board