#ifndef LED_ACTIVE_OBJECT_H_
#define LED_ACTIVE_OBJECT_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/
#include "active_object.h"
#include "task_led_attribute.h"

/********************** macros ***********************************************/

/********************** typedef **********************************************/

/********************** external data declaration ****************************/
 extern h_led_t h_led[LED_QTY];

/********************** external functions declaration ***********************/
void led_ao_open(h_led_t * led_ao);
void led_ao_release(h_led_t * led_ao);
BaseType_t led_ao_send(h_led_t * led_ao, void *event_);

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* LED_ACTIVE_OBJECT_H_ */
/********************** end of file ******************************************/
