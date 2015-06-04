/**
 * \file
 *
 * \brief User board initialization template
 *
 */

#include <asf.h>
#include <board.h>
#include <conf_board.h>
#include "delay.h"
#include "wdt.h"
#include "DAC.h"

void board_init(void)
{
	/* This function is meant to contain board-specific initialization code
	 * for, e.g., the I/O pins. The initialization can rely on application-
	 * specific board configuration, found in conf_board.h.
	 */
	delay_ms(10); // seems PLL works better with this line...
	sysclk_init();
	wdt_disable(WDT);
	pmc_enable_periph_clk(ID_PIOA);
	pio_set_output(PIOA, PIO_PA17, LOW, DISABLE, DISABLE);
	pio_set_output(PIOA, PIO_PA18, LOW, DISABLE, DISABLE);
	dac_init();
	
}
