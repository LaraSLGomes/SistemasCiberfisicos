#include <WiFi.h>
#include <esp_now.h>
#include <DHT.h>
#include <ArduinoJson.h>

#define ID "equipe50.2"
#define DHTPIN 14
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);


uint8_t intermediate_mac[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x01};

int msg_counter = 0;

void onSend(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Mensagem enviada: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Sucesso" : "Falha");
}

void setup() {
  Serial.begin(115200);
  dht.begin(); // Inicializa o sensor DHT

  // Configura o Wi-Fi para o modo estação (STA) para usar ESP-NOW
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); 

  // Inicializa o ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ao iniciar ESP-NOW");
    return;
  }

  // Registra a função de callback para o envio de dados
  esp_now_register_send_cb(onSend);


  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, intermediate_mac, 6); 
  peerInfo.channel = 0;   
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo); 
}

void loop() {
  // Lê os valores de temperatura e umidade do sensor DHT
  float temp = dht.readTemperature();
  float umid = dht.readHumidity();

  // Verifica se a leitura do sensor foi bem-sucedida
  if (isnan(temp) || isnan(umid)) {
    Serial.println("Erro ao ler sensor!");
    delay(2000); // Espera antes de tentar novamente
    return;
  }

  // Cria um documento JSON para empacotar os dados
  StaticJsonDocument<128> doc; 

  // Preenche o documento JSON com os dados
  doc["ID"] = ID;
  doc["contador"] = msg_counter++; 
  doc["temperatura"] = temp;
  doc["umidade"] = umid;

  // Serializa o documento JSON para uma string (buffer de caracteres)
  char buffer[128]; 
  serializeJson(doc, buffer);

  // Envia a mensagem ESP-NOW para o endereço MAC intermediário/receptor
  esp_now_send(intermediate_mac, (uint8_t *)buffer, strlen(buffer) + 1);

  delay(5000);
}