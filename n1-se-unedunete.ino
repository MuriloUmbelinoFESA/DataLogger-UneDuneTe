#include <LiquidCrystal_I2C.h>  // Biblioteca para LCD I2C
#include <RTClib.h>             // Biblioteca para Relógio em Tempo Real
#include <Wire.h>               // Biblioteca para comunicação I2C
#include <EEPROM.h>
#include "DHT.h"

#define LOG_OPTION 1     // Opção para ativar a leitura do log
#define SERIAL_OPTION 0  // Opção de comunicação serial: 0 para desligado, 1 para ligado
#define UTC_OFFSET 0     // Ajuste de fuso horário para UTC-3

// Configurações do DHT11
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal_I2C lcd(0x27, 16, 2); // Endereço de acesso: 0x3F ou 0x27
RTC_DS1307 RTC;

// LEDs e BUZZER
#define led_vermelho 7
#define buzzer 13

// Botões
#define BTN_HOME 3
#define BTN_TEMP 4
#define BTN_UMID 5
#define BTN_LUMI 6

unsigned long previousMillis = 0;
const long interval = 500;

// Configurações da EEPROM
const int maxRecords = 100;
const int recordSize = 10; // Tamanho de cada registro em bytes
int startAddress = 0;
int endAddress = maxRecords * recordSize;
int currentAddress = 0;

int currentScreen = 0; // 0=Home | 1=Temp | 2=Umidade | 3=Luminosidade
int lastLoggedMinute = -1;

// Triggers de temperatura, umidade e luminosidade
float trigger_t_min = 20.0;
float trigger_t_max = 30.0;
float trigger_u_min = 30;
float trigger_u_max = 50;
float trigger_l_min = 0;
float trigger_l_max = 30;

// Variáveis de botão
int lastButtonHome = HIGH;
int lastButtonTemp = HIGH;
int lastButtonUmid = HIGH;
int lastButtonLumi = HIGH;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  dht.begin();
  Serial.begin(9600);

  // Botões
  pinMode(BTN_HOME, INPUT_PULLUP);
  pinMode(BTN_TEMP, INPUT_PULLUP);
  pinMode(BTN_UMID, INPUT_PULLUP);
  pinMode(BTN_LUMI, INPUT_PULLUP);

  // LCD e RTC
  lcd.init();
  lcd.backlight();
  RTC.begin();
  RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // RTC.adjust(DateTime(2024, 5, 6, 08, 15, 00)); // Usar apenas 1x para ajustar

  EEPROM.begin();

  // Splash screen antes de tudo
  showSplash();
  pinMode(led_vermelho, OUTPUT);
}

void loop() {
  DateTime now = RTC.now();
  int offsetSeconds = UTC_OFFSET * 3600;
  now = now.unixtime() + offsetSeconds;
  DateTime adjustedTime = DateTime(now);

  if (LOG_OPTION) get_log();

  // Valores dos sensores
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  int luminosity = analogRead(A0);
  luminosity = map(luminosity, 0, 1023, 100, 0);

  // Verifica se o minuto atual é diferente do último log
  if (adjustedTime.minute() != lastLoggedMinute) {
    lastLoggedMinute = adjustedTime.minute();

    // Condições de alerta
    if ((temperature < trigger_t_min || temperature > trigger_t_max ||
        humidity < trigger_u_min || humidity > trigger_u_max || luminosity < trigger_l_min || luminosity < trigger_l_max) && currentScreen == 0) {

      int tempInt = (int)(temperature * 100);
      int humiInt = (int)(humidity * 100);
      int lumiInt = (int)(luminosity * 100); 

      // Salva na EEPROM
      EEPROM.put(currentAddress, now.unixtime());
      EEPROM.put(currentAddress + 4, tempInt);
      EEPROM.put(currentAddress + 6, humiInt);
      EEPROM.put(currentAddress + 8, lumiInt);

      digitalWrite(led_vermelho, HIGH);
      delay(1000);

      // Captura a temperatura
      if (temperature < trigger_t_min || temperature > trigger_t_max) {
        tone(buzzer, 2000, 500);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ANOMALIA");
        lcd.setCursor(0, 1);
        lcd.print("TEMP: ");
        lcd.print(temperature);
        lcd.print(" ");
        lcd.print((char)223);
        lcd.print("C");
        delay(3000);
        digitalWrite(led_vermelho, LOW);
      }

      // Captura a umidade
      if (humidity < trigger_u_min || humidity > trigger_u_max) {
        tone(buzzer, 2000, 500);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ANOMALIA");
        lcd.setCursor(0, 1);
        lcd.print("UMI: ");
        lcd.print(humidity);
        lcd.print(" %");
        delay(3000);
        digitalWrite(led_vermelho, LOW);
      }

      // Captura a luminosidade
      if (luminosity < trigger_l_min || luminosity > trigger_l_max) {
        tone(buzzer, 2000, 500);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ANOMALIA");
        lcd.setCursor(0, 1);
        lcd.print("LUM: ");
        lcd.print(luminosity);
        lcd.print(" %");
        delay(3000);
        digitalWrite(led_vermelho, LOW);
      }

      getNextAddress();
      lcd.clear();
    }
    digitalWrite(led_vermelho, LOW);
  }

  // Leitura dos botões
  int btnHome = digitalRead(BTN_HOME);
  int btnTemp = digitalRead(BTN_TEMP);
  int btnUmid = digitalRead(BTN_UMID);
  int btnLumi = digitalRead(BTN_LUMI);

  // Detecta quando o botão foi pressionado (bordo de descida)
  if (btnHome == HIGH) currentScreen = 0;
  if (btnTemp == HIGH) currentScreen = 1;
  if (btnUmid == HIGH) currentScreen = 2;
  if (btnLumi == HIGH) currentScreen = 3;

  // Atualiza os estados anteriores
  lastButtonHome = btnHome;
  lastButtonTemp = btnTemp;
  lastButtonUmid = btnUmid;
  lastButtonLumi = btnLumi;

  // Exibe a tela de acordo com o botão
    showScreen(adjustedTime, temperature, humidity, luminosity);
}

// ========== FUNÇÕES ==========

void showScreen(DateTime adjustedTime, float temperature, float humidity, int luminosity) {
  lcd.clear();
  switch (currentScreen) {
    case 0: // Home
      lcd.setCursor(0, 0);
      lcd.print("DATA: ");
      lcd.print(adjustedTime.day() < 10 ? "0" : "");
      lcd.print(adjustedTime.day());
      lcd.print("/");
      lcd.print(adjustedTime.month() < 10 ? "0" : "");
      lcd.print(adjustedTime.month());
      lcd.print("/");
      lcd.print(adjustedTime.year());

      lcd.setCursor(0, 1);
      lcd.print("HORA: ");
      lcd.print(adjustedTime.hour() < 10 ? "0" : "");
      lcd.print(adjustedTime.hour());
      lcd.print(":");
      lcd.print(adjustedTime.minute() < 10 ? "0" : "");
      lcd.print(adjustedTime.minute());
      lcd.print(":");
      lcd.print(adjustedTime.second() < 10 ? "0" : "");
      lcd.print(adjustedTime.second());
      break;

    case 1: // Temperatura
      lcd.setCursor(0, 0);
      lcd.print("Temperatura:");
      lcd.setCursor(0, 1);
      lcd.print(temperature);
      lcd.print((char)223);
      lcd.print("C");
      break;

    case 2: // Umidade
      lcd.setCursor(0, 0);
      lcd.print("Umidade:");
      lcd.setCursor(0, 1);
      lcd.print(humidity);
      lcd.print(" %");
      break;

    case 3: // Luminosidade
      lcd.setCursor(0, 0);
      lcd.print("Luminosidade:");
      lcd.setCursor(0, 1);
      lcd.print(luminosity);
      lcd.print(" %");
      break;
  }
}

void getNextAddress() {
  currentAddress += recordSize;
  if (currentAddress >= endAddress) {
    currentAddress = 0;
  }
}

void get_log() {
  Serial.println("Data stored in EEPROM:");
  Serial.println("Timestamp\t\tTemperature\tHumidity");

  for (int address = startAddress; address < endAddress; address += recordSize) {
    long timeStamp;
    int tempInt, humiInt, lumiInt;

    EEPROM.get(address, timeStamp);
    EEPROM.get(address + 4, tempInt);
    EEPROM.get(address + 6, humiInt);
    EEPROM.get(address + 8, lumiInt);

    float temperature = tempInt / 100.0;
    float humidity = humiInt / 100.0;
    float luminosity = lumiInt / 100.0;

    if (timeStamp != 0xFFFFFFFF) {
      DateTime dt = DateTime(timeStamp);
      Serial.print(dt.timestamp(DateTime::TIMESTAMP_FULL));
      Serial.print("\t");
      Serial.print(temperature);
      Serial.print(" C\t\t");
      Serial.print(humidity);
      Serial.println(" %");
      Serial.print(luminosity);
      Serial.println(" %");
    }
  }
}


//INICIA A EXECUÇÃO DA TELA DE INICIO
byte rolha[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};

void showSplash() {
  lcd.createChar(0, rolha);
  //lcd.createChar(1, rolha);

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Carregando ...");

  for (int pos = 0; pos < 15; pos++) {
    lcd.setCursor(pos, 1);
    lcd.write(byte(0)); // onda
    delay(300);
  }

  lcd.clear();

  lcd.createChar(1, rolha);
  lcd.setCursor(0,0);
  lcd.write(byte(1));
  String titulo = "UNE DUNE TE";
  for (int i = 0; i <= titulo.length(); i++) {
    lcd.setCursor(2, 0);
    lcd.print(titulo.substring(0, i));
    delay(200);
  }
  lcd.setCursor(15,0);
  lcd.write(byte(1));

  delay(500);

  lcd.setCursor(0,1);
  lcd.write(byte(1));
  String subtitulo = "DATA LOGGER";
  for (int i = 0; i <= subtitulo.length(); i++) {
    lcd.setCursor(2, 1);
    lcd.print(subtitulo.substring(0, i));
    delay(200);
  }
  lcd.setCursor(15,1);
  lcd.write(byte(1));

  delay(1500);
  lcd.clear();
}
