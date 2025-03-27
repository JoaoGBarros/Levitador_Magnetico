//Programa: Conectando Sensor Ultrassonico HC-SR04 ao Arduino
//Autor: MakerHero

//Carrega a biblioteca do sensor ultrassonico
#include <Ultrasonic.h>
#include <RotaryEncoder.h>


//Define os pinos para o trigger e echo
#define pino_trigger 4
#define pino_echo 5
#define X0 16.5

RotaryEncoder encoder(A2, A3);

//Inicializa o sensor nos pinos definidos acima
Ultrasonic ultrasonic(pino_trigger, pino_echo);
float cmMsec, soma, media, erro, erroPrev = 0;
long microsec;
int controle, Kp = 0, Kd = 0, Ki = 0, select = 0, valor = 0;


void setup()
{
  Serial.begin(9600);
  Serial.println("Lendo dados do sensor...");
  pinMode(3, OUTPUT);
  pinMode(7, INPUT);
  TCCR2A = (1 << COM2B1) | (1 << WGM21) | (1 << WGM20); // Modo Fast PWM, não-invertido
  TCCR2B = (1 << CS20); // Prescaler de 8
   
   // Define o valor do registrador OCR2B para controlar o duty cycle
  OCR2B = 255;
}

void loop()
{
  valor = digitalRead(7);
  if (valor != 1)
  {
    if(select == 2) select = 0;
    else select++;
    while (digitalRead(7) == 0)
      delay(10);
  }

  static int pos = 0;
  encoder.tick();
  RotaryEncoder::Direction dir = encoder.getDirection();

  if (dir == RotaryEncoder::Direction::COUNTERCLOCKWISE) {
    Serial.print("entrou");
    if(select == 0){
      Kp++;
      Serial.print("Kd: ");
      Serial.print(Kd);
      Serial.print(" Kp: ");
      Serial.println(Kp);
      Serial.print(" Ki: ");
      Serial.println(Ki);
    }
    else if(select == 1){
      Kd++;
      Serial.print("Kd: ");
      Serial.print(Kd);
      Serial.print(" Kp: ");
      Serial.println(Kp);
      Serial.print(" Ki: ");
      Serial.println(Ki);
    }
    else if(select == 2){
      Ki++;
      Serial.print("Kd: ");
      Serial.print(Kd);
      Serial.print(" Kp: ");
      Serial.println(Kp);
      Serial.print(" Ki: ");
      Serial.println(Ki);
    }
  }

  else if (dir == RotaryEncoder::Direction::CLOCKWISE) {
  
    if(select == 0){
      Kp--;
      Serial.print("Kd: ");
      Serial.print(Kd);
      Serial.print(" Kp: ");
      Serial.println(Kp);
      Serial.print(" Ki: ");
      Serial.println(Ki);
    }
    else if(select == 1){
      Kd--;
      Serial.print("Kd: ");
      Serial.print(Kd);
      Serial.print(" Kp: ");
      Serial.println(Kp);
      Serial.print(" Ki: ");
      Serial.println(Ki);
    }
    else if(select == 2){
      Ki--;
      Serial.print("Kd: ");
      Serial.print(Kd);
      Serial.print(" Kp: ");
      Serial.println(Kp);
      Serial.print(" Ki: ");
      Serial.println(Ki);
    }
  }
  else{
    if(select == 0){
      Serial.print("Kd: ");
      Serial.print(Kd);
      Serial.print(" Kp: ");
      Serial.println(Kp);
      Serial.print(" Ki: ");
      Serial.println(Ki);
    }
    else if(select == 1){
      Serial.print("Kd: ");
      Serial.print(Kd);
      Serial.print(" Kp: ");
      Serial.println(Kp);
      Serial.print(" Ki: ");
      Serial.println(Ki);
    }
    else if(select == 2){
      Serial.print("Kd: ");
      Serial.print(Kd);
      Serial.print(" Kp: ");
      Serial.println(Kp);
      Serial.print(" Ki: ");
      Serial.println(Ki);
    }  
  }

  //Le as 
  soma = 0;
  //Le as informacoes do sensor, em cm e pol
  for(int i = 0;i < 20; i++){
    microsec = ultrasonic.timing();
    cmMsec = ultrasonic.convert(microsec, Ultrasonic::CM);
    soma += cmMsec;
  } 
  media = soma / 20;

  erro = X0 - media;
  
  controle = (erro * Kp) + ((erroPrev - erro) * Kd);

  OCR2B = 128 + controle;

  //Serial.print("Media de distancia em cm: ");

  Serial.print("Media: ");
  Serial.println(media);

  Serial.print("Erro: ");
  Serial.println(erro);

  Serial.print("Controle: ");
  Serial.println(controle);
  
  //OCR2B = analogRead(A0) / 4;
  erroPrev = erro;
}
