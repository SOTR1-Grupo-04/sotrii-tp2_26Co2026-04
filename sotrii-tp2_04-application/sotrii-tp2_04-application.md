## TP2 - Actividad 04 - Active Objects Led y Btn

## Objetivo

Integrar los Active Objects **Led** (uno por LED) y **Btn** (uno por boton) con el coordinador **Sys**, de modo que la
logica de operacion dependa de **que boton** se presiono y de **cuanto tiempo** se mantuvo apretado.

---

## Arquitectura

La aplicacion cuenta con **5 Active Objects**, cada uno con su tarea y (cuando corresponde) su cola:

| Active Object | Tarea       | Rol                                   |
|---------------|-------------|---------------------------------------|
| **Sys**       | Tarea Sys   | Statechart central; coordina los LEDs |
| **Btn A**     | Tarea Btn A | Lee B1 y envia eventos a Sys          |
| **Btn B**     | Tarea Btn B | Lee D7 y envia eventos a Sys          |
| **Led A**     | Tarea led A | Controla LD2                          |
| **Led B**     | Tarea led B | Controla D12                          |
| **Led C**     | Tarea led C | Controla D11                          |

**Flujo de eventos:**

1. Cada tarea Btn detecta transiciones UP/DOWN y encola en la cola de Sys un `sys_event_t` con
   `{ id, type, timestamp }`.
2. La tarea Sys (gatekeeper) recibe el evento, ejecuta su statechart y encola comandos `led_ev_t` en la cola de cada
   Led.
3. Cada tarea Led aplica la accion sobre su pin GPIO (encender, apagar o parpadear).

El evento hacia Sys incluye el **identificador del boton** (`SYS_ID_BTN_A` o `SYS_ID_BTN_B`) y un **timestamp**
acumulado mientras el boton permanece presionado (incrementos de 50 ms).

---

## Estado inicial (IDLE)

Al arrancar la aplicacion:

- **LED A** y **LED B**: encendidos.
- **LED C**: apagado.

---

## Comportamiento por boton

### Boton A (B1)

Al **presionar** (evento ON):

| LED | Accion    |
|-----|-----------|
| A   | Parpadeo  |
| B   | Apagado   |
| C   | Encendido |

Timeout inicial: **T_A = 5000** (unidades de tick del Btn/Sys, 50 ms por paso → ~5 s).

### Boton B (D7)

Al **presionar** (evento ON):

| LED | Accion    |
|-----|-----------|
| A   | Apagado   |
| B   | Parpadeo  |
| C   | Encendido |

Timeout inicial: **T_B = 10000** (~10 s).

---

## Pulsacion corta vs larga

Al **soltar** el boton (evento OFF), Sys compara el `timestamp` del Btn con el timeout correspondiente:

| Tipo      | Condicion        | Comportamiento observado                                                                                                    |
|-----------|------------------|-----------------------------------------------------------------------------------------------------------------------------|
| **Corta** | `timestamp < T`  | El patron de LEDs **sigue activo** hasta cumplir **T** desde el inicio de la pulsacion, aunque el boton ya se haya soltado. |
| **Larga** | `timestamp >= T` | Vuelve a **IDLE de inmediato** al soltar y **actualiza T** al valor del `timestamp` recibido (para la proxima vez).         |

Vuelta a **IDLE**: LED A y B encendidos, LED C apagado.

---

## Comportamiento observado en el video

En [sotrii-tp2_04-application.mp4](sotrii-tp2_04-application.mp4) se muestra un ejemplo con **dos pulsaciones cortas
consecutivas**: primero el **boton A** una vez y luego el **boton B** una vez.

### Secuencia observada

1. **Estado inicial**  
   LD2 (A) y D12 (B) encendidos; D11 (C) apagado.

2. **Pulsacion corta en boton A**
    - Al apretar B1: A parpadea, B se apaga, C se enciende.
    - Al soltar antes de ~5 s: el patron **continua** (A parpadeando, B apagado, C encendido).
    - Tras ~5 s desde que Sys detecto el press: vuelve a IDLE (A y B encendidos, C apagado).

3. **Pulsacion corta en boton B**
    - Al apretar D7: A se apaga, B parpadea, C se enciende.
    - Al soltar antes de ~10 s: el patron **continua** (A apagado, B parpadeando, C encendido).
    - Tras ~10 s desde el press: vuelve a IDLE.