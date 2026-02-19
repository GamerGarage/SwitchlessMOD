/*
 * ----------------------------------------------------------------------------
 * Sega Mega Drive - PIC16F684 Switchless Region & Reset Mod
 * ----------------------------------------------------------------------------
 * Autor:   Javi (GAMER GARAGE)
 * Web:     https://www.gamergarage.es
 * Versión: 1.0
 * * Descripción: 
 * Firmware para controlar el reinicio y la región (PAL/JAP/USA) de una 
 * consola Sega Mega Drive/Genesis utilizando el botón de RESET original
 * y un indicador LED RGB.
 * * Licencia: MIT (Puedes usar, modificar y distribuir este código libremente, 
 * siempre y cuando se mantenga la mención al autor original).
 * ----------------------------------------------------------------------------
 */

// PIC16F684 Configuration Bit Settings
#pragma config FOSC = INTOSCIO  // Oscilador interno activado
// ... (aquí sigue tu código normal)

// PIC16F684 Configuration Bit Settings
#pragma config FOSC = INTOSCIO  // Oscilador interno activado
#pragma config WDTE = OFF       // Watchdog Timer desactivado
#pragma config PWRTE = OFF      // Power-up Timer desactivado
#pragma config MCLRE = OFF      // Pin MCLR configurado como pin de I/O digital
#pragma config CP = OFF         // Protección de código desactivada
#pragma config CPD = OFF        // Protección de datos desactivada
#pragma config BOREN = OFF      // Brown-out Reset desactivado
#pragma config IESO = ON        // Internal External Switchover activado
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor activado

#include <xc.h>
#include <stdio.h>
#include <pic16f684.h>

// Definición de la frecuencia del oscilador para el uso de la función __delay_ms()
#define _XTAL_FREQ 4000000

// Asignación de pines para el LED RGB indicador de región
#define LED_RED    RC3
#define LED_GREEN  RC4
#define LED_BLUE   RC5

// Asignación de pines para el control de Reset
#define RESET_IN   RA4  // Botón de entrada para el reset y cambio de región
#define RESET_OUT  RA5  // Salida que ejecuta el reset físico en la consola

// Asignación de pines para la configuración de región (Idioma y Video)
#define LANGUAGE   RC0
#define VIDEO      RA2

// Constantes arbitrarias para identificar cada región en la EEPROM
#define PAL        0x50
#define JAP        0x4A
#define USA        0x55

/**
 * Guarda el modo de región actual en la memoria EEPROM.
 */
void saveMode(unsigned char dir, unsigned char data) {
    eeprom_write(dir, data);
}

/**
 * Configura las salidas y el LED para el modo PAL (Europa).
 */
void setPAL(void){
    LANGUAGE = 1;
    VIDEO = 0;
    LED_RED = 1;
    LED_GREEN = 0;
    LED_BLUE = 0;
}

/**
 * Configura las salidas y el LED para el modo JAP (Japón).
 */
void setJAP(void){
    LANGUAGE = 0;
    VIDEO = 1;
    LED_RED = 0;
    LED_GREEN = 1;
    LED_BLUE = 0;
}

/**
 * Configura las salidas y el LED para el modo USA (Estados Unidos).
 */
void setUSA(void){
    LANGUAGE = 1;
    VIDEO = 1;
    LED_RED = 0;
    LED_GREEN = 0;
    LED_BLUE = 1;
}

/**
 * Lee la memoria EEPROM y aplica la configuración de región guardada.
 * Si no reconoce el dato (ej. primera vez que se enciende), carga PAL por defecto.
 */
void loadMode(unsigned char dir) {
    unsigned char storedMode = eeprom_read(dir);
    
    switch (storedMode){
        case PAL:
            setPAL();
            break;
        case JAP:
            setJAP();
            break;
        case USA:
            setUSA();
            break;
        default:
            setPAL();          
    }
}

/**
 * Bucle de selección de región.
 * Cambia cíclicamente entre PAL, JAP y USA cada 500ms mientras el botón
 * se mantenga pulsado. Al soltarlo, guarda la selección final en la EEPROM.
 */
void changeMode(void){
    unsigned char selectedMode;
    // Mientras el botón esté presionado (!RESET_IN por el pull-up)
    while (!RESET_IN){
        setPAL(); 
        selectedMode = PAL;
        __delay_ms (500);
        if (RESET_IN) break; // Si se suelta el botón, salimos del bucle

        setJAP();    
        selectedMode = JAP;
        __delay_ms (500);
        if (RESET_IN) break;

        setUSA();  
        selectedMode = USA;
        __delay_ms (500);
        if (RESET_IN) break;
    }
    // Guardar el modo seleccionado en la dirección 0x00 de la EEPROM
    saveMode(0x00, selectedMode);
}

void main(void) {
    // Configuración inicial del microcontrolador
    ANSEL = 0;               // Desactiva todas las entradas analógicas (todo digital)
    TRISA = 0b00010000;      // Puerto A: RA4 como entrada (Botón), resto como salidas
    TRISC = 0b00000000;      // Puerto C: Todos los pines como salidas

    // Configuración de la resistencia Pull-up interna para el botón
    OPTION_REGbits.nRAPU = 0;  // Habilita el control global de los pull-ups en el Puerto A
    WPUA |= (1 << 4);          // Activa el pull-up específico en el pin RA4

    // Estado inicial seguro
    RESET_OUT = 1;           // Mantiene la señal de reset de la consola en estado inactivo

    // Cargar la última configuración de región guardada
    loadMode(0x00);

    int tiempo_pulsado = 0;

    // Bucle principal del programa
    while (1) {
        RESET_OUT = 1;              // Asegura que el RESET esté desactivado    
        int tiempo_pulsado = 0;     // Reinicia el contador de tiempo de pulsación

        // Bucle de espera activa: cuenta los milisegundos mientras el botón está pulsado
        while (!RESET_IN) {
            __delay_ms(1);
            tiempo_pulsado++;

            // Condición de Pulsación Larga: Modo de cambio de región (> 2 segundos)
            if (tiempo_pulsado >= 2000) {
                changeMode();

                // Esperar a que el usuario suelte el botón por completo antes de continuar
                while (!RESET_IN);     // Bucle infinito hasta que RESET_IN vuelva a ser 1
                __delay_ms(50);        // Pequeño retardo antirrebote
                break;                 // Salir de este bucle para volver al principal
            }
        }

        // Condición de Pulsación Corta: Ejecutar Reset de la consola (entre 20ms y 2s)
        if (tiempo_pulsado > 20 && tiempo_pulsado < 2000) {
            RESET_OUT = 0;            // Activar señal de reset (Lógica negativa)        
            __delay_ms(200);          // Mantener el reset presionado artificialmente por 200ms
            RESET_OUT = 1;            // Liberar el reset
        }

        // Pequeña espera antirrebote general al terminar cualquier acción
        __delay_ms(50);
    }
    
    return;

}
