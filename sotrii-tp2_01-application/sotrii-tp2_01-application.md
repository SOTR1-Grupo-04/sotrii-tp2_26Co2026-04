# Análisis de Código: RTOS - Sistema Activado por Eventos (ETS)

Este documento detalla el análisis de la arquitectura y el funcionamiento del código fuente del sistema embebido basado en FreeRTOS.

## Arquitectura General del Sistema

El flujo de información del sistema sigue una arquitectura lineal de tipo "Productor-Consumidor", donde las tareas reaccionan a eventos físicos, procesan la lógica y generan salidas visuales:

1. **Entrada (`task_btn.c`)**: Lee el hardware físico (botones) y envía un evento a la cola del sistema.
2. **Lógica Central (`task_sys.c`)**: Recibe el evento del botón, actualiza el estado lógico del sistema y envía un comando de acción a la cola de los LEDs.
3. **Salida (`task_led.c`)**: Recibe el comando del sistema y controla el hardware físico (LEDs) para encenderlos, apagarlos o hacerlos parpadear.

---

## Análisis por Componentes

### 1. Configuración Principal: `app.c`
Este archivo es el corazón de la configuración de la aplicación.
* **Inicialización**: La función `app_init` configura y arranca la aplicación.
* **Creación de Colas**: Se crean explícitamente dos colas de mensajes de FreeRTOS para intercomunicar las tareas: `h_sys_task_q` (de Botón a Sistema) y `h_led_task_q` (de Sistema a LED).
* **Creación de Tareas**: Se instancian las tareas `task_a`, `task_b`, `task_led`, `task_sys` y `task_btn`.
* **Prioridades**: Las tareas A y B tienen prioridad 2, mientras que las tareas de LED, Sistema y Botón tienen prioridad 1.

### 2. Cabeceras de Atributos: `task_*_attribute.h`
Estos archivos definen las estructuras de datos y estados para mantener el código organizado y modular.
* **Herencia de Eventos**: Un detalle de diseño interesante es cómo los eventos se mapean de un módulo a otro. Por ejemplo, en `task_sys_attribute.h`, el evento de sistema apagado (`EV_SYS_OFF`) se asocia directamente a que el botón no esté presionado (`EV_BTN_UP`), y `EV_SYS_ON` a `EV_BTN_DOWN`. De manera similar, los eventos del LED (`EV_LED_OFF`, `EV_LED_ON`, `EV_LED_BLINK`) se mapean a los eventos del sistema.
* **Máquinas de Estados**: Cada módulo define enums para sus estados, como `ST_SYS_IDLE`, `ST_SYS_ACTIVE_0` y `ST_SYS_ACTIVE_1` para el sistema.
* **Estructuras**: Contienen las variables necesarias para gestionar los puertos GPIO, el estado de los pines y el seguimiento de *ticks* (tiempo) de las máquinas de estado.

### 3. Lectura de Entradas: `task_btn.c`
Se encarga de monitorear el estado de los botones (entradas físicas).
* **Bucle Principal**: La tarea se ejecuta en un bucle infinito que verifica el estado del hardware cada 50 milisegundos (`DEL_BTN_MIN`).
* **Máquina de Estados (`task_btn_statechart`)**: Evalúa si el botón está en estado `ST_BTN_UP` o `ST_BTN_DOWN`.
* **Generación de Eventos**: Cuando detecta una transición (por ejemplo, el botón se presiona y cambia a `EV_BTN_DOWN`), envía este evento a la cola del sistema (`h_sys_task_q`) usando la API `xQueueSend` de FreeRTOS.

### 4. Lógica de Control: `task_sys.c`
Actúa como el "cerebro" central de esta interacción.
* **Recepción de Eventos**: Se bloquea de forma no bloqueante (con un *timeout* de 0) leyendo la cola `h_sys_task_q`. Si no hay eventos, asume `EV_SYS_NONE`.
* **Transiciones Lógicas**: Implementa una máquina de estados secuencial en `task_sys_statechart`.
    * Si está en `ST_SYS_IDLE` y recibe `EV_SYS_ON`, avanza a `ST_SYS_ACTIVE_0` y manda a encender los LEDs (`EV_SYS_ON`) a través de la cola `h_led_task_q`.
    * Si está en `ST_SYS_ACTIVE_0` y recibe `EV_SYS_ON`, avanza a `ST_SYS_ACTIVE_1` y manda a parpadear los LEDs (`EV_SYS_BLINK`).
    * Si está en `ST_SYS_ACTIVE_1` y recibe `EV_SYS_ON`, vuelve a `ST_SYS_IDLE` y apaga los LEDs (`EV_SYS_OFF`).

### 5. Manejo de Salidas: `task_led.c`
Es la tarea encargada de traducir los comandos lógicos a señales de hardware.
* **Recepción y Retardo**: Al igual que las otras tareas, lee su cola (`h_led_task_q`) e itera cada 50 milisegundos (`DEL_LED_MIN`).
* **Acciones Físicas**: Dependiendo del estado ordenado, utiliza las funciones de la capa de abstracción de hardware (HAL) como `HAL_GPIO_WritePin` para encender o apagar el pin, o `HAL_GPIO_TogglePin` para invertir su estado.
* **Lógica de Parpadeo (`ST_LED_BLINK`)**: Maneja internamente el tiempo de parpadeo restando de a 50ms al temporizador hasta llegar a 0. Cuando esto ocurre, invierte el estado del LED y reinicia el temporizador a 500ms (`DEL_LED_BLINK`)