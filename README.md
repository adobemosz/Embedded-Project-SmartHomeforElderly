//USER CONFIGURATION SETTINGS

// 1. BLYNK SETTINGS (From Blynk Console)
// ------------------------------------------
#define BLYNK_TEMPLATE_ID   "TMPL6n3U3mg8I"      // Replace with your Template ID
#define BLYNK_TEMPLATE_NAME "SmartHome"          // Replace with your Device Name
#define BLYNK_AUTH_TOKEN    "Your_Blynk_Token"   // Replace with your Auth Token

// 2. WIFI SETTINGS
#define WIFI_SSID           "Your_WiFi_Name"     // Your Network Name (2.4GHz)
#define PASSWORD            "Your_WiFi_Password" // Your Network Password

// 3. LINE MESSAGING API SETTINGS (From developers.line.biz)
// The long string found under "Channel access token"
#define LINE_ACCESS_TOKEN   "Your_Long_Channel_Access_Token" 
// The string starting with 'U' found under "Your user ID"
#define LINE_USER_ID        "Your_User_ID"       

// 4. GOOGLE SHEETS SETTINGS (From Apps Script Deployment)
// The Web App URL (must end with /exec)
String GAS_URL = "https://script.google.com/macros/s/Your_Script_ID/exec";

// 5. HARDWARE PIN MAPPING (Based on README)
#define RELAY_PUMP_PIN      4   // Active LOW Relay
#define BUZZER_PIN          21  // Active Buzzer
#define MIC_PIN             35  // KY-038 Microphone (Analog AO)
#define LED_PIR_PIN         32  // Alarm LED (Latch Mode)
#define LED_NIGHT_PIN       19  // Night Light LED
#define LED_ALERT_PIN       18  // General Alert LED

// 6. SENSOR THRESHOLDS (Adjust via testing)
int SMOKE_LIMIT = 2000;         // Trigger value for MQ2
int LIGHT_LIMIT = 1000;         // Trigger value for LDR (Darkness)
int VOLUME_THRESHOLD = 1500;    // Loudness trigger for "Help" detection backup

