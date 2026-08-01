# sotrii-tp2_26Co2026-04
# CESE - Sistemas Operativos de Tiempo Real II - Trabajo Práctico N°: 2 – Active Object
### Cohorte-Grupo: 26Co2026-04
### Modelo de placa: Nucleo-F446RE

### Responsable de la entrega:

| N° SIU | Apellidos, Nombres       | Fecha     | Deadline  |
| :----- | :---------------------   | :------:  | :-------: |
| e2601  | Botalla, Tomás Enrique   |           |           |
| e2615  | Restovich, Joaquin       |           |           |
| e2606  | Lazcano, Luca Mauricio   |           |           |

## Notas

- En Linux, usando STM32CubeIDE, cambiar:
    - "Run" > "Debug Configurations" > "Debugger" > "OpenOCD Command" por "${stm32cubeide_openocd_path}/openocd"
    - "Run" > "Debug Configurations" > "Debugger" > "Configuration Script" > "Mode Setup" > por "Software system reset"
    - "Run" > "Debug Configurations" > "Startup" > `monitor arm semihosting enable`
