/* 
 * Monulator ARM LPC11U14 application.
 * 
 * Copyright (C) 2013 Jared Boone, ShareBrained Technology, Inc.
 * 
 * This file is part of the Monulator project.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "lpc11u.h"
#include "serial.h"

#include "cc3000.h"
#include "cc3000_spi.h"
#include "cc3000_nvram.h"
#include "cc3000_wlan.h"
#include "cc3000_socket.h"

#include "http.h"
#include "csv_parser.h"
#include "aurora_swpc.h"

//#include "patch.h"

#include "TEST_WLAN_INFO.h"

#include <cstdint>

void turn_on_crystal_oscillator() {
	SCB.SYSOSCCTRL = 0;
	SCB.enable_power(SCB.PDRUNCFG_SYSOSC);
}

void start_system_pll() {
	// Start up system PLL: 12MHz crystal to 48 MHz.
	SCB.set_system_pll_dividers(1, 3);
	SCB.set_system_pll_clock_source(SCB.SYSPLLCLKSEL_SEL_CRYSTAL_OSCILLATOR);
	SCB.enable_power(SCB.PDRUNCFG_SYSPLL);
	SCB.wait_for_system_pll_lock();
}

void set_main_clock_to_system_pll() {
	// Switch main (and system clocks) to system PLL
	SCB.set_main_clock_source(SCB.MAINCLKSEL_SEL_PLL_OUTPUT);
}

void start_usb_pll() {
	// Start up USB PLL: 12MHz crystal to 48 MHz.
	SCB.set_usb_pll_dividers(1, 3);
	SCB.set_usb_pll_clock_source(SCB.USBPLLCLKSEL_SEL_SYSTEM_OSCILLATOR);
	SCB.enable_power(SCB.PDRUNCFG_USBPLL);
	SCB.wait_for_usb_pll_lock();
}

void set_usb_clock_to_main_clock() {
	// NOTE: Both clock sources must be running in order to switch!
	// So if you want to use the main clock instead of the USB PLL,
	// you have to start the USB PLL in order to switch away from it!
	SCB.set_usb_clock_source(SCB.USBCLKSEL_SEL_MAIN_CLOCK);
}

void set_usb_clock_to_usb_pll() {
	SCB.set_usb_clock_source(SCB.USBCLKSEL_SEL_USB_PLL_OUT);
}

void enable_peripheral_clocks() {
	SCB.set_system_clock_divider(1);
	SCB.enable_clocks(
		  SCB.SYSAHBCLKCTRL_GPIO
		//| SCB.SYSAHBCLKCTRL_CT16B0
		//| SCB.SYSAHBCLKCTRL_CT16B1
		//| SCB.SYSAHBCLKCTRL_CT32B0
		| SCB.SYSAHBCLKCTRL_CT32B1
		| SCB.SYSAHBCLKCTRL_USART
		| SCB.SYSAHBCLKCTRL_SSP1
		| SCB.SYSAHBCLKCTRL_USB
		| SCB.SYSAHBCLKCTRL_IOCON
		//| SCB.SYSAHBCLKCTRL_RAM1	// Available on larger parts.
		| SCB.SYSAHBCLKCTRL_USBRAM
	);
}

void enable_clock_output() {
	SCB.set_clkout_clock_source(SCB.CLKOUTSEL_SEL_MAIN_CLOCK);
	SCB.set_clkout_divider(48);
	IOCON.PIO0_1 = 1;
}

void configure_pins() {
	// Enable LED output
	GPIO_PORT.DIR0 |=
		(1 << 7)
		;
	
	// CT16B0_MAT PWM outputs
	/*
	IOCON.PIO0_8 = 2;
	IOCON.PIO0_9 = 2;
	IOCON.PIO1_15 = 2;
	*/
	
	// CT32B1_MAT PWM outputs
	/*
	IOCON.TDO_PIO0_13 = 3;	// CT32B1_MAT0
	IOCON.TRST_PIO0_14 = 3;	// CT32B1_MAT1
	IOCON.PIO0_16 = 2;		// CT32B1_MAT3
	*/
	
	// Configure USART
	IOCON.PIO0_18 = 1;		// RXD
	IOCON.PIO0_19 = 1;		// TXD
	
	// Configure USB I/O
	IOCON.PIO0_3 = 1;		// USB_VBUS
	IOCON.PIO0_6 = 1;		// USB_CONNECT#
	SCB.enable_power(SCB.PDRUNCFG_USBPAD);
	
	// Configure CC3000 EN and IRQ
	cc3000.disable();
	
	// CC3000_EN:   PIO, output
	GPIO_PORT.DIR1 |= (1 << 22);
	IOCON.PIO1_22 = 0;		
	
	// CC3000_IRQ#: PIO, input
	// Pull-up ensures IRQ# is inactive when module is off.
	GPIO_PORT.DIR1 &= ~(1 << 27);
	IOCON.PIO1_27 = (2 << 3); // pull-up resistor enabled.
	
	// CC3000_CS#:  PIO, output
	GPIO_PORT.DIR1 |= (1 << 23);
	IOCON.PIO1_23 = 0;
	
	// Configure SPI I/O
	IOCON.PIO1_20 = 2;		// CC3000_SCLK: SCLK1
	IOCON.PIO0_21 = 2;		// CC3000_MOSI: MOSI1
	IOCON.PIO1_21 = 2;		// CC3000_MISO: MISO1
}

void configure_led_pwm_timer_ct16b0() {
	// Configure 16 bit timer/match 0
	CT16B0.TCR = (1 << 1);
	CT16B0.TCR = 0;

	CT16B0.PC = 0;
	CT16B0.TC = 0;

	CT16B0.PR = 0;
	
	CT16B0.MR[0] = 12000;	// Channel 1 low time
	CT16B0.MR[1] = 24000;	// Channel 2 low time
	CT16B0.MR[2] = 36000;	// Channel 3 low time
	CT16B0.MR[3] = 48000;	// Cycle count.
	CT16B0.MCR = (1 << 10);
	CT16B0.EMR = 0;
	CT16B0.PWMC =
		  (1 << 2)
		| (1 << 1)
		| (1 << 0)
		;
		
	CT16B0.TCR = (1 << 0);
}

void configure_led_pwm_timer_ct32b1() {
	// Configure 32 bit timer/match 1
	CT32B1.TCR = (1 << 1);
	CT32B1.TCR = 0;

	CT32B1.PC = 0;
	CT32B1.TC = 0;

	CT32B1.PR = 0;
	
	CT32B1.MR[0] = 12000;	// Channel 1 low time
	CT32B1.MR[1] = 24000;	// Channel 2 low time
	CT32B1.MR[2] = 48000;	// Cycle count.
	CT32B1.MR[3] = 36000;	// Channel 4 low time
	CT32B1.MCR = (1 << 7);
	CT32B1.EMR = 0;
	CT32B1.PWMC =
		  (1 << 3)
		| (1 << 1)
		| (1 << 0)
		;
		
	CT32B1.TCR = (1 << 0);
}

void configure_usb() {
	// TODO: Start up USB PLL separately, just to free up main clock to be adjustable?
	set_usb_clock_to_usb_pll();
	SCB.set_usb_clock_divider(1);

	usb_endpoint_list_t* endpoint_list = reinterpret_cast<usb_endpoint_list_t*>(0x20004000);
	endpoint_list->ep[0].out[0] = (64 << 16) | (0x200040C0 >> 6);
	endpoint_list->ep[0].out[1] = (0x20004080 >> 6);
	endpoint_list->ep[0].in[0] = (64 << 16) | (0x20004100 >> 6);
	endpoint_list->ep[0].in[1] = 0;
	
	USB.set_endpoint_list_start_address(endpoint_list);
	USB.set_data_buffer_start_address(0x20004000);
	USB.enable_device();
	USB.clear_all_interrupts();
	USB.enable_all_interrupts();
	USB.enable_connect();
}

void init_cc3000(const simplelink_patches_request_t patches_request=SIMPLELINK_PATCHES_REQUEST_DEFAULT) {
	delay(5000000);

	serial_write_line("disabling...");
	cc3000.disable();

	delay(5000000);
	
	serial_write_string("enabling with patches: ");
	switch(patches_request) {
	case SIMPLELINK_PATCHES_REQUEST_DEFAULT:
		serial_write_line("default");
		break;

	case SIMPLELINK_PATCHES_REQUEST_FORCE_HOST:
		serial_write_line("force host");
		break;

	case SIMPLELINK_PATCHES_REQUEST_FORCE_NONE:
		serial_write_line("force none");
		break;
	}
	
	cc3000.enable(patches_request);
	
	cc3000_wlan.smartconfig_set_prefix();
	cc3000_wlan.ioctl_set_connection_policy(false, false, false);
	cc3000_wlan.ioctl_del_profile(255);
	cc3000_wlan.set_event_mask((hci_event_mask_t)(
		  HCI_EVENT_MASK_WLAN_KEEPALIVE
		| HCI_EVENT_MASK_WLAN_INIT
		| HCI_EVENT_MASK_WLAN_ASYNC_PING_REPORT
		)
	);
}

void display_buffers_info() {
	const auto buffer_size = cc3000.read_buffer_size();
	serial_write_string("buffers: 0x");
	serial_write_hex(buffer_size.number_of_free_buffers);
	serial_write_string(" x 0x");
	serial_write_hex(buffer_size.buffer_length);
	serial_write_line();
}

void display_sp_version() {
	const auto sp_version = cc3000_nvram.read_sp_version();
	serial_write_string("sp_version: 0x");
	serial_write_hex(sp_version.package_id);
	serial_write_string(".0x");
	serial_write_hex(sp_version.package_build_number);
	serial_write_line();
}

void dump_nvram_file(const nvram_file_id_t file_id, const size_t file_length) {
	std::array<uint8_t, 150> buffer;
	const size_t offset_end = file_length;
	for(size_t offset=0; offset<file_length; offset+=buffer.size()) {
		const size_t remaining = offset_end - offset;
		const size_t length = (remaining < buffer.size()) ? remaining : buffer.size();
		if( cc3000_nvram.read(file_id, length, offset, buffer.data()) == HCI_EVENT_STATUS_OK ) {
			//serial_write_hex(offset, 4);
			//serial_write(':');
			for(size_t i=0; i<length; i++) {
				serial_write(' ');
				serial_write_hex(buffer[i], 2);
			}
			serial_write(',');
		}
	}
}
/*
class http_response_sink_log_t : public sink_t {
	virtual int do_sink(const void* const data, const size_t length) override {
		serial_write_buffer((const uint8_t*)data, length);
		return 0;
	}
};

class csv_sink_log_t : public csv_sink_t {
	virtual int do_csv_cell(const size_t column, const size_t row, const void* const data, const size_t data_length) override {
		serial_write_hex(column, 1);
		serial_write(',');
		serial_write_hex(row, 1);
		serial_write('=');
		serial_write_buffer((const uint8_t*)data, data_length);
		serial_write_line();
		return 0;
	}
};
*/
extern "C" int main() {
	turn_on_crystal_oscillator();
	start_system_pll();
	start_usb_pll();
	set_main_clock_to_system_pll();
	enable_peripheral_clocks();
	serial_init();

	configure_pins();
	
	configure_led_pwm_timer_ct32b1();

	//enable_clock_output();
	/*
	configure_usb();
	
	while(1) {
		GPIO_WORD.W[7] = USB.setup_token_received();
		//GPIO_PORT.NOT0 = (1 << 7);
		//do_work();
	}
	*/
	
	SCB.set_ssp1_clock_divider(1);
	SCB.reset_ssp1_peripheral();

	// Bit frequency = pclk / (cpsdvsr * (scr + 1))
	// pclk = 48MHz
	const uint32_t scr = 0;
	const uint32_t cpsdvsr = 48;
		
	SSP1.CR0 =
		  (scr << 8)
		| (1 << 7)		// CPHA=1 (capture data on clock return to idle)
		| (0 << 6)		// CPOL=0 (clock low between frames)
		| (0 << 4)		// SPI
		| (7 << 0)		// 8-bit transfer
		;
		
	SSP1.CR1 =
		  (0 << 2)		// SPI mode: master
		| (0 << 1)		// SPI controller: disabled
		| (0 << 0)		// Loopback mode: off (normal)
		;
		
	SSP1.CPSR =
		  (cpsdvsr << 0)
		;
	
	SSP1.IMSC =
		  (0 << 3)		// TXIM
		| (0 << 2)		// RXIM
		| (0 << 1)		// RTIM
		| (0 << 0)		// RORIM
		;
	
	SSP1.ICR =
		  (1 << 1)		// RTIC
		| (1 << 0)		// RORIC
		;

	// Enable SPI
	SSP1.CR1 |= (1 << 1);
	
	NVIC.enable_interrupts();
	
	init_cc3000();

	cc3000_wlan.connect(WLAN_SSID, {{0,0,0,0,0,0}}, WLAN_KEY);
	while( !cc3000_wlan.is_dhcp() ) {
		cc3000.check_for_unsolicited_events();
	}
	
	GPIO_PORT.SET0 = (1 << 7);
	
	aurora_swpc_t aurora_swpc;
	csv_parser_t csv_parser(' ', aurora_swpc);
	//csv_sink_log_t csv_log_sink;
	//csv_parser_t csv_parser(' ', csv_log_sink);
	http_connection_t connection(csv_parser);
	
	//http_response_sink_log_t http_response_sink;
	//http_connection_t connection(http_response_sink);
	
	connection.open("www.swpc.noaa.gov", 80, "/ftpdir/lists/hpi/activity.dat");
	connection.close();
	
	serial_write_string("aurora: ");
	serial_write_hex(aurora_swpc.level(), 1);
	serial_write_line();
	
	while(true) {
		cc3000.check_for_unsolicited_events();
		
		if( cc3000_wlan.is_connected() ) {
			GPIO_PORT.SET0 = (1 << 7);
		} else {
			GPIO_PORT.CLR0 = (1 << 7);
		}
	}
	
	return 0;
}

extern "C" void SystemInit() {
	
}
/*
#include <errno.h>
extern "C" int _kill(int pid, int sig) {
	errno = ENOSYS;
	return -1;
}
*/