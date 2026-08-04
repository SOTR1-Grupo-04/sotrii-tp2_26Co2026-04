#ifndef BTN_ACTIVE_OBJECT_H_
#define BTN_ACTIVE_OBJECT_H_

#include "task_btn_attribute.h"
#include "task_sys_attribute.h"

void btn_ao_open(h_btn_t *btn_ao, QueueHandle_t sys_queue);

BaseType_t btn_ao_send(h_btn_t *btn_ao, const sys_event_t *message);

void btn_ao_release(h_btn_t *btn_ao);

#endif
