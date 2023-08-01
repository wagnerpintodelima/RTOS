#include <Arduino.h>

/*
                             Semáforo BINÁRIO
                             
  O uso mais comum do semáforo binário é garantir a exclusão mútua para proteger
  seções críticas de código, onde dados compartilhados podem ser corrompidos ou
  gerar comportamentos indesejados se acessados simultaneamente
  por vários threads ou processos.

  Nesse caso, o led red vai piscar 10x e só depois dele executar eu aciono o led green. 
  Imediatamento eu deleto essas duas tasks.
*/

#define SERIAL_MONITOR        1
#define   RED                 27
#define   GREEN               14
#define   BLUE                12

//----- PWM LEDs
const int freq = 5e3;
const int ledChannel1 = 1,
          ledChannel2 = 2,
          ledChannel3 = 3;
const int resolution = 12;
const int resolution_levels = ((int)pow(2, resolution))-1;

// Tempo do pisca-pisca
uint16_t  delay_ = 500;

// put function declarations here:
void red(void * params);
void green(void * params);
void setRGB(int red_, int green_, int blue_);

//--- Shemaphore
xSemaphoreHandle semaforoBin = xSemaphoreCreateBinary();
TaskHandle_t taskRed = NULL, taskBlue = NULL;

void setup() {
  #if SERIAL_MONITOR
    Serial.begin(9600);
    delay(1e3);
    
    Serial.println("Bits: " + (String)resolution + " \n Levels: " + (String)resolution_levels);
  #endif

  // configure LED PWM functionalitites
  ledcSetup(ledChannel1, freq, resolution);
  ledcSetup(ledChannel2, freq, resolution);
  ledcSetup(ledChannel3, freq, resolution);

  // attach the channel to the GPIO to be controlled
  ledcAttachPin(RED, ledChannel1);
  ledcAttachPin(GREEN, ledChannel2);
  ledcAttachPin(BLUE, ledChannel3);

  delay(1e3);

  xTaskCreate(&red, "R", 2048, NULL, 1, &taskRed);  
  xTaskCreate(&green, "G", 2048, NULL, 1, &taskBlue);    
}

void loop() {}

// Valores em porcentagem.
// Mínimo: 0 e Máximo: 100
// Default: setRGB(); Desliga o led
void setRGB(int red_, int green_, int blue_){    
  
  uint16_t mappedValueRed = 0,
  mappedValueGreen = 0,
  mappedValueBlue = 0;

  if(red_ > 0)    mappedValueRed = map(red_, 0, 100, 0, resolution_levels);    
  if(green_ > 0)  mappedValueGreen = map(green_, 0, 100, 0, resolution_levels);    
  if(blue_ > 0)   mappedValueBlue = map(blue_, 0, 100, 0, resolution_levels);    

  ledcWrite(ledChannel1, mappedValueRed);
  ledcWrite(ledChannel2, mappedValueGreen);
  ledcWrite(ledChannel3, mappedValueBlue);       
}

void red(void * params){
  while(1){        
    for(byte i = 1; i <= 10; i++){
      #if SERIAL_MONITOR
        Serial.println("Task RED (" + (String)i + ") finalizada.");
      #endif
      setRGB(100, 0, 0);              
      vTaskDelay(delay_ / portTICK_PERIOD_MS);
      setRGB(0,0,0);              
      vTaskDelay(delay_ / portTICK_PERIOD_MS);      
    }      
    xSemaphoreGive(semaforoBin); // Já está tudo certo aqui, pode liberar a próxima task para executar        
    vTaskDelay(delay_ / portTICK_PERIOD_MS);
  }
}

void green(void * params){
  while(1){        
    xSemaphoreTake(semaforoBin, portMAX_DELAY); // A outra task já executou e deu sinal para continua, vai esparar infinito
    setRGB(0,100,0);
    #if SERIAL_MONITOR
      Serial.println("Task green finalizada.");
    #endif
    vTaskDelete(taskRed);
    vTaskDelete(taskBlue);    
  }
}
