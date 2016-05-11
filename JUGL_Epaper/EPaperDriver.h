/*
Copyright (c) 2014, Samuel Knight

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef EPaperDriver_H
#define EPaperDriver_H

#include <SPI.h>
#include <DataFlash.h>
#include "JUGL.h"

namespace JUGL
{

typedef enum
{
	EPD_1_44,        // 128 x 96
	EPD_2_0,         // 200 x 96
	EPD_2_7          // 264 x 176
} EPD_size;

typedef enum             // Image pixel -> Display pixel
{
	EPD_compensate,  // B -> W, W -> B (Current Image)
	EPD_white,       // B -> N, W -> W (Current Image)
	EPD_inverse,     // B -> N, W -> B (New Image)
	EPD_normal       // B -> B, W -> W (New Image)
} EPD_stage;

class EPaperDriver : public IScreen
{
private:
	int EPD_Pin_EPD_CS;
	int EPD_Pin_PANEL_ON;
	int EPD_Pin_BORDER;
	int EPD_Pin_DISCHARGE;
	int EPD_Pin_PWM;
	int EPD_Pin_RESET;
	int EPD_Pin_BUSY;
	int EDP_Pin_flash_cs;
	int EDP_Pin_flash_reset;
	int EDP_Pin_flash_sw2;
	int EDP_Pin_TEMPERATURE;

	EPD_size size;
	uint16_t stage_time;
	uint16_t factored_stage_time;
	uint16_t lines_per_display;
	uint16_t dots_per_line;
	uint16_t bytes_per_line;
	uint16_t bytes_per_scan;
	PROGMEM const uint8_t *gate_source;
	uint16_t gate_source_length;
	PROGMEM const uint8_t *channel_select;
	uint16_t channel_select_length;
	
	bool filler;
	
	float get_temp();
	void flash_read(uint16_t index, uint16_t count, uint8_t* buff);
	void clearFrameBuff();
	
	// chip communication
	DataFlash dataflash; 
	uint16_t bufferedPage;
	
public:
	// power up and power down the EPD panel
	void Begin();
	void End();
	void DrawPoint(Point p);
	
	void Flush()
	{
		Begin(); // power up. Adds waiting time, but also ensures board can adjust to change in temp	
		
		// flush the cache
		dataflash.begin();
		dataflash.bufferToPage(1, bufferedPage);
		dataflash.waitUntilReady();
		dataflash.end();
		
		// modify the screen
		this->frame_fixed_repeat(0xaa, EPD_compensate);
		this->frame_fixed_repeat(0xaa, EPD_white);
		this->frame_data_repeat(EPD_inverse);
		this->frame_data_repeat(EPD_normal);
		
		End();
	}

	void setFactor(int temperature = 25) {
		this->factored_stage_time = this->stage_time * this->temperature_to_factor_10x(temperature) / 10;
	}

	// clear display (anything -> white)
	virtual void Clear(Color c = BLACK) {
	
		clearFrameBuff();
	
		Begin(); // power up. Adds waiting time, but also ensures board can adjust to tempurature	
		
		this->frame_fixed_repeat(0xff, EPD_compensate);
		this->frame_fixed_repeat(0xff, EPD_white);
		this->frame_fixed_repeat(0xaa, EPD_inverse);
		this->frame_fixed_repeat(0xaa, EPD_normal);
		
		End();
	}
	
	// Low level API calls
	// ===================

	// single frame refresh
	void frame_fixed(uint8_t fixed_value, EPD_stage stage);
	void frame_data(EPD_stage stage);
	void frame_fixed_repeat(uint8_t fixed_value, EPD_stage stage);
	void frame_data_repeat(EPD_stage stage);

	// convert temperature to compensation factor
	int temperature_to_factor_10x(int temperature);

	// single line display - very low-level
	void line(uint16_t line, const uint8_t *data, uint8_t fixed_value, bool read_progmem, EPD_stage stage);

	EPaperDriver(EPD_size size,
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

};

// this was initially implemented to support templating, so a header was inlined
#include "Epaper_header/EPaperDriverImpl.h"
}

#endif // EPaperDriver_H