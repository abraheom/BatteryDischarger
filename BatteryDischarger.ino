#include <LiquidCrystal.h>

// Configuracion de pines
const int PIN_RELAY = 8;
const int PIN_CONTRASTE = 9;
const int PIN_BUZZER = 10;
const int PIN_CURRENT_CONTROL = 11;
const int rs = 2, en = 3, d4 = 4, d5 = 5, d6 = 6, d7 = 7;

// Variables de descarga
const float MINIMO_VOLTAJE_BATERIA = 3.00;
float corrienteDescargada = 0.00;
boolean finalizado = false;

// Variables para lectura de voltaje
const int ITERACIONES_VOLTAJE = 500;
const float VIN_REFERENCIA = 4.569;
float voltajeBateria;
float corrienteDescarga;

// Variables de tiempo
int segundos, minutos, horas;
int inicioDescarga = 0, finDescarga, ultimoSegundoRegistrado;
int segundosDescargando;

// Configuracion de pines para pantalla LCD
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


void setup() {
  Serial.begin(9600);
  // Configuracion de pantalla LCD
  lcd.begin(16, 2);

  // Definicion del comportamiento de los pines
  pinMode(PIN_RELAY,  OUTPUT);
  pinMode(PIN_CURRENT_CONTROL, OUTPUT);


  // Ajustar el contraste de pantalla LCD
  analogWrite(PIN_CONTRASTE, 120);


  // Definir el texto de inicio en la pantalla LCD
  lcd.print("Battery");
  lcd.setCursor(0, 1);
  lcd.print("Discharger");
  delay(2500);
}

void loop() {
  // Obtener los segundos transcurridos;
  int segundosTranscurridos = millis() / 1000;

  // Asigna un valor PWM para el control de corriente
  analogWrite(PIN_CURRENT_CONTROL, 100); // 100

  // Lectura de tensiones
  voltajeBateria    = leerVoltajeEnPin(A1, true);
  corrienteDescarga = leerVoltajeEnPin(A0, false);
   
  Serial.print(" v: ");
  Serial.print(voltajeBateria, 3);
  Serial.print(" I: ");
  Serial.println(corrienteDescarga, 3);
  if (!finalizado && segundosTranscurridos > 5 && voltajeBateria >= MINIMO_VOLTAJE_BATERIA) {
    if (ultimoSegundoRegistrado != segundosTranscurridos) {
      // Utilizado para no registrar mas de una vez la descarga por segundo (Evita errores si el loop se repite varias veces por segundo)
      ultimoSegundoRegistrado = segundosTranscurridos;
      // Activar descarga de bateria
      if (inicioDescarga == 0) {
        inicioDescarga = segundosTranscurridos;
        activarDescarga();
        tone(PIN_BUZZER, 2500, 300);
      }

      // Obtiene la cantidad de segundos desde que se inicio la descarga
      segundosDescargando = segundosTranscurridos - inicioDescarga;
      corrienteDescargada += corrienteDescarga / (60 * 60);

      // Calcular horas, minutos y segundos
      horas     = (int) segundosDescargando / 60 / 60;
      minutos   = (segundosDescargando - (horas * 60 * 60)) / 60;
      segundos  = segundosDescargando - ((horas * 60 * 60) + (minutos * 60));
    }
  }
  else {
    desactivarDescarga();
    if (segundosTranscurridos > 5 && !finalizado && ultimoSegundoRegistrado > 0) {
      alertar();
      analogWrite(PIN_CURRENT_CONTROL, 0);
    }
    if (segundosTranscurridos > 5) {
      finalizado = true;
    }
  }

  Serial.println(segundos);
  // Imprimir datos en pantalla
  lcd.clear(); // Limpiar pantalla LCD
  lcd.setCursor(0, 0);
  lcd.print("V:");
  lcd.print(voltajeBateria, 3);
  lcd.print(" I:");
  lcd.print(corrienteDescarga, 3);
  lcd.setCursor(0, 1);
  lcd.print("L:");
  lcd.print(corrienteDescargada, 3);
  lcd.print(" T:");
  lcd.print(horas);
  lcd.print(":");
  lcd.print(minutos);
  lcd.print(":");
  lcd.print(segundos);

  delay(500);
}


void activarDescarga() {
  digitalWrite(PIN_RELAY, HIGH);
}

void desactivarDescarga() {
  digitalWrite(PIN_RELAY, LOW);
}

float leerVoltajeEnPin(int pin, boolean leerReferencia) {
  float voltaje = 0;
  float voltajeReferencia = leerReferencia ? calcularVoltajeReferencia() : 5;
  for (int i = 0; i < ITERACIONES_VOLTAJE; i++) {
    voltaje += analogRead(pin) * voltajeReferencia / 1023;
  }

  // Retornar el valor promedio de voltajes obtenidos
  return voltaje / ITERACIONES_VOLTAJE;
}

float calcularVoltajeReferencia() {
  float APIN = 0.00;
  for (int i = 0; i < ITERACIONES_VOLTAJE; i++) {
    APIN += analogRead(A3);
  }
  APIN = (APIN / ITERACIONES_VOLTAJE);
  float refV = VIN_REFERENCIA * 1023 / APIN;
  Serial.println(APIN);
  Serial.println(refV, 3);
  Serial.println("-------------");
  return refV;
}

void alertar() {
  tone(PIN_BUZZER, 2500, 100);
  delay(200);
  tone(PIN_BUZZER, 2500, 100);
  delay(200);
  tone(PIN_BUZZER, 2500, 100);
}
