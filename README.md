# Sega Mega Drive - PIC16F684 Switchless Region & Reset Mod

Este proyecto contiene el firmware en C para un microcontrolador PIC16F684 diseñado específicamente para realizar un mod "switchless" (sin interruptores físicos) en la **Sega Mega Drive** (o Genesis). 

Permite controlar el reinicio y cambiar la región (PAL/JAP/USA) de la consola utilizando únicamente el botón de RESET original. Además, utiliza un LED RGB para indicar visualmente en qué región se encuentra el sistema en cada momento.

## 🚀 Características

* **Control con un solo botón:** Discrimina entre pulsaciones cortas (reinicio) y pulsaciones largas (cambio de región) interceptando el botón de RESET original de la Mega Drive.
* **Memoria EEPROM:** Guarda la última región seleccionada para que la consola arranque con ella la próxima vez que se encienda.
* **Indicador LED RGB:** Muestra visualmente la región actual (Rojo = PAL, Verde = JAP, Azul = USA). Sustituye al LED rojo original de la consola.
* **Antirrebote por software:** Evita falsas pulsaciones y reinicios accidentales.

## 🧩 Circuito Impreso (PCB) Personalizado y Gerbers

Para facilitar la instalación y hacerla lo más limpia posible, he diseñado una placa de circuito impreso (PCB) a medida para este mod.

### Vista de la PCB
![Vista 3D de la PCB](images/pcb_3d.png)

### Esquemático (KiCad)
![Esquemático del circuito](images/esquematico.png)

* **Descarga de Gerbers:** Puedes descargar los archivos **Gerber** incluidos en este repositorio para pedir tus placas en JLCPCB, PCBWay, etc.
* **Resistencias LED RGB:** Deben ir soldadas exactamente en las posiciones marcadas como **R1, R2 y R3** en la serigrafía.
* **Uso Multipropósito (Breakout Board):** El diseño de la placa es genérico. Si puenteas con estaño las posiciones de las resistencias R1, R2 y R3, puedes usar esta PCB como un adaptador genérico para cualquier otro proyecto con el PIC16F684.
* **Puerto de Programación (ICSP):** La placa incluye pines para programar el PIC directamente sobre la PCB con un PICkit 3.

## ⚙️ Funcionamiento de la Intercepción del Reset

El PIC actúa como un "intermediario" entre el botón físico de la consola y el procesador. 

1.  **Pulsación Corta (menos de 2 segundos):** El PIC detecta la pulsación y transmite una señal de reinicio (LOW) al procesador durante 200ms. Funciona como un reset tradicional.
2.  **Pulsación Larga (más de 2 segundos):** El PIC **NO** envía la señal de reinicio a la consola. En su lugar, entra en el "Modo de Selección" de región. 
    * El LED RGB ciclará entre los tres colores cada 500ms.
    * Al soltar el botón en el color deseado, la selección se guarda en la EEPROM y cambia la región al instante, sin reiniciar el juego.

### Zonas y Colores
| Región | Video | Idioma | Color LED RGB |
| :--- | :--- | :--- | :--- |
| **PAL (Europa)** | 50Hz (0) | Inglés (1) | 🔴 Rojo |
| **JAP (Japón)** | 60Hz (1) | Japonés (0) | 🟢 Verde |
| **USA (América)**| 60Hz (1) | Inglés (1) | 🔵 Azul |

## 🔌 Conexiones de la PCB al Hardware

La serigrafía de mi PCB está diseñada para ser intuitiva. Aquí tienes la correspondencia de los pads con los puntos de soldadura en la consola:

| Pad en la PCB | Pin PIC | Función / Conexión en la Mega Drive |
| :--- | :---: | :--- |
| **RESET IN** | `RA4` | Al botón físico de Reset (la parte que va a GND al pulsar). |
| **RESET OUT** | `RA5` | Al punto de la pista de Reset que va hacia el procesador principal. |
| **LANG** | `RC0` | Al punto del jumper de configuración de Idioma. |
| **50/60hz** | `RA2` | Al punto del jumper de configuración de Frecuencia. |
| **RC3/R** | `RC3` | Al ánodo Rojo del LED RGB (vía resistencia R1). |
| **RC4/G** | `RC4` | Al ánodo Verde del LED RGB (vía resistencia R2). |
| **RC5/B** | `RC5` | Al ánodo Azul del LED RGB (vía resistencia R3). |
| **VCC** | `VDD` | Alimentación de +5V de la consola. |
| **GND** | `VSS` | Tierra (GND) de la consola. |

*Nota: El cátodo común del LED RGB se conecta a cualquiera de los pads **GND** extra disponibles en la PCB.*

## ✂️ Guía de Instalación: Ejemplo Placa IC BD M5 PAL / VA6

⚠️ **IMPORTANTE:** Es **ESTRICTAMENTE NECESARIO** cortar pistas en la placa base original. Si conectas el PIC sin aislar los pines, provocarás un cortocircuito que podria dañará la consola o el microcontrolador.

Para ilustrar el proceso, aquí tienes mi propia instalación en una revisión **VA6**:

### 1. Intercepción del botón RESET
Debes localizar la pista que une el botón de Reset con el procesador y **cortarla**. 
* El lado de la pista que viene del botón físico se suelda al pad **RESET IN** de la PCB.
* El lado de la pista que va hacia el procesador se suelda al pad **RESET OUT** de la PCB.

### 2. Configuración de Región (Jumpers)
En la revisión VA6, el idioma y la frecuencia vienen fijados por los jumpers **JP2** y **JP3**. 
* **Cortar pistas:** Corta la pista de cobre que une los pads de estos jumpers para romper la conexión de fábrica (+5V y GND). Comprueba con un multímetro que están aislados.
* **Conexión de señales:** Suelda los pads **LANG** y **50/60hz** del mod a los puntos correspondientes de los jumpers que van hacia el chip de video.
* **Alimentación:** Puedes aprovechar el otro extremo de los jumpers cortados para alimentar el mod. El lado cortado de JP2 te dará los +5V (suelda a **VCC**) y el lado cortado de JP3 te dará la masa (suelda a **GND**).

![Corte de pistas JP2, JP3 y Reset](images/cortes_placa_va6.jpg)
![Instalación finalizada con PCB](images/instalacion_final_va6.jpg)

## 🛠️ Compilación y Programación

* **IDE:** MPLAB X IDE
* **Compilador:** XC8 Compiler
* **Frecuencia del Oscilador:** 4MHz (Oscilador Interno)

Utiliza un programador como el **PICkit 3** conectándolo a los pines ICSP de la PCB para flashear el archivo `.hex`.

## ⚠️ Advertencia Legal y de Hardware
Modificar hardware original conlleva riesgos. Asegúrate de tener conocimientos de electrónica y revisar bien los puntos de corte de tu revisión específica de placa base (VA0, VA4, VA6, etc.) antes de proceder. El software y los esquemas se proporcionan "tal cual". No me hago responsable por daños ocasionados a tu equipo.

---

## 👨‍💻 Autor y Contacto

Creado por **Javi** de **GAMER GARAGE**. 

Si te ha resultado útil este mod o quieres ver más proyectos de reparación, modificación y electrónica retro, pásate por mi web:
🌐 **[www.gamergarage.es](https://www.gamergarage.es)**
