#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "edge 40 neo_8360";
const char* password = "12345678";

// Grafana Cloud settings
const String USER_ID = "2336262";
const String API_KEY = "glc_eyJvIjoiMTM4MjAxMyIsIm4iOiJzdGFjay0xMjA2MDA5LWludGVncmF0aW9uLWVzcDMyX3BvbGljeSIsImsiOiJwMWg5MWlhUTIwRzcxM1BTNzlyRnF6aGQiLCJtIjp7InIiOiJwcm9kLXNhLWVhc3QtMSJ9fQ==";
const String grafana_url = "https://graphite-prod-40-prod-sa-east-1.grafana.net/graphite/metrics";

// Metric values
float valueKp = 15.3;
float valueKd = 12.7;
float valueKi = 11.5;
float valueErro = 9.4;

void setup() {
  Serial.begin(115200);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Increment values
  valueKp += 1.0;
  valueKd += 1.0;
  valueKi += 1.0;
  valueErro += 1.0;
  
  // Get current timestamp
  time_t timestamp = time(nullptr);
  
  // Create JSON document
  DynamicJsonDocument doc(1024);
  JsonArray body = doc.to<JsonArray>();
  
  // Add Kp metric
  JsonObject metric1 = body.createNestedObject();
  metric1["name"] = "test.metric";
  metric1["interval"] = 1;
  metric1["value"] = valueKp;
  metric1["time"] = timestamp;
  JsonArray tags1 = metric1.createNestedArray("tags");
  tags1.add("tag=Kp");
  tags1.add("source=grafana_cloud_docs");
  
  // Add Kd metric
  JsonObject metric2 = body.createNestedObject();
  metric2["name"] = "test.metric";
  metric2["interval"] = 1;
  metric2["value"] = valueKd;
  metric2["time"] = timestamp;
  JsonArray tags2 = metric2.createNestedArray("tags");
  tags2.add("tag=Kd");
  tags2.add("source=grafana_cloud_docs");
  
  // Add Ki metric
  JsonObject metric3 = body.createNestedObject();
  metric3["name"] = "test.metric";
  metric3["interval"] = 1;
  metric3["value"] = valueKi;
  metric3["time"] = timestamp;
  JsonArray tags3 = metric3.createNestedArray("tags");
  tags3.add("tag=Ki");
  tags3.add("source=grafana_cloud_docs");
  
  // Add Erro metric
  JsonObject metric4 = body.createNestedObject();
  metric4["name"] = "test.metric";
  metric4["interval"] = 1;
  metric4["value"] = valueErro;
  metric4["time"] = timestamp;
  JsonArray tags4 = metric4.createNestedArray("tags");
  tags4.add("tag=erro");
  tags4.add("source=grafana_cloud_docs");
  
  // Serialize JSON to string
  String payload;
  serializeJson(doc, payload);
  
  // Send HTTP POST request
  HTTPClient http;
  http.begin(grafana_url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + USER_ID + ":" + API_KEY);
  
  int httpResponseCode = http.POST(payload);
  
  // Print response
  Serial.print("Status Code: ");
  Serial.println(httpResponseCode);
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.print("Response: ");
    Serial.println(response);
    
    // Try to parse JSON response
    DynamicJsonDocument responseDoc(1024);
    DeserializationError error = deserializeJson(responseDoc, response);
    
    if (error) {
      Serial.print("Error parsing JSON: ");
      Serial.println(error.c_str());
      Serial.print("Raw content: ");
      Serial.println(response);
    } else {
      Serial.print("JSON Response: ");
      serializeJsonPretty(responseDoc, Serial);
      Serial.println();
    }
  } else {
    Serial.print("Error on HTTP request: ");
    Serial.println(http.errorToString(httpResponseCode));
  }
  
  http.end();
  
  // Wait before next iteration
  delay(5000); // 5 seconds delay
}
