tema#include <avr/io.h>

#include <avr/interrupt.h>
#include <util/delay.h>
#define _DISABLE_ARDUINO_TIMER0_INTERRUPT_HANDLER_

#include <wiring.c>
#include <SD.h>
#define sdCS 10
#define maxVolts 100 
/*
 * Conectar el divisor de tensión en A0
 * Conectar corriente en A1
 * Actualmente no transforma los valores de corriente
*/
/*
//---Conexión del módulo SD---//
Pin MOSI al Digital 11
Pin MISO al Digital 12
Pin SCK al Digital 13
Pin CS al Digital 10
*/
uint16_t counter = 0;
uint16_t intervals = 10000; // 10000 -> 1  segundo
bool writingData = false;
float voltageP1 = 0;
float currentP1 = 0;

File panel1;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  SD.begin(sdCS);
  if (!SD.begin()) {
    return; //si no lee nada, se acaba el programa
  }
  startTimers();
  configADC();
  
  sei();
  Serial.println("Tarjeta preparada");

}

void loop() {
  if(writingData){
    startADC(); 
   //guardar información en la tarjeta SD
   panel1 = SD.open("panel1.txt", FILE_WRITE);
   if(panel1){
      panel1.print(voltageP1);
      panel1.print(",");
      panel1.print(currentP1);
      panel1.println(" "); 
      panel1.close(); 
   }

   //reiniciar contador
   counter = 0; 
   writingData = false; 
  }
  
}

void startTimers() {
  TCCR0A = (1 << WGM01);
  TCCR0B = (1 << CS01);
  TIMSK0 = (1 << OCIE0A);
  OCR0A = 250;
}

void configADC(){
  ADMUX = 0x40; 
  ADCSRA |= (1 << ADEN) | (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2) | (1 << ADIE); 
  ADCSRA |= (1 << ADSC); 
}

void startADC(){ // leer los puertos ADC
  ADMUX = 0x40;
  ADCSRA |= (1 << ADSC);
  while(ADCSRA & (1 << ADSC));  
  ADMUX = 0x41; 
  ADCSRA |= (1 << ADSC);
  while(ADCSRA & (1 << ADSC));  
}

ISR(TIMER0_COMPA_vect) {
  if(!writingData){
     counter++;
  } 
  if (counter >= intervals) {
    writingData = true; 
  }
}

ISR(ADC_vect){
  uint8_t adl = ADCL;
  uint16_t adh = (ADCH <<8) | adl; 
    switch(ADMUX){
      case 0x40:
      voltageP1 =(adh / 1023.0) * maxVolts; 
      break;

      case 0x41:
      currentP1 = adh;
      break;
      
    }  
}
