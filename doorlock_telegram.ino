// ================================================
// SISTEMA DE ACCESO CON CLAVES, MELODIAS Y TELEGRAM
// Incluye: Teclado 4x4, Pantalla TFT, Buzzer pasivo,
// conexión WiFi y envío de mensajes por Telegram.
// Con servo que se abre si el PIN es correcto
// Autor: ChatGPT para CleiM
// ================================================

#include <Keypad.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <HTTPClient.h>

// ========================
// CREDENCIALES DE WiFi
// ========================
const char* ssid     = "*****";
const char* password = "*****";

// ========================
// CREDENCIALES DE TELEGRAM
// ========================
String botToken = "8055049592:AAEw6VNt1W2y25qzrg4JsHXv2ITNkct0ndQ";  // Token real
String chatID   = "8422986565";  // ID de chat real

// ========================
// CONFIGURACION DE TFT
// ========================
#define TFT_CS    5
#define TFT_RST   2
#define TFT_DC    4
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// ========================
// CONFIGURACION DE TECLADO
// ========================
const byte FILAS = 4;
const byte COLUMNAS = 4;
char teclas[FILAS][COLUMNAS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte pinesFila[FILAS] = {13, 12, 14, 27};
byte pinesColumna[COLUMNAS] = {25, 26, 33, 32};
Keypad teclado = Keypad(makeKeymap(teclas), pinesFila, pinesColumna, FILAS, COLUMNAS);

// ========================
// CLAVES DE USUARIOS
// ========================
String pinAntonia = "56789711A";
String pinMama    = "8074C";
String pinMartina = "58516376";
String ingreso = "";

// ========================
// BUZZER Y NOTAS MUSICALES
// ========================
#define BUZZER 19

#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_A5  880
#define REST     0

// ========================
// SERVO PARA CERRADURA
// ========================
#define PIN_SERVO 15
Servo cerradura;

void setup() {
  Serial.begin(115200);  // Para ver errores por monitor serie

  // Inicializar pantalla
  tft.initR(INITR_144GREENTAB);
  tft.fillScreen(ST77XX_WHITE);
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 20);
  tft.print("Conectando WiFi...");

  // Inicializar buzzer
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  // Inicializar servo
  cerradura.attach(PIN_SERVO);
  cerradura.write(0); // Puerta cerrada al iniciar

  // Conectar a WiFi
  WiFi.begin(ssid, password);
  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 20) {
    delay(500);
    tft.print(".");
    intentos++;
  }

  // Resultado conexión
  if (WiFi.status() == WL_CONNECTED) {
    tft.fillScreen(ST77XX_WHITE);
    tft.setCursor(10, 20);
    tft.print("WiFi OK!");
    delay(1000);
  } else {
    tft.fillScreen(ST77XX_RED);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, 20);
    tft.print("WiFi ERROR");
    delay(2000);
  }

  // Mensaje inicial
  tft.fillScreen(ST77XX_WHITE);
  tft.setTextColor(ST77XX_BLACK);
  tft.setCursor(10, 20);
  tft.print("Ingrese PIN:");
}

void loop() {
  char tecla = teclado.getKey();
  if (tecla) {
    ingreso += tecla;
    tft.setCursor(10, 50);
    tft.fillRect(10, 50, 120, 20, ST77XX_WHITE);
    tft.print(ingreso);

    if (tecla == '#') {
      String codigo = ingreso.substring(0, ingreso.length() - 1);
      ingreso = "";

      if (codigo == pinAntonia) {
        accesoPermitido("Antonia");
        enviarTelegram("🔓 Acceso correcto: Antonia");
      } else if (codigo == pinMama) {
        accesoPermitido("Mama");
        enviarTelegram("🔓 Acceso correcto: Mama");
      } else if (codigo == pinMartina) {
        accesoPermitido("Martina");
        enviarTelegram("🔓 Acceso correcto: Martina");
      } else {
        accesoDenegado();
        enviarTelegram("❌ Intento fallido con código: " + codigo);
      }

      delay(2000);
      tft.fillScreen(ST77XX_WHITE);
      tft.setCursor(10, 20);
      tft.print("Ingrese PIN:");
    }
  }
}

void accesoPermitido(String nombre) {
  tft.fillScreen(ST77XX_GREEN);
  tft.setCursor(10, 30);
  tft.print("Hola ");
  tft.print(nombre);
  dibujarCarita(true);

  // Abrir cerradura (servo)
  cerradura.write(90);   // abrir
  delay(3000);            // mantener abierta
  cerradura.write(0);    // cerrar

  // Melodía personalizada
  if (nombre == "Antonia") melodiaAntonia();
  else if (nombre == "Mama") melodiaMama();
  else if (nombre == "Martina") melodiaMartina();
}

void accesoDenegado() {
  tft.fillScreen(ST77XX_RED);
  tft.setTextColor(ST77XX_BLACK);
  tft.setCursor(10, 30);
  tft.print("Clave incorrecta");
  dibujarCarita(false);
  playMelodiaError();
}

void dibujarCarita(bool feliz) {
  tft.drawCircle(100, 90, 20, ST77XX_BLACK);
  tft.fillCircle(92, 85, 2, ST77XX_BLACK);
  tft.fillCircle(108, 85, 2, ST77XX_BLACK);

  if (feliz) {
    tft.drawLine(92, 98, 95, 102, ST77XX_BLACK);
    tft.drawLine(95, 102, 100, 104, ST77XX_BLACK);
    tft.drawLine(100, 104, 105, 102, ST77XX_BLACK);
    tft.drawLine(105, 102, 108, 98, ST77XX_BLACK);
  } else {
    tft.drawLine(92, 104, 95, 100, ST77XX_BLACK);
    tft.drawLine(95, 100, 100, 98, ST77XX_BLACK);
    tft.drawLine(100, 98, 105, 100, ST77XX_BLACK);
    tft.drawLine(105, 100, 108, 104, ST77XX_BLACK);
  }
}

void enviarTelegram(String mensaje) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "https://api.telegram.org/bot" + botToken + "/sendMessage";
    url += "?chat_id=" + chatID + "&text=" + mensaje;

    http.begin(url);
    int httpResponseCode = http.GET();
    Serial.println("Telegram status: " + String(httpResponseCode));
    http.end();
  }
}

void playNote(int freq, int duracion) {
  if (freq == REST) {
    noTone(BUZZER);
  } else {
    tone(BUZZER, freq, duracion);
  }
  delay(duracion * 1.3);
}

void playMelodiaError() {
  int melodia[] = {NOTE_E4, REST, NOTE_E4, REST, NOTE_C4};
  int dur[]     = {150, 100, 150, 100, 400};
  for (int i = 0; i < 5; i++) playNote(melodia[i], dur[i]);
}

void melodiaAntonia() {
  int notas[] = {NOTE_C4, NOTE_D4, NOTE_E4, NOTE_G4, NOTE_C5};
  int dur[]   = {200, 200, 200, 300, 500};
  for (int i = 0; i < 5; i++) playNote(notas[i], dur[i]);
}

void melodiaMama() {
  int notas[] = {NOTE_G4, NOTE_E4, NOTE_D4, NOTE_C4, REST, NOTE_C4};
  int dur[]   = {300, 300, 300, 400, 200, 400};
  for (int i = 0; i < 6; i++) playNote(notas[i], dur[i]);
}

void melodiaMartina() {
  int notas[] = {NOTE_E4, NOTE_C4, NOTE_G4, NOTE_E5, NOTE_D5};
  int dur[]   = {150, 150, 150, 300, 400};
  for (int i = 0; i < 5; i++) playNote(notas[i], dur[i]);
}
