/********************** inclusions *******************************************/
/* Project includes */
#include "main.h"
#include "cmsis_os.h"

/* Demo includes */
#include "logger.h"
#include "dwt.h"

/* Application & Tasks includes */
#include "board.h"
#include "app.h"
#include "task_led.h"
#include "task_led_attribute.h"
#include "led_active_object.h"

/********************** macros and definitions *******************************/
#define G_TASK_LED_CNT_INI	0ul

#define DEL_LED_MIN			(pdMS_TO_TICKS(50ul))
#define DEL_LED_BLINK		(pdMS_TO_TICKS(500ul))

#define TASK_LED_DEL_ZERO	(pdMS_TO_TICKS(0ul))
#define TASK_LED_DEL_MAX	DEL_LED_MIN

#define QUEUE_LENGTH__		(1)
#define QUEUE_ITEM_SIZE__	(sizeof(led_ev_t))

/********************** internal data declaration ****************************/
led_t led_hw[LED_QTY] = {{LED_A, LED_A_PORT, LED_A_PIN, LED_A_OFF},
			     	  {LED_B, LED_B_PORT, LED_B_PIN, LED_B_OFF},
					  {LED_C, LED_C_PORT, LED_C_PIN, LED_C_OFF}};

led_sc_t led_sc[LED_QTY] = {{ST_LED_OFF, EV_LED_NONE, ZERO},
							{ST_LED_OFF, EV_LED_NONE, ZERO},
							{ST_LED_OFF, EV_LED_NONE, ZERO}};

active_object_t led_ao[LED_QTY] = {{NULL, NULL, "Cola led A", "Tarea led A"},
									{NULL, NULL, "Cola led B", "Tarea led B"},
									{NULL, NULL, "Cola led C", "Tarea led C"}};

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

/********************** external data declaration ****************************/
h_led_t h_led[LED_QTY] = {{&led_hw[LED_A], &led_sc[LED_A], &led_ao[LED_A]},
				    	  {&led_hw[LED_B], &led_sc[LED_B], &led_ao[LED_B]},
					  	  {&led_hw[LED_C], &led_sc[LED_C], &led_ao[LED_C]}};

/********************** external functions definition ************************/
void led_ao_open(h_led_t * led_ao) {
	BaseType_t ret;

	led_ao->ao->h_queue = xQueueCreate(QUEUE_LENGTH__, QUEUE_ITEM_SIZE__);
	configASSERT(NULL != led_ao->ao->h_queue);
	vQueueAddToRegistry(led_ao->ao->h_queue, led_ao->ao->queue_txt);

	ret = xTaskCreate(task_led,
					  led_ao->ao->task_txt,
					  configMINIMAL_STACK_SIZE,
					  (void *) led_ao,
					  tskIDLE_PRIORITY + 1ul,
					  &led_ao->ao->h_task);

	configASSERT(pdPASS == ret);
}

void led_ao_release(h_led_t * led_ao) {
	vQueueUnregisterQueue(led_ao->ao->h_queue);
	vQueueDelete(led_ao->ao->h_queue);
	vTaskDelete(led_ao->ao->h_task);
}

BaseType_t led_ao_send(h_led_t * led_ao, void *event_) {
	return xQueueSend((QueueHandle_t) led_ao->ao->h_queue, event_, 0);
}

/********************** end of file ******************************************/
