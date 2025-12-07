#include <esp_now.h>
#include <WiFi.h>
#include "DHT.h"

#define DHTPIN 19
#define DHTTYPE DHT11
#define PIR_PIN 21
#define IR_PIN 32    
#define LDR_PIN 35
#define MQ_PIN 34
 
uint8_t broadcastAddress[] = {0x94, 0xB9, 0x7E, 0xD9, 0xA7, 0xB8}; // เลขเดิมของคุณ

typedef struct struct_message {
  float temp;
  int humidity;
  int pir;
  int ir;      
  int light;
  int smoke;
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;
DHT dht(DHTPIN, DHTTYPE);

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {}

void setup() {
  Serial.begin(115200);
  
  pinMode(PIR_PIN, INPUT);
  pinMode(IR_PIN, INPUT); 
  
  dht.begin();
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) return;

  esp_now_register_send_cb((esp_now_send_cb_t)OnDataSent);

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) return;
}

void loop() {
  myData.temp = dht.readTemperature();
  myData.humidity = dht.readHumidity();
  myData.pir = digitalRead(PIR_PIN);
  myData.ir = digitalRead(IR_PIN);    
  myData.light = analogRead(LDR_PIN);
  myData.smoke = analogRead(MQ_PIN);

  if (isnan(myData.temp)) myData.temp = 0.0;

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

  Serial.print("Temp: "); Serial.print(myData.temp, 1);
  Serial.print("C");
  
  Serial.print(" | Smoke: "); Serial.print(myData.smoke);
  Serial.print(" | Light: "); Serial.print(myData.light);
  
  Serial.print(" | PIR: "); Serial.print(myData.pir);
  
  Serial.print(" | IR: "); Serial.println(myData.ir);

  delay(200); // ส่งถี่ๆ ทุก 0.2 วินาที
}
