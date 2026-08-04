/*
 * Copyright (c) 2026 Sebastian Bedin <sebabedin@gmail.com> &
 * 					  Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @author : Sebastian Bedin <sebabedin@gmail.com> &
 * 			 Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>
 */

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
#include "task_led_attribute.h"

/********************** macros and definitions *******************************/
#define G_TASK_LED_CNT_INI	0ul

#define DEL_LED_MIN			(pdMS_TO_TICKS(50ul))
#define DEL_LED_BLINK		(pdMS_TO_TICKS(500ul))

#define TASK_LED_DEL_ZERO	(pdMS_TO_TICKS(0ul))
#define TASK_LED_DEL_MAX	DEL_LED_MIN
#define TASK_LED_LOG(h_led_, fmt, ...) \
	LOGGER_INFO("  %s: " fmt, (h_led_)->ao->task_txt, ##__VA_ARGS__)

/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/
static const char *led_ev_str(led_ev_t ev);
static void led_log_action(h_led_t *h_led_, const char *action);
void task_led_statechart(h_led_t *h_led_);

/********************** internal data definition *****************************/

/********************** external data declaration ****************************/
uint32_t g_task_led_cnt;

/********************** internal functions definition ************************/
static const char *led_ev_str(led_ev_t ev) {
	switch (ev) {
		case EV_LED_OFF:
			return "APAGADO";
		case EV_LED_ON:
			return "ENCENDIDO";
		case EV_LED_BLINK:
			return "PARPADEO";
		case EV_LED_NONE:
			return "NINGUNO";
		default:
			return "?";
	}
}

static void led_log_action(h_led_t *h_led_, const char *action)
{
	TASK_LED_LOG(h_led_, "%s", action);
}

/********************** external functions definition ************************/
/* Task thread */
void task_led(void *parameters)
{
	/*  Declare & Initialize Task Function variables */
	g_task_led_cnt = G_TASK_LED_CNT_INI;
	h_led_t *p_h_led = (h_led_t *)parameters;

	/* Print out: Task Initialized */
	LOGGER_INFO(" ");
	TASK_LED_LOG(p_h_led, "en ejecucion - Tick [mS] = %lu", xTaskGetTickCount());

	/* As per most tasks, this task is implemented in an infinite loop. */
	for (;;)
    {
		/* Update Task Counter */
		g_task_led_cnt++;

		/* Get Events to excite Statechart */
		if (pdPASS == xQueueReceive(p_h_led->ao->h_queue, (void *)&p_h_led->led_sc->ev_in, (TickType_t)ZERO))
		{
			TASK_LED_LOG(p_h_led, "ev=%lu %s",
						 (uint32_t) p_h_led->led_sc->ev_in,
						 led_ev_str(p_h_led->led_sc->ev_in));
		}
		else
		{
			p_h_led->led_sc->ev_in = EV_LED_NONE;
		}

		/* Run Statechart */
    	task_led_statechart(p_h_led);

    	/* We want this task to execute every 50 milliseconds. */
		vTaskDelay(TASK_LED_DEL_MAX);
	}
}

void task_led_statechart(h_led_t *h_led_)
{
	switch (h_led_->led_sc->state)
	{
		case ST_LED_OFF:
		case ST_LED_ON:

			switch (h_led_->led_sc->ev_in)
			{
				case EV_LED_OFF:

					h_led_->led_sc->state = ST_LED_OFF;
					h_led_->led->pin_state = LED_OFF;
					h_led_->led_sc->tick = ZERO;

					HAL_GPIO_WritePin(h_led_->led->gpio_port, h_led_->led->pin, h_led_->led->pin_state);

					led_log_action(h_led_, "apagado");

					break;

				case EV_LED_ON:

					h_led_->led_sc->state = ST_LED_ON;
					h_led_->led->pin_state = LED_ON;
					h_led_->led_sc->tick = ZERO;

					HAL_GPIO_WritePin(h_led_->led->gpio_port, h_led_->led->pin, h_led_->led->pin_state);

					led_log_action(h_led_, "encendido");

					break;

				case EV_LED_BLINK:

					h_led_->led_sc->state = ST_LED_BLINK;
					h_led_->led->pin_state = HAL_GPIO_ReadPin(h_led_->led->gpio_port, h_led_->led->pin);
					h_led_->led_sc->tick = DEL_LED_BLINK;

					HAL_GPIO_TogglePin(h_led_->led->gpio_port, h_led_->led->pin);

					led_log_action(h_led_, "inicio parpadeo");

					break;

				case EV_LED_NONE:

					break;
			}

			break;

		case ST_LED_BLINK:

			switch (h_led_->led_sc->ev_in)
			{
				case EV_LED_OFF:

					h_led_->led_sc->state = ST_LED_OFF;
					h_led_->led->pin_state = LED_OFF;
					h_led_->led_sc->tick = ZERO;

					HAL_GPIO_WritePin(h_led_->led->gpio_port, h_led_->led->pin, h_led_->led->pin_state);

					led_log_action(h_led_, "apagado");

					break;

				case EV_LED_ON:

					h_led_->led_sc->state = ST_LED_ON;
					h_led_->led->pin_state = LED_ON;
					h_led_->led_sc->tick = ZERO;

					HAL_GPIO_WritePin(h_led_->led->gpio_port, h_led_->led->pin, h_led_->led->pin_state);

					led_log_action(h_led_, "encendido");

					break;

				case EV_LED_BLINK:
				case EV_LED_NONE:

					h_led_->led_sc->state = ST_LED_BLINK;
					h_led_->led_sc->tick -= DEL_LED_MIN;

					if (ZERO == h_led_->led_sc->tick)
					{
						h_led_->led->pin_state = HAL_GPIO_ReadPin(h_led_->led->gpio_port, h_led_->led->pin);
						h_led_->led_sc->tick = DEL_LED_BLINK;

						HAL_GPIO_TogglePin(h_led_->led->gpio_port, h_led_->led->pin);

						led_log_action(h_led_, "parpadeo");
					}

					break;
			}

			break;
	}
}

/********************** end of file ******************************************/
