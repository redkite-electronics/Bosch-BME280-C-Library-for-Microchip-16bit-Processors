/*
 Test Code for
 Bosch BME280 humidity, temperature & pressure sensor, 400khz I2C Mode for a PIC24FJ128GA204
 I Stewart
 Version 1.0 26/02/2017
 */

#define FCY 8000000UL //XTAL speed 8Mhz
#pragma config POSCMD = 3, FNOSC = FRC, BOREN=1, IESO=1, FWDTEN= 0, SOSCSEL=1, JTAGEN=0// 8MHZ Internal OSC, 32Khz Secondary OSC

#include <xc.h>
#include <libpic30.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "serial24fj128ga204.c"
#include "i2c.c"
#include "bme280.h"
#include "bme280.c"

float Temperture = 0;
float Pressure =0;
float Humidity=0;

 void putch(unsigned char x) {
    put_serial_ch(x);
}

int main(void) {

    ANSB = 0;

    TRISBbits.TRISB2 = 1; //set to allow TX from serial coms
    TRISCbits.TRISC9 = 0; //LED Output
    LATCbits.LATC9=1;

    serial_setup(19200);
    __delay_ms(250);
    printf("\r\rPIC24FJ128GA204 BME280 Test");

    i2c_init(400); //400khz

    i2c_start();
    __delay_ms(100);

    BME280_Setup();

    __delay_ms(100);

    while (1) {
        
       Temperture = BME280_readTemperature();
       Pressure = BME280_readPressure();
       Humidity = BME280_readHumidity();

       unsigned int temp;

       printf("Temperture: %#.1f",Temperture/100);
       printf("'C ");
       printf("Pressure: %4.0f", Pressure);
       printf("mb ");
       printf("Humidity: %2.0f", Humidity);
       printf("%% ");
       printf("\r");

       __delay_ms(1000);

    }

    return 0;
}
