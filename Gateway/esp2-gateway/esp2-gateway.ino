#define BLYNK_TEMPLATE_ID   "TMPL6n3U3mg8I"
#define BLYNK_TEMPLATE_NAME "SmartHome"
#define BLYNK_AUTH_TOKEN    "lKkhvTJ6sDQ_jdUsatIn_gzczVe15Zws"

#define WIFI_SSID   "Mosy"            
#define PASSWORD    "tummaitonghai" 


#include <esp_now.h>
#include <WiFi.h>
#include <HTTPClient.h>             
#include <BlynkSimpleEsp32.h>       
#include <WiFiClientSecure.h> 

// AI Library
#include <adobemosz-project-1_inferencing.h>

// LINE Messaging API
#define LINE_ACCESS_TOKEN "ObHMqewvNtjj+aM6L9AhzC33pIDDhbs+J2euEaXzRGRZWEAoSxOxMvl9NiHNq7OtRa/CyQ6cFDzkdL1G9ZapwUnYL1VfJM90urtLltLjcNpvmixSKMyXBgEoZZrlsCcg+wbZrD/V+5kef+woIiGb6AdB04t89/1O/w1cDnyilFU="
#define LINE_USER_ID      "Ua891789973188d082543ecd94cbc3f3e"

// Google Sheets
String GAS_URL = "https://script.google.com/macros/s/AKfycbyhSATucodBn0M0oKyrILGZfuOxYIvJ5dZNX0uj8oBvNpp8rkhznyW89L1AOZ8bk8myAA/exec";

// Pin Config
#define RELAY_PUMP_PIN 4   
#define BUZZER_PIN     21  
#define LED_ALERT_PIN  18  // ไฟแจ้งเตือนทั่วไป (IR ล้ม)
#define LED_NIGHT_PIN  19  // ไฟกลางคืน (LDR)
#define LED_PIR_PIN    32  // ไฟกันขโมย (PIR ตัวใหม่)
#define MIC_PIN        35  

// AI Settings
#define EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW 3
static float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];

int SMOKE_LIMIT = 2000;
int LIGHT_LIMIT = 1000;

int VOLUME_THRESHOLD = 1500; 

unsigned long lastSheetTime = 0;
unsigned long lastLineTime = 0;
unsigned long lastAiTime = 0;

volatile bool newData = false;

bool pir_latch_state = false; 
int manual_night_light = 0;   // ตัวรับค่าสวิตช์ V6

typedef struct struct_message {
  float temp;
  int humidity;
  int pir;
  int ir;
  int light;
  int smoke;
} struct_message;

struct_message incomingReadings;

void playTone(int freq) {
  if (freq > 0) ledcWriteTone(BUZZER_PIN, freq);
  else ledcWriteTone(BUZZER_PIN, 0);
}


BLYNK_WRITE(V6) {
  manual_night_light = param.asInt(); 
}

BLYNK_WRITE(V7) {
  if (param.asInt() == 1) {
    pir_latch_state = false; // ปลดล็อคไฟ PIR
    Serial.println("🔄 PIR Alarm Reset via Blynk!");
  }
}

void sendToGoogleSheets() {
  if (millis() - lastSheetTime > 15000) {
    if(WiFi.status() == WL_CONNECTED){
      WiFiClientSecure client;
      client.setInsecure();
      HTTPClient http;
      http.begin(client, GAS_URL); 
      http.addHeader("Content-Type", "application/json");
      
      String json = "{";
      json += "\"temp\":" + String(incomingReadings.temp) + ",";
      json += "\"humid\":" + String(incomingReadings.humidity) + ",";
      json += "\"smoke\":" + String(incomingReadings.smoke) + ",";
      json += "\"light\":" + String(incomingReadings.light) + ",";
      json += "\"pir\":" + String(incomingReadings.pir) + ",";
      json += "\"ir\":" + String(incomingReadings.ir);
      json += "}";
      
      int httpResponseCode = http.POST(json);
      http.end();
      lastSheetTime = millis();
    }
  }
}

void sendLineMessage(String message) {
  if (millis() - lastLineTime > 10000) { 
    if (WiFi.status() == WL_CONNECTED) {
      WiFiClientSecure client;
      client.setInsecure(); 

      if (client.connect("api.line.me", 443)) {
        String json = "{\"to\":\"" + String(LINE_USER_ID) + "\",\"messages\":[{\"type\":\"text\",\"text\":\"" + message + "\"}]}";
        
        client.println("POST /v2/bot/message/push HTTP/1.1");
        client.println("Host: api.line.me");
        client.println("Authorization: Bearer " + String(LINE_ACCESS_TOKEN));
        client.println("Content-Type: application/json");
        client.print("Content-Length: ");
        client.println(json.length());
        client.println();
        client.println(json);
        
        Serial.println("✅ LINE Sent: " + message);
        lastLineTime = millis();
      }
    }
  }
}

void runAI() {
  if (millis() - lastAiTime > 5000) {
    
    float max_volume = 0;

    for (int i = 0; i < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; i++) {
        int raw = analogRead(MIC_PIN);
        features[i] = raw;
        if (abs(raw - 2000) > max_volume) {
           max_volume = abs(raw - 2000);
        }
        delayMicroseconds((1000000 / EI_CLASSIFIER_FREQUENCY) - 50); 
    }
    
    Serial.print("🔊 Volume: "); Serial.println(max_volume);

    signal_t signal;
    numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
    ei_impulse_result_t result = { 0 };
    run_classifier(&signal, &result, false);

    bool ai_detected = false;
    
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        String label = String(result.classification[ix].label);
        if ((label == "Help" || label == "help" || label == "_") && result.classification[ix].value > 0.7) {
            // ai_detected = true; 
        }
    }

    if (max_volume > VOLUME_THRESHOLD) {
        Serial.println("🔊 Loud Noise Detected! (Assume: Help)");
        ai_detected = true;
    }

    if (ai_detected) {
         Serial.println("🆘 AI DETECTED: HELP!");
         sendLineMessage("🗣️ ได้ยินเสียงร้องขอความช่วยเหลือ!!");
         playTone(2000);
         delay(1500);
         playTone(0);
    }

    lastAiTime = millis();
  }
}

void OnDataRecv(const esp_now_recv_info_t * info, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  newData = true;
}

void processSystem() {
  // ส่ง Blynk
  Blynk.virtualWrite(V0, incomingReadings.temp);
  Blynk.virtualWrite(V1, incomingReadings.humidity);
  Blynk.virtualWrite(V2, incomingReadings.smoke);
  Blynk.virtualWrite(V3, incomingReadings.light);
  Blynk.virtualWrite(V4, incomingReadings.pir);
  Blynk.virtualWrite(V5, incomingReadings.ir);

  Serial.print("Rx | Smoke: "); Serial.print(incomingReadings.smoke);
  Serial.print(" | PIR: "); Serial.println(incomingReadings.pir);

  if (incomingReadings.smoke > SMOKE_LIMIT) {
    digitalWrite(RELAY_PUMP_PIN, HIGH); // สั่ง LOW เพื่อเปิดปั๊ม
    playTone(3000);
    sendLineMessage("🔥 ไฟไหม้! ควันโขมง (Pump ON)"); 
  } else {
    digitalWrite(RELAY_PUMP_PIN, LOW); // สั่ง HIGH เพื่อปิด
  }
    
  if (incomingReadings.ir == 0) { 
    playTone(2000); 
    sendLineMessage("⚠️ ช่วยด้วย! มีคนล้ม (Fall Detected)");
  } else {
    if (incomingReadings.smoke <= SMOKE_LIMIT) playTone(0);
  }

  // ถ้าเจอคนขยับ ให้ล็อคค่าเป็นจริง (Latch)
  if (incomingReadings.pir == 1) {
    if (pir_latch_state == false) {
       pir_latch_state = true;
       sendLineMessage("🏃 มีผู้บุกรุก! (Alarm Locked)");
    }
  }

  if (pir_latch_state == true) {
    digitalWrite(LED_PIR_PIN, HIGH); // ติดค้างยาวๆ
  } else {
    digitalWrite(LED_PIR_PIN, LOW);  // ดับเมื่อกด Reset ที่ V7
  }

  // ถ้าเปิดสวิตช์ในแอป (Manual) หรือ แสงน้อย (Auto) -> ไฟติด
  if (manual_night_light == 1 || incomingReadings.light < LIGHT_LIMIT) {
    digitalWrite(LED_NIGHT_PIN, HIGH);
  } else {
    digitalWrite(LED_NIGHT_PIN, LOW);
  }

  if (incomingReadings.ir == 0) {
    digitalWrite(LED_ALERT_PIN, HIGH);
  } else {
    digitalWrite(LED_ALERT_PIN, LOW);
  }
}

void setup() {
  Serial.begin(115200);
  
  Serial.println("Allocating AI Memory...");
  for (int i = 0; i < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; i++) features[i] = 0;
  signal_t signal;
  int err = numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
  ei_impulse_result_t result = { 0 };
  run_classifier(&signal, &result, false);
  Serial.println("✅ AI Ready!");

  pinMode(RELAY_PUMP_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_ALERT_PIN, OUTPUT);
  pinMode(LED_NIGHT_PIN, OUTPUT);
  pinMode(LED_PIR_PIN, OUTPUT); 

  digitalWrite(RELAY_PUMP_PIN, LOW); // ปิดปั๊มก่อน (Active Low)

  ledcAttach(BUZZER_PIN, 2000, 8);

  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, PASSWORD);
  
  sendLineMessage("🤖 System Online: Full Option Ready!");

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
  
  Serial.println("--- SYSTEM COMPLETE ---");
}

void loop() {
  Blynk.run(); 
  
  if (newData) {
    newData = false;
    processSystem();
  }
  
  runAI();
  sendToGoogleSheets();
}
