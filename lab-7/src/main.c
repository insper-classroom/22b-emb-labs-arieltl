/************************************************************************/
/* includes                                                             */
/************************************************************************/

#include <asf.h>
#include <string.h>
#include "ili9341.h"
#include "lvgl.h"
#include "touch/touch.h"

LV_FONT_DECLARE(dseg70);
LV_FONT_DECLARE(dseg50);
LV_FONT_DECLARE(dseg30);
LV_FONT_DECLARE(celcius);
LV_FONT_DECLARE(icons);



/************************************************************************/
/* LCD / LVGL                                                           */
/************************************************************************/

#define LV_HOR_RES_MAX          (320)
#define LV_VER_RES_MAX          (240)

/*A static or global variable to store the buffers*/
static lv_disp_draw_buf_t disp_buf;

/*Static or global buffer(s). The second buffer is optional*/
static lv_color_t buf_1[LV_HOR_RES_MAX * LV_VER_RES_MAX];
static lv_disp_drv_t disp_drv;          /*A variable to hold the drivers. Must be static or global.*/
static lv_indev_drv_t indev_drv;

lv_obj_t * labelFloor;
lv_obj_t * labelClock;
lv_obj_t * labelSetValue;
lv_obj_t * labelSettings;

volatile int controlMode = 0;
volatile int power = 1;
typedef struct  {
	uint32_t year;
	uint32_t month;
	uint32_t day;
	uint32_t week;
	uint32_t hour;
	uint32_t minute;
	uint32_t second;
} calendar;

/************************************************************************/
/* RTOS                                                                 */
/************************************************************************/
SemaphoreHandle_t xSemaphoreClock;
#define TASK_LCD_STACK_SIZE                (1024*6/sizeof(portSTACK_TYPE))
#define TASK_LCD_STACK_PRIORITY            (tskIDLE_PRIORITY)

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,  signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);
void lv_termostato(void);
extern void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName) {
	printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);
	for (;;) {	}
}

extern void vApplicationIdleHook(void) { }

extern void vApplicationTickHook(void) { }

extern void vApplicationMallocFailedHook(void) {
	configASSERT( ( volatile void * ) NULL );
}

void RTC_init(Rtc *rtc, uint32_t id_rtc, calendar t, uint32_t irq_type) {
	/* Configura o PMC */
	pmc_enable_periph_clk(ID_RTC);

	/* Default RTC configuration, 24-hour mode */
	rtc_set_hour_mode(rtc, 0);

	/* Configura data e hora manualmente */
	rtc_set_date(rtc, t.year, t.month, t.day, t.week);
	rtc_set_time(rtc, t.hour, t.minute, t.second);

	/* Configure RTC interrupts */
	NVIC_DisableIRQ(id_rtc);
	NVIC_ClearPendingIRQ(id_rtc);
	NVIC_SetPriority(id_rtc, 4);
	NVIC_EnableIRQ(id_rtc);

	/* Ativa interrupcao via alarme */
	rtc_enable_interrupt(rtc,  irq_type);
}

void RTC_Handler(void) {
	uint32_t ul_status = rtc_get_status(RTC);
	
	/* seccond tick */
	if ((ul_status & RTC_SR_SEC) == RTC_SR_SEC) {
		// o código para irq de segundo vem aqui
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		xSemaphoreGiveFromISR(xSemaphoreClock,&xHigherPriorityTaskWoken);
		
	}
	
	/* Time or date alarm */
	if ((ul_status & RTC_SR_ALARM) == RTC_SR_ALARM) {
		// o código para irq de alame vem aqui
	}
    rtc_clear_status(RTC, RTC_SCCR_SECCLR);
    rtc_clear_status(RTC, RTC_SCCR_ALRCLR);
    rtc_clear_status(RTC, RTC_SCCR_ACKCLR);
    rtc_clear_status(RTC, RTC_SCCR_TIMCLR);
    rtc_clear_status(RTC, RTC_SCCR_CALCLR);
    rtc_clear_status(RTC, RTC_SCCR_TDERRCLR);
	/// ...
}
/************************************************************************/
/* lvgl                                                                 */
/************************************************************************/

static void event_handler(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);

	if(code == LV_EVENT_CLICKED) {
		LV_LOG_USER("Clicked");
	}
	else if(code == LV_EVENT_VALUE_CHANGED) {
		LV_LOG_USER("Toggled");
	}
}
static void up_handler(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);
	char *c;
	int temp;
	if(code == LV_EVENT_CLICKED) {
		if (controlMode==0){
			c = lv_label_get_text(labelSetValue);
			temp = atoi(c);
			lv_label_set_text_fmt(labelSetValue, "%02d", temp +1);
		} else if (controlMode == 1){
			uint32_t current_hour, current_min, current_sec;
			rtc_get_time(RTC, &current_hour, &current_min, &current_sec);
			rtc_set_time(RTC, ++current_hour, current_min, current_sec);
		} else {
			uint32_t current_hour, current_min, current_sec;
			rtc_get_time(RTC, &current_hour, &current_min, &current_sec);
			rtc_set_time(RTC, current_hour, ++current_min, current_sec);
		}

	}
}

static void down_handler(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);
	char *c;
	int temp;
	if(code == LV_EVENT_CLICKED) {
		if (controlMode==0){
			c = lv_label_get_text(labelSetValue);
			temp = atoi(c);
			lv_label_set_text_fmt(labelSetValue, "%02d", temp -1);
		} else if (controlMode == 1){
		uint32_t current_hour, current_min, current_sec;
		rtc_get_time(RTC, &current_hour, &current_min, &current_sec);
		rtc_set_time(RTC, --current_hour, current_min, current_sec);
	} else {
		uint32_t current_hour, current_min, current_sec;
		rtc_get_time(RTC, &current_hour, &current_min, &current_sec);
		rtc_set_time(RTC, current_hour, --current_min, current_sec);
	}

	}
}
static void power_handler(lv_event_t * e) {
		lv_event_code_t code = lv_event_get_code(e);
	
		if(code == LV_EVENT_CLICKED) {
			power = !power;
			lv_obj_clean(lv_scr_act());
			lv_termostato();
		}
}
static void settings_handler(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);
	char *c;
	int temp;
	if(code == LV_EVENT_CLICKED) {
		controlMode++;
		if (controlMode>2){
			controlMode = 0;
		}
		if (controlMode==0){
			lv_label_set_text(labelSettings, "\ue1ff");
		} else if (controlMode ==1){
			lv_label_set_text(labelSettings, "\ue425");
		} else {
			lv_label_set_text(labelSettings, "\ue8b5");
			
		}
	}
}
void lv_ex_btn_1(void) {
	lv_obj_t * label;

	lv_obj_t * btn1 = lv_btn_create(lv_scr_act());
	lv_obj_add_event_cb(btn1, event_handler, LV_EVENT_ALL, NULL);
	lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -40);

	label = lv_label_create(btn1);
	lv_label_set_text(label, "Corsi");
	lv_obj_center(label);

	lv_obj_t * btn2 = lv_btn_create(lv_scr_act());
	lv_obj_add_event_cb(btn2, event_handler, LV_EVENT_ALL, NULL);
	lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 40);
	lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
	lv_obj_set_height(btn2, LV_SIZE_CONTENT);

	label = lv_label_create(btn2);
	lv_label_set_text(label, "Toggle");
	lv_obj_center(label);
}
void lv_termostato(void){
	static lv_style_t style;
	lv_style_init(&style);
	lv_style_set_bg_color(&style, lv_color_black());
	lv_style_set_border_width(&style, 0);

	lv_obj_t * btnPower = lv_btn_create(lv_scr_act());
	lv_obj_add_event_cb(btnPower, power_handler, LV_EVENT_ALL, NULL);
	lv_obj_align(btnPower, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    lv_obj_add_style(btnPower, &style, 0);
	lv_obj_set_width(btnPower, 60);  
	lv_obj_set_height(btnPower, 60);
	
	lv_obj_t * labelPower;
	labelPower = lv_label_create(btnPower);
	lv_label_set_text(labelPower, "[ " LV_SYMBOL_POWER);
	lv_obj_center(labelPower);
	
	if (!power){
		return;
	}
	lv_obj_t * btnClock = lv_btn_create(lv_scr_act());
	lv_obj_add_event_cb(btnClock, event_handler, LV_EVENT_ALL, NULL);
	lv_obj_set_width(btnClock, 60);
	lv_obj_set_height(btnClock, 60);
	lv_obj_align_to(btnClock, btnPower, LV_ALIGN_OUT_RIGHT_MID, -5, 0);
	lv_obj_add_style(btnClock, &style, 0);

	
	lv_obj_t * labelClockbt;
	labelClockbt = lv_label_create(btnClock);
	lv_label_set_text(labelClockbt, "| " LV_SYMBOL_REFRESH " | ");
	lv_obj_center(labelClockbt);
	
	lv_obj_t * btnSettings = lv_btn_create(lv_scr_act());
	lv_obj_add_event_cb(btnSettings, settings_handler, LV_EVENT_ALL, NULL);
	lv_obj_set_width(btnSettings, 60);
	lv_obj_set_height(btnSettings, 60);
	lv_obj_align_to(btnSettings, btnClock, LV_ALIGN_OUT_RIGHT_MID, -10, 0);
	lv_obj_add_style(btnSettings, &style, 0);

	
	labelSettings = lv_label_create(btnSettings);
	lv_obj_set_style_text_font(labelSettings, &icons, LV_STATE_DEFAULT);
	lv_label_set_text(labelSettings, "\ue1ff");
	lv_obj_center(labelSettings);
	
	lv_obj_t * labelBracket;
	labelBracket = lv_label_create(lv_scr_act());
	lv_obj_set_style_text_color(labelBracket, lv_color_white(), LV_STATE_DEFAULT);
	lv_label_set_text(labelBracket, " ]" );

	lv_obj_align_to(labelBracket,btnSettings, LV_ALIGN_OUT_RIGHT_MID, -15 , 0);

	
	lv_obj_t * btnDown = lv_btn_create(lv_scr_act());
	lv_obj_add_event_cb(btnDown, down_handler, LV_EVENT_ALL, NULL);
	lv_obj_set_width(btnDown, 60);
	lv_obj_set_height(btnDown, 60);
	lv_obj_align(btnDown, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
	lv_obj_add_style(btnDown, &style, 0);

	
	lv_obj_t * labelDown;
	labelDown = lv_label_create(btnDown);
	lv_label_set_text(labelDown, LV_SYMBOL_DOWN " ]");
	lv_obj_center(labelDown);

	lv_obj_t * btnUp = lv_btn_create(lv_scr_act());
	lv_obj_add_event_cb(btnUp, up_handler, LV_EVENT_ALL, NULL);
	lv_obj_set_width(btnUp, 60);
	lv_obj_set_height(btnUp, 60);
	lv_obj_align_to(btnUp, btnDown, LV_ALIGN_OUT_LEFT_MID, 0, 0);
	lv_obj_add_style(btnUp, &style, 0);

	
	lv_obj_t * labelUp;
	labelUp = lv_label_create(btnUp);
	lv_label_set_text(labelUp, "[ " LV_SYMBOL_UP );
	lv_obj_center(labelUp);
	
	labelFloor = lv_label_create(lv_scr_act());
	lv_obj_align(labelFloor, LV_ALIGN_LEFT_MID, 35 , -45);
	lv_obj_set_style_text_font(labelFloor, &dseg70, LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(labelFloor, lv_color_white(), LV_STATE_DEFAULT);
	lv_label_set_text_fmt(labelFloor, "%02d", 23);
	
	labelClock = lv_label_create(lv_scr_act());
	lv_obj_align(labelClock, LV_ALIGN_TOP_RIGHT, -20 , 10);
	lv_obj_set_style_text_font(labelClock, &dseg30, LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(labelClock, lv_color_white(), LV_STATE_DEFAULT);
	lv_label_set_text_fmt(labelClock, "%02d:%02d", 17,46);
	
	
	labelSetValue = lv_label_create(lv_scr_act());
	lv_obj_set_style_text_font(labelSetValue, &dseg50, LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(labelSetValue, lv_color_white(), LV_STATE_DEFAULT);
	lv_label_set_text_fmt(labelSetValue, "%02d", 22);

	lv_obj_align_to(labelSetValue,labelClock, LV_ALIGN_OUT_BOTTOM_MID, 5 , 10);
	
	lv_obj_t * labelDecimal;
	labelDecimal = lv_label_create(lv_scr_act());
	lv_obj_set_style_text_font(labelDecimal, &dseg30, LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(labelDecimal, lv_color_white(), LV_STATE_DEFAULT);
	lv_label_set_text_fmt(labelDecimal, ".%01d", 4);

	lv_obj_align_to(labelDecimal,labelFloor, LV_ALIGN_OUT_RIGHT_BOTTOM, 0 , 0);
	
	lv_obj_t * labelDegree;
	labelDegree = lv_label_create(lv_scr_act());
	lv_obj_set_style_text_font(labelDegree, &celcius, LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(labelDegree, lv_color_white(), LV_STATE_DEFAULT);
	lv_label_set_text(labelDegree, "\u00B0C" );

	lv_obj_align_to(labelDegree,labelSetValue, LV_ALIGN_OUT_RIGHT_TOP, 0 , 0);


}
/************************************************************************/
/* TASKS                                                                */
/************************************************************************/
static void task_clock(void *pvParameters) {
	calendar rtc_initial = {2018, 3, 19, 12, 15, 45 ,1};
	
	RTC_init(RTC, ID_RTC, rtc_initial, RTC_IER_SECEN);
	uint32_t current_hour, current_min, current_sec;
	int blink = 1;

	for (;;)  {
		 if (xSemaphoreTake(xSemaphoreClock, 1000)) {
			 rtc_get_time(RTC, &current_hour, &current_min, &current_sec);
			 if (blink){
				lv_label_set_text_fmt(labelClock, "%02d:%02d", current_hour,current_min);
	
			 } else {
				lv_label_set_text_fmt(labelClock, "%02d %02d", current_hour,current_min);
				
			 }
			 blink = !blink;
			 
		 }
	}
}

static void task_lcd(void *pvParameters) {
	int px, py;

	//lv_ex_btn_1();
	lv_termostato();
	for (;;)  {
		lv_tick_inc(50);
		lv_task_handler();
		vTaskDelay(50);
	}
}

/************************************************************************/
/* configs                                                              */
/************************************************************************/

static void configure_lcd(void) {
	/**LCD pin configure on SPI*/
	pio_configure_pin(LCD_SPI_MISO_PIO, LCD_SPI_MISO_FLAGS);  //
	pio_configure_pin(LCD_SPI_MOSI_PIO, LCD_SPI_MOSI_FLAGS);
	pio_configure_pin(LCD_SPI_SPCK_PIO, LCD_SPI_SPCK_FLAGS);
	pio_configure_pin(LCD_SPI_NPCS_PIO, LCD_SPI_NPCS_FLAGS);
	pio_configure_pin(LCD_SPI_RESET_PIO, LCD_SPI_RESET_FLAGS);
	pio_configure_pin(LCD_SPI_CDS_PIO, LCD_SPI_CDS_FLAGS);
	
	ili9341_init();
	ili9341_backlight_on();
}

static void configure_console(void) {
	const usart_serial_options_t uart_serial_options = {
		.baudrate = USART_SERIAL_EXAMPLE_BAUDRATE,
		.charlength = USART_SERIAL_CHAR_LENGTH,
		.paritytype = USART_SERIAL_PARITY,
		.stopbits = USART_SERIAL_STOP_BIT,
	};

	/* Configure console UART. */
	stdio_serial_init(CONSOLE_UART, &uart_serial_options);

	/* Specify that stdout should not be buffered. */
	setbuf(stdout, NULL);
}

/************************************************************************/
/* port lvgl                                                            */
/************************************************************************/

void my_flush_cb(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p) {
	ili9341_set_top_left_limit(area->x1, area->y1);   ili9341_set_bottom_right_limit(area->x2, area->y2);
	ili9341_copy_pixels_to_screen(color_p,  (area->x2 + 1 - area->x1) * (area->y2 + 1 - area->y1));
	
	/* IMPORTANT!!!
	* Inform the graphics library that you are ready with the flushing*/
	lv_disp_flush_ready(disp_drv);
}

void my_input_read(lv_indev_drv_t * drv, lv_indev_data_t*data) {
	int px, py, pressed;
	
	if (readPoint(&px, &py))
		data->state = LV_INDEV_STATE_PRESSED;
	else
		data->state = LV_INDEV_STATE_RELEASED; 
	
	data->point.x = px;
	data->point.y = py;
}

void configure_lvgl(void) {
	lv_init();
	lv_disp_draw_buf_init(&disp_buf, buf_1, NULL, LV_HOR_RES_MAX * LV_VER_RES_MAX);
	
	lv_disp_drv_init(&disp_drv);            /*Basic initialization*/
	disp_drv.draw_buf = &disp_buf;          /*Set an initialized buffer*/
	disp_drv.flush_cb = my_flush_cb;        /*Set a flush callback to draw to the display*/
	disp_drv.hor_res = LV_HOR_RES_MAX;      /*Set the horizontal resolution in pixels*/
	disp_drv.ver_res = LV_VER_RES_MAX;      /*Set the vertical resolution in pixels*/

	lv_disp_t * disp;
	disp = lv_disp_drv_register(&disp_drv); /*Register the driver and save the created display objects*/
	
	/* Init input on LVGL */
	lv_indev_drv_init(&indev_drv);
	indev_drv.type = LV_INDEV_TYPE_POINTER;
	indev_drv.read_cb = my_input_read;
	lv_indev_t * my_indev = lv_indev_drv_register(&indev_drv);
}

/************************************************************************/
/* main                                                                 */
/************************************************************************/
int main(void) {
	/* board and sys init */
	board_init();
	sysclk_init();
	configure_console();

	/* LCd, touch and lvgl init*/
	configure_lcd();
	configure_touch();
	configure_lvgl();
	xSemaphoreClock = xSemaphoreCreateBinary();

	/* Create task to control oled */
	if (xTaskCreate(task_lcd, "LCD", TASK_LCD_STACK_SIZE, NULL, TASK_LCD_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create lcd task\r\n");
	}
	if (xTaskCreate(task_clock, "clock", TASK_LCD_STACK_SIZE, NULL, TASK_LCD_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create clock task\r\n");
	}
	
	/* Start the scheduler. */
	vTaskStartScheduler();

	while(1){ }
}
