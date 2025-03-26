#include <WiFi.h>
#include <time.h>
#include <HTTPClient.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* graphite_url = "https://graphite-prod-40-prod-sa-east-1.grafana.net/graphite/metrics";  // URL do Graphite-web
const char* user_id = "2336262";
const char* api_key = "glc_eyJvIjoiMTM4MjAxMyIsIm4iOiJzdGFjay0xMjA2MDA5LWludGVncmF0aW9uLWVzcDMyX3BvbGljeSIsImsiOiJwMWg5MWlhUTIwRzcxM1BTNzlyRnF6aGQiLCJtIjp7InIiOiJwcm9kLXNhLWVhc3QtMSJ9fQ==";
const char* name    = "test-metric";


void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }

  Serial.println("WiFi Conectado!");
  int value = 0.0;
}

void loop() {
  value += 1.0;
  HTTPClient http;
  http.begin(graphite_url);
  
  // Adiciona cabeçalhos de autenticação
  http.addHeader("Content-Type", "application/json");
  http.addHeader("user_id", user_id);
  http.addHeader("api_key", api_key);
  
  // Cria o JSON com os dados da métrica
  String jsonKp = "[{\"name\": \"%s\", \"value\": %f, \"time\": " + String(time(NULL)) + "\"tags\": [\"tag=Kp\", \"source=grafana_cloud_docs\"]}]", name, value;
  String jsonKd = "[{\"name\": \"%s\", \"value\": %f, \"time\": " + String(time(NULL)) + "\"tags\": [\"tag=Kd\", \"source=grafana_cloud_docs\"]}]", name, value;
  String jsonKi = "[{\"name\": \"%s\", \"value\": %f, \"time\": " + String(time(NULL)) + "\"tags\": [\"tag=Ki\", \"source=grafana_cloud_docs\"]}]", name, value;
  String json_erro   = "[{\"name\": \"%s\", \"value\": %f, \"time\": " + String(time(NULL)) + "\"tags\": [\"tag=erro\", \"source=grafana_cloud_docs\"]}]", name, value;
  // Serial.println("Enviando JSON: " + jsonKp);
  
  // Envia os dados via POST
  int httpKpResponseCode = http.POST(jsonKp);
  // Exibe a resposta do servidor
  // Serial.println("Resposta HTTP: " + String(httpKpResponseCode));
  // if (httpKpResponseCode > 0) {
    // Serial.println("Resposta do servidor: " + http.getString());
  // } else {
    // Serial.println("Erro ao enviar a requisição!");
  // }
  int httpKdResponseCode = http.POST(jsonKd);
  int httpKiResponseCode = http.POST(jsonKi);
  int httpErroResponseCode = http.POST(json_erro);

  http.end();
}