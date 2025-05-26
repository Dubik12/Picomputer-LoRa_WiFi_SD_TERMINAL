

void EditorLoop(String filename, String textBuffer, unsigned long filePosition) {

  int cursorX = 0;
  int cursorY = 0;
  int maxCols = 40;
  int maxLines = 16;
  int FontWeight = 12;
  int FontHigh = 16;
  const int editorHeight = 300;
  int selectedMenuIndex = -1;  // Wybrana opcja menu
  int previousMenuIndex = 0;
  bool inMenuMode = false;                // Tryb początkowy: edytor
  int cursorIndex = textBuffer.length();  // kursor na końcu tekstu
  tft.setCursor(0, 0);
  Serial.print("File position: ");
  Serial.println(filePosition);
  //  drawMenu(tft, selectedMenuIndex, inMenuMode); // Rysowanie Menu
  tft.setTextColor(GREEN_TERM, TFT_BLACK);
  tft.print(textBuffer);     // Rysowanie ekranu
  while (true) {             // Główna pętla programu (nieskończona)



    String key = readKey();  // Odczytanie wciśniętego klawisza

    if (!inMenuMode) {
      if (key.length() == 1) {    // Jeśli wciśnięto pojedynczy znak
        char ch = key.charAt(0);  // Pobranie znaku z obiektu String
        if (ch >= 32 && ch <= 126) {
          int col = tft.getCursorX() / FontWeight;
          int line = tft.getCursorY() / FontHigh;
          if (col >= maxCols  && line >= maxLines ) {
            // Brak miejsca: zablokuj pisanie
            continue;
          }
        col = tft.getCursorX() / FontWeight;  // Oblicz kolumnę
    if (col >= maxCols - 1) {
      textBuffer += ch;       // Dodaj znak
      textBuffer += "\n";     // Dodaj przejście do nowej linii
    } else {
      textBuffer += ch;       // Tylko znak
    }
          cursorX = tft.getCursorX();
          cursorY = tft.getCursorY();
           tft.fillRect(cursorX , cursorY, FontWeight, 18, TFT_BLACK);  // czysci kursor na ekranie
         

          tft.setCursor(cursorX, cursorY);  // Ustaw pozycję kursora ekranowego (przeliczając w pikselach)
         tft.setCursor(0, 0);
      tft.setTextColor(GREEN_TERM, TFT_BLACK);
      tft.print(textBuffer);  // Rysowanie ekranu       
        }
      }
      if (key == "del") {
        if (textBuffer.length() != 0) {
          textBuffer.remove(textBuffer.length() - 1);  //usuwanie ostatniego znaku
           cursorX = tft.getCursorX();              // sprawdzanie pozycji kursora
           cursorY = tft.getCursorY();
          tft.fillRect(cursorX - FontWeight, cursorY, 24, 18, TFT_BLACK);  // czysci ostatnią litere i kursor na ekranie
        }
      } else if (key == "ent") {  // Sprawdzenie, czy wciśnięto klawisz Enter
  int cursorX = tft.getCursorX();  // Pobranie aktualnej pozycji X kursora
  int cursorY = tft.getCursorY();  // Pobranie aktualnej pozycji Y kursora
  int line = cursorY / FontHigh;   // Obliczenie numeru linii, na której znajduje się kursor

  if (line < maxLines - 1) {  // Jeśli nie jesteśmy jeszcze na ostatniej linii ekranu
    tft.fillRect(cursorX, cursorY, FontWeight, FontHigh, TFT_BLACK);  // Wyczyść miejsce po starym kursorze (np. zielona kreska)

    textBuffer += '\n';       // Dodaj znak nowej linii do bufora tekstowego

    cursorX = 0;              // Przenieś kursor X na początek nowej linii
    cursorY += FontHigh;      // Przesuń kursor Y o jedną linię w dół

    tft.setCursor(0, 0);      // Ustaw kursor wyświetlacza na początek ekranu
    tft.setTextColor(GREEN_TERM, TFT_BLACK);  // Ustaw kolor tekstu i tła
    tft.print(textBuffer);    // Narysuj cały tekst od początku z aktualnym buforem
  }
}
else if (key == "esc") {  // Przejście do trybu menu po naciśnięciu ESC
        inMenuMode = true;
        drawMenu(tft, selectedMenuIndex, inMenuMode);  // Wyswietlenie pierwszego klawisza

      } else if (key == "lt") {
        cursorX = tft.getCursorX();
        cursorY = tft.getCursorY();
        if (cursorIndex > 0) {
          cursorIndex--;
          tft.setCursor(cursorY, cursorX + 1);
        }
      } else if (key == "rt") {
        cursorX = tft.getCursorX();
        cursorY = tft.getCursorY();
        if (cursorIndex < textBuffer.length()) {
          cursorIndex++;
          tft.setCursor(cursorY, cursorX - 1);
        }
      } else if (key == "up") {


      } else if (key == "dn") {
       textBuffer = "";  
        String filename = "/dzien" ;
        Serial.println (filename) ; 
  FileContent content = loadFile(filename, filePosition);  // Pobierz tekst i nową pozycję
  textBuffer = content.textBuffer;                         // Przypisz pobrany tekst
  filePosition = content.filePosition;  
         Serial.println (filename) ;          Serial.println (" - ") ;         Serial.println ("filePosition -" ) ;         Serial.println (filePosition) ; 
                     // Przypisz nową pozycję pliku
      tft.setCursor(0, 0);      // Ustaw kursor wyświetlacza na początek ekranu
    tft.setTextColor(GREEN_TERM, TFT_BLACK);  // Ustaw kolor tekstu i tła
    tft.print(textBuffer);    // Narysuj cały tekst od początku z aktualnym buforem 
      }
      tft.setCursor(0, 0);
      tft.setTextColor(GREEN_TERM, TFT_BLACK);
      tft.print(textBuffer);  // Rysowanie ekranu
 //Miganie kursora
                                    static unsigned long lastBlink = 0;
      static bool cursorVisible = false;
      if (millis() - lastBlink > 400) {
        lastBlink = millis();
        cursorVisible = !cursorVisible;
        cursorX = tft.getCursorX();  // zapamiętaj pozycję kursora
        cursorY = tft.getCursorY();
        tft.fillRect(cursorX, cursorY + FontHigh, FontWeight, 2, cursorVisible ? GREEN_TERM : TFT_BLACK);
      }  // Koniec Migania kursora
    
    }    // koniec warunku !inmenumode

    else {  // Jeśli jesteśmy w trybie menu
      if (key == "ent") {
        if (selectedMenuIndex == 0 && filename.length() > 0) {  // Jeśli wybrano "Save" i jest nazwa pliku
          File file = SD.open(filename, FILE_WRITE);            // Otworzenie pliku do zapisu
          if (file) {                                           // Jeśli plik udało się otworzyć
            for (int i = 0; i < 15; i++) {
              file.println(textBuffer);  // Zapisanie każdej linii tekstu
            }
            file.close();  // Zamknięcie pliku
          }
        } else if (selectedMenuIndex == 1) {  // Load File
          drawMenu(tft, -1, inMenuMode);      //

        } else if (selectedMenuIndex == 2) {  // Cancel
          drawMenu(tft, -1, inMenuMode);      // -1 gasi wszystkie klawiszemenu

        } else if (selectedMenuIndex == 3) {                  // Exit to editor
          tft.fillRect(0, editorHeight, 480, 20, TFT_BLACK);  // Rysowanie paska menu o wysokości 20 pikseli
          tft.setTextColor(GREEN_TERM, TFT_BLACK);
          Serial.print(textBuffer);

        } else if (selectedMenuIndex == 4) {  //Quit > back to Terminal
          delay(100);
          tft.fillScreen(TFT_BLACK);
          tft.setCursor(0, 0);
          return;
        }
        inMenuMode = false;                                // Wyjście z Edytora
      } else if (key == "lt") {                            // Strzałka w lewo
        previousMenuIndex = selectedMenuIndex;             // Zapamiętaj poprzedni wybór
        selectedMenuIndex--;                               // Przesunięcie na poprzednią opcję
        if (selectedMenuIndex < 0) selectedMenuIndex = 4;  // Zawinięcie na koniec
      } else if (key == "rt") {                            // Strzałka w prawo
        previousMenuIndex = selectedMenuIndex;             // Zapamiętaj poprzedni wybór
        selectedMenuIndex++;                               // Przesunięcie na następną opcję
        if (selectedMenuIndex > 4) selectedMenuIndex = 0;  // Zawinięcie na początek
      }
    }

    if (inMenuMode) {
      if (previousMenuIndex != selectedMenuIndex) {
        drawMenu(tft, selectedMenuIndex, inMenuMode);
        previousMenuIndex = selectedMenuIndex;  // Zapamiętaj poprzedni wybór
      }
    }
  }  /// Koniec głównej pętli
}





// Funkcja edytora pliku
void editor(String filename) {
  String textBuffer = "";  // Bufor tekstu

  unsigned long filePosition = 0;  // Pozycja markera w pliku pliku
  if (filename.length() == 0) {
    EditorLoop(filename, textBuffer, 0);
  }
  FileContent content = loadFile(filename, filePosition);  // Pobierz tekst i nową pozycję
  textBuffer = content.textBuffer;                         // Przypisz pobrany tekst
  filePosition = content.filePosition;                     // Przypisz nową pozycję pliku

  EditorLoop(filename, textBuffer, filePosition);
}

// Funkcja wczytująca plik, zwraca strukturę FileContent
FileContent loadFile(String filename, unsigned long filePosition) {
  FileContent content;
  content.textBuffer = "";                // Inicjalizacja pustego bufora tekstu
  File file = SD.open(filename);          // Otwarcie pliku z karty SD

  if (file) {                             // Sprawdzenie, czy plik został otwarty poprawnieedit
    int lineCount = 0;                    // Licznik linii do wyświetlenia (maks. 16)
    file.seek(filePosition);             // Przesunięcie wskaźnika pliku na określoną pozycję

    while (file.available() && lineCount < 16) {  // Czytaj dopóki są dane i nie przekroczono 16 linii
      String line = file.readStringUntil('\n');   // Czytaj linię do znaku końca linii
      content.filePosition = file.position();     // Zapisz aktualną pozycję w pliku
      line = cleanText(line);                     // Wyczyść linię z niepożądanych znaków

      // Oblicz ile fizycznych linii zajmie ta linia na ekranie (80 znaków na linię)
      int segments = (line.length() + 79) / 80;   // Zaokrąglone w górę dzielenie
      for (int i = 0; i < segments && lineCount < 16; i++) {
        // Dodaj tylko tyle segmentów, ile mieści się w 16 liniach ekranu
        content.textBuffer += line.substring(i * 80, (i + 1) * 80) + "\n";
        lineCount + segments;  // Zwiększ licznik fizycznych linii


      }
                     lineCount ++;  // Zwiększ licznik fizycznych linii 
    }

    file.close();  // Zamknij plik po zakończeniu odczytu
  }

  return content;  // Zwróć strukturę z buforem tekstowym i pozycją w pliku
}




String cleanText(String line) {  // do funkcji LOAD - czysci ze znakow niedrukowalnych
  String cleanedLine = "";
  for (unsigned int i = 0; i < line.length(); i++) {
    char c = line.charAt(i);
    // Usuwanie znaku powrotu karetki '\r'
    if (c != '\r' && isPrintable(c)) {  // Sprawdza, czy znak jest drukowalny i nie jest '\r'
      cleanedLine += c;                 // Dodaje do oczyszczonej linii
    }
  }
  return cleanedLine;
}

void drawMenu(TFT_eSPI &tft, int selectedMenuIndex, bool inMenuMode) {
  const int editorHeight = 300;  // Wysokość pola edytora (15 linii × 20px)
  // Rysowanie paska menu na dole ekranu
  tft.fillRect(0, editorHeight, 480, 20, TFT_DARKGREY);  // Rysowanie paska menu o wysokości 20 pikseli

  String menuOptions[5] = { "Save", "LOAD", "Cancel", "Exit", "Quit" };  // Tablica z opcjami menu
  int optionWidth = 480 / 5;                                             // Obliczenie szerokości jednej opcji (1/4 szerokości ekranu)

  // Pętla rysująca opcje menu
  for (int i = 0; i < 5; i++) {
    int x = i * optionWidth;  // Pozycja X dla każdej opcji
    if (i == selectedMenuIndex && inMenuMode) {
      tft.fillRect(x, editorHeight, optionWidth, 20, TFT_BLUE);  // Podświetlenie wybranej opcji na niebiesko
      tft.setTextColor(TFT_WHITE, TFT_BLUE);                     // Biały tekst na niebieskim tle
    } else {
      tft.setTextColor(TFT_BLACK, TFT_DARKGREY);  // Czarny tekst na ciemnoszarym tle
    }
    tft.setCursor(x + 10, editorHeight + 2);  // Pozycjonowanie tekstu wewnątrz paska opcji
    tft.print(menuOptions[i]);                // Wyświetlenie napisu opcji
    tft.setCursor(0, 0);
  }
}

void redrawEditor(TFT_eSPI &tft, String &textBuffer) {
  tft.setTextSize(2);
  tft.setTextColor(GREEN_TERM, TFT_BLACK);

  const int lineHeight = 20;
  const int maxLines = 16;  // Mieści się 14 linii (0–14)

  int y = 0;
  String currentLine = "";
  int linesDrawn = 0;

  for (unsigned int i = 0; i < textBuffer.length(); i++) {
    char c = textBuffer.charAt(i);

    if (c == '\n' || currentLine.length() >= 40) {
      tft.setCursor(0, y);
      Serial.print(currentLine);
      y += lineHeight;
      linesDrawn++;
      currentLine = "";

      if (linesDrawn >= maxLines) break;

      if (c != '\n') currentLine += c;  // Jeśli linia przerwana bo za długa, zachowaj ostatni znak
    } else {
      currentLine += c;
    }
  }

  // Narysuj pozostały tekst, jeśli nie zakończono \n
  if (linesDrawn < maxLines && currentLine.length() > 0) {
    tft.setCursor(0, y);
    tft.print(currentLine);
  }
}



