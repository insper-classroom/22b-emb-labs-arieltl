/**
 * 5 semestre - Eng. da Computação - Insper
 * Rafael Corsi - rafael.corsi@insper.edu.br
 *
 * Projeto 0 para a placa SAME70-XPLD
 *
 * Objetivo :
 *  - Introduzir ASF e HAL
 *  - Configuracao de clock
 *  - Configuracao pino In/Out
 *
 * Material :
 *  - Kit: ATMEL SAME70-XPLD - ARM CORTEX M7
 */

/************************************************************************/
/* includes                                                             */
/************************************************************************/

#include "asf.h"

/************************************************************************/
/* defines                                                              */
/************************************************************************/
#define LED1_PIO PIOA  // periferico que controla o LED
// # (1)
#define LED1_PIO_ID ID_PIOA  // ID do periférico PIOC (controla LED)
#define LED1_PIO_IDX 0       // ID do LED no PIO
#define LED1_PIO_IDX_MASK (1 << LED1_PIO_IDX)
#define LED3_PIO PIOB  // periferico que controla o LED
// # (1)
#define LED3_PIO_ID ID_PIOB  // ID do periférico PIOC (controla LED)
#define LED3_PIO_IDX 2       // ID do LED no PIO
#define LED3_PIO_IDX_MASK (1 << LED3_PIO_IDX)

#define LED2_PIO PIOC  // periferico que controla o LED
// # (1)
#define LED2_PIO_ID ID_PIOC  // ID do periférico PIOC (controla LED)
#define LED2_PIO_IDX 30      // ID do LED no PIO
#define LED2_PIO_IDX_MASK (1 << LED2_PIO_IDX)

#define BUT1_PIO PIOD
#define BUT1_PIO_ID ID_PIOD
#define BUT1_PIO_IDX 28
#define BUT1_IDX_MASK (1u << BUT1_PIO_IDX)

#define BUT2_PIO PIOC
#define BUT2_PIO_ID ID_PIOC
#define BUT2_PIO_IDX 31
#define BUT2_IDX_MASK (1u << BUT2_PIO_IDX)

#define BUT3_PIO PIOA
#define BUT3_PIO_ID ID_PIOA
#define BUT3_PIO_IDX 19
#define BUT3_IDX_MASK (1u << BUT3_PIO_IDX)
/************************************************************************/
/* constants                                                            */
/************************************************************************/

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/

/************************************************************************/
/* prototypes                                                           */
/************************************************************************/

void init(void);

/************************************************************************/
/* interrupcoes                                                         */
/************************************************************************/

/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

// Função de inicialização do uC

Pio *buttons_pio[3] = {BUT1_PIO, BUT2_PIO, BUT3_PIO};
Pio *leds_pio[3] = {LED1_PIO, LED2_PIO, LED3_PIO};
uint32_t buttons_pio_id[3] = {BUT1_PIO_ID, BUT2_PIO_ID, BUT3_PIO_ID};
uint32_t leds_pio_id[3] = {LED1_PIO_ID, LED2_PIO_ID, LED3_PIO_ID};
uint32_t buttons_pio_mask[3] = {BUT1_IDX_MASK, BUT2_IDX_MASK, BUT3_IDX_MASK};
uint32_t leds_pio_mask[3] = {LED1_PIO_IDX_MASK, LED2_PIO_IDX_MASK,
                        LED3_PIO_IDX_MASK};


void init(void) {
  // Initialize the board clock
  sysclk_init();

  // Desativa WatchDog Timer
  WDT->WDT_MR = WDT_MR_WDDIS;
  for (int i = 0; i < 3; i++) {
    pmc_enable_periph_clk(leds_pio_id[i]);
    pio_set_output(leds_pio[i], leds_pio_mask[i], 1, 0, 0);
    pmc_enable_periph_clk(buttons_pio_id[i]);
    pio_set_input(buttons_pio[i], buttons_pio_mask[i],
                  PIO_DEFAULT | PIO_PULLUP | PIO_DEBOUNCE);
    pio_set_debounce_filter(buttons_pio[i], buttons_pio_mask[i], 10);
  }
}

/************************************************************************/
/* Main                                                                 */
/************************************************************************/

// Funcao principal chamada na inicalizacao do uC.
int main(void) {
	init();
	int leds_state[3] = {0, 0, 0};

  while (1) {


	
    for (int i = 0; i < 3; i++) {
	    if (!pio_get(buttons_pio[i], PIO_INPUT, buttons_pio_mask[i])) {
		    leds_state[i] = !leds_state[i];

		    } else {
		    leds_state[i] = 0;
	    }
	    if (leds_state[i]) {
		    pio_clear(leds_pio[i], leds_pio_mask[i]);
		    } else {
		    pio_set(leds_pio[i], leds_pio_mask[i]);
	    }
    }
   delay_ms(200);

    
  }
  return 0;
}
