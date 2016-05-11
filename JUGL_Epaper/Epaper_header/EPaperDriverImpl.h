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

This is an adaptation of Pervasive Displays' 2013 EPaper drivers published with
Apache licencse 2.0

*/

// The 1.44” frame of data is 96 lines * 128 dots.
// The 2” frame of data is 96 lines * 200 dots.
// The 2.7” frame of data is 176 lines * 264 dots.

#ifndef EPaperDriver_IMPL_H
#define EPaperDriver_IMPL_H

#include "EPaperDriver.h"
#include "Arduino.h"

#define PAGE_SIZE 528

// defining some orphaned values from the previous implementation
#ifndef ULONG_MAX
#define ULONG_MAX (~(unsigned long)0)
#endif

// inline arrays
#define ARRAY(type, ...) ((type[]){__VA_ARGS__})
#define CU8(...) (ARRAY(const uint8_t, __VA_ARGS__))	
	
#include <Arduino.h>
#include <limits.h>
#include <SPI.h>

// delays - more consistent naming
#define Delay_ms(ms) delay(ms)
#define Delay_us(us) delayMicroseconds(us)

// inline arrays
#define ARRAY(type, ...) ((type[]){__VA_ARGS__})
#define CU8(...) (ARRAY(const uint8_t, __VA_ARGS__))

static void PWM_start(int pin);
static void PWM_stop(int pin);

static void SPI_on();
static void SPI_off();
static void SPI_put(uint8_t c);
static void SPI_put_wait(uint8_t c, int busy_pin);
static void SPI_send(uint8_t cs_pin, const uint8_t *buffer, uint16_t length);
	
EPaperDriver::EPaperDriver(EPD_size size,
		     int panel_on_pin,
		     int border_pin,
		     int discharge_pin,
		     int pwm_pin,
		     int reset_pin,
		     int busy_pin,
		     int chip_select_pin,
			 int temp_pin,
			 int flash_cs_pin, // not included in the original edp, but was set to high in the sketch
			 int flash_reset_pin,
			 int flash_sw2_pin
			 ) : IScreen(128, 96),
			
	EPD_Pin_EPD_CS(chip_select_pin),
	EPD_Pin_PANEL_ON(panel_on_pin),
	EPD_Pin_BORDER(border_pin),
	EPD_Pin_DISCHARGE(discharge_pin),
	EPD_Pin_PWM(pwm_pin),
	EPD_Pin_RESET(reset_pin),
	EPD_Pin_BUSY(busy_pin),
	EDP_Pin_flash_cs(flash_cs_pin),
	EDP_Pin_flash_reset(flash_reset_pin),
	EDP_Pin_flash_sw2(flash_sw2_pin),
	EDP_Pin_TEMPERATURE(temp_pin)		{

	this->size = size;
	this->stage_time = 480; // milliseconds
	this->lines_per_display = 96;
	this->dots_per_line = 128;
	this->bytes_per_line = 128 / 8;
	this->bytes_per_scan = 96 / 4;
	this->filler = false;


	// display size dependant items
	{
		static uint8_t cs[] = {0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0x00};
		static uint8_t gs[] = {0x72, 0x03};
		this->channel_select = cs;
		this->channel_select_length = sizeof(cs);
		this->gate_source = gs;
		this->gate_source_length = sizeof(gs);
	}

	// set up size structure
	switch (size) {
	default:
	case EPD_1_44:  // default so no change
		break;

	case EPD_2_0: {
		_w = 200;
		_h = 96;
		this->lines_per_display = 96;
		this->dots_per_line = 200;
		this->bytes_per_line = 200 / 8;
		this->bytes_per_scan = 96 / 4;
		this->filler = true;
		static uint8_t cs[] = {0x72, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xe0, 0x00};
		static uint8_t gs[] = {0x72, 0x03};
		this->channel_select = cs;
		this->channel_select_length = sizeof(cs);
		this->gate_source = gs;
		this->gate_source_length = sizeof(gs);
		break;
	}

	case EPD_2_7: {
		_w = 264;
		_h = 176;
		this->stage_time = 630; // milliseconds
		this->lines_per_display = 176;
		this->dots_per_line = 264;
		this->bytes_per_line = 264 / 8;
		this->bytes_per_scan = 176 / 4;
		this->filler = true;
		static uint8_t cs[] = {0x72, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xfe, 0x00, 0x00};
		static uint8_t gs[] = {0x72, 0x00};
		this->channel_select = cs;
		this->channel_select_length = sizeof(cs);
		this->gate_source = gs;
		this->gate_source_length = sizeof(gs);
		break;
	}
	}

	this->factored_stage_time = this->stage_time;
	
	SPI.begin();
	dataflash.setup(9, 10, 12);
	bufferedPage = 0;

	clearFrameBuff();
	dataflash.begin();		
	dataflash.pageToBuffer(bufferedPage, 1);
	dataflash.end();
}

void EPaperDriver::DrawPoint(Point p)
{	
	if(p.X > _w || p.Y > _h)
		return; // out of bounds
	
	dataflash.begin();
	
	// get page number and offset	
	uint16_t buffOffset = ((_w/8)*p.Y) + p.X/8;
	uint16_t page = buffOffset/PAGE_SIZE;
	uint16_t pageOffset = buffOffset - (page*PAGE_SIZE);
	
	// ensure the right page is loaded into the buffer
	if(bufferedPage != page)
	{
		// Cache miss! need to flush and swap
		dataflash.bufferToPage(1, bufferedPage);		
		dataflash.waitUntilReady();
		dataflash.pageToBuffer(page, 1);	
		dataflash.waitUntilReady();
		bufferedPage = page;
	}
	
	// fetch the byte from buffer mask it	
	dataflash.bufferRead(1, pageOffset);
	uint8_t data = SPI.transfer(0xff);	
	
	if(_drawColor.Value == WHITE)
		data &= ~(1 << (p.X % 8));		
	else
		data |= 1 << (p.X % 8);
	
	// push the change to buffer	
	dataflash.bufferWrite(1, pageOffset);
	SPI.transfer(data);
	dataflash.waitUntilReady();
	
	dataflash.end();
}

float EPaperDriver::get_temp () 
{
	float temp = analogRead(0)*5/1024.0;
	temp = temp - 0.5;
	temp = temp / 0.01;
	return (int)(temp - 4.0);
};

void EPaperDriver::Begin() {

	pinMode(EPD_Pin_PWM, OUTPUT);
	pinMode(EPD_Pin_BUSY, INPUT);
	pinMode(EPD_Pin_RESET, OUTPUT);
	pinMode(EPD_Pin_PANEL_ON, OUTPUT);
	pinMode(EPD_Pin_DISCHARGE, OUTPUT);
	pinMode(EPD_Pin_BORDER, OUTPUT);
	pinMode(EPD_Pin_EPD_CS, OUTPUT);
	pinMode(EDP_Pin_TEMPERATURE, INPUT);
	pinMode(EDP_Pin_flash_cs, OUTPUT);

	digitalWrite(EPD_Pin_PWM, LOW);
	digitalWrite(EPD_Pin_RESET, LOW);
	digitalWrite(EPD_Pin_PANEL_ON, LOW);
	digitalWrite(EPD_Pin_DISCHARGE, LOW);
	digitalWrite(EPD_Pin_BORDER, LOW);
	digitalWrite(EPD_Pin_EPD_CS, LOW);
	digitalWrite(EDP_Pin_flash_cs, HIGH);
	
	// power up sequence
	digitalWrite(this->EPD_Pin_RESET, LOW);
	digitalWrite(this->EPD_Pin_PANEL_ON, LOW);
	digitalWrite(this->EPD_Pin_DISCHARGE, LOW);
	digitalWrite(this->EPD_Pin_BORDER, LOW);
	digitalWrite(this->EPD_Pin_EPD_CS, LOW);

	SPI_on();

	PWM_start(this->EPD_Pin_PWM);
	Delay_ms(5);
	digitalWrite(this->EPD_Pin_PANEL_ON, HIGH);
	Delay_ms(10);

	digitalWrite(this->EPD_Pin_RESET, HIGH);
	digitalWrite(this->EPD_Pin_BORDER, HIGH);
	digitalWrite(this->EPD_Pin_EPD_CS, HIGH);
	Delay_ms(5);

	digitalWrite(this->EPD_Pin_RESET, LOW);
	Delay_ms(5);

	digitalWrite(this->EPD_Pin_RESET, HIGH);
	Delay_ms(5);

	// wait for COG to become ready
	while (HIGH == digitalRead(this->EPD_Pin_BUSY)) {
	}

	// channel select
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x01), 2);
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, this->channel_select, this->channel_select_length);

	// DC/DC frequency
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x06), 2);
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0xff), 2);

	// high power mode osc
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x07), 2);
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x9d), 2);


	// disable ADC
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x08), 2);
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x00), 2);

	// Vcom level
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x09), 2);
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0xd0, 0x00), 3);

	// gate and source voltage levels
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x04), 2);
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, this->gate_source, this->gate_source_length);

	Delay_ms(5);  //???

	// driver latch on
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x03), 2);
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x01), 2);

	// driver latch off
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x03), 2);
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x00), 2);

	Delay_ms(5);

	// charge pump positive voltage on
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x05), 2);
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x01), 2);

	// final delay before PWM off
	Delay_ms(30);
	PWM_stop(this->EPD_Pin_PWM);

	// charge pump negative voltage on
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x05), 2);
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x03), 2);

	Delay_ms(30);

	// Vcom driver on
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x05), 2);
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x0f), 2);

	Delay_ms(30);

	// output enable to disable
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x02), 2);
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x24), 2);

	SPI_off();
	
	setFactor(get_temp());
}


void EPaperDriver::End() {

	// dummy frame
	this->frame_fixed(0x55, EPD_normal);

	// dummy line and border
	if (EPD_1_44 == this->size) {
		// only for 1.44" EPD
		this->line(0x7fffu, 0, 0xaa, false, EPD_normal);

		Delay_ms(250);

	} else {
		// all other display sizes
		this->line(0x7fffu, 0, 0x55, false, EPD_normal);

		Delay_ms(25);

		digitalWrite(this->EPD_Pin_BORDER, LOW);
		Delay_ms(250);
		digitalWrite(this->EPD_Pin_BORDER, HIGH);
	}

	SPI_on();

	// latch reset turn on
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x03), 2);
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x01), 2);

	// output enable off
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x02), 2);
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x05), 2);

	// Vcom power off
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x05), 2);
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x0e), 2);

	// power off negative charge pump
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x05), 2);
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x02), 2);

	// discharge
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x04), 2);
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x0c), 2);

	Delay_ms(120);

	// all charge pumps off
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x05), 2);
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x00), 2);

	// turn of osc
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x07), 2);
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x0d), 2);

	// discharge internal - 1
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x04), 2);
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x50), 2);

	Delay_ms(40);

	// discharge internal - 2
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x04), 2);
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0xA0), 2);

	Delay_ms(40);

	// discharge internal - 3
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x04), 2);
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x00), 2);
	Delay_us(10);

	// turn of power and all signals
	digitalWrite(this->EPD_Pin_RESET, LOW);
	digitalWrite(this->EPD_Pin_PANEL_ON, LOW);
	digitalWrite(this->EPD_Pin_BORDER, LOW);

	// ensure SPI MOSI and CLOCK are Low before CS Low
	SPI_off();
	digitalWrite(this->EPD_Pin_EPD_CS, LOW);

	// discharge pulse
	digitalWrite(this->EPD_Pin_DISCHARGE, HIGH);
	Delay_ms(150);
	digitalWrite(this->EPD_Pin_DISCHARGE, LOW);
}


// convert a temperature in Celcius to
// the scale factor for frame_*_repeat methods
int EPaperDriver::temperature_to_factor_10x(int temperature) {
	if (temperature <= -10) {
		return 170;
	} else if (temperature <= -5) {
		return 120;
	} else if (temperature <= 5) {
		return 80;
	} else if (temperature <= 10) {
		return 40;
	} else if (temperature <= 15) {
		return 30;
	} else if (temperature <= 20) {
		return 20;
	} else if (temperature <= 40) {
		return 10;
	}
	return 7;
}

void EPaperDriver::frame_fixed(uint8_t fixed_value, EPD_stage stage) {
	for (uint8_t line = 0; line < this->lines_per_display ; ++line) {
		this->line(line, 0, fixed_value, false, stage);
	}
}

void EPaperDriver::frame_data(EPD_stage stage){
		
		uint8_t frameBuff[264 >> 3]; // initialized to the maximum possible (for the 2.7")
	for(uint16_t i = 0; i < sizeof(frameBuff); i++)
		frameBuff[i] = 1;
		
	for (uint16_t line = 0; line < this->lines_per_display ; line+=1) {
		flash_read((_w >> 3)*line, _w >> 3, frameBuff);
		this->line(line, frameBuff, 0, false, stage); // turn off progmem mode
	}
}

void EPaperDriver::clearFrameBuff()
{	
	const int framesToClear = 11; // todo fix this
	dataflash.begin();
	
	dataflash.bufferWrite(1, 0);
		
	for(long i = 0; i < PAGE_SIZE; i++)
		SPI.transfer(0);
	
	dataflash.waitUntilReady();
	
	for(int i = 0; i < framesToClear; i++)
	{
		dataflash.bufferToPage(1, i);
		dataflash.waitUntilReady();
	}
	
	dataflash.end();
}

void EPaperDriver::flash_read(uint16_t index, uint16_t count, uint8_t* buffer)
{
	dataflash.begin();
	uint16_t offset = index % PAGE_SIZE;
	uint16_t page = index / PAGE_SIZE; // number of bytes / page size
	
	dataflash.arrayRead(page, offset);
	
	for(uint16_t i = 0; i < count; i++) {
		buffer[i] = SPI.transfer(0);
	}	
	
	dataflash.waitUntilReady();
	
	// teardown
	dataflash.end();
}


void EPaperDriver::frame_fixed_repeat(uint8_t fixed_value, EPD_stage stage) {
	long stage_time = this->factored_stage_time;
	do {
		unsigned long t_start = millis();
		this->frame_fixed(fixed_value, stage);
		unsigned long t_end = millis();
		if (t_end > t_start) {
			stage_time -= t_end - t_start;
		} else {
			stage_time -= t_start - t_end + 1 + ULONG_MAX;
		}
	} while (stage_time > 0);
}


void EPaperDriver::frame_data_repeat(EPD_stage stage) {
	long stage_time = this->factored_stage_time;
	do {
		unsigned long t_start = millis();
		this->frame_data(stage);
		unsigned long t_end = millis();
		if (t_end > t_start) {
			stage_time -= t_end - t_start;
		} else {
			stage_time -= t_start - t_end + 1 + ULONG_MAX;
		}
	} while (stage_time > 0);
}


void EPaperDriver::line(uint16_t line, const uint8_t *data, uint8_t fixed_value, bool read_progmem, EPD_stage stage) {

	SPI_on();

	// charge pump voltage levels
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x04), 2);
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, this->gate_source, this->gate_source_length);

	// send data
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x0a), 2);
	Delay_us(10);

	// CS low
	digitalWrite(this->EPD_Pin_EPD_CS, LOW);
	SPI_put_wait(0x72, this->EPD_Pin_BUSY);

	// border byte only necessary for 1.44" EPD
	if (EPD_1_44 == this->size) {
		SPI_put_wait(0x00, this->EPD_Pin_BUSY);
		//SPI_send(this->EPD_Pin_EPD_CS, CU8(0x00), 1);
	}

	// even pixels
	for (uint16_t b = this->bytes_per_line; b > 0; --b) {
		if (0 != data) {
#if defined(__MSP430_CPU__)
			uint8_t pixels = data[b - 1] & 0xaa;
#else
			// AVR has multiple memory spaces
			uint8_t pixels;
			if (read_progmem) {
				pixels = pgm_read_byte_near(data + b - 1) & 0xaa;
			} else {
				pixels = data[b - 1] & 0xaa;
			}
#endif
			switch(stage) {
			case EPD_compensate:  // B -> W, W -> B (Current Image)
				pixels = 0xaa | ((pixels ^ 0xaa) >> 1);
				break;
			case EPD_white:       // B -> N, W -> W (Current Image)
				pixels = 0x55 + ((pixels ^ 0xaa) >> 1);
				break;
			case EPD_inverse:     // B -> N, W -> B (New Image)
				pixels = 0x55 | (pixels ^ 0xaa);
				break;
			case EPD_normal:       // B -> B, W -> W (New Image)
				pixels = 0xaa | (pixels >> 1);
				break;
			}
			SPI_put_wait(pixels, this->EPD_Pin_BUSY);
		} else {
			SPI_put_wait(fixed_value, this->EPD_Pin_BUSY);
		}	}

	// scan line
	for (uint16_t b = 0; b < this->bytes_per_scan; ++b) {
		if (line / 4 == b) {
			SPI_put_wait(0xc0 >> (2 * (line & 0x03)), this->EPD_Pin_BUSY);
		} else {
			SPI_put_wait(0x00, this->EPD_Pin_BUSY);
		}
	}

	// odd pixels
	for (uint16_t b = 0; b < this->bytes_per_line; ++b) {
		if (0 != data) {
#if defined(__MSP430_CPU__)
			uint8_t pixels = data[b] & 0x55;
#else
			// AVR has multiple memory spaces
			uint8_t pixels;
			if (read_progmem) {
				pixels = pgm_read_byte_near(data + b) & 0x55;
			} else {
				pixels = data[b] & 0x55;
			}
#endif
			switch(stage) {
			case EPD_compensate:  // B -> W, W -> B (Current Image)
				pixels = 0xaa | (pixels ^ 0x55);
				break;
			case EPD_white:       // B -> N, W -> W (Current Image)
				pixels = 0x55 + (pixels ^ 0x55);
				break;
			case EPD_inverse:     // B -> N, W -> B (New Image)
				pixels = 0x55 | ((pixels ^ 0x55) << 1);
				break;
			case EPD_normal:       // B -> B, W -> W (New Image)
				pixels = 0xaa | pixels;
				break;
			}
			uint8_t p1 = (pixels >> 6) & 0x03;
			uint8_t p2 = (pixels >> 4) & 0x03;
			uint8_t p3 = (pixels >> 2) & 0x03;
			uint8_t p4 = (pixels >> 0) & 0x03;
			pixels = (p1 << 0) | (p2 << 2) | (p3 << 4) | (p4 << 6);
			SPI_put_wait(pixels, this->EPD_Pin_BUSY);
		} else {
			SPI_put_wait(fixed_value, this->EPD_Pin_BUSY);
		}
	}

	if (this->filler) {
		SPI_put_wait(0x00, this->EPD_Pin_BUSY);
	}

	// CS high
	digitalWrite(this->EPD_Pin_EPD_CS, HIGH);

	// output data to panel
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x70, 0x02), 2);
	Delay_us(10);
	SPI_send(this->EPD_Pin_EPD_CS, CU8(0x72, 0x2f), 2);

	SPI_off();
}


static void SPI_on() {
	SPI.end();
	SPI.begin();
	SPI.setBitOrder(MSBFIRST);
	SPI.setDataMode(SPI_MODE2);
	SPI.setClockDivider(SPI_CLOCK_DIV2);
	SPI_put(0x00);
	SPI_put(0x00);
	Delay_us(10);
}


static void SPI_off() {
	// SPI.begin();
	// SPI.setBitOrder(MSBFIRST);
	SPI.setDataMode(SPI_MODE0);
	// SPI.setClockDivider(SPI_CLOCK_DIV2);
	SPI_put(0x00);
	SPI_put(0x00);
	Delay_us(10);
	SPI.end();
}


static void SPI_put(uint8_t c) {
	SPI.transfer(c);
}


static void SPI_put_wait(uint8_t c, int busy_pin) {

	SPI_put(c);

	// wait for COG ready
	while (HIGH == digitalRead(busy_pin)) {
	}
}


static void SPI_send(uint8_t cs_pin, const uint8_t *buffer, uint16_t length) {
	// CS low
	digitalWrite(cs_pin, LOW);

	// send all data
	for (uint16_t i = 0; i < length; ++i) {
		SPI_put(*buffer++);
	}

	// CS high
	digitalWrite(cs_pin, HIGH);
}


static void PWM_start(int pin) {
	analogWrite(pin, 128);  // 50% duty cycle
}


static void PWM_stop(int pin) {
	analogWrite(pin, 0);
}

#endif // EPaperDriver_IMPL_H