#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED Tanımları
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Fonksiyon Prototipleri
void handleModeSelection();
void visualFeedback(int count);
void applySpeedLevel();
void generateSequence();
void playGame();
void showSequence(int level);
bool getPlayerInput(int level);
int waitForButtonPress();
void gameOver();
void showOLEDStatus(String message, int textSize = 2, int yPos = 16, int xPos = 0);

// Pin Tanımları
const int ledPins[] = {3, 5, 7, 9};       // Mavi, Yeşil, Kırmızı, Sarı LED
const int buttonPins[] = {2, 4, 6, 8};    // Mavi, Yeşil, Kırmızı, Sarı Buton
const int buttonTuruncu = 11;             // Hız Modu (Turuncu Buton)
const int buttonSiyah = 12;               // LED Modu (Siyah Buton)
const int buzzer = 10;                    // Buzzer

// Oyun Değişkenleri
int ledMode = 3;
int speedLevel = 1;
int speedDelay = 800;
int sequence[30];
int playerInput[30];
int currentLevel = 1;

unsigned long lastActionTime = 0;
bool ledModeSelected = false;
bool speedModeSelected = false;
bool gameStarted = false;

void setup() {
  for (int i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
    pinMode(buttonPins[i], INPUT);
  }
  pinMode(buttonTuruncu, INPUT);
  pinMode(buttonSiyah, INPUT);
  pinMode(buzzer, OUTPUT);
  randomSeed(analogRead(0));

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true); // OLED bulunamazsa sonsuz döngü
  }
  delay(200);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  showOLEDStatus("SaymonSays", 2, 14, 20); // Başlangıç yazısı
  delay(1500);
}

void loop() {
  if (!gameStarted) {
    handleModeSelection();
  } else {
    playGame();
  }
}

void handleModeSelection() {
  if (digitalRead(buttonSiyah) == HIGH) {
    ledMode = (ledMode == 3) ? 4 : 3;
    visualFeedback(ledMode);
    showOLEDStatus(String(ledMode) + " LED", 2, 20, 30);
    lastActionTime = millis();
    ledModeSelected = true;
  }

  if (digitalRead(buttonTuruncu) == HIGH) {
    speedLevel++;
    if (speedLevel > 3) speedLevel = 1;
    visualFeedback(speedLevel);
    showOLEDStatus("Hiz " + String(speedLevel), 2, 20, 30);
    lastActionTime = millis();
    speedModeSelected = true;
  }

  if ((ledModeSelected || speedModeSelected) && millis() - lastActionTime >= 3000) {
    applySpeedLevel();
    generateSequence();
    gameStarted = true;
    showOLEDStatus("Oyun Basladi!", 2, 20, 5);
  }
}

void visualFeedback(int count) {
  for (int i = 0; i < count && i < 4; i++) digitalWrite(ledPins[i], HIGH);
  delay(400);
  for (int i = 0; i < count && i < 4; i++) digitalWrite(ledPins[i], LOW);
}

void applySpeedLevel() {
  if (speedLevel == 1) speedDelay = 800;
  else if (speedLevel == 2) speedDelay = 500;
  else speedDelay = 300;
}

void generateSequence() {
  for (int i = 0; i < 30; i++) sequence[i] = random(ledMode);
  currentLevel = 1;
}

void playGame() {
  for (int level = 1; level <= 30; level++) {
    showOLEDStatus("Seviye " + String(level), 2, 20, 15);
    showSequence(level);
    if (!getPlayerInput(level)) {
      gameOver();
      return;
    }
    currentLevel++;
    delay(800);
  }
}

void showSequence(int level) {
  for (int i = 0; i < level; i++) {
    int ledIndex = sequence[i];
    digitalWrite(ledPins[ledIndex], HIGH);
    tone(buzzer, 1000);
    delay(speedDelay);
    digitalWrite(ledPins[ledIndex], LOW);
    noTone(buzzer);
    delay(200);
  }
}

bool getPlayerInput(int level) {
  for (int i = 0; i < level; i++) {
    int pressed = waitForButtonPress();
    if (pressed == -1) return false;
    playerInput[i] = pressed;

    digitalWrite(ledPins[pressed], HIGH);
    tone(buzzer, 1000);
    delay(300);
    digitalWrite(ledPins[pressed], LOW);
    noTone(buzzer);

    if (playerInput[i] != sequence[i]) {
      showOLEDStatus("Yanlis Tus!", 2, 20, 10);
      return false;
    }
  }
  return true;
}

int waitForButtonPress() {
  unsigned long timeout = millis() + 5000;
  while (millis() < timeout) {
    for (int i = 0; i < ledMode; i++) {
      if (digitalRead(buttonPins[i]) == HIGH) {
        while (digitalRead(buttonPins[i]) == HIGH);
        return i;
      }
    }
  }
  return -1;
}

void gameOver() {
  showOLEDStatus("OyunBitti", 2, 14, 15); // Değiştirilen metin
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < ledMode; j++) digitalWrite(ledPins[j], HIGH);
    tone(buzzer, 800);
    delay(300);
    for (int j = 0; j < ledMode; j++) digitalWrite(ledPins[j], LOW);
    noTone(buzzer);
    delay(300);
  }

  gameStarted = false;
  ledModeSelected = false;
  speedModeSelected = false;
  lastActionTime = 0;
}

void showOLEDStatus(String message, int textSize, int yPos, int xPos) {
  display.clearDisplay();
  display.setTextSize(textSize);
  display.setCursor(xPos, yPos);
  display.println(message);
  display.display();
  delay(50);
}
