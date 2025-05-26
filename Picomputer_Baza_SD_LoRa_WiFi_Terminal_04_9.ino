#define SPEAKER_PIN 0

#define TFT_MISO  20
#define TFT_MOSI  19
#define TFT_SCLK  18
#define TFT_CS   21  // Chip select control pin
#define TFT_DC   16  // Data Command control pin
#define TFT_RST  -1  // Reset pin (could connect to Arduino RESET pin)
#define TFT_BL    -1 // LED back-light

#define TOUCH_CS 17     // Chip select pin (T_CS) of touch screen
#define MISO  12 // AKA SPI1 RX
#define MOSI  11  // AKA SPI1 TX
#define SCK  10
//SD
#define SD_CS  13
// Konfiguracja pinów LoRa
#define LoRa_CS 26     // GPIO13 jako CS
#define LoRa_IRQ 27   // GPIO25 jako IRQ (DIO0)
#define LoRa_RESET 28  // GPI28 RESET
String outgoing;              // outgoing message
#define AMBER_COLOR 0xCA00
#define GREEN_TERM 0x07F1  // czysty zielony RGB565
//#include <Fonts/FreeMonoBold12pt7b.h>
#include <LittleFS.h>
#include "FS.h"
#include <SPI.h>
#include <TFT_eSPI.h>      // Hardware-specific library
#include <SD.h>
#include <LoRa.h>
#include <WiFi.h>
#include <Ticker.h>


Ticker ledTicker;
bool ledState = false;


TFT_eSPI tft = TFT_eSPI(480,320); // Invoke custom library

#define CALIBRATION_FILE "/TouchCalData1"
File calibrationFile;       // Plik kalibracyjny na karcie SD
// Repeat calibration if you change the screen rotation.
#define REPEAT_CAL false


//Klawiatura
const int NUM_ROWS = 6;  // liczba wierszy
const int NUM_COLS = 6;  // liczba kolumn

// Mapa klawiszy dla różnych trybów
String keys1[NUM_ROWS][NUM_COLS] = {
  {" ", ".", "m", "n", "b", "dn"},
  {"ent", "l", "k", "j", "h", "lt"},
  {"p", "o", "i", "u", "y", "up"},
  {"del", "z", "x", "c", "v", "rt"},
  {"a", "s", "d", "f", "g", "esc"},
  {"q", "w", "e", "r", "t", "alt"}
};

String keys2[NUM_ROWS][NUM_COLS] = {
  {":", ";", "M", "N", "B", "dn"},
  {"ent", "L", "K", "J", "H", "lt"},
  {"P", "O", "I", "U", "Y", "up"},
  {"del", "Z", "X", "C", "V", "rt"},
  {"A", "S", "D", "F", "G", "esc"},
  {"Q", "W", "E", "R", "T", "alt"}
};

String keys3[NUM_ROWS][NUM_COLS] = {
  {"_", ",", ">", "<", "\"", "{"},
  {"~", "-", "*", "&", "+", "["},
  {"0", "9", "8", "7", "6", "}"},
  {"=", "(", ")", "?", "/", "]"},
  {"!", "@", "#", "$", "%", "\\"},
  {"1", "2", "3", "4", "5", "alt"}
};

// Przypisanie pinów
const int rowPins[] = {6, 9, 15, 8, 7, 22};  // piny wierszy
const int colPins[] = {1, 2, 3, 4, 5, 14};   // piny kolumn
const int ShiftLed = LED_BUILTIN;            // pin diody LED

// Zmienne
int currentMode = 0; // 0 - małe litery, 1 - duże litery, 2 - inne znaki
unsigned long previousMillis = 0; // Czas ostatniego migania
const long interval = 500; // Czas migania w milisekundach
unsigned long lastDebounceTime = 0; // Ostatni czas zmiany stanu
const long debounceDelay = 300; // Opóźnienie debouncingu
void printTerminalText(String text, int delayMs, uint16_t textColor, uint16_t bgColor = TFT_BLACK);
struct FileContent { // Struct d
  String textBuffer;  // Bufor tekstu
  unsigned long filePosition;  // Pozycja pliku
 String currentPath = "/"; 
};


void setup() {
  Serial.begin(115200);
   pinMode(SPEAKER_PIN, OUTPUT); 
// InitKeyboard
  pinMode(LoRa_RESET , OUTPUT); 
 pinMode(LoRa_CS, OUTPUT);
 pinMode(SD_CS, OUTPUT);
// Initialise Keyboard
   pinMode(ShiftLed, OUTPUT);
  digitalWrite(ShiftLed, LOW);
   for (int i = 0; i < NUM_ROWS; i++) {
    pinMode(rowPins[i], OUTPUT);
    digitalWrite(rowPins[i], LOW);
  }
   for (int i = 0; i < NUM_COLS; i++) {
    pinMode(colPins[i], INPUT_PULLUP);
  }
  // Initialise the TFT screen
  tft.init();  
  tft.setRotation(1);// Set the rotation before we calibrate
  tft.invertDisplay( false ); // Where i is true or false
      tft.fillScreen(TFT_BLACK);
 
  if (!LittleFS.begin()) {
    Serial.println("Błąd montowania LittleFS!");
     tft.println("❌ Błąd montowania LittleFS!" );   
    return;
  }
     printTerminalText("LittleFS! zamontowany" , 10); 
  // Set SPI_1    
   SPI1.setRX(MISO);
    SPI1.setTX(MOSI);
    SPI1.setSCK(SCK);
//   printTerminalText("STARTING SYSTEM", 60);
InitSD();
touch_calibrate();
InitLoRa();
InitWiFi();
//---------------Terminal-------------
      tft.fillScreen(TFT_BLACK);
 tft.setTextColor(GREEN_TERM, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0, 0);
  while (true) {
//editor("/dzien") ;
//editor("") ;
//handleFileScrolling();
//NC() ;
Terminal() ;
 }
}

void loop() { 
 //  CheckLoRa();
  uint16_t x, y; // ← Dodaj to
if (tft.getTouch(&x, &y)) {
  tft.fillCircle(x, y, 3, TFT_RED);
  Serial.printf("Dotyk: x=%d, y=%d\n", x, y);
}

  // Odczyt klawisza (bez debouncingu)
  String key = readKey() ; 
  if (key != "") {  // Jeśli klawisz został naciśnięty
    Serial.print("Wspolrzedne klawisza: ");
    Serial.println(key);  // Wyświetl informacje o klawiszu
  }
}

  void CheckLoRa(){
   // Odbiór pakietów LoRa
  int packetSize = LoRa.parsePacket();  // Sprawdzamy, czy są dostępne dane
  if (packetSize) {                    // Jeśli są dane do odbioru
    onReceive(packetSize);              // Wywołujemy funkcję obsługi odbioru
  }
 }

void InitWiFi(){
String ssid = "NETIASPOT-2.4GHz-s5HB";
String password = "jvH3452M"; 
 // We start by connecting to a WiFi network
   Serial.println("\nConnecting to WiFi...");
//   printTerminalText("Connecting to WiFi... " , 60);  
WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  int retryCount = 0;

  // Próba połączenia z licznikiem prób
  while (WiFi.status() != WL_CONNECTED && retryCount < 10) {
    delay(1000);
    Serial.print(".");
    retryCount++;
 //  printTerminalText("." , 60) ;
    Serial.print(" Retry ");
    Serial.println(retryCount);
  }

  if (WiFi.status() == WL_CONNECTED) {
    tft.println("\nWiFi connected!");
    tft.print("IP address: ");
    tft.println(WiFi.localIP());
      Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());  
  } else {
    tft.println("\nFailed to connect to W ");
  printTerminalText("Failed to connect to WiFi " ,60 , TFT_RED);

}
}

void InitSD(){
  // InitSD  
           digitalWrite(LoRa_CS, HIGH); // LoRa off  
         digitalWrite(SD_CS, LOW); // SD on  
//   printTerminalText("Initializing SD card...",60);              
   Serial.println("\nInitializing SD card...");

  bool sdInitialized = false;
    sdInitialized = SD.begin(SD_CS, SPI1);
 if (!sdInitialized) {
      printTerminalText("initialization failed. Things to check:" , 30);

  } else {
    printTerminalText("Wiring is correct and a card is present." , 10);
 delay(500);   
  }
}
void InitLoRa(){
    const int maxRetries = 10;  // Maksymalna liczba prób inicjalizacji LoRa
    int retryCount = 0;

    // Dezaktywuj SD, aktywuj LoRa
    digitalWrite(SD_CS, HIGH);  // Wyłącz SD
    delayMicroseconds(50);      
    digitalWrite(LoRa_CS, LOW); // Włącz LoRa
    
    // Inicjalizacja LoRa
    digitalWrite(LoRa_RESET, LOW); // Reset LoRa
    digitalWrite(LoRa_RESET, HIGH); // Ustawienie LoRa po resecie
    delay(1000); // Odczekaj chwilę, by LoRa się zresetowało
    
    // Przypisanie magistrali SPI1 do LoRa
    LoRa.setSPI(SPI1);  // Ustaw SPI na SPI1
    LoRa.setPins(LoRa_CS, LoRa_RESET, LoRa_IRQ); // Ustawienia pinów LoRa

    // Próba uruchomienia LoRa
    while (retryCount < maxRetries) {
        if (LoRa.begin(433E6)) {
            Serial.println("LoRa zainicjalizowane na SPI1!");
  //  printTerminalText("LoRa zainicjalizowane na SPI1!" ,60);
            delay(1000);  // Czas na ustabilizowanie LoRa
            return;  // Zakończ inicjalizację, jeśli udało się połączyć
        } else {
            Serial.println("Nie udało się zainicjalizować LoRa, ponawiam próbę...");
  //printTerminalText("Nie udało się zainicjalizować LoRa, ponawiam próbę...", 60);
            retryCount++;
            delay(1000);  // Odczekaj chwilę przed kolejną próbą
        }
    }

    // Jeśli LoRa nie udało się zainicjalizować po maxRetries, wyświetl komunikat o błędzie i przejdź dalej
    Serial.println("LoRa nie zainicjalizowane po wielu próbach.");
  printTerminalText("LoRa nie zainicjalizowane po wielu próbach.", 60 ) ;
}

void onReceive(int packetSize) {

  if (packetSize) {
        // received a packet
        Serial.print("Received packet '");

        String recv = "";
        // read packet
        while (LoRa.available()) {
            recv += (char)LoRa.read();
        }
        Serial.println(recv);
    // Wyświetlenie siły sygnału (RSSI)
    int rssi = LoRa.packetRssi();
    Serial.print("RSSI: ");
    Serial.println(rssi);
  }
}

void sendMessage(String outgoing) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(outgoing.length());        // add payload length
//  LoRa.print("HELLOO");  
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
}

void blinkLED() {
  ledState = !ledState;
  digitalWrite(ShiftLed, ledState ? HIGH : LOW);
}

// --------------------------------------
// Funkcja do aktualizacji trybu pracy LED
void updateLedMode() {
  ledTicker.detach(); // zatrzymaj migotanie

  if (currentMode == 0) {
    digitalWrite(ShiftLed, LOW);
  } else if (currentMode == 1) {
    digitalWrite(ShiftLed, HIGH);
  } else if (currentMode == 2) {
    ledTicker.attach_ms(interval, blinkLED); // migaj co interval ms
  }
}

String readKey() {
  for (int x = 0; x < NUM_ROWS; x++) {
    // Ustawienie jednego wiersza na LOW
    for (int row = 0; row < NUM_ROWS; row++) {
      digitalWrite(rowPins[row], x != row);
    }
    for (int y = 0; y < NUM_COLS; y++) {
      if (digitalRead(colPins[y]) == LOW) { // Klawisz wciśnięty
        unsigned long currentMillis = millis();
        if (currentMillis - lastDebounceTime > debounceDelay) {
          lastDebounceTime = currentMillis; // Zaktualizuj czas debouncingu 
 //            Serial.print("X_ ") ; Serial.println (x) ;
 //      Serial.print("Y_ ") ; Serial.println (y) ;

          return getKey(x, y);
        }
      }
    }
  }
  return "";
}

String getKey(int row, int col) {
  // Zwraca odpowiedni klawisz w zależności od trybu
  if (currentMode == 0) {
    handleKey(keys1[row][col]);
    return keys1[row][col]; // Tryb małych liter
  } else if (currentMode == 1) {
      handleKey(keys2[row][col]);
    return keys2[row][col]; // Tryb dużych liter
  } else {
      handleKey(keys3[row][col]);
    return keys3[row][col]; // Tryb innych znaków
  }
}

void handleKey(String key) {
  if (key == "alt") {
    currentMode = (currentMode + 1) % 3; // Przełącz między 3 trybami
  updateLedMode();  
  } else if (key == "del") {
    Serial.println("Clear screen");
  }
}

void touch_calibrate() {
  uint16_t calData[5];
  uint8_t calDataOK = 0;
 // Sprawdzenie, czy plik kalibracyjny istnieje
  if (SD.exists("/TouchCalData1")) {
    if (REPEAT_CAL) {
      // Usuwanie pliku kalibracyjnego, jeśli REPEAT_CAL jest włączone
      SD.remove("/TouchCalData1");
    } else {
      // Odczyt danych kalibracyjnych z pliku na karcie SD
      calibrationFile = SD.open("/TouchCalData1", FILE_READ);
      if (calibrationFile) {
        if (calibrationFile.readBytes((char *)calData, 14) == 14) {
          calDataOK = 1;
        }
        calibrationFile.close();
//      printTerminalText("Wczytano kalibracje z karty SD" , 60 );
      
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // Jeśli dane kalibracyjne są poprawne, użyj ich
    tft.setTouch(calData);
  } else {
    // Jeśli dane nie są dostępne lub chcesz przeprowadzić kalibrację
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println("Dotknij rogów ekranu jak pokazano");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Ustaw REPEAT_CAL na false, aby zatrzymać ponowną kalibrację!");
    }

    // Kalibracja dotyku
    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    // Zapis danych kalibracyjnych na karcie SD
    calibrationFile = SD.open("/TouchCalData1", FILE_WRITE);
    if ("/TouchCalData1") {
      calibrationFile.write((const uint8_t *)calData, 14);
      calibrationFile.close();
    }
    tft.fillScreen(TFT_BLACK);

    // Informacja o zakończeniu kalibracji
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Kalibracja zakończona!");
    delay(1000);
  }
}

void printTerminalText(String text, int delayMs) {
  printTerminalText(text, delayMs, GREEN_TERM, TFT_BLACK);  // lub inne domyślne kolory
}

void printTerminalText(String text, int delayMs, uint16_t textColor, uint16_t bgColor) {
  int cursorY = tft.getCursorY(); // pozycja Y kursora
  const int screenWidth = 480;
  const int lineHeight = 16;

  tft.setTextSize(2);
  tft.setTextColor(textColor, bgColor);

  // Czyści tylko linię, na której piszemy
  tft.fillRect(0, cursorY, screenWidth, lineHeight, bgColor);

  for (unsigned int i = 0; i < text.length(); i++) {
    tft.print(text[i]);
    delay(delayMs);
  }

  // Przesunięcie kursora na następną linię
  cursorY += lineHeight;  // Zwiększ Y, aby przejść do następnej linii
  tft.setCursor(0, cursorY);  // Ustaw kursor w nowej pozycji
}