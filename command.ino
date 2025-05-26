bool readDirectory(const String& fullPath, String files[], int& fileCount) {
  File root;
  fileCount = 0;

  // Dodajemy ".." jako pierwszy element — zawsze
  files[fileCount++] = "..";

  // Wybór systemu plików
  if (fullPath.startsWith("sd:")) {
    root = SD.open(fullPath.substring(3));
  } else if (fullPath.startsWith("fs:")) {
    root = LittleFS.open(fullPath.substring(3), "r");
  } else {
    return false; // Nieznany system plików
  }

  if (!root || !root.isDirectory()) {
    return false;
  }

  File file = root.openNextFile();
  while (file && fileCount < 100) {
    String name = file.name();
    if (file.isDirectory()) {
      name += "/";
    }
    files[fileCount++] = name;
    file.close();
    file = root.openNextFile();
  }

  return true;
}


  void cmd_rmdir_FS(String path ) {
if (!path.endsWith("/")) path += "/";
    Serial.println(path) ;
  File dir = LittleFS.open(path, "r");

    if (!dir || !dir.isDirectory()) {
      printTerminalText("Its not Directory", 60, TFT_RED);
      Serial.println("To nie katalog");
      return;
    }

    File file = dir.openNextFile();
    while (file) {
      String fullPath = String(path)  + file.name();
      if (file.isDirectory()) {
        file.close();
        cmd_rmdir_FS(fullPath.c_str());  // Rekurencyjnie usuń podkatalog
      } else {
        file.close();
        if (!LittleFS.remove(fullPath)) {
          printTerminalText("Directory not deleted: " + fullPath, 60, TFT_RED);
          return;
        }
      }
      file = dir.openNextFile();
    }
    dir.close();
  if (LittleFS.exists(path)) {
    if (LittleFS.rmdir(path)) {
      printTerminalText("Directory removed: " + String(path), 60);
    } else {
      printTerminalText("Directory not deleted: " + String(path), 60, TFT_RED);
    }
  } else {
    printTerminalText("Diectory no more exist: " + String(path), 60, TFT_YELLOW);
  }
  }

  void mkdir_SD(String path){
      if (SD.mkdir(path.c_str())) {
        printTerminalText("making directory", 60, TFT_YELLOW);
      } else {
        printTerminalText("error", 60, TFT_RED);
      }
    
  }

  void mkdir_FS(String path) {
    if (LittleFS.mkdir(path.c_str())) {
      printTerminalText("making directory", 60, TFT_YELLOW);
    } else {
      printTerminalText("error", 60, TFT_RED);
    }
  }


  bool cmd_rmdir_SD(String path) {
    if (!SD.exists(path)) {
      Serial.println(String("Not exist: ") + path);
      return false;
    }

    File dir = SD.open(path);
    if (!dir || !dir.isDirectory()) {
      Serial.println(String("Not directory: ") + path);
      dir.close();
      return false;
    }

    File entry = dir.openNextFile();
    while (entry) {
      String entryPath = String(path) + "/" + entry.name();

      if (entry.isDirectory()) {
        // Rekurencyjnie usuń podkatalog
        if (!cmd_rmdir_SD(entryPath.c_str())) {
          entry.close();
          dir.close();
          return false;
        }
      } else {
        // Usuń plik
        if (!SD.remove(entryPath.c_str())) {
          Serial.println(String("Error: ") + entryPath);
          entry.close();
          dir.close();
          return false;
        }
      }

      entry.close();
      entry = dir.openNextFile();
    }
    dir.close();

    // Teraz usuń pusty katalog
    if (!SD.rmdir(path)) {
      Serial.println(String("Error: ") + path);
      return false;
    }

    Serial.println(String("Directory no more exist: ") + path);
    return true;
  }


  // Funkcja do kopiowania zawartości pliku
  void copyFile(File &srcFile, File &destFile) {
size_t totalBytes = 0;
while (srcFile.available()) {
  totalBytes += destFile.write(srcFile.read());
}
Serial.println("Bytes copy: " + String(totalBytes));
  }

void copyDirectory(String sourcePath, String destPath, String sourceFS, String destFS) {
  File dir;

  // Otwórz katalog źródłowy
  if (sourceFS == "SD") {
    dir = SD.open(sourcePath.c_str());
  } else {
    dir = LittleFS.open(sourcePath.c_str(), "r");
  }

  if (!dir || !dir.isDirectory()) {
    Serial.println("❌ Błąd: Nie można otworzyć katalogu: " + sourcePath);
    return;
  }

  // Tworzenie katalogu docelowego
  if (destFS == "SD") {
    SD.mkdir(destPath.c_str());
  } else {
    LittleFS.mkdir(destPath.c_str());
  }

  // Policz wszystkie pliki w katalogu źródłowym, aby obliczyć postęp
  int totalFiles = 0;
  File entry = dir.openNextFile();
  while (entry) {
    totalFiles++;
    entry = dir.openNextFile();
  }

  // Jeśli nie znaleziono plików, zakończ
  if (totalFiles == 0) {
    Serial.println("❌ Brak plików do skopiowania w katalogu: " + sourcePath);
    dir.close();
    return;
  }

  // Resetowanie wskaźnika katalogu
  dir.rewindDirectory();
  
  int copiedFiles = 0;  // Licznik skopiowanych plików

  entry = dir.openNextFile();
  while (entry) {
    String entryName = entry.name();

    // ✅ Poprawne wyliczanie ścieżki względnej
    String safeSource = sourcePath;
    if (!safeSource.endsWith("/")) safeSource += "/";

    String relativePath = entryName;
    if (relativePath.startsWith(safeSource)) {
      relativePath = relativePath.substring(safeSource.length());
    }

    String newSourcePath = sourcePath + "/" + relativePath;
    String newDestPath = destPath + "/" + relativePath;

    if (entry.isDirectory()) {
      Serial.println("📁 Katalog: " + newSourcePath);
      copyDirectory(newSourcePath, newDestPath, sourceFS, destFS);
    } else {
      File srcFile, destFile;

      if (sourceFS == "SD") {
        srcFile = SD.open(newSourcePath.c_str());
      } else {
        srcFile = LittleFS.open(newSourcePath.c_str(), "r");
      }

      if (destFS == "SD") {
        destFile = SD.open(newDestPath.c_str(), FILE_WRITE);
      } else {
        destFile = LittleFS.open(newDestPath.c_str(), "w");
      }

      if (srcFile && destFile) {
        printTerminalText("📄 Plik: " + newSourcePath, 0, 0, TFT_YELLOW);
        copyFile(srcFile, destFile);
        destFile.close();
        srcFile.close();
        copiedFiles++;

        // Oblicz procentowy postęp
        int progress = (copiedFiles * 100) / totalFiles;
        Serial.print("Postęp: ");
        Serial.print(progress);
        Serial.println("%");
      } else {
        Serial.println("❌ Błąd otwarcia pliku: " + newSourcePath);
      }
    }

    entry = dir.openNextFile();
  }

  dir.close();
  Serial.println("✅ Skończono katalog: " + sourcePath);
}
void cmd_rmdir(String path) {
  path.trim();

  if (path.startsWith("sd:/")) {
    path = path.substring(3);  // Usuwamy "sd:"
    if (SD.rmdir(path.c_str())) {
      printTerminalText("directory deleted", 60, TFT_YELLOW);
    } else {
      printTerminalText("can't delete directory", 60, TFT_RED);
    }
  } else if (path.startsWith("fs:/")) {
    path = path.substring(3);  // Usuwamy "fs:"
    cmd_rmdir_FS(path);        // Rekurencyjna funkcja dla LittleFS
  } else {
    printTerminalText("Unknown FS", 60, TFT_RED);
  }
}

void cmd_mkdir(String path) {
  path.trim();

  if (path.startsWith("fs:/")) {
    path = path.substring(3);  // Usuwamy "fs:"
    mkdir_FS(path);            // Twoja funkcja dla LittleFS
  } else if (path.startsWith("sd:/")) {
    path = path.substring(3);  // Usuwamy "sd:"
    mkdir_SD(path);            // Twoja funkcja dla SD
  } else {
    printTerminalText("Unknown FS", 60, TFT_RED);
  }
}






  // Funkcja do obsługi komendy kopiowania
void cmd_copy(String cmd) {
  // Znajdź pierwszy i jedyny odstęp (oddzielający źródło i cel)
  int delimiterIndex = cmd.indexOf(' ');
  if (delimiterIndex == -1) {
    printTerminalText("Blad skladni: uzyj <surce> <cel>", 60, TFT_RED);
    return;
  }

  String source = cmd.substring(0, delimiterIndex);
  String destination = cmd.substring(delimiterIndex + 1);

  source.trim();
  destination.trim();

  String sourceFS, destFS;
  String sourcePath, destPath;

  // Określenie systemu plików źródła
  if (source.startsWith("sd:/")) {
    sourceFS = "SD";
    sourcePath = source.substring(4);
  } else if (source.startsWith("fs:/")) {
    sourceFS = "FS";
    sourcePath = source.substring(4);
  } else {
    Serial.println("Nieznany system plików źródła");
    return;
  }

  // Określenie systemu plików celu
  if (destination.startsWith("sd:/")) {
    destFS = "SD";
    destPath = destination.substring(4);
  } else if (destination.startsWith("fs:/")) {
    destFS = "FS";
    destPath = destination.substring(4);
  } else {
    Serial.println("Nieznany system plików celu") ;
    return;
  }

  // Debug
  Serial.println("Kopiowanie z " + sourceFS + " (" + sourcePath + ") do " + destFS + " (" + destPath + ")");

  // Otwórz źródło
  File srcFile;
  if (sourceFS == "SD") {
    srcFile = SD.open(sourcePath.c_str());
  } else {
    srcFile = LittleFS.open(sourcePath, "r");
  }

  if (!srcFile) {
    Serial.println("Nie udało się otworzyć źródła: " + sourcePath);
    return;
  }

  if (srcFile.isDirectory()) {
    // Zabezpieczenie przed kopiowaniem katalogu do siebie lub do podkatalogu
    if (sourceFS == destFS && (destPath == sourcePath || destPath.startsWith(sourcePath + "/"))) {
      Serial.println("Błąd: próba kopiowania katalogu do samego siebie lub podkatalogu!");
      printTerminalText("Błąd: katalog docelowy zawiera się w źródłowym!", 60, TFT_RED);
      srcFile.close();
      return;
    }

    Serial.println("Źródło to katalog. Kopiujemy katalog rekurencyjnie...");
    copyDirectory(sourcePath, destPath, sourceFS, destFS);
    Serial.println("Skopiowano katalog: " + source);
  } else {
    // Tworzymy pełną ścieżkę pliku docelowego
    String destFullPath = destPath;
    if (!destPath.endsWith("/")) destFullPath += "/";
    int slashIndex = sourcePath.lastIndexOf('/');
    String fileName = (slashIndex != -1) ? sourcePath.substring(slashIndex + 1) : sourcePath;
    destFullPath += fileName;

    File destFile;
    if (destFS == "SD") {
      destFile = SD.open(destFullPath.c_str(), FILE_WRITE);
    } else {
      destFile = LittleFS.open(destFullPath, "w");
    }

    if (!destFile) {
      Serial.println("Nie można utworzyć pliku: " + destFullPath);
    } else {
      Serial.println("Kopiowanie pliku: " + fileName);
      copyFile(srcFile, destFile);
      Serial.println("Skopiowano plik: " + fileName);
      destFile.close();
    }
  }

  srcFile.close();
}


void cmd_cat(String path) {
  const int visibleLines = 16;
  const int maxLineLength = 128;
  const int maxOffsets = 512;
  uint32_t offsets[maxOffsets];
  int offsetCount = 0;
  int topLineIndex = 0;

  File file;

  // Wybór systemu plików
  if (path.startsWith("sd:/")) {
    // Usuwamy "sd:" bo SD.open nie chce tego prefiksu
    String realPath = path.substring(3); 
    file = SD.open(realPath.c_str());
  } else if (path.startsWith("fs:/")) {
    // Usuwamy "fs:" bo LittleFS.open nie chce tego prefiksu
    String realPath = path.substring(3);
    file = LittleFS.open(realPath, "r");
  } else {
    printTerminalText("Unknown filesystem prefix", 60, TFT_RED);
    return;
  }

  if (!file || file.isDirectory()) {
    printTerminalText("Can't open file or is directory", 60, TFT_RED);
    return;
  }

  // Wczytujemy offsety (pozycje linii)
  offsetCount = 0;
  offsets[offsetCount++] = 0;

  while (file.available() && offsetCount < maxOffsets) {
    String temp = "";
    while (file.available()) {
      char c = file.read();
      if (c == '\n') {
        offsets[offsetCount++] = file.position();
        break;
      }
      if (c != '\r') temp += c;
      if (temp.length() >= maxLineLength) {
        // Pomijamy resztę linii do następnego '\n'
        while (file.available()) {
          char c2 = file.read();
          if (c2 == '\n') {
            offsets[offsetCount++] = file.position();
            break;
          }
        }
        break;
      }
    }
  }

  // Funkcja wyświetlania linii
  auto displayLines = [&]() {
    for (int i = 0; i < visibleLines; i++) {
      int lineIdx = topLineIndex + i;
      if (lineIdx >= offsetCount) break;

      file.seek(offsets[lineIdx]);
      String line = "";
      while (file.available()) {
        char c = file.read();
        if (c == '\n') break;
        if (c != '\r') line += c;
        if (line.length() >= maxLineLength) break;
      }
      int y = i * 16;
      tft.fillRect(0, y, tft.width(), 16, TFT_BLACK);
      tft.setCursor(0, y);
      tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      tft.print(line);
    }

    if (topLineIndex + visibleLines >= offsetCount) {
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
      tft.setCursor(0, tft.height() - 16);
      tft.print("-------end of file--");
    }
  };

  displayLines();

  while (true) {
    String key = readKey();
    if (key == "esc") break;
    if (key == "dn" && topLineIndex + visibleLines < offsetCount) {
      topLineIndex++;
      displayLines();
    }
    if (key == "up" && topLineIndex > 0) {
      topLineIndex--;
      displayLines();
    }
  }

  file.close();
}

void cmd_rm(String path) {
  path.trim();
 if (path.startsWith("sd:/")) {
      path = path.substring(3);  
    if (SD.remove(path.c_str())) {
      printTerminalText("deleted", 60, TFT_YELLOW);
    } else {
      printTerminalText("can't delete", 60, TFT_RED);
    }
  } else if (path.startsWith("fs:/")) {
         path = path.substring(3); 
    if (LittleFS.remove(path.c_str())) {
      printTerminalText("deleted", 60, TFT_YELLOW);
    } else {
      printTerminalText("can't delete", 60, TFT_RED);
    }
  } else {
    printTerminalText("Unknown FS", 60, TFT_RED);
  }
}


  void cmd_help(){                   // Komenda - help
    tft.setCursor( 0 , 0) ;
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.println("Dostepne komendy:");
    tft.println(" ls       - lista plikow");
    tft.println(" cd ..    - wroc katalog");
    tft.println(" cd <dir> - wejdź do katalogu");
    tft.println(" cd <sd:><fs:> - zmien nosnik");  
    tft.println(" clr      - wyczysc ekran");
    tft.println(" edit     - edytor bez pliku");
    tft.println(" edit <plik> - edytuj plik");
    tft.println(" cat <plik>  - pokaz plik");
    tft.println(" rm <plik>   - usun plik");
    tft.println(" mkdir <nazw> - nowy katalog");
    tft.println(" rmdir <nazwa> - usun katalog recur") ;
    tft.println(" copy <full path> <full path>- zmiana nosnika") ; 
    tft.println(" help     - pomoc");
  }
