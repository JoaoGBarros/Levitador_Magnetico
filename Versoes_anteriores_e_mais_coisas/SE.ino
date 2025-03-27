#include <LiquidCrystal.h>
#include <RotaryEncoder.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h> // Para time() e NTP

#define PWM_PIN 33           // Pino de saída PWM
#define PWM_FREQUENCY 10000  // Frequência em Hz
#define PWM_RESOLUTION 8
#define sensor 4
#define clk 27
#define dt 26
#define sw 14
#define X0 20

// WiFi credentials
const char* ssid = "edge 40 neo_8360";
const char* password = "12345678";

// Grafana Cloud settings
const String USER_ID = "2336262";
const String API_KEY = "glc_eyJvIjoiMTM4MjAxMyIsIm4iOiJzdGFjay0xMjA2MDA5LWludGVncmF0aW9uLWVzcDMyX3BvbGljeSIsImsiOiJwMWg5MWlhUTIwRzcxM1BTNzlyRnF6aGQiLCJtIjp7InIiOiJwcm9kLXNhLWVhc3QtMSJ9fQ==";
const String grafana_url = "https://graphite-prod-40-prod-sa-east-1.grafana.net/graphite/metrics";


LiquidCrystal lcd(19, 23, 18, 17, 16, 15);
RotaryEncoder encoder(dt, clk);

float soma, media = 0, erro = 0, erroPrev = 0, erroAcumulado = 0, volts, distancia;
int controle, Kp = 85, Kd = 0, Ki = 0, prevKp = 85, prevKd = 0, prevKi = 0, valor = 0, newPos = 0, sel = 0;

void LeEncoder(void *pvParameters);
void syncTime();
void formatBody(char* name, int interval, float value, time_t time, String tag, JsonArray body);

void setup() {
  Serial.begin(115200);
  ledcAttach(PWM_PIN, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcWrite(PWM_PIN, 128);
  pinMode(sensor, INPUT);
  pinMode(sw, INPUT);
  lcd.begin(16, 2);

  imprimeLCD();

  xTaskCreatePinnedToCore(
    LeEncoder,   // Task function
    "Core0_Task",// Task name
    4096,       // Stack size (bytes)
    NULL,         // Parameters
    1,            // Priority (0 to configMAX_PRIORITIES-1)
    NULL,         // Task handle
    0             // Core ID (0 or 1)
  );

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

  // Sincroniza o horário
  syncTime();
}

void loop() {
   // Get current timestamp
  time_t timestamp = time(nullptr);
  // Create JSON document
  DynamicJsonDocument doc(1024);
  JsonArray body = doc.to<JsonArray>();

  formatBody("test.metric",1,Kp,timestamp,"Kp",body);
  formatBody("test.metric",1,Kd,timestamp,"Kd",body);
  formatBody("test.metric",1,Ki,timestamp,"Ki",body);
  formatBody("test.metric",1,erro,timestamp,"erro",body);
  // Serialize JSON to string
  String payload;
  serializeJson(doc, payload);
  
  // Send HTTP POST request
  HTTPClient http;
  http.begin(grafana_url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + USER_ID + ":" + API_KEY);
  
  int httpResponseCode = http.POST(payload);
  
  //comentar depois
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

  soma = 0;
  for (int i = 0; i < 20; i++) {
    volts = analogRead(sensor) * 0.0008056640625; // value from sensor * (3.3/4096)
    distancia = 29.988 * pow( volts, -1.173);
    soma += distancia;
    delay(10);
  }
  media = soma / 20;


  Serial.print("Distância: ");
  Serial.print(media);
  Serial.println("cm");

  erro = X0 - media;
  erroAcumulado += erro;
  if(media > 17.3 && media < 17.7) erroAcumulado = 0;
  controle = (erro * Kp) + ((erroPrev - erro) * Kd) + (erroAcumulado * Ki);

  int pwm_value = 128 + controle;
  pwm_value = constrain(pwm_value, 0, 255);
  ledcWrite(PWM_PIN, pwm_value);
  if (prevKp != Kp || prevKd != Kd || prevKi != Ki || erroPrev != erro) {
    imprimeLCD();
  }

  erroPrev = erro;
  prevKp = Kp;
  prevKd = Kd;
  prevKi = Ki;

  // if (millis() - lastThingerUpdate > 10) {  // Executa a cada 10ms
    // thing.handle();
    // lastThingerUpdate = millis();
  // }
}

void updatePIDLCD() {
  lcd.setCursor(0, 0);
  lcd.print("Kp:");
  lcd.print(Kp);
  lcd.setCursor(7, 0);
  lcd.print("Kd:");
  lcd.print(Kd);
  lcd.setCursor(0, 1);
  lcd.print("Ki:");
  lcd.print(Ki);
}

void updateErroLCD() {
  lcd.setCursor(7, 1);
  lcd.print("Erro:");
  lcd.print(erro);
}

void imprimeLCD() {
  lcd.clear();
  updatePIDLCD();
  updateErroLCD(); // Ponto-e-vírgula adicionado
}

void LeEncoder(void *pvParameters){
  while(1){
    //Verifica se o botao do encoder foi pressionado
    valor = digitalRead(14);
    if (valor != 1)
    {
      if(sel == 2) sel = 0;
      else sel++;
      while (digitalRead(14) == LOW)
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    //Le as informacoes do encoder
    static int pos = 0;
    encoder.tick();
    int newPos = encoder.getPosition();
    //Se a posicao foi alterada, mostra o valor
    //no Serial Monitor
    if (pos - newPos < 0) {
      if(sel == 0){
        Kp++;
      }
      if(sel == 1){
        Kd++;
      }
      if(sel == 2){
        Ki++;
      }
      pos = newPos;
    }
    if (pos - newPos > 0) {
      if(sel == 0){
        Kp--;
      }
      if(sel == 1){
        Kd--;
      }
      if(sel == 2){
        Ki--;
      }
      pos = newPos;
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// Sincroniza o horário via NTP
void syncTime() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov"); // Configura NTP
  Serial.println("Waiting for NTP time sync...");
  while (time(nullptr) < 24 * 3600) { // Espera até obter um timestamp válido
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nTime synchronized!");
}

void formatBody(char* name, int interval, float value, time_t time, String tag, JsonArray body){
  JsonObject metric = body.createNestedObject();
  metric["name"] = name;
  metric["interval"] = interval;
  metric["value"] = value;
  metric["time"] = time;
  JsonArray tags = metric.createNestedArray("tags");
  tags.add("tag="+tag);
  tags.add("source=grafana_cloud_docs");
}

