#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

#define TRIG_PIO PIOD
#define TRIG_PIO_ID ID_PIOD
#define TRIG_PIO_PIN 30
#define TRIG_PIO_PIN_MASK (1 << TRIG_PIO_PIN)

#define ECHO_PIO PIOA
#define ECHO_PIO_ID ID_PIOA
#define ECHO_PIO_PIN 6
#define ECHO_PIO_PIN_MASK (1 << ECHO_PIO_PIN)
void echo_callback();


volatile int rising_flag = 1;
volatile int update_flag = 0;
volatile int distance = 0;
const uint16_t pllPreScale = (int) (32768.0 / 17241.3793); //prescale para 1 count ser 1cm

void io_init(void)
{

  // Configura led
	pmc_enable_periph_clk(TRIG_PIO_ID);
	pio_configure(TRIG_PIO, PIO_OUTPUT_0, TRIG_PIO_PIN_MASK, PIO_DEFAULT);

  // Inicializa clock do periférico PIO responsavel pelo botao
	pmc_enable_periph_clk(ECHO_PIO_ID);

  // Configura PIO para lidar com o pino do botão como entrada
  // com pull-up
	pio_configure(ECHO_PIO, PIO_INPUT, ECHO_PIO_PIN_MASK, PIO_DEFAULT);

  // Configura interrupção no pino referente ao botao e associa
  // função de callback caso uma interrupção for gerada
  // a função de callback é a: but_callback()
  pio_handler_set(ECHO_PIO,
                  ECHO_PIO_ID,
                  ECHO_PIO_PIN_MASK,
                  PIO_IT_EDGE,
                  &echo_callback);

  // Ativa interrupção e limpa primeira IRQ gerada na ativacao
  pio_enable_interrupt(ECHO_PIO, ECHO_PIO_PIN_MASK);
  pio_get_interrupt_status(ECHO_PIO);
  
  // Configura NVIC para receber interrupcoes do PIO do botao
  // com prioridade 4 (quanto mais próximo de 0 maior)
  NVIC_EnableIRQ(ECHO_PIO_ID);
  NVIC_SetPriority(ECHO_PIO_ID, 4); // Prioridade 4
}

void echo_callback(){
	if (rising_flag){
		rtt_init(RTT,pllPreScale);
	} else {
		int dist = rtt_read_timer_value(RTT);
		if (dist>+2 && dist<=400){
			distance = dist;
		} else if (dist < 2) {
			distance = -1;
		} else {
			distance =-2;
		}
		update_flag = 1;
		
		
	}
	rising_flag=!rising_flag;
	
}

void init(){
	board_init();
	sysclk_init();
	delay_init();

	// Init OLED
	gfx_mono_ssd1306_init();
	io_init();
}
int main (void)
{
	init();
	
  
	int last_y = 0;
  
  
	int count = 0;
	int x = 50;
	int y;
	char dist_txt[10];
  /* Insert application code here, after the board has been initialized. */
	while(1) {
		if (update_flag){
			if (distance==-2){
				gfx_mono_draw_string(">400", 0,10, &sysfont);
				y=30;

			} else if (distance==-1){
				gfx_mono_draw_string("erro", 0,10, &sysfont);
				y = 1;
			} else {
				y = (int)((((float) distance) * 29.0)/400.0)+1;
				
				sprintf(dist_txt,"%d     ",distance);
				gfx_mono_draw_string(dist_txt, 0,10, &sysfont);
			}
			gfx_mono_draw_line(x,31-last_y,x+2,31-y,GFX_PIXEL_SET);
				
			x+=2;
			if (x>122){
				x =50;
				gfx_mono_draw_filled_rect(50,0,98,32,GFX_PIXEL_CLR);
			}
			update_flag = 0;
			last_y = y;
			
		}
		if (count > 300){
			
			count = 0;
			pio_set(TRIG_PIO,TRIG_PIO_PIN_MASK);
			delay_us(10);
			pio_clear(TRIG_PIO,TRIG_PIO_PIN_MASK);
		}
	
		delay_ms(1);
		count++;
		
	}
}
