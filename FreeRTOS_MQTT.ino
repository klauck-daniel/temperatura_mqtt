// Projeto de Redes Industriais :: Sensor de temperatura, envio e recebimento de comandos por MQTT
// Daniel, Douglas e Juliane
// 2021/02

#include <WiFi.h>
#include <PubSubClient.h> //biblioteca para MQTT
#include <DHT.h> //biblioteca do sensor de temperatura

//Pino que será conectado o ESP32
//#define DHT_PIN 26 //GPIO 26
//#define DHTTYPE DHT22
DHT dht(26, DHT22);

#define COOLER_PIN 2 //GPIO 2 (utilizamos um LED AZUL para simular) (GPIO 2 = BuiltIn LED)
#define AUTO_PIN 21 //GPIO 21 (LED VERMELHO)

//Tópicos Assinados e Publicados do ESP32
#define ID_MQTT  "redes_industriais_ESP32"

//TÓPICOS ASSINADOS (recebe mensagem)
#define TOPICO_SUB_COOLER "topico_liga_cooler" //recebe pelo MQTT o comando para ligar ou desligar o cooler

//TÓPICOS DE ENVIO (envia mensagens)
#define TOPICO_PUB_TEMPERATURA "topico_sensor_temperatura" //envia o valor da temperatura por MQTT
#define TOPICO_PUB_UMIDADE "topico_sensor_umidade" //envia o valor da umidade por MQTT
#define TOPICO_PUB_COOLER "topico_liga_cooler" //Envia o status do cooler, se ligado ou desligado 
                                                       //0 = OFF; 1 = ON; 2 = AUTO 
// Variáveis para conexão WiFi
const char* ssid       = "REDE WI-FI";
const char* password   = "SENHA DA REDE WI-FI";

const char* BROKER_MQTT = "test.mosquitto.org"; //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883; // Porta do Broker MQTT

//Variáveis e objetos globais
WiFiClient esp32_redes_industriais_Client; // Cria o objeto esp32_redes_industriais_Client
PubSubClient MQTT(esp32_redes_industriais_Client); // Instancia o Cliente MQTT passando o objeto esp32_redes_industriais_Client

// Define as task que serão implementadas 
void TaskLeituraSensor( void *pvParameters );
void TaskAcionamentoCooler( void *pvParameters );
//void TaskLeituraMQTT( void *pvParameters );

float temperatura = 0.0;
float umidade = 0.0;
char temperatura_str[10] = {0};
char umidade_str[10] = {0};
int automatico = true; //Automatico=true acionamento controlado de forma autmática pela variação de temperatura
bool cooler = false; //False=cooler desligado	True=cooler ligado

void setup(){
 
  Serial.begin(115200);
  dht.begin();
    
  pinMode(COOLER_PIN, OUTPUT);
  pinMode(AUTO_PIN, OUTPUT);

  MQTT.setServer(BROKER_MQTT, BROKER_PORT);   //informa qual broker e porta deve ser conectado
  MQTT.setCallback(callback);            //atribui função de callback (função chamada quando qualquer informação de um dos tópicos subescritos chega)
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){
      delay(500);
      Serial.print(".");
  }
  Serial.println("CONNECTADO A REDE WI-FI");

  initMQTT();

  MQTT.subscribe(TOPICO_SUB_COOLER);

  //Configurar as tasks
  xTaskCreatePinnedToCore(TaskLeituraSensor, "LeituraSensorTemperatura" /*Nome*/, 2024 /*tamanho em bytes*/, NULL, 2 /*0 a 3*/, NULL, 0 /*Core que irá rodar*/);

  xTaskCreatePinnedToCore(TaskAcionamentoCooler, "AcionamentoCooler", 2024, NULL, 2, NULL, 1);

/*  xTaskCreatePinnedToCore(
    TaskLeituraMQTT, "LeituraMQTT", 1024, NULL, 3, NULL, 1);*/
}

void loop(){
  MQTT.loop(); //keep-alive da comunicação com o broker MQTT
  
  // Empty. Things are done in Tasks.
  }

void initMQTT(void){
    
    while (!MQTT.connected()){
      Serial.print("* Tentando se conectar ao Broker MQTT: ");
      Serial.println(BROKER_MQTT);
      if (MQTT.connect(ID_MQTT)){
        Serial.println("Conectado com sucesso ao broker MQTT!");  
      } 
      else{
        Serial.println("Falha ao reconectar no broker.");
        Serial.println("Havera nova tentatica de conexao.");
			  vTaskDelay(5000/portTICK_PERIOD_MS);
        //delay(2000);
      }
    }
}

void callback(char* topic, byte* payload, unsigned int length) 
{
    String msg;
  
    /* obtem a string do payload recebido */
    for(int i = 0; i < length; i++){
      char c = (char)payload[i];
      msg += c;
    }
 
    Serial.print("Chegou a seguinte string via MQTT: ");
    Serial.println(msg);
    
    /* toma ação dependendo da string recebida */
    if (msg.equals("1")){
      cooler = true;
      automatico=false;
      Serial.print("Cooler ativado por comando MQTT \n");
      //digitalWrite (AUTO_PIN, HIGH);
    }
  
    if (msg.equals("0")){
      cooler = false;
      automatico=false;
      Serial.print("Cooler desativado por comando MQTT \n");
      //digitalWrite (AUTO_PIN, HIGH); 
    }
	
	  if (msg.equals("2")){
      automatico = true;
      Serial.print("Cooler setado para automatico por comando MQTT \n");
      //digitalWrite (AUTO_PIN, LOW); 
    }
}
/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskLeituraSensor(void *pvParameters){
  for(;;){
    
    temperatura = dht.readTemperature();
    umidade = dht.readHumidity();

    sprintf(temperatura_str,"%.2f *C", temperatura);
    sprintf(umidade_str,"%.2f %%", umidade);
    
    Serial.println(temperatura_str);
    Serial.println(umidade_str);

    MQTT.publish(TOPICO_PUB_TEMPERATURA, temperatura_str);
    MQTT.publish(TOPICO_PUB_UMIDADE, umidade_str);

	if(automatico){  
		if (temperatura > 25){
		  cooler=true;
		  }
		else{
		 cooler=false;
		}
	}
    vTaskDelay(5000/portTICK_PERIOD_MS);
    }
}
void TaskAcionamentoCooler(void* pvParameters){
  for(;;){

    //MQTT.subscribe(TOPICO_PUB_TEMPERATURA, temperatura_str);

    if(automatico){
      digitalWrite (AUTO_PIN, LOW);
      }
    else{
      digitalWrite (AUTO_PIN, HIGH);
      }
    if(cooler){
      digitalWrite(COOLER_PIN, HIGH);
    }
    else{
      digitalWrite(COOLER_PIN, LOW);
    }
    vTaskDelay(5000/portTICK_PERIOD_MS);
}
}
/*
xTaskCreatePinnedToCore(Ponteiro da task, "label", size(tamanho do stack), ponteiro genérico, prioridade(0a3), PID da task, indice do core que executa a task(0 ou 1)*/
