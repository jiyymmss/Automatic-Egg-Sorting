#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h> // Needed for JSON parsing

// ================= WIFI =================
const char* ssid     = "vivo Y27";
const char* password = "12356789";

// ================= SERVER =================
const char* serverURL = "http://10.220.130.140/eggsorting/api/add_egg.php";
const char* scheduleURL = "http://10.220.130.140/eggsorting/api/schedule.php"; // <-- New endpoint for feeding schedules

WiFiClient client;

// ---------------- UART ----------------
SoftwareSerial unoSerial(4, 5); // RX, TX (pins you choose)
// ================= SEND TO MYSQL =================
void sendToMySQL(char eggType) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    return;
  }

  HTTPClient http;
  http.begin(client, serverURL);  // <-- pass WiFiClient to begin()
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String postData = "type=" + String(eggType);
  int httpResponse = http.POST(postData);

  if (httpResponse > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponse);
    String payload = http.getString();
    Serial.print("Server response: ");
    Serial.println(payload);
  } else {
    Serial.print("HTTP POST failed, error: ");
    Serial.println(http.errorToString(httpResponse));
  }

  http.end();
}


// ================== FEED SCHEDULE ==================
unsigned long lastFetch = 0;
const unsigned long fetchInterval = 10000; // 60 seconds

void sendScheduleToUno(int hour, int minute) {
  char buffer[12];
  snprintf(buffer, sizeof(buffer), "F,%02d:%02d:1", hour, minute); 
  unoSerial.println(buffer);
  // optional debug to Serial monitor
  Serial.print("Sent to Uno: "); Serial.println(buffer);
}

void fetchSchedules() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(client, scheduleURL);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);
    if (!error) {
      for (JsonObject schedule : doc.as<JsonArray>()) {
        String timeStr = schedule["feed_time"];
        int hour   = timeStr.substring(0,2).toInt();
        int minute = timeStr.substring(3,5).toInt();
        sendScheduleToUno(hour, minute);
      }
      Serial.println("END"); // Signal Uno all schedules sent
    }
  }

  http.end();
}
// ================= SETUP =================
void setup() {
  Serial.begin(9600);         
  unoSerial.begin(9600);              // Enable RX/TX on GPIO1/3 for UNO

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
  Serial.println("ESP8266 Ready. Waiting for UNO...");
}

// ================= LOOP =================
void loop() {
  // ---------- SEND EGG DATA ----------
  if (unoSerial.available()) {
    char egg = unoSerial.read();

    if (egg == 'S' || egg == 'M' || egg == 'L') {
      Serial.print("Received egg: ");
      Serial.println(egg);

      sendToMySQL(egg);
    }
  }

  // ---------- FETCH FEED SCHEDULE EVERY MINUTE ----------
  if (millis() - lastFetch > fetchInterval) {
    fetchSchedules();
    lastFetch = millis();
  }
}
