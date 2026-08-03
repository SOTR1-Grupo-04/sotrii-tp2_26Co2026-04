#ifndef BTN_ACTIVE_OBJECT_H_
#define BTN_ACTIVE_OBJECT_H_

#include "task_btn_attribute.h"

void btn_ao_open(h_btn_t *btn_ao, QueueHandle_t sys_queue);

BaseType_t btn_ao_send(h_btn_t *btn_ao, const btn_msg_t *message);

#endif
