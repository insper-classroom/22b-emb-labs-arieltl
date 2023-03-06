#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"
#define LED1_PIO PIOA // periferico que controla o LED
// # (1)
#define LED1_PIO_ID ID_PIOA // ID do periférico PIOC (controla LED)
#define LED1_PIO_IDX 0      // ID do LED no PIO
#define LED1_PIO_IDX_MASK (1 << LED1_PIO_IDX)
#define LED3_PIO PIOB // periferico que controla o LED
// # (1)
#define LED3_PIO_ID ID_PIOB // ID do periférico PIOC (controla LED)
#define LED3_PIO_IDX 2      // ID do LED no PIO
#define LED3_PIO_IDX_MASK (1 << LED3_PIO_IDX)

#define LED2_PIO PIOC // periferico que controla o LED
// # (1)
#define LED2_PIO_ID ID_PIOC // ID do periférico PIOC (controla LED)
#define LED2_PIO_IDX 30     // ID do LED no PIO
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

volatile int but1_pressed = 0;
volatile int but2_pressed = 0;
volatile int but3_pressed = 0;
volatile int but1_was_pressed = 0;

unsigned int t = 100;

void set_led(int state){
	if (state){
		pio_clear(LED1_PIO, LED1_PIO_IDX_MASK);
		return;
	}
	pio_set(LED1_PIO, LED1_PIO_IDX_MASK);
}
void pisca_led(int *count){
	static int state = 0;
	static unsigned int led_t = 0;

	led_t+=10;
	if  (led_t>t){
		led_t = 0;
		state = !state;
		set_led(state);
		if (state){
			(*count)++;
		}
	}
	
	
}
void but1_callback(){
	but1_pressed = !pio_get(BUT1_PIO,PIO_INPUT,BUT1_IDX_MASK);
	if (but1_pressed){
		but1_was_pressed = 1;
	}
}
void but2_callback(){
	but2_pressed = 1;
}
void but3_callback(){
	but3_pressed = 1;
}

void init(){
	board_init();
	sysclk_init();
	delay_init();

	// Init OLED
	gfx_mono_ssd1306_init();
	WDT->WDT_MR = WDT_MR_WDDIS;
	for (int i = 0; i < 3; i++) {
		pmc_enable_periph_clk(leds_pio_id[i]);
		pio_set_output(leds_pio[i], leds_pio_mask[i], 1, 0, 0);
		pmc_enable_periph_clk(buttons_pio_id[i]);
		pio_set_input(buttons_pio[i], buttons_pio_mask[i], PIO_PULLUP | PIO_DEBOUNCE);
		//_pio_pull_up(buttons_pio[i],buttons_pio_mask[i],1);
		pio_set_debounce_filter(buttons_pio[i], buttons_pio_mask[i], 10);
		
		pio_enable_interrupt(buttons_pio[i], buttons_pio_mask[i]);
		pio_get_interrupt_status(buttons_pio[i]);
	
		// Configura NVIC para receber interrupcoes do PIO do botao
		// com prioridade 4 (quanto mais próximo de 0 maior)
		NVIC_EnableIRQ(buttons_pio_id[i]);
		NVIC_SetPriority(buttons_pio_id[i], 4); // Prioridade 4
	}
	pio_handler_set(BUT1_PIO,
	BUT1_PIO_ID,
	BUT1_IDX_MASK,
	PIO_IT_EDGE,
	but1_callback);
	pio_handler_set(BUT2_PIO,
	BUT2_PIO_ID,
	BUT2_IDX_MASK,
	PIO_IT_RISE_EDGE,
	but2_callback);
	pio_handler_set(BUT3_PIO,
	BUT3_PIO_ID,
	BUT3_IDX_MASK,
	PIO_IT_RISE_EDGE,
	but3_callback);
}
int main (void)
{
	init();
	int count = 0;
	int n_blinks = 30;
	char int_str[10];

	unsigned int but_count =0;
  

  
  

  /* Insert application code here, after the board has been initialized. */
	while(1) {
			if (but1_pressed || but1_was_pressed){
				but_count++;
				but1_was_pressed = 0;
			} else if(but_count!=0){
				if (but_count>30 && t<300){
					t+=10;
				} else if (t>=20) {
					t-=10;
				}
				but_count = 0;
			}
			if (but2_pressed){
				but2_pressed = 0;
				if (t<300){
					t+=10;
				}
			}
			sprintf(
			int_str, "%d", t) ;
			gfx_mono_draw_filled_rect(0, 10, 128, 15, GFX_PIXEL_CLR);
			gfx_mono_draw_string(int_str, 10,8, &sysfont);
			pisca_led(&count);
			
			
			
				
			gfx_mono_draw_filled_rect((count*4), 0, 4, 6, GFX_PIXEL_SET);
				
				
		
		
			if(but3_pressed || count>n_blinks ){
				count = 0;
				
					
				gfx_mono_draw_filled_rect(0, 0, 128, 6, GFX_PIXEL_CLR);
					
					
				set_led(0);
				but3_pressed = 0;
				pmc_sleep(SAM_PM_SMODE_SLEEP_WFI); // (1)
				

			}
			

			delay_ms(10);
			
			
			
			
	}
}
