#include <dummy.h> 
#include <DHT.h> 
#include <DHT_U.h> 
#include <esp_now.h> 
#include <WiFi.h> 
#include <ArduinoJson.h> 

#define ID "equipe50.2"
#define DHTPIN 14 
#define DHTTYPE DHT11 
#define LED_PIN 33    
#define SEND_INTERVAL 5000  
DHT dht(DHTPIN, DHTTYPE); 
uint8_t receiverMac[] = {0xA0, 0xB7, 0x65, 0xDD, 0x28, 0xD4}; 
typedef struct node_info { 
char id[2]; 
int msgCount; 
float temperature; 
float humidity; 
bool control; 
} node_info; 
typedef struct message_data { 
char id[2]; 
int msgCount; 
float temperature; 
float humidity; 
bool control; 
unsigned long timestamp; 
} message_data; 
node_info myNodeInfo; 
unsigned long lastSendTime = 0; 
int msgCounter = 0; 
#define MAX_STORED_MSGS 20 
message_data recentMessages[MAX_STORED_MSGS]; 
int msgIndex = 0; 
bool controlState = false; 
float getSimulatedTemperature() { 
return dht.readTemperature(); 
} 
float getSimulatedHumidity() { 
return dht.readHumidity(); 
} 
bool isMessageDuplicate(const char* id, int msgCount) { 
for (int i = 0; i < MAX_STORED_MSGS; i++) { 
if (strcmp(recentMessages[i].id, id) == 0 && recentMessages[i].msgCount == 
msgCount) return true; 
} 
return false; 
} 
void storeMessage(const char* id, int msgCount, float temp, float hum, bool ctrl, 
unsigned long timestamp) { 
strcpy(recentMessages[msgIndex].id, id); // strcpy = string copy 
recentMessages[msgIndex].msgCount = msgCount; 
recentMessages[msgIndex].temperature = temp; 
recentMessages[msgIndex].humidity = hum; 
recentMessages[msgIndex].control = ctrl; 
recentMessages[msgIndex].timestamp = timestamp; 
msgIndex = (msgIndex + 1) % MAX_STORED_MSGS; 
} 
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) { 
digitalWrite(LED_PIN, HIGH); 
Serial.print("Status de envio: "); 
Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Sucesso\n" : "Falha\n"); 
digitalWrite(LED_PIN, LOW); 
} 
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int 
len) { 
digitalWrite(LED_PIN, HIGH); 
char jsonBuffer[250]; 
memcpy(jsonBuffer, incomingData, len); 
jsonBuffer[len] = '\0'; 
Serial.print("\nMensagem recebida: "); 
Serial.println(jsonBuffer); 
// Parsear JSON 
StaticJsonDocument<250> doc; 
DeserializationError error = deserializeJson(doc, jsonBuffer); 
if (error) { 
Serial.print("Erro ao deserializar JSON: "); 
Serial.println(error.c_str()); 
digitalWrite(LED_PIN, LOW); 
return; 
} 
// Extrair dados 
const char* id = doc["id"]; 
int msgCount = doc["msgCount"]; 
float temperature = doc["temperature"]; 
float humidity = doc["humidity"]; 
bool control = doc["control"]; 
unsigned long timestamp = doc["timestamp"]; 
// Verificar se a mensagem é duplicada 
if (isMessageDuplicate(id, msgCount)) { 
Serial.println("Mensagem duplicada, ignorando"); 
digitalWrite(LED_PIN, LOW); 
return; 
} 
// Armazenar mensagem processada 
storeMessage(id, msgCount, temperature, humidity, control, timestamp); 
// Processar mensagem (por exemplo, verificar se é para este nó) 
if (strcmp(id, NODE_ID) != 0) { 
Serial.println("Retransmitindo mensagem..."); 
// Retransmitir a mensagem para outros nós 
esp_now_send(receiverMac, (uint8_t *)jsonBuffer, strlen(jsonBuffer)); 
// Esperar um tempo aleatório para evitar colisões 
delay(random(100, 300)); 
} 
digitalWrite(LED_PIN, LOW); 
} 
void setup() { 
randomSeed(analogRead(0));  
Serial.begin(115200); 
while (!Serial) { delay(10); } 
pinMode(LED_PIN, OUTPUT); 
WiFi.mode(WIFI_STA); 
//Wifi.channel(0); 
while (!WiFi.STA.started()) { delay(100); } 
Serial.print("Endereço MAC do dispositivo: "); 
Serial.println(WiFi.macAddress()); 
if (esp_now_init() != ESP_OK) { 
Serial.println("Erro ao inicializar ESP-NOW"); 
ESP.restart(); 
return; 
} 
// Registrar callbacks 
esp_now_register_send_cb(OnDataSent); 
esp_now_register_recv_cb(OnDataRecv); 
esp_now_peer_info_t peerInfo; 
memset(&peerInfo, 0, sizeof(esp_now_peer_info_t)); // Zera a estrutura para evitar 
lixo de memória 
memcpy(peerInfo.peer_addr, receiverMac, 6); 
peerInfo.channel = 0; 
peerInfo.encrypt = false; 
if (esp_now_add_peer(&peerInfo) != ESP_OK) { 
Serial.println("Falha ao adicionar peer"); 
return; 
} 
// Inicializar informações do nó 
strcpy(myNodeInfo.id, ID); 
myNodeInfo.msgCount = 0; 
myNodeInfo.temperature = getSimulatedTemperature(); 
myNodeInfo.humidity = getSimulatedHumidity(); 
myNodeInfo.control = false; 
// Inicializar array de mensagens recentes 
for (int i = 0; i < MAX_STORED_MSGS; i++) { 
strcpy(recentMessages[i].id, ""); 
recentMessages[i].msgCount = -1; 
} 
dht.begin(); 
Serial.println("Inicialização completa! Usando valores SIMULADOS para temperatura 
e umidade."); 
} 
void loop() { 
unsigned long currentMillis = millis(); 
// Enviar dados periodicamente 
if (currentMillis - lastSendTime >= SEND_INTERVAL) { 
lastSendTime = currentMillis; 
// Gerar valores simulados de temperatura e umidade 
float humidity = getSimulatedHumidity(); 
float temperature = getSimulatedTemperature(); 
Serial.print("Valores simulados - Temperatura: "); 
Serial.print(temperature); 
Serial.print("°C, Umidade: "); 
Serial.print(humidity); 
Serial.println("%"); 
// Atualizar informações do nó 
myNodeInfo.temperature = temperature; 
myNodeInfo.humidity = humidity; 
myNodeInfo.msgCount++; 
// Alternar o estado de controle a cada 5 mensagens 
if (myNodeInfo.msgCount % 5 == 0) { 
myNodeInfo.control = !myNodeInfo.control; 
} 
// Criar objeto JSON 
StaticJsonDocument<250> doc; 
doc["id"] = myNodeInfo.id; 
doc["msgCount"] = myNodeInfo.msgCount; 
doc["temperature"] = myNodeInfo.temperature; 
doc["humidity"] = myNodeInfo.humidity; 
doc["control"] = myNodeInfo.control; 
doc["timestamp"] = millis(); 
// Serializar JSON para string 
char jsonBuffer[250]; 
size_t jsonSize = serializeJson(doc, jsonBuffer); 
// Enviar mensagem 
digitalWrite(LED_PIN, HIGH); 
esp_now_send(receiverMac, (uint8_t *)jsonBuffer, jsonSize); 
Serial.print("Enviando: "); 
Serial.println(jsonBuffer); 
digitalWrite(LED_PIN, LOW); 
// Armazenar nossa própria mensagem para evitar retransmiti-la 
storeMessage(myNodeInfo.id, myNodeInfo.msgCount, myNodeInfo.temperature,  
myNodeInfo.humidity, myNodeInfo.control, millis()); 
} 
}