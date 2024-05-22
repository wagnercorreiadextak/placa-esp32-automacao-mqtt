//Bibliotecas
#include "ArduinoJson.h"
#include "EspMQTTClient.h"
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is conntec to the Arduino digital pin 26
int onWireBus = 26;

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(onWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

//Declaração de variáveis
int ledStatus = 2;
int rele1 = 22;
int estadoRele1 = 0;

// variáveis para o botão:
int btn1 = 23;
int estadoAtual = 0;
int ultimoEstado = 1;

int limiteBounce = 30;
int ultimoTempoLimiteBounce = 0;

int wifiConectada = 0;

float temperatura = 0.0;

//-------------------------------------------------//
//configurações da conexão MQTT
EspMQTTClient client(
  "wagner-nokia",                             //nome da sua rede Wi-Fi
  "13041968",                            //senha da sua rede Wi-Fi
  "test.mosquitto.org",                          // MQTT Broker server ip padrão da tago
  "",                               // username
  "",  // Código do Token DA PLATAFORMA TAGOIO
  "unifebe-si05-001",                            // Client name that uniquely identify your device
  1883                                     // The MQTT port, default to 1883. this line can be omitted
);
//-------------------------------------------------//

//Tópico
char topicoMqtt[] = "unifebe/si05/sensor001";

/*
Esta função é chamada assim que tudo estiver conectado (Wifi e MQTT)]
AVISO: VOCÊ DEVE IMPLEMENTÁ-LO SE USAR EspMQTTClient
*/
void onConnectionEstablished() {
  client.subscribe(topicoMqtt, onMessageReceived);
}

void onMessageReceived(const String& msg) {
  digitalWrite(ledStatus, LOW);
  Serial.println("Mensagem recebida:");
  Serial.println(msg);

  StaticJsonBuffer<300> JSONBuffer;                  //Memory pool
  JsonObject& parsed = JSONBuffer.parseObject(msg);  //Parse message

  if (parsed.success()) {
    //int rele = parsed["led"];

    //Serial.println("Recebeu dados...");

  } else {
    Serial.println("Falha ao realizar parsing do JSON");
  }
}

void transmitirStatus() {
  StaticJsonBuffer<300> JSONbuffer;
  JsonObject& JSONencoder = JSONbuffer.createObject();
  JSONencoder["temperatura"] = temperatura;

  char JSONmessageBuffer[300];
  JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));

  client.publish(topicoMqtt, JSONmessageBuffer);
}

void lerTemperatura() {
  sensors.requestTemperatures(); 
  temperatura = sensors.getTempCByIndex(0);
  Serial.print(temperatura); 
}

void setup() {

  //Ativando saída serial
  Serial.begin(9600);

  //Ativando principais portas

  //Relê
  //pinMode(rele1, OUTPUT);
  //digitalWrite(rele1, HIGH);

  //Informação de Placa ligada
  pinMode(ledStatus, OUTPUT);
  digitalWrite(ledStatus, LOW);

  //Cria a tarefa para o loop2
  Serial.printf("\nsetup() em core: %d", xPortGetCoreID());
  xTaskCreatePinnedToCore(loop2, "loop2", 8192, NULL, 1, NULL, 0);
}

void verificarWifi() {
  //Alternar o led "azul" do ESP32 conforme estado na conexão
  if (client.isWifiConnected()) {
    digitalWrite(ledStatus, HIGH);
    wifiConectada = 1;
  } else {
    digitalWrite(ledStatus, LOW);
    wifiConectada = 0;
  }
}

void loop() {

  lerTemperatura();

  if(wifiConectada == 1) {
    transmitirStatus();
  }

  delay(5000);
}

void loop2(void* z) {
  //Mostra no monitor em qual core o loop2() foi chamado
  Serial.printf("\nloop2() em core: %d", xPortGetCoreID());

  while (1)  //Pisca o led infinitamente
  {
    client.loop();
    verificarWifi();
    delay(10);
  }
}