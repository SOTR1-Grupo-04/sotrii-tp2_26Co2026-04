#include "main.h"
#include "cmsis_os.h"

#include "task_btn.h"
#include "btn_active_object.h"

void btn_ao_open(h_btn_t *btn_ao, QueueHandle_t sys_queue)
{
    BaseType_t ret;

    btn_ao->sys_queue = sys_queue;

    btn_ao->ao->h_queue = NULL;

    ret = xTaskCreate(
        task_btn,
        "Task Btn     ",
        configMINIMAL_STACK_SIZE,
        (void *)btn_ao,
        tskIDLE_PRIORITY + 1ul,
        &btn_ao->ao->h_task);

    configASSERT(pdPASS == ret);
}
