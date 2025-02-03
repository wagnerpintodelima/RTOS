#include <Arduino.h>

/*
  TODO ESSE CURSO É BASEADO NAS VÍDEO AULAS DO PROFESSOR DESTE LINK:
  https://www.youtube.com/watch?v=7rbZFcqTaGc

  Thanks teacher!!!

  God Is Good
*/

/*
                             Notifications
                             
  Nesse tipo de controle, nós conseguimos fazer uma task notificar outra task.
  Isso funciona de uma maneira bem simples, há um emissor e outro que é o 
  receptor. O emissor simplesmente dispara uma notificação, que, na verdade
  é só um comando simples, sem parâmetros ou qualquer outra coisa do gênero.
  O receptor fica parado aguardando a notify para continuar o seu processo.  
*/

#define SERIAL_MONITOR        1


// put function declarations here:
void emissor(void * params);
void receptor(void * params);

//--- Shemaphore
TaskHandle_t taskReceptor = NULL;

void setup() {
  #if SERIAL_MONITOR
    Serial.begin(9600);
    delay(1e3);        
  #endif
  
  pinMode(14, OUTPUT);
  digitalWrite(14, LOW);
  
  xTaskCreate(&emissor, "emissor", 2048, NULL, 1, NULL);  
  xTaskCreate(&emissor, "emissor2", 2048, NULL, 1, NULL);  
  xTaskCreate(&receptor, "receptor", 2048, NULL, 1, &taskReceptor);    
  
}

void loop() {}

void emissor(void * params){
  while(1){
    #if SERIAL_MONITOR
      Serial.println("Emissor vai dar um sinal em.");
      for(char i = 0; i <= 10; i++){
        Serial.print(".");
        vTaskDelay(1e3 / portTICK_PERIOD_MS);
      }
      Serial.println("");
      Serial.println("Enviado notificacao");
    #endif
    xTaskNotifyGive(taskReceptor);  // Envia a notificacao para a task alvo
    vTaskDelete(NULL);
  }
}

void receptor(void * params){
  while(1){

    // Não passa daqui enquando não houver notificações.
    unsigned long qtd = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    #if SERIAL_MONITOR
      Serial.println("Notificacao recebida(s): " + (String)qtd);
    #endif    
  }
}
