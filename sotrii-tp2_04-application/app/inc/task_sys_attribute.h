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

#ifndef TASK_SYS_ATTRIBUTE_H_
#define TASK_SYS_ATTRIBUTE_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/
#include "cmsis_os.h"

#include "active_object.h"
#include "task_btn_attribute.h"

/********************** macros ***********************************************/
#define SYS_AO_QUEUE_LENGTH		(5u)
#define SYS_AO_POLL_PERIOD_MS	(50ul)

/********************** typedef **********************************************/
/* Events of Statechart */

typedef enum sys_id {
    SYS_ID_NONE = 0,
    SYS_ID_BTN_A,
    SYS_ID_BTN_B,
} sys_id_t;
typedef enum sys_ev {
    EV_SYS_OFF = EV_BTN_UP,
    EV_SYS_ON = EV_BTN_DOWN,
    EV_SYS_BLINK,
    EV_SYS_NONE,
} sys_ev_t;

/* States of Statechart */
typedef enum sys_st {
    ST_SYS_IDLE, 
    ST_SYS_BTN_A_PRESSED,
    ST_SYS_BTN_B_PRESSED,
} sys_st_t;

/* ioctl commands - Solo uno de referencia ya que no se pide ninguno en particular */
typedef enum sys_ao_ioctl {
    SYS_AO_IOCTL_GET_STATE = 0ul
} sys_ao_ioctl_t;

/* Event notification: debe recibir evento + tiempo */
typedef struct {
    sys_id_t id;
    sys_ev_t type;
    TickType_t timestamp;
} sys_event_t;

/* Structure of Statechart */
typedef struct {
    sys_st_t state;
    sys_ev_t ev_in;
    TickType_t tick;
    sys_ev_t ev_out;
    TickType_t tick_out;
} sys_statechart_t;

/* Sys AO: "hereda" del active_object_t + logica propia */
typedef struct {
    active_object_t ao;
    sys_statechart_t sc;
    TickType_t poll_period;
} sys_active_object_t;

/********************** external data declaration ****************************/

/********************** external functions declaration ***********************/

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* TASK_SYS_ATTRIBUTE_H_ */

/********************** end of file ******************************************/
