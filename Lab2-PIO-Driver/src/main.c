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

/*  Default pin configuration (no attribute). */
#define _PIO_DEFAULT             (0u << 0)
/*  The internal pin pull-up is active. */
#define _PIO_PULLUP              (1u << 0)
/*  The internal glitch filter is active. */
#define _PIO_DEGLITCH            (1u << 1)
/*  The internal debouncing filter is active. */
#define _PIO_DEBOUNCE            (1u << 3)
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
void _pio_set(Pio *p_pio, const uint32_t ul_mask)
{
	p_pio->PIO_SODR = ul_mask;
}
void _pio_clear(Pio *p_pio, const uint32_t ul_mask)
{
	p_pio->PIO_CODR = ul_mask;

}
void _pio_pull_up(Pio *p_pio, const uint32_t ul_mask, const uint32_t ul_pull_up_enable){
	if (ul_pull_up_enable){
		p_pio->PIO_PUER = ul_mask;
	} else {
		p_pio->PIO_PUDR = ul_mask;
	
	}
	

 }
 void _pio_set_input(Pio *p_pio, const uint32_t ul_mask,
 const uint32_t ul_attribute)
 {	
	if (ul_attribute & _PIO_DEBOUNCE ){
		p_pio->PIO_IFSCER = ul_mask;
	}
	if (ul_attribute & _PIO_DEGLITCH ){
		p_pio->PIO_IFSCDR = ul_mask;
	}
	_pio_pull_up(p_pio,ul_mask,ul_attribute & _PIO_PULLUP);

	
 }
 
void _pio_set_output(Pio *p_pio, const uint32_t ul_mask,
const uint32_t ul_default_level,
const uint32_t ul_multidrive_enable,
const uint32_t ul_pull_up_enable)
{
	p_pio->PIO_PER = ul_mask;
	p_pio->PIO_OER = ul_mask;
	ul_default_level ? _pio_set(p_pio,ul_mask) : _pio_clear(p_pio,ul_mask);
	if (ul_multidrive_enable) {
		p_pio->PIO_MDER = ul_mask;
	} else {
		p_pio->PIO_MDDR = ul_mask;
	}
	_pio_pull_up(p_pio,ul_mask,ul_pull_up_enable);
	
}
void init(void) {
  // Initialize the board clock
  sysclk_init();

  // Desativa WatchDog Timer
  WDT->WDT_MR = WDT_MR_WDDIS;
  for (int i = 0; i < 3; i++) {
    pmc_enable_periph_clk(leds_pio_id[i]);
    _pio_set_output(leds_pio[i], leds_pio_mask[i], 1, 0, 0);
    pmc_enable_periph_clk(buttons_pio_id[i]);
	_pio_set_input(buttons_pio[i], buttons_pio_mask[i], _PIO_PULLUP | _PIO_DEBOUNCE);
	//_pio_pull_up(buttons_pio[i],buttons_pio_mask[i],1);
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
		    _pio_clear(leds_pio[i], leds_pio_mask[i]);
		    } else {
		    _pio_set(leds_pio[i], leds_pio_mask[i]);
	    }
    }
   delay_ms(200);

    
  }
  return 0;
}
