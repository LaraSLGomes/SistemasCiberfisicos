#include <WiFi.h> 
#include <esp_now.h> 
#include <ArduinoJson.h> 
void onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len) { 
StaticJsonDocument<200> doc; 
DeserializationError error = deserializeJson(doc, data, len); 
if (!error) { 
Serial.println("=== Dados recebidos do intermediário ==="); 
Serial.print("ID: "); 
Serial.println(doc["ID"].as<String>()); 
Serial.print("Contador: "); 
Serial.println(doc["contador"].as<int>()); 
Serial.print("Temperatura: "); 
Serial.println(doc["temperatura"].as<float>()); 
Serial.print("Umidade: "); 
Serial.println(doc["umidade"].as<float>()); 
Serial.println("========================================"); 
} else { 
Serial.println("Erro ao decodificar JSON"); 
} 
} 
void setup() { 
Serial.begin(115200); 
WiFi.mode(WIFI_STA); 
WiFi.disconnect(); 
if (esp_now_init() != ESP_OK) { 
Serial.println("Erro ao iniciar ESP-NOW"); 
return; 
} 
esp_now_register_recv_cb(onReceive); 
} 
void loop() { 
} 