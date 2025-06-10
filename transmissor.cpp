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
dht.begin(); 
WiFi.mode(WIFI_STA); 
WiFi.disconnect(); 
if (esp_now_init() != ESP_OK) { 
Serial.println("Erro ao iniciar ESP-NOW"); 
return; 
} 
esp_now_register_send_cb(onSend); 
esp_now_peer_info_t peerInfo = {}; 
memcpy(peerInfo.peer_addr, intermediate_mac, 6); 
peerInfo.channel = 0; 
peerInfo.encrypt = false; 
esp_now_add_peer(&peerInfo); 
} 
void loop() { 
float temp = dht.readTemperature(); 
float umid = dht.readHumidity(); 
if (isnan(temp) || isnan(umid)) { 
Serial.println("Erro ao ler sensor!"); 
delay(2000); 
return; 
} 
StaticJsonDocument<128> doc; 
doc["ID"] = ID; 
doc["contador"] = msg_counter++; 
doc["temperatura"] = temp; 
doc["umidade"] = umid; 
char buffer[128]; 
serializeJson(doc, buffer); 
esp_now_send(intermediate_mac, (uint8_t *)buffer, strlen(buffer) + 1); 
delay(5000); 
}