#include <Arduino.h>

/*
                             Semáforo MUTEX
                             
  Em resumo, o semáforo mutex é utilizado para sincronizar o acesso a recursos 
  compartilhados, garantindo que apenas uma thread ou processo por vez possa 
  executar o código protegido pelo mutex, evitando assim problemas como 
  condições de corrida e resultados indeterminados. Isso ajuda a manter
  a integridade e consistência dos dados compartilhados e torna a programação
  concorrente mais segura e confiável.
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
uint16_t  delay_ = 10,
          timeout_rtos = 500;



// put function declarations here:
void red(void * params);
void green(void * params);
void blue(void * params);
void setRGB(int red_, int green_, int blue_);

//--- Shemaphore
xSemaphoreHandle mutexRGB = xSemaphoreCreateMutex();

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

  xTaskCreate(&red, "R", 2048, NULL, 1, NULL);  
  xTaskCreate(&green, "G", 2048, NULL, 1, NULL);  
  xTaskCreate(&blue, "B", 2048, NULL, 1, NULL);
}

void loop() {}

// Valores em porcentagem.
// Mínimo: 0 e Máximo: 100
// Default: setRGB(); Desliga o led
void setRGB(int red_, int green_, int blue_){    

  #if SERIAL_MONITOR
    Serial.print("Fn setRGB: ");
    Serial.println((String)red_ + ", " + (String)green_ + ", " + (String)blue_);
  #endif  
  
  uint16_t mappedValueRed = 0,
  mappedValueGreen = 0,
  mappedValueBlue = 0;

  if(red_ > 0)    mappedValueRed = map(red_, 0, 100, 0, resolution_levels);    
  if(green_ > 0)  mappedValueGreen = map(green_, 0, 100, 0, resolution_levels);    
  if(blue_ > 0)   mappedValueBlue = map(blue_, 0, 100, 0, resolution_levels);    

  ledcWrite(ledChannel1, mappedValueRed);
  ledcWrite(ledChannel2, mappedValueGreen);
  ledcWrite(ledChannel3, mappedValueBlue);  

  #if SERIAL_MONITOR
    Serial.print("PWM: ");
    Serial.println("Red: " + (String)mappedValueRed + ", Green: " + (String)mappedValueGreen + ", Blue: " + (String)mappedValueBlue);
  #endif   
  
   
}

void red(void * params){
  while(1){
    if(xSemaphoreTake(mutexRGB, 1000 / portTICK_PERIOD_MS)){
      ESP_LOGI("Red", "Ocupando processador");
      for(byte i = 1; i <= 100; i++){
        setRGB(i, 0, 0);              
      }      
      xSemaphoreGive(mutexRGB);
    }else{    
      #if SERIAL_MONITOR      
        ESP_LOGE("Red", "Rotine Busy");
      #endif   
    } 
    vTaskDelay(delay_ / portTICK_PERIOD_MS);
  }
}

void green(void * params){
  while(1){
    if(xSemaphoreTake(mutexRGB, 1000 / portTICK_PERIOD_MS)){
      ESP_LOGI("Green", "Ocupando processador");
      for(byte i = 1; i <= 100; i++){
        setRGB(0, i, 0);        
      }      
      xSemaphoreGive(mutexRGB);
    }else{    
      #if SERIAL_MONITOR      
        ESP_LOGE("Green", "Rotine Busy");
      #endif   
    } 
    vTaskDelay(delay_ / portTICK_PERIOD_MS);
  }
}

void blue(void * params){
  while(1){
    if(xSemaphoreTake(mutexRGB, 1000 / portTICK_PERIOD_MS)){
      ESP_LOGI("Blue", "Ocupando processador");
      for(byte i = 1; i <= 100; i++){
        setRGB(0, 0, i);      
      }      
      xSemaphoreGive(mutexRGB);
    }else{    
      #if SERIAL_MONITOR              
        ESP_LOGE("Blue", "Rotine Busy");
      #endif   
    } 
    vTaskDelay(delay_ / portTICK_PERIOD_MS);
  }
}