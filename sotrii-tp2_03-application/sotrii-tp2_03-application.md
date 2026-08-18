# Actividad 03 - Active Object BTN

## Descripción

En esta actividad se implementó un Active Object para gestionar un botón mediante FreeRTOS.

El botón se consulta periódicamente mediante polling. La tarea `task_btn` funciona como Gatekeeper, ya que es la encargada de acceder al GPIO, ejecutar la máquina de estados y generar los eventos correspondientes.

El Active Object BTN envía hacia el sistema un mensaje compuesto por:

- Identificador del botón.
- Evento detectado.
- Tiempo asociado al evento.

```c
typedef struct
{
    btn_id_t id;
    btn_ev_t event;
    TickType_t time;
} btn_msg_t;
````

## Arquitectura

El flujo de información implementado es:

```
GPIO del botón
      |
      v
task_btn
      |
      | btn_msg_t
      v
Cola BTN -> SYS
      |
      v
task_sys
```

La comunicación entre BTN y SYS es asincrónica y se realiza mediante una cola de FreeRTOS.

## Estructuras utilizadas

### Active Object genérico

```c
typedef struct active_object_t
{
    TaskHandle_t h_task;
    QueueHandle_t h_queue;
} active_object_t;
```

La estructura contiene:

* `h_task`: handle de la tarea asociada al Active Object.
* `h_queue`: cola de entrada propia del Active Object, cuando corresponda.

### Handle del botón

```c
typedef struct
{
    btn_t *btn;
    btn_sc_t *btn_sc;
    active_object_t *ao;
    QueueHandle_t sys_queue;
} h_btn_t;
```

La cola `sys_queue` es una referencia a la cola de entrada del Active Object SYS. El botón utiliza esta cola para enviar eventos.

## Máquina de estados del botón

La máquina de estados tiene dos estados:

```
ST_BTN_UP
ST_BTN_DOWN
```

### Transiciones

```
ST_BTN_UP + EV_BTN_DOWN
    -> ST_BTN_DOWN
    -> envía evento de botón presionado

ST_BTN_DOWN + EV_BTN_UP
    -> ST_BTN_UP
    -> envía evento de botón liberado y duración
```

Mientras el botón permanece presionado, la tarea acumula tiempo en intervalos de 50 ms.

## Funciones de interfaz

### `btn_ao_open()`

Inicializa el Active Object BTN y crea su tarea de FreeRTOS.

```c
void btn_ao_open(
    h_btn_t *btn_ao,
    QueueHandle_t sys_queue
);
```

La función:

1. Guarda una referencia a la cola del sistema.
2. Crea la tarea `task_btn`.
3. Guarda el handle de la tarea en `btn_ao->ao->h_task`.

### `btn_ao_send()`

Envía un mensaje generado por el botón hacia la cola del sistema.

```c
BaseType_t btn_ao_send(
    h_btn_t *btn_ao,
    const btn_msg_t *message
);
```

La operación se realiza sin bloqueo. Si la cola está llena, la función devuelve `pdFAIL`.

### `btn_ao_release()`

Elimina la tarea asociada al Active Object BTN.

```c
void btn_ao_release(
    h_btn_t *btn_ao
);
```

La función no elimina la cola del sistema porque dicha cola pertenece al Active Object SYS.

## Gestión del periférico

La gestión del botón se realiza mediante polling:

```c
HAL_GPIO_ReadPin(
    p_h_btn->btn->gpio_port,
    p_h_btn->btn->pin
);
```

La tarea se ejecuta periódicamente cada 50 ms:

```c
vTaskDelay(pdMS_TO_TICKS(50));
```

Esta tarea es el único componente que accede directamente al GPIO del botón.

---

## Pruebas realizadas

Se verificó el siguiente comportamiento:

1. Al presionar el botón se genera un evento `EV_BTN_DOWN`.
2. Al liberar el botón se genera un evento `EV_BTN_UP`.
3. El mensaje enviado contiene el identificador, el evento y el tiempo.
4. SYS recibe correctamente el mensaje.
5. La aplicación conserva el comportamiento original del LED:

   * Primera pulsación: LED encendido.
   * Segunda pulsación: LED titilando.
   * Tercera pulsación: LED apagado.

Ejemplo de salida observada:

```
BTN id=0 event=1 time=0 ms
BTN id=0 event=0 time=650 ms
```

## Decisiones de diseño

* Patrón de comunicación: asincrónico.
* Gestión del botón: polling.
* Período de polling: 50 ms.
* Comunicación BTN -> SYS: cola de FreeRTOS.
* Asignación de la cola: dinámica mediante `xQueueCreate`.
* Tarea Gatekeeper: `task_btn`.
* El Active Object BTN no crea la cola de SYS; solamente conserva una referencia a ella.
* La cola pertenece al receptor, es decir, al Active Object SYS.
