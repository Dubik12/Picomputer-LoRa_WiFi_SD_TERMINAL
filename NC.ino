// Stałe określające wymiary ekranu i interfejsu
const int SCREEN_WIDTH = 480;              // Szerokość całego ekranu
const int SCREEN_HEIGHT = 320;             // Wysokość całego ekranu
const int PANEL_WIDTH = SCREEN_WIDTH / 2;  // Szerokość pojedynczego panelu (połowa ekranu)
const int PANEL_HEIGHT = 256;              // Wysokość panelu (głównego obszaru z plikami)
const int LINE_HEIGHT = 16;                // Wysokość jednej linii tekstu
const int MENU_HEIGHT = 20;                // Wysokość dolnego menu
const int MAX_VISIBLE_LINES = 14;          // Maksymalna liczba widocznych plików w panelu
// Etykiety przycisków
String labels[] = { "rm", "rmdir", "mkdir", "copy", "cat", "edit", "EXIT" };

// Rysowanie pojedynczego panelu (lewego lub prawego)
void drawPanel(int x, String files[], int fileCount, int selectedIndex, int scrollOffset, bool isActive) {
  int y = 5;             // Górna pozycja panelu
  int w = PANEL_WIDTH;   // Szerokość panelu
  int h = PANEL_HEIGHT;  // Wysokość panelu

  // Ustawienie kolorów zależnych od aktywności panelu
  uint16_t borderColor = TFT_WHITE;                       // Kolor ramki
  uint16_t bgColor = isActive ? TFT_NAVY : TFT_DARKGREY;  // Tło zależne od aktywności
  uint16_t textColor = TFT_WHITE;                         // Kolor tekstu

  // Rysowanie zewnętrznej i wewnętrznej ramki
  tft.drawRect(x, y, w, h, borderColor);                  // Zewnętrzna ramka
  tft.drawRect(x + 3, y + 3, w - 6, h - 6, borderColor);  // Wewnętrzna ramka

  // Wyświetlenie nazwy nośnika na górze (SD lub FS)
  tft.setTextDatum(MC_DATUM);                              // Ustawienie środka tekstu jako punktu odniesienia
  tft.setTextColor(TFT_CYAN, TFT_DARKGREY);                // Kolor tekstu i tła
// tft.drawString(x == 0 ? "SD" : "FS", x + w / 2, y + 5);  // Nazwa: lewy panel to SD, prawy to FS

  // Wypełnienie wnętrza panelu kolorem tła
  tft.fillRect(x + 4, y + 10, w - 8, h - 14, bgColor);

  // Wyświetlenie listy plików
  tft.setTextDatum(TL_DATUM);  // Tekst od górnego lewego rogu
  int lineY = y + 12;          // Początkowa pozycja tekstu (linia)

  for (int i = 0; i < MAX_VISIBLE_LINES && (i + scrollOffset) < fileCount; i++) {
    int fileIndex = i + scrollOffset;  // Oblicz indeks pliku względem przewinięcia

    if (fileIndex == selectedIndex && isActive) {
      // Zaznaczony plik - inne kolory tła i tekstu
      tft.fillRect(x + 4, lineY - 2, w - 8, 14, TFT_BLUE);
      tft.setTextColor(TFT_YELLOW, TFT_BLUE);
    } else {
      // Zwykły plik - standardowe kolory
      tft.setTextColor(textColor, bgColor);
    }

    // Wyświetlenie nazwy pliku
    tft.setCursor(x + 6, lineY);
    tft.print(files[fileIndex]);
    lineY += LINE_HEIGHT;  // Przejście do następnej linii
  }
}

// Pasek ścieżki aktywnego panelu
void drawPathBar(String path) {
  int y = PANEL_HEIGHT + 8;                            // Pozycja paska ścieżki pod panelami
  tft.fillRect(0, y, SCREEN_WIDTH, 35, TFT_BLACK);  // Wyczyść pasek
  tft.setTextColor(GREEN_TERM, TFT_BLACK);            // Kolor tekstu ścieżki
  tft.setTextDatum(TL_DATUM);                          // Tekst od górnego lewego rogu
  tft.setCursor(6, y + 1);                             // Pozycja kursora
  tft.print(path);                       // Wyświetlenie ścieżki
}

// Dolne menu przycisków
void drawMenu(int selected, bool active) {
  int y = SCREEN_HEIGHT - MENU_HEIGHT;           // Pozycja pionowa menu
  int buttonCount = 7;                           // Liczba przycisków
  int buttonWidth = SCREEN_WIDTH / buttonCount;  // Szerokość jednego przycisku
  int buttonHeight = MENU_HEIGHT;                // Wysokość przycisków



  for (int i = 0; i < buttonCount; i++) {
    int x = i * buttonWidth;                                                        // Pozycja X przycisku
    uint16_t fillColor = (active && i == selected) ? TFT_DARKGREY : TFT_LIGHTGREY;  // Kolor tła
    uint16_t textColor = (active && i == selected) ? TFT_WHITE : TFT_BLACK;         // Kolor tekstu

    tft.fillRect(x, y, buttonWidth, buttonHeight, fillColor);              // Tło przycisku
    tft.drawRect(x, y, buttonWidth, buttonHeight, TFT_WHITE);              // Ramka przycisku
    tft.setTextDatum(MC_DATUM);                                            // Tekst centrowany
    tft.setTextColor(textColor, fillColor);                                // Ustaw kolory
    tft.drawString(labels[i], x + buttonWidth / 2, y + buttonHeight / 2);  // Tekst przycisku
  }
}

// Główna funkcja programu NC
#define MAX_FILES 100  // Maksymalna liczba plików do wyświetlenia w panelu

void NC() {
  int buttonCount = 7;  // Liczba przycisków w dolnym menu

  // Tablice z nazwami plików
  String leftFiles[MAX_FILES];
  String rightFiles[MAX_FILES];
  int leftFileCount = 0;
  int rightFileCount = 0;
  int leftScrollOffset = 0;
  int rightScrollOffset = 0;
  String cmd = "";
  // Indeksy zaznaczonych plików
  int leftSelectedIndex = 0;
  int rightSelectedIndex = 0;
  int selectedMenu = 0;
bool command = false ;
  bool menuActive = false;
  bool leftPanelActive = true;
  bool rightPanelActive = false;

  // Ścieżki do katalogów
  String leftPath = "sd:/";
  String rightPath = "fs:/";

  // Wczytanie zawartości katalogów
  if (!readDirectory(leftPath, leftFiles, leftFileCount)) {
    Serial.println("Blad odczytu katalogu SD");
  }
  if (!readDirectory(rightPath, rightFiles, rightFileCount)) {
    Serial.println("Blad odczytu katalogu FS");
  }

  // Rysowanie interfejsu
  tft.fillScreen(TFT_DARKGREY);
  drawPanel(0, leftFiles, leftFileCount, leftSelectedIndex, leftScrollOffset, leftPanelActive);
  drawPanel(PANEL_WIDTH, rightFiles, rightFileCount, rightSelectedIndex, rightScrollOffset, rightPanelActive);
  drawMenu(selectedMenu, menuActive);

  // Główna pętla obsługi interfejsu
  while (true) {
    String key = readKey();  // Odczytaj naciśnięty klawisz
    if (key != "") {
      if (menuActive) {
        // Nawigacja po menu
        if (key == "rt") selectedMenu = (selectedMenu + 1) % buttonCount;
        else if (key == "lt") selectedMenu = (selectedMenu - 1 + buttonCount) % buttonCount;
 else if (key == "ent") {

  // Zamykamy menu
  menuActive = false;

  // Pobieramy nazwę polecenia z wybranego elementu menu
  cmd = labels[selectedMenu];
  Serial.print("cmd- "); Serial.println(cmd);
if (cmd.startsWith("copy")){
 // cmd = cmd + " " +leftPath + " " + rightPath;
  cmd = cmd + " " + (leftPanelActive ? leftPath : rightPath) + " " + (!leftPanelActive ? leftPath : rightPath); 
} else if (cmd.startsWith("cat") ||  cmd.startsWith("edit") || cmd.startsWith("rm") ||cmd.startsWith("rmdir")){
  // Ustalamy, który panel jest aktywny i pobieramy jego ścieżkę
  String path = leftPanelActive ? leftPath : rightPath;
  Serial.print("path- "); Serial.println(path);
  cmd = cmd + " " + path ;
 }else if (  cmd.startsWith("mkdir")  ){
  String folderName = getUserInput("Folder name:");
  if (folderName.length() > 0) {
    String basePath = leftPanelActive ? leftPath : rightPath;
    cmd = "mkdir " + basePath + "/" + folderName;
    // Wykonaj komendę mkdir z tym parametrem
  } else {
    Serial.println("No folder name provided.");
  }

 }
   Serial.println("cmd- " ); Serial.println(cmd );
  // Wywołujemy polecenie z argumentem ścieżki
  executeCommand(cmd);

 cmd = "" ;
// Po wykonaniu komendy - odśwież zawartość katalogów
leftFileCount = 0;
rightFileCount = 0;

readDirectory(leftPath, leftFiles, leftFileCount);
readDirectory(rightPath, rightFiles, rightFileCount);

// Upewnij się, że wybrany indeks nie wychodzi poza nową liczbę plików
if (leftSelectedIndex >= leftFileCount) leftSelectedIndex = max(0, leftFileCount - 1);
if (rightSelectedIndex >= rightFileCount) rightSelectedIndex = max(0, rightFileCount - 1);

// Ponownie narysuj panele i menu
drawPanel(0, leftFiles, leftFileCount, leftSelectedIndex, leftScrollOffset, leftPanelActive);
drawPanel(PANEL_WIDTH, rightFiles, rightFileCount, rightSelectedIndex, rightScrollOffset, rightPanelActive);
drawMenu(selectedMenu, menuActive);
drawPathBar(leftPanelActive ? leftPath : rightPath);
}else if (key == "esc") menuActive = false;
      } else {
        // Nawigacja po plikach
        if (key == "dn") {
          if (leftPanelActive && leftSelectedIndex < leftFileCount - 1) {
            leftSelectedIndex++;
            if (leftSelectedIndex >= leftScrollOffset + MAX_VISIBLE_LINES) leftScrollOffset++;
          }
          if (rightPanelActive && rightSelectedIndex < rightFileCount - 1) {
            rightSelectedIndex++;
            if (rightSelectedIndex >= rightScrollOffset + MAX_VISIBLE_LINES) rightScrollOffset++;
          }
        }
        if (key == "up") {
          if (leftPanelActive && leftSelectedIndex > 0) {
            leftSelectedIndex--;
            if (leftSelectedIndex < leftScrollOffset) leftScrollOffset--;
          }
          if (rightPanelActive && rightSelectedIndex > 0) {
            rightSelectedIndex--;
            if (rightSelectedIndex < rightScrollOffset) rightScrollOffset--;
          }
        }

        // Zmiana aktywnego panelu
        if (key == "lt") {
          leftPanelActive = true;
          rightPanelActive = false;
        }
        if (key == "rt") {
          leftPanelActive = false;
          rightPanelActive = true;
        }

        // Wejście do katalogu lub cofnięcie
        if (key == "ent") {
          if (leftPanelActive) {
            String selectedFile = leftFiles[leftSelectedIndex];
            if (selectedFile == "..") {
              if (leftPath == "sd:/" || leftPath == "fs:/") {
                leftPath = "";
                leftFileCount = 2;
                leftFiles[0] = "sd:/";
                leftFiles[1] = "fs:/";
                leftSelectedIndex = 0;
              } else {
                leftPath = goUpPath(leftPath);
                readDirectory(leftPath, leftFiles, leftFileCount);
                leftSelectedIndex = 0;
              }
            } else if (selectedFile.endsWith("/")) {
              leftPath += selectedFile;
              readDirectory(leftPath, leftFiles, leftFileCount);
              leftSelectedIndex = 0;
            }else {
      // 🎯 TU dodajemy to, co chcesz:
      leftPath  = leftPath + selectedFile;
      // Możesz np. zapisać tę ścieżkę jako argument do dalszego użycia
//      drawPathBar("Wybrano plik: " + fullPath);
      // Wywołanie gotowej komendy — przykład:
//      cmd_cat(fullPath);  // <- zastąp konkretną komendą


    }
          } else if (rightPanelActive) {
            String selectedFile = rightFiles[rightSelectedIndex];
            if (selectedFile == "..") {
              if (rightPath == "sd:/" || rightPath == "fs:/") {
                rightPath = "";
                rightFileCount = 2;
                rightFiles[0] = "sd:/";
                rightFiles[1] = "fs:/";
                rightSelectedIndex = 0;
              } else {
                rightPath = goUpPath(rightPath);
                readDirectory(rightPath, rightFiles, rightFileCount);
                rightSelectedIndex = 0;
              }
            } else if (selectedFile.endsWith("/")) {
              rightPath += selectedFile;
              readDirectory(rightPath, rightFiles, rightFileCount);
              rightSelectedIndex = 0;
            }else {
      // 🎯 TU dodajemy to, co chcesz:
      rightPath  = rightPath + selectedFile;
      // Możesz np. zapisać tę ścieżkę jako argument do dalszego użycia
//      drawPathBar("Wybrano plik: " + fullPath);
      // Wywołanie gotowej komendy — przykład:
//      cmd_cat(fullPath);  // <- zastąp konkretną komendą


    }
          }
        }

        // Wejście do menu
        if (key == "esc") {
          menuActive = !menuActive ; //Aktywacja dezaktywacja menu
        
        }
        
      }

      // Aktualizacja interfejsu
      drawPanel(0, leftFiles, leftFileCount, leftSelectedIndex, leftScrollOffset, leftPanelActive);
      drawPanel(PANEL_WIDTH, rightFiles, rightFileCount, rightSelectedIndex, rightScrollOffset, rightPanelActive);
      drawMenu(selectedMenu, menuActive);
      drawPathBar(leftPanelActive ? leftPath : rightPath);
      delay(80);  // Małe opóźnienie, aby nie mrugało
    }
  }
}

// Funkcja do cofania ścieżki o jeden poziom wyżej
String goUpPath(String path) {
  if (path.endsWith("/")) path.remove(path.length() - 1);  // Usuń ostatni "/"
  int lastSlash = path.lastIndexOf('/');                   // Znajdź ostatni "/"
  if (lastSlash != -1 && lastSlash > 3) {
    return path.substring(0, lastSlash + 1);  // Zwróć ścieżkę do katalogu wyżej
  } else {
    return path.substring(0, 4);  // Jeśli w głównym, zwróć "sd:/" lub "fs:/"
  }
}

String getUserInput(String prompt) {
  String folderName = "";
  String key;
  
  // Wyświetl podpowiedź
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(10, 10);
  tft.print(prompt);
  tft.setCursor(10, 40);
  tft.print(">");



  while (true) {
    String key = readKey();  // Czytamy klawisz z klawiatury
    if (key != "") {
      if (key == "ent") break;  // ENTER kończy wprowadzanie
      else if (key == "del" && folderName.length() > 0) {
        // Usuwanie znaku – cofamy kursor i czyścimy znak na ekranie
        folderName.remove(folderName.length() - 1);
        int x = tft.getCursorX() - 12;
        int y = tft.getCursorY();
        tft.fillRect(x, y, 24, 20, TFT_BLACK);  // Zakrycie poprzedniego znaku
        tft.setCursor(x, y);
      } else if (key.length() == 1 && isPrintable(key.charAt(0))) {
        // Dodajemy tylko pojedyncze znaki ASCII
        folderName += key;
        int x = tft.getCursorX();
        int y = tft.getCursorY();
        tft.fillRect(x, y, 24, 20, TFT_BLACK);
        tft.print(key);
      }
    }

    // Migający kursor
    delay(10);
    static unsigned long lastBlink = 0;
    static bool cursorVisible = false;
    if (millis() - lastBlink > 400) {
      lastBlink = millis();
      cursorVisible = !cursorVisible;
      int cursorX = tft.getCursorX();
      int cursorY = tft.getCursorY();
      tft.fillRect(cursorX, cursorY + 16, 12, 2, cursorVisible ? GREEN_TERM : TFT_BLACK);
    }
  }


  return folderName;
}