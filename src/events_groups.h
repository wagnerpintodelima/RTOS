#include <Arduino.h>

/*
                             Events Groups
                             
  Esse é bem interessante porque ele libera uma task quando outros
  n eventos ocorrem, e isso é muito útil em várias situações.

  O exemplo aqui simula uma conexão com internet e mqtt. Portanto, 
  temos uma task que conecta na internet, e, quando ela consegue, 
  acionamos a task que conecta no mqtt. Para isso, utilizamos um
  semáforo binário, simples e prático. No entanto, a callback do mqtt,
  só processa os dados quando as duas task acima derem seu aval.

*/

#define SERIAL_MONITOR        1

xSemaphoreHandle ligarMqtt;
EventGroupHandle_t eventGroupConectividade;
const int conexaoEthernet = BIT0;
const int conexaoMqtt = BIT1;


// put function declarations here:
void connectEthe(void * params);
void connectMqtt(void * params);
void callbackMqtt(void * params);

//--- Shemaphore
TaskHandle_t taskEthe = NULL, taskMqtt = NULL, taskCallback = NULL;

void setup() {
  #if SERIAL_MONITOR
    Serial.begin(9600);
    delay(1e3);        
  #endif
  
  pinMode(14, OUTPUT);
  digitalWrite(14, LOW);

  ligarMqtt = xSemaphoreCreateBinary();
  eventGroupConectividade = xEventGroupCreate();

  xTaskCreate(&connectEthe, "eth", 2048, NULL, 1, &taskEthe);  
  xTaskCreate(&connectMqtt, "mqtt", 2048, NULL, 1, &taskMqtt);    
  xTaskCreate(&callbackMqtt, "callback", 2048, NULL, 1, &taskCallback);    
}

void loop() {}

void connectEthe(void * params){
  while(1){        
    #if SERIAL_MONITOR
      Serial.print("Conectando na internet.");
      for(char i = 0; i <= 10; i++){
        Serial.print(".");
        vTaskDelay(1e3 / portTICK_PERIOD_MS);
      }
      Serial.println("");
      Serial.println("CONECTADO ETHE0.");
    #endif
    xSemaphoreGive(ligarMqtt); // Pode conectar no mqtt
    xEventGroupSetBits(eventGroupConectividade, conexaoEthernet);
    vTaskDelete(taskEthe);
  }
}

void connectMqtt(void * params){
  while(1){   
    xSemaphoreTake(ligarMqtt, portMAX_DELAY);   // Só passa daqui quando liberar  
    #if SERIAL_MONITOR
      Serial.print("Conectando no mqtt.");
      for(char i = 0; i <= 10; i++){
        Serial.print(".");
        vTaskDelay(1e3 / portTICK_PERIOD_MS);
      }
      Serial.println("");
      Serial.println("CONECTADO MQTT.");
    #endif        
    xEventGroupSetBits(eventGroupConectividade, conexaoMqtt);        
    vTaskDelete(taskMqtt);
  }
}


void callbackMqtt(void * params){
  while(1){
    // Isso aqui segura o processamento até que as condições sejam satisfeitas
    xEventGroupWaitBits(eventGroupConectividade, conexaoEthernet | conexaoMqtt, true, true, portMAX_DELAY);    

    #if SERIAL_MONITOR
      Serial.print("Processando dados!!!!!");      
    #endif            
        
    vTaskDelay(2e3 / portTICK_PERIOD_MS);
  }
}