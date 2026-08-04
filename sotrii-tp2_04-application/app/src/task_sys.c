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
#define TASK_SYS_LOG_TAG		"Tarea Sys"
#define TASK_SYS_LOG(fmt, ...) \
	LOGGER_INFO("  " TASK_SYS_LOG_TAG ": " fmt, ##__VA_ARGS__)

static BaseType_t timeoutA_ms = 5000;
static BaseType_t timeoutB_ms = 10000;

/********************** internal functions declaration ***********************/
static const char *sys_id_str(sys_id_t id);
static const char *sys_ev_str(sys_ev_t type);
static void task_sys_statechart(sys_active_object_t *ao);
static void transitionToIdle(sys_active_object_t *ao);

/********************** external data declaration ****************************/
uint32_t g_task_sys_cnt;

/********************** internal functions definition ************************/
static const char *sys_id_str(sys_id_t id) {
    switch (id) {
        case SYS_ID_BTN_A:
            return "BTN_A";
        case SYS_ID_BTN_B:
            return "BTN_B";
        default:
            return "NONE";
    }
}

static const char *sys_ev_str(sys_ev_t type) {
    switch (type) {
        case EV_SYS_ON:
            return "ON";
        case EV_SYS_OFF:
            return "OFF";
        default:
            return "-";
    }
}

/** task_sys_statechart emite eventos hacia la queue de LED */
static void task_sys_statechart(sys_active_object_t *ao) {
    led_ev_t led_event;
    
    switch (ao->sc.state) {
        case ST_SYS_IDLE:
            if (EV_SYS_ON == ao->sc.ev_in.type) {
                if (SYS_ID_BTN_A == ao->sc.ev_in.id) {
                    ao->sc.state = ST_SYS_BTN_A_PRESSED;
                    TASK_SYS_LOG("op BTN_A start (T_A=%lu)", (uint32_t) timeoutA_ms);
                    led_event = EV_LED_BLINK;
                    (void)led_ao_send(&h_led[LED_A], &led_event);
                    led_event = EV_LED_OFF;
                    (void)led_ao_send(&h_led[LED_B], &led_event);
                    led_event = EV_LED_ON;
                    (void)led_ao_send(&h_led[LED_C], &led_event);
                    ao->sc.tick = 0;
                    ao->sc.en_transition = false;

                } else if (SYS_ID_BTN_B == ao->sc.ev_in.id) {
                    ao->sc.state = ST_SYS_BTN_B_PRESSED;
                    TASK_SYS_LOG("op BTN_B start (T_B=%lu)", (uint32_t) timeoutB_ms);
                    led_event = EV_LED_OFF;
                    (void)led_ao_send(&h_led[LED_A], &led_event);
                    led_event = EV_LED_BLINK;
                    (void)led_ao_send(&h_led[LED_B], &led_event);
                    led_event = EV_LED_ON;
                    (void)led_ao_send(&h_led[LED_C], &led_event);
                    ao->sc.tick = 0;
                    ao->sc.en_transition = false;
                }
                
            }
            break;

        case ST_SYS_BTN_A_PRESSED:
            ao->sc.tick += ao->poll_period;

            if (EV_SYS_OFF == ao->sc.ev_in.type && SYS_ID_BTN_A == ao->sc.ev_in.id) {
                ao->sc.en_transition = true;
                if (ao->sc.ev_in.timestamp >= timeoutA_ms) {
                    timeoutA_ms = ao->sc.ev_in.timestamp;
                    TASK_SYS_LOG("BTN_A OFF long ts=%lu T_A=%lu",
                                 (uint32_t) ao->sc.ev_in.timestamp,
                                 (uint32_t) timeoutA_ms);
                    transitionToIdle(ao);
                } else {
                    TASK_SYS_LOG("BTN_A OFF short ts=%lu (wait T_A)",
                                 (uint32_t) ao->sc.ev_in.timestamp);
                    break;
                }
            }

            if (ao->sc.en_transition && ao->sc.tick >= timeoutA_ms) {
                TASK_SYS_LOG("BTN_A timeout tick=%lu", (uint32_t) ao->sc.tick);
                transitionToIdle(ao);
            }
            break;

        case ST_SYS_BTN_B_PRESSED:
            ao->sc.tick += ao->poll_period;

            if (EV_SYS_OFF == ao->sc.ev_in.type && SYS_ID_BTN_B == ao->sc.ev_in.id) {
                ao->sc.en_transition = true;
                if (ao->sc.ev_in.timestamp >= timeoutB_ms) {
                    timeoutB_ms = ao->sc.ev_in.timestamp;
                    TASK_SYS_LOG("BTN_B OFF long ts=%lu T_B=%lu",
                                 (uint32_t) ao->sc.ev_in.timestamp,
                                 (uint32_t) timeoutB_ms);
                    transitionToIdle(ao);
                } else {
                    TASK_SYS_LOG("BTN_B OFF short ts=%lu (wait T_B)",
                                 (uint32_t) ao->sc.ev_in.timestamp);
                    break;
                }
            }

            if (ao->sc.en_transition && ao->sc.tick >= timeoutB_ms) {
                TASK_SYS_LOG("BTN_B timeout tick=%lu", (uint32_t) ao->sc.tick);
                transitionToIdle(ao);
            }
            break;
    }
}

static void transitionToIdle(sys_active_object_t *ao) {
    led_ev_t led_event;

    TASK_SYS_LOG("-> IDLE");
    ao->sc.state = ST_SYS_IDLE;
    led_event = EV_LED_ON;
    (void)led_ao_send(&h_led[LED_A], &led_event);
    (void)led_ao_send(&h_led[LED_B], &led_event);
    led_event = EV_LED_OFF;
    (void)led_ao_send(&h_led[LED_C], &led_event);
    ao->sc.en_transition = true;
}

/********************** external functions definition ************************/
/** Bucle principal de la tarea Sys:
 *      - Recibe eventos por la queue de su objeto activo
 *      - Actualiza su statechart y emite eventos hacia la queue de Led
 */
void task_sys_gatekeeper(void *parameters) {
    sys_active_object_t *ao = (sys_active_object_t *) parameters;
    sys_event_t event;
    BaseType_t got_event = pdFAIL;

    g_task_sys_cnt = G_TASK_SYS_CNT_INI;

    LOGGER_INFO(" ");
    TASK_SYS_LOG("en ejecucion - Tick [mS] = %lu", xTaskGetTickCount());

    for (;;) {
        g_task_sys_cnt++;

        got_event = pdFAIL;

        if (pdPASS == xQueueReceive(ao->ao.h_queue, (void *) &event, (TickType_t) ZERO)) {
            ao->sc.ev_in = event;
            ao->sc.tick_out = event.timestamp;
            got_event = pdPASS;

            TASK_SYS_LOG("%s %s ts=%lu",
                         sys_id_str(event.id),
                         sys_ev_str(event.type),
                         (uint32_t) event.timestamp);
        }

        if (pdFAIL == got_event) {
            ao->sc.ev_in.id = SYS_ID_NONE;
            ao->sc.ev_in.type = EV_SYS_NONE;
            ao->sc.ev_in.timestamp = ZERO;
        }

        task_sys_statechart(ao);

        vTaskDelay(pdMS_TO_TICKS(ao->poll_period)); // Polling cada 50ms, definido en SYS_AO_POLL_PERIOD_MS
    }
}

/********************** end of file ******************************************/
