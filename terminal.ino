
String currentPath = "/";
String currentFS = "sd:";

void Terminal() {
  tft.fillScreen(TFT_BLACK);

  tft.setCursor(0, 0);
  tft.setTextColor(GREEN_TERM, TFT_BLACK);

  while (true) {                       // Terminal pracuje ciągle
        int y = tft.getCursorY();
        if (y >= 320) {  tft.setCursor(0, 0);  
         tft.fillScreen(TFT_BLACK);} // powrót na początek ekranu
    String cmd = getUserCommand();  // wpisz komendę
    if (cmd.length() > 0) {
      executeCommand(cmd);  // wykonaj komendę
    }
  }
}

// Czeka aż użytkownik naciśnie ENTER lub ESC
bool waitForEnter() {
  while (true) {
    String key = readKey();
    if (key == "ent") return false;  // kontynuuj
    if (key == "esc") return true;   // przerwij
  }
}

// Sprawdza, czy ścieżka jest absolutna (czy zaczyna się od sd:/ lub fs:/)
bool isAbsolutePath(const String &path) {
  return path.startsWith("sd:/") || path.startsWith("fs:/");
}

// Pobiera tekst wpisany przez użytkownika, automatycznie uzupełniając ścieżki
String getUserCommand() {
  tft.setTextColor(GREEN_TERM, TFT_BLACK);
  tft.print(currentFS + currentPath + " ");

  String input = "";

  while (true) {
    String key = readKey();  // Czytamy klawisz z klawiatury
    if (key != "") {
      if (key == "ent") break;  // ENTER kończy wprowadzanie
      else if (key == "del" && input.length() > 0) {
        // Usuwanie znaku – cofamy kursor i czyścimy znak na ekranie
        input.remove(input.length() - 1);
        int x = tft.getCursorX() - 12;
        int y = tft.getCursorY();
        tft.fillRect(x, y, 24, 20, TFT_BLACK);  // Zakrycie poprzedniego znaku
        tft.setCursor(x, y);
      } else if (key.length() == 1 && isPrintable(key.charAt(0))) {
        // Dodajemy tylko pojedyncze znaki ASCII
        input += key;
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

  tft.println();  // Nowa linia po ENTER

  // Sprawdzamy czy to jedna z komend, która przyjmuje ścieżkę
  if (input.startsWith("copy ") || input.startsWith("cat ") || 
      input.startsWith("edit ") || input.startsWith("rm ") || 
      input.startsWith("mkdir ") || input.startsWith("rmdir ")) {

    int spaceIndex = input.indexOf(' ');
    if (spaceIndex != -1) {
      String cmd = input.substring(0, spaceIndex);         // np. "copy"
      String arg = input.substring(spaceIndex + 1);        // np. "plik.txt"
      arg.trim();

      // Jeśli użytkownik nie podał pełnej ścieżki – dodajemy ją
      if (!isAbsolutePath(arg)) {
        String basePath = currentPath;
        if (!basePath.endsWith("/")) basePath += "/";
        arg = currentFS + basePath + arg;
      }

      input = cmd + " " + arg;
    }
  }

  // Debugowanie końcowej komendy
  Serial.println(">> Komenda do wykonania: " + input);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.println(">> " + input);

  return input;
}




// Wykonuje komendę wpisaną przez użytkownika
void executeCommand(String cmd) {
  Serial.print("COMMAND- ") ; Serial.println (cmd) ;
  cmd.trim();
  tft.println(cmd);
  if (cmd == "ls") {
    String fileList[100];
    int fileCount;
  
    if (readDirectory(currentFS + "/" + currentPath, fileList, fileCount)) {

      for (int i = 0; i < fileCount; i++) {
        Serial.println(fileList[i]);
        printTerminalText(fileList[i], 20);
      }
    } else {
      Serial.println("Błąd podczas czytania katalogu.");
    }
  } else if (cmd.startsWith("nc")) {
    NC();
  } else if (cmd.startsWith("copy ")) {
    String file = cmd.substring(5);
    tft.println(file);
    cmd_copy(file);
  } else if (cmd.startsWith("edit")) {
    if (cmd.length() == 4) {
      tft.fillScreen(TFT_BLACK);
      editor("");
    } else {
      String file = cmd.substring(5);
      file.trim();
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0);
      Serial.println(file);
      editor(file);
    }
  } else if (cmd.startsWith("cat ")) {  // CAT command
    String filename = cmd.substring(4);
    filename.trim();

    Serial.println(filename);
    cmd_cat(filename);

  } else if (cmd.startsWith("rm ")) {
    String filename = cmd.substring(3);

    Serial.print(filename);
    cmd_rm(filename);

  } else if (cmd.startsWith("rmdir ")) {
  String folderName = cmd.substring(6);
  folderName.trim();

  // Upewniamy się, że currentPath nie kończy się na '/'
  if (currentPath.endsWith("/")) {
    currentPath.remove(currentPath.length() - 1);
  }

  // Upewniamy się, że folderName nie zaczyna się od '/'
  if (folderName.startsWith("/")) {
    folderName = folderName.substring(1);
  }
  Serial.println(folderName);  // Debug
  cmd_rmdir(folderName);
}
else if (cmd.startsWith("mkdir ")) {
  String dirname = cmd.substring(6);
  dirname.trim();

  // Upewnij się, że currentPath nie kończy się na "/"
  if (currentPath.endsWith("/")) {
    currentPath.remove(currentPath.length() - 1);
  }

  // Usuń początkowy "/" z dirname jeśli jest
  if (dirname.startsWith("/")) {
    dirname = dirname.substring(1);
  }

  String path = currentFS + (currentPath != "" ? currentPath + "/" : "/") + dirname;

  Serial.println(dirname);  // Debug
  cmd_mkdir(dirname);
}
 else if (cmd == "help") {
    cmd_help();
  } else if (cmd.startsWith("cd ")) {  // Sprawdzamy, czy komenda zaczyna się od "cd "
    String newDir = cmd.substring(3);  // Pobieramy część komendy po "cd ", czyli nazwę katalogu
    newDir.trim();                     // Usuwamy nadmiarowe spacje z początku i końca nazwy katalogu
    if (newDir == "fs:") {
      currentPath = "/";  // Przechodzimy do głównego katalogu systemu plików FS
      currentFS = "fs:";  // Ustawiamy aktualny nośnik na FS
      printTerminalText("Zmiana na system plikow FS", 60);
    } else if (newDir == "sd:") {
      currentPath = "/";  // Przechodzimy do głównego katalogu SD
      currentFS = "sd:";  // Ustawiamy aktualny nośnik na SD
      printTerminalText("Zmiana na kartę SD", 60);
    } else if (newDir == "..") {                           // Sprawdzamy, czy użytkownik podał ".." (chce przejść do katalogu nadrzędnego)
      int slashPos = currentPath.lastIndexOf('/');         // Szukamy ostatniego wystąpienia '/' w ścieżce, żeby znaleźć początek katalogu nadrzędnego
      if (slashPos > 0) {                                  // Jeśli znaleźliśmy '/', to przechodzimy do katalogu nadrzędnego
        currentPath = currentPath.substring(0, slashPos);  // Nowa ścieżka to wszystko przed ostatnim '/' (katalog nadrzędny)
      } else {                                             // Jeśli nie ma '/', to jesteśmy już w katalogu głównym
        currentPath = "/";                                 // Zmieniamy ścieżkę na "/"
      }
    } else {
      String testPath = currentPath;
      if (!testPath.endsWith("/")) testPath += "/";
      testPath += newDir;

      File test;
      if (currentFS == "sd:") {
        test = SD.open(testPath.c_str());
      } else if (currentFS == "fs:") {
        test = LittleFS.open(testPath.c_str(), "r");  // dla katalogów tryb "r" też działa
      }

      if (test && test.isDirectory()) {
        currentPath = testPath;
      } else {
        printTerminalText("Nie ma takiego katalogu", 60);
      }
      test.close();
    }
  }

  else {
    printTerminalText("Nieznana komenda: " + cmd, 20, TFT_RED);
  }
}
