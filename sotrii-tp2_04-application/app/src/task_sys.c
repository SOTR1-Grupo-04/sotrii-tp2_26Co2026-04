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

/* Application & Tasks includes */
#include "board.h"
#include "app.h"
#include "task_sys_attribute.h"
#include "task_sys.h"
#include "led_active_object.h"

/********************** macros and definitions *******************************/
#define G_TASK_SYS_CNT_INI		0ul

/********************** internal functions declaration ***********************/
static void task_sys_statechart(sys_active_object_t *ao);

/********************** external data declaration ****************************/
uint32_t g_task_sys_cnt;

/********************** internal functions definition ************************/
/** task_sys_statechart emite eventos hacia la queue de LED */
static void task_sys_statechart(sys_active_object_t *ao) {
    switch (ao->sc.state) {
        case ST_SYS_IDLE:
            if (EV_SYS_ON == ao->sc.ev_in) {
                ao->sc.state = ST_SYS_ACTIVE_0;
                ao->sc.tick = ZERO;
                ao->sc.ev_out = EV_SYS_ON;

                led_ao_send(&h_led[LED_A], (void *) &ao->sc.ev_out);
            } else {
                ao->sc.tick += ao->poll_period;
            }
            break;

        case ST_SYS_ACTIVE_0:
            if (EV_SYS_ON == ao->sc.ev_in) {
                ao->sc.state = ST_SYS_ACTIVE_1;
                ao->sc.tick = ZERO;
                ao->sc.ev_out = EV_SYS_BLINK;

                led_ao_send(&h_led[LED_A], (void *) &ao->sc.ev_out);
            } else {
                ao->sc.tick += ao->poll_period;
            }
            break;

        case ST_SYS_ACTIVE_1:
            if (EV_SYS_ON == ao->sc.ev_in) {
                ao->sc.state = ST_SYS_IDLE;
                ao->sc.tick = ZERO;
                ao->sc.ev_out = EV_SYS_OFF;

                led_ao_send(&h_led[LED_A], (void *) &ao->sc.ev_out);
            } else {
                ao->sc.tick += ao->poll_period;
            }
            break;
    }
}

/********************** external functions definition ************************/
/** Bucle principal de la tarea Sys:
 *      - Recibe eventos por la queue de su objeto activo
 *      - Actualiza su statechart y emite eventos hacia la queue de Led
 *      - Espera a al ack (semaforo) del objeto activo antes de procesar el proximo mensaje
 */
void task_sys_gatekeeper(void *parameters) {
    sys_active_object_t *ao = (sys_active_object_t *) parameters;
    sys_event_t event;
    BaseType_t got_event = pdFAIL;
    BaseType_t sync_ack = pdFAIL;

    g_task_sys_cnt = G_TASK_SYS_CNT_INI;

    LOGGER_INFO(" ");
    LOGGER_INFO("  %s is running - Tick [mS] = %lu", ao->ao.task_txt, xTaskGetTickCount());

    for (;;) {
        g_task_sys_cnt++;

        got_event = pdFAIL;
        sync_ack = pdFAIL;

        if (pdPASS == xQueueReceive(ao->ao.h_queue, (void *) &event, (TickType_t) ZERO)) {
            ao->sc.ev_in = event.type;
            ao->sc.tick_out = event.timestamp;
            got_event = pdPASS;
            sync_ack = pdPASS;

            LOGGER_INFO("  %s: ev=%lu ts=%lu",
                        pcTaskGetName(NULL),
                        (uint32_t) event.type,
                        (uint32_t) event.timestamp);
        }

        if (pdFAIL == got_event) {
            ao->sc.ev_in = EV_SYS_NONE;
        }

        task_sys_statechart(ao);

        /* Ack synchronous por medio del semaforo: libera a send_sys_ao() tras run-to-completion. */
        if (pdPASS == sync_ack) {
            xSemaphoreGive(ao->ao_sync_sem);
        }

        vTaskDelay(ao->poll_period); // Polling cada 50ms, definido en SYS_AO_POLL_PERIOD_MS
    }
}

/********************** end of file ******************************************/
