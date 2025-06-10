#include <WiFi.h> 
#include <esp_now.h> 

uint8_t final_mac[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x02};
void onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len) { 
esp_now_send(final_mac, data, len); 
} 
void onSend(const uint8_t *mac_addr, esp_now_send_status_t status) { 
Serial.print("Reenvio: "); 
Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Sucesso" : "Falha"); 
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
esp_now_register_send_cb(onSend); 
esp_now_peer_info_t peerInfo = {}; 
memcpy(peerInfo.peer_addr, final_mac, 6); 
peerInfo.channel = 0; 
peerInfo.encrypt = false; 
esp_now_add_peer(&peerInfo); 
} 
void loop() { 
    
}