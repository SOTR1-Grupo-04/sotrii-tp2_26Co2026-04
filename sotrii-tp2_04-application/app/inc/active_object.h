#ifndef ACTIVE_OBJECT_H_
#define ACTIVE_OBJECT_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/
// #include "task_led_attribute.h"

/********************** macros ***********************************************/

/********************** typedef **********************************************/
typedef struct active_object_t {
    TaskHandle_t h_task;
    QueueHandle_t h_queue;
    char queue_txt[15];
    char task_txt[15];
} active_object_t;

/********************** external data declaration ****************************/

/********************** external functions declaration ***********************/
// extern void task_led(void *parameters);

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* ACTIVE_OBJECT_H_ */
/********************** end of file ******************************************/
