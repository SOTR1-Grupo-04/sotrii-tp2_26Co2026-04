/*
 * Copyright (c) 2026 Sebastian Bedin <sebabedin@gmail.com> &
 * 					  Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>.
 *
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

#include "logger.h"
#include "dwt.h"
#include "app_it.h"
#include "task_btn.h"
#include "task_led.h"
#include "btn_active_object.h"
#include "sys_active_object.h"
#include "led_active_object.h"

uint32_t volatile g_app_tick_cnt;
uint32_t g_task_idle_cnt;
uint32_t g_app_stack_overflow_cnt;

void app_init(void)
{
	led_ev_t event = EV_LED_ON;

	LOGGER_INFO("%s is running - Tick [mS] = %lu",
				GET_NAME(app_init), xTaskGetTickCount());

	/* Cada AO crea su cola y su tarea gatekeeper. */
	configASSERT(pdPASS == open_sys_ao(&sys_ao));

	led_ao_open(&h_led[LED_A]);
	led_ao_open(&h_led[LED_B]);
	led_ao_open(&h_led[LED_C]);
	
	btn_ao_open(&h_btn[BTN_A], sys_ao.ao.h_queue);
	btn_ao_open(&h_btn[BTN_B], sys_ao.ao.h_queue);

	/* Estado inicial: LED_A y LED_B encendidos; LED_C permanece apagado. */
	(void)led_ao_send(&h_led[LED_A], &event);
	(void)led_ao_send(&h_led[LED_B], &event);

	app_it_init();
	cycle_counter_init();
}
