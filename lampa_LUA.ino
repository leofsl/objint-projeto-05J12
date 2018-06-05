#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define MQTT_VERSION MQTT_VERSION_3_1_1

// Configuracao do Wifi
const char* WIFI_SSID = "Nome da Rede";
const char* WIFI_PASSWORD = "Senha da Rede";

// Configuracao do Protocolo MQTT
const PROGMEM char* MQTT_CLIENT_ID = "livingroom1_light1";
const PROGMEM char* MQTT_SERVER_IP = "192.168.0.106";
const PROGMEM uint16_t MQTT_SERVER_PORT = 1883;
const PROGMEM char* MQTT_USER = "homeassistant";
const PROGMEM char* MQTT_PASSWORD = "lua";

// Topics MQTT
const char* MQTT_LIGHT_STATE_TOPIC = "livingroom1/light1/status";
const char* MQTT_LIGHT_COMMAND_TOPIC = "livingroom1/light1/switch";

const char* LIGHT_ON = "ON";
const char* LIGHT_OFF = "OFF";

const PROGMEM uint8_t LED_PIN = 5; // Pino do NodeMCU
boolean estadoLuz = false; // Pino inicia desligado
WiFiClient wifiClient;
PubSubClient client(wifiClient);

// Funcao para publicar o estado do rele (ligado/desligado)
void publicaEstadoLuz() {
  if (estadoLuz) {
    client.publish(MQTT_LIGHT_STATE_TOPIC, LIGHT_ON, true);
  } else {
    client.publish(MQTT_LIGHT_STATE_TOPIC, LIGHT_OFF, true);
  }
}

// Funcao para ligar/deslidar rele
void setEstadoLuz() {
  if (estadoLuz) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}

// Funcao para quando chegar uma msg MQTT
void callback(char* p_topic, byte* p_payload, unsigned int p_length) {
  // Concatena o payload em string
  String payload;
  for (uint8_t i = 0; i < p_length; i++) {
    payload.concat((char)p_payload[i]);
  }
  
  if (String(MQTT_LIGHT_COMMAND_TOPIC).equals(p_topic)) {
    // Testa se o payload e "ON" ou "OFF"
    if (payload.equals(String(LIGHT_ON))) {
      if (estadoLuz != true) {
        estadoLuz = true;
        setEstadoLuz();
        publicaEstadoLuz();
      }
    } else if (payload.equals(String(LIGHT_OFF))) {
      if (estadoLuz != false) {
        estadoLuz = false;
        setEstadoLuz();
        publicaEstadoLuz();
      }
    }
  }
}

void reconnect() {
  // Loop de reconexao
  while (!client.connected()) {
    if (client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
      // Quando conectado avisa o servidor
      publicaEstadoLuz();
      client.subscribe(MQTT_LIGHT_COMMAND_TOPIC);
    } else {
      Serial.print("ERRO - código: ");
      Serial.print(client.state());
      // Espera 5 segundos
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Configuracao do rele
  pinMode(LED_PIN, OUTPUT);
  analogWriteRange(255);
  setEstadoLuz();

  Serial.print("Conectado em: ");
  WiFi.mode(WIFI_STA);
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Serial.println("WiFi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Conexao MQTT
  client.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}