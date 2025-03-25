#include <Ultrasonic.h>
#include <LiquidCrystal.h>
#include <WiFiMulti.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include <WiFi.h>

// Definições para a conexão Wi-Fi e InfluxDB
WiFiMulti wifiMulti;
#define WIFI_SSID "SSID"
#define WIFI_PASSWORD "PASSWORD"
#define INFLUXDB_URL "http://localhost:8086"
#define INFLUXDB_TOKEN "WAHU2NA34uJI9rWKQEiNPUT-yomPtcNGGr8IDzG-s_7aYWgzYC3RbHIev5RdluRGNpMC__hNuMSkdVlyUXDJLQ=="
#define INFLUXDB_ORG "myorg"
#define INFLUXDB_BUCKET "Levitador Magnetico"
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

Point results("Control_variables"); // Ponto para envio de dados

// Definições para o sensor ultrassônico
#define pino_trigger 4
#define pino_echo 5
#define X0 16.5

Ultrasonic ultrasonic(pino_trigger, pino_echo);
float cmMsec, soma, media, erro, erroPrev = 0;
long microsec;
int controle, Kp = 110, Kd = 50, Ki = 0, prevKp = 110, prevKd = 50, prevKi = 0;

// Definições para o display LCD
LiquidCrystal lcd(13, 12, 11, 10, 9, 8);

// Configuração do PWM usando LEDC no ESP32
#define PWM_PIN 3           // Pino de saída PWM
#define PWM_CHANNEL 0       // Canal LEDC
#define PWM_FREQUENCY 5000  // Frequência em Hz
#define PWM_RESOLUTION 8    // Resolução de 8 bits

void processaEntradaSerial() {
  Serial.println("Aguardando valores de PID pela Serial...");
  while (true) {
    while (!Serial.available());
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.length() == 0) break;

    if (input.startsWith("P:")) {
      Kp = input.substring(2).toInt();
    } else if (input.startsWith("I:")) {
      Ki = input.substring(2).toInt();
    } else if (input.startsWith("D:")) {
      Kd = input.substring(2).toInt();
    } else if (input.startsWith("PI:")) {
      int piIndex = input.indexOf(',');
      Kp = input.substring(3, piIndex).toInt();
      Ki = input.substring(piIndex + 1).toInt();
    } else if (input.startsWith("PD:")) {
      int pdIndex = input.indexOf(',');
      Kp = input.substring(3, pdIndex).toInt();
      Kd = input.substring(pdIndex + 1).toInt();
    } else if (input.startsWith("ID:")) {
      int idIndex = input.indexOf(',');
      Ki = input.substring(3, idIndex).toInt();
      Kd = input.substring(idIndex + 1).toInt();
    } else if (input.startsWith("PID:")) {
      int pIndex = input.indexOf(',');
      int iIndex = input.indexOf(',', pIndex + 1);
      Kp = input.substring(4, pIndex).toInt();
      Ki = input.substring(pIndex + 1, iIndex).toInt();
      Kd = input.substring(iIndex + 1).toInt();
      break;
    }
  }
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

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(100);
  }

  if (client.validateConnection()) {
    Serial.print("Conectado ao InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("Falha na conexão: ");
    Serial.println(client.getLastErrorMessage());
  }
  
  processaEntradaSerial();
  imprimeLCD();
  
  // Configurar e enviar dados iniciais
  results.addField("Kp", Kp);
  results.addField("Ki", Ki);
  results.addField("Kd", Kd);
  if (!client.writePoint(results)) {
    Serial.print("Falha ao escrever: ");
    Serial.println(client.getLastErrorMessage());
  }
}

void loop() {
  soma = 0;
  for (int i = 0; i < 20; i++) {
    microsec = ultrasonic.timing();
    cmMsec = ultrasonic.convert(microsec, Ultrasonic::CM);
    soma += cmMsec;
    delay(2);
  }
  media = soma / 20;

  erro = X0 - media;
  controle = (erro * Kp) + ((erroPrev - erro) * Kd);

  int pwm_value = 128 + controle;
  pwm_value = constrain(pwm_value, 0, 255);


  Serial.print("Kd: ");
  Serial.print(Kd);
  Serial.print(" Kp: ");
  Serial.println(Kp);

  // Preparar e enviar dados para o InfluxDB
  results.clearFields();
  results.addField("Kp", Kp);
  results.addField("Ki", Ki);
  results.addField("Kd", Kd);
  results.addField("Erro", erro);
  if (!client.writePoint(results)) {
    Serial.print("Falha ao escrever: ");
    Serial.println(client.getLastErrorMessage());
  }

  if (prevKp != Kp || prevKd != Kd || prevKi != Ki || erroPrev != erro) {
    imprimeLCD();
  }

  erroPrev = erro;
  prevKp = Kp;
  prevKd = Kd;
  prevKi = Ki;
  delay(50);
}