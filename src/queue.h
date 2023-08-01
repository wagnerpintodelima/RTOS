#include <Arduino.h>

/*
                             Queue - Filas
                             
  Você cria uma fila de x elementos e uma task alimenta essa fila e a 
  outra consome. Por exemplo num caso onde tenhamos um LDR e queremos 
  obter a média da leitura. Podemos criar uma task de leitura de sensores
  e outra que calcula a média.

  Nesse exemplo nossa Queue tem 10 elementos e é alimentado pela task readLdr.
  Há uma outra task que calcula a média e exibe ela, ou seja, consome os dados da
  fila. 

  Caso a task esteja alimentando a fila, mas a outra task nao esteja consumindo os 
  dados, entao, assim que a fila atingir 10 leituras ela não manda mais dados.
*/

#define SERIAL_MONITOR        1
#define   LDR                 26


xQueueHandle fila_ldr;
uint8_t quantidade_elementos_na_fila = 10;

uint16_t delay_ = 1e3;
float ldr_avg = 0.00;

// put function declarations here:
void read(void * params);
void avg(void * params);

//--- Shemaphore
xSemaphoreHandle semaforoBin = xSemaphoreCreateBinary();
TaskHandle_t taskRead = NULL, taskAvg = NULL;

void setup() {
  #if SERIAL_MONITOR
    Serial.begin(9600);
    delay(1e3);        
  #endif

  pinMode(LDR, INPUT);
  pinMode(14, OUTPUT);
  digitalWrite(14, LOW);

  fila_ldr = xQueueCreate(quantidade_elementos_na_fila, sizeof(float)); // Cria a fila

  xTaskCreate(&read, "read", 2048, NULL, 1, &taskRead);  
  xTaskCreate(&avg, "avg", 2048, NULL, 1, &taskAvg);    
}

void loop() {}

void read(void * params){
  while(1){        
    float temp = analogRead(LDR);
    long response = xQueueSend(fila_ldr, &temp, 1e3 / portTICK_PERIOD_MS); // Envio minha leitura para a fila
    if(response){
      #if SERIAL_MONITOR
        Serial.println("+1 elemento add a fila.");
      #endif
    }else{
      #if SERIAL_MONITOR
        Serial.println("Fila cheia. Nao conseguiu enviar para a fila.");
      #endif
    }
    vTaskDelay(delay_ / portTICK_PERIOD_MS);
  }
}

void avg(void * params){

  float leitura_current = 0.00,
        leitura_prev    = 0.00;

  while(1){        
    if(xQueueReceive(fila_ldr, &leitura_current, 5e3 / portTICK_PERIOD_MS)){
      ldr_avg = (leitura_prev + leitura_current) / 2;
      leitura_prev = leitura_current;
      #if SERIAL_MONITOR
        Serial.println("AVG: " + (String)ldr_avg);
      #endif
    }else{
      #if SERIAL_MONITOR
        Serial.println("Fila nao esta disponivel.");
      #endif
    }
    vTaskDelay(delay_ / portTICK_PERIOD_MS);
  }
}
