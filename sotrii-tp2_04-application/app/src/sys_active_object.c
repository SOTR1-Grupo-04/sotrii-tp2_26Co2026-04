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
#include "main.h"
#include "cmsis_os.h"
#include "logger.h"
#include "dwt.h"

#include "board.h"
#include "app.h"
#include "task_sys_attribute.h"
#include "task_sys.h"
#include "sys_active_object.h"

/********************** macros and definitions *******************************/
#define TASK_SYS_STACK			(configMINIMAL_STACK_SIZE)
#define TASK_SYS_PRIORITY		(tskIDLE_PRIORITY + 1ul)

#define SEND_SYS_AO_TIMEOUT		(portMAX_DELAY)

#define SYS_AO_WCET_US(t0)		((cycle_counter_get() - (t0)) / (SystemCoreClock / 1000000ul))

/********************** internal data declaration ****************************/
static StaticQueue_t sys_ao_queue_cb;
static uint8_t sys_ao_queue_storage[SYS_AO_QUEUE_LENGTH * sizeof(sys_event_t)];

/* Variables para calcular WCET [us] en las distintas funciones */
uint32_t g_sys_ao_open_us;
uint32_t g_sys_ao_release_us;
uint32_t g_sys_ao_send_us;
uint32_t g_sys_ao_ioctl_us;

/********************** internal functions declaration ***********************/
static BaseType_t create_sys_ao_gatekeeper_task(sys_active_object_t *ao);

/********************** external data declaration ****************************/
sys_active_object_t sys_ao = {
    .ao = {
        .h_task = NULL,
        .h_queue = NULL,
        .queue_txt = "Queue SYS AO",
        .task_txt = "Tarea Sys    ",
    },
    .sc = {
        .state = ST_SYS_IDLE,
        .ev_in = {
            .id = SYS_ID_NONE,
            .type = EV_SYS_NONE,
            .timestamp = ZERO,
        },
        .tick = ZERO,
        .ev_out = {
            .id = SYS_ID_NONE,
            .type = EV_SYS_NONE,
            .timestamp = ZERO,
        },
        .tick_out = ZERO,
        .en_transition = true,
    },
    .poll_period = pdMS_TO_TICKS(SYS_AO_POLL_PERIOD_MS),
};

/********************** internal functions definition ************************/
static BaseType_t create_sys_ao_gatekeeper_task(sys_active_object_t *ao) {
    BaseType_t ret;

    if ((NULL == ao) || (NULL != ao->ao.h_task)) {
        return pdFAIL;
    }

    ret = xTaskCreate(task_sys_gatekeeper, ao->ao.task_txt,
                      TASK_SYS_STACK, (void *) ao,
                      TASK_SYS_PRIORITY, &ao->ao.h_task);

    return ret;
}

/********************** external functions definition ************************/
BaseType_t open_sys_ao(sys_active_object_t *ao) {
    uint32_t t0 = cycle_counter_get();

    if (NULL == ao) {
        return pdFAIL;
    }

    ao->sc.state = ST_SYS_IDLE;
    ao->sc.ev_in = (sys_event_t) {
        .id = SYS_ID_NONE,
        .type = EV_SYS_NONE,
        .timestamp = ZERO,
    };
    ao->sc.tick = ZERO;
    ao->sc.ev_out = (sys_event_t) {
        .id = SYS_ID_NONE,
        .type = EV_SYS_NONE,
        .timestamp = ZERO,
    };
    ao->sc.tick_out = ZERO;
    ao->poll_period = pdMS_TO_TICKS(SYS_AO_POLL_PERIOD_MS);

    // Queue el AO
    ao->ao.h_queue = xQueueCreateStatic(SYS_AO_QUEUE_LENGTH,
                                        sizeof(sys_event_t), sys_ao_queue_storage, &sys_ao_queue_cb);
    configASSERT(NULL != ao->ao.h_queue);
    vQueueAddToRegistry(ao->ao.h_queue, ao->ao.queue_txt);

    // Creacion de la tarea
    configASSERT(pdPASS == create_sys_ao_gatekeeper_task(ao));

    g_sys_ao_open_us = SYS_AO_WCET_US(t0);

    LOGGER_INFO("  %s: queue=%p task=%p wcet=%lu us", GET_NAME(open_sys_ao),
                ao->ao.h_queue, ao->ao.h_task, g_sys_ao_open_us);

    return pdPASS;
}

BaseType_t release_sys_ao(sys_active_object_t *ao) {
    uint32_t t0 = cycle_counter_get();

    if (NULL == ao) {
        return pdFAIL;
    }

    if (NULL != ao->ao.h_task) {
        vTaskDelete(ao->ao.h_task);
        ao->ao.h_task = NULL;
    }

    if (NULL != ao->ao.h_queue) {
        vQueueDelete(ao->ao.h_queue);
        ao->ao.h_queue = NULL;
    }

    g_sys_ao_release_us = SYS_AO_WCET_US(t0);

    return pdPASS;
}

BaseType_t send_sys_ao(sys_active_object_t *ao, const sys_event_t *event) {
    uint32_t t0 = cycle_counter_get();

    if ((NULL == ao) || (NULL == event) || (NULL == ao->ao.h_queue)) {
        return pdFAIL;
    }

    if (pdPASS != xQueueSend(ao->ao.h_queue, (const void *) event,
                             SEND_SYS_AO_TIMEOUT)) {
        return pdFAIL;
    }

    g_sys_ao_send_us = SYS_AO_WCET_US(t0);

    return pdPASS;
}

BaseType_t ioctl_sys_ao(sys_active_object_t *ao, uint32_t cmd, void *arg) {
    uint32_t t0 = cycle_counter_get();
    BaseType_t ret = pdPASS;

    if (NULL == ao) {
        return pdFAIL;
    }

    switch ((sys_ao_ioctl_t) cmd) {
        // Solo un comando para obtener el statechart
        case SYS_AO_IOCTL_GET_STATE:
            if (NULL == arg) {
                ret = pdFAIL;
                break;
            }

            taskENTER_CRITICAL();
            *(sys_st_t *) arg = ao->sc.state;
            taskEXIT_CRITICAL();
            break;

        default:
            ret = pdFAIL;
            break;
    }

    g_sys_ao_ioctl_us = SYS_AO_WCET_US(t0);

    return ret;
}

/********************** end of file ******************************************/
