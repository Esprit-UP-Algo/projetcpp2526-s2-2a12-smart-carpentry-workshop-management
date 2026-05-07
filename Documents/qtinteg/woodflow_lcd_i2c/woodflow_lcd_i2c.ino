/*
 * WoodFlow – Arduino Unified Sketch
 * ──────────────────────────────────
 * Hardware:
 *   - HX711 load cell amplifier  → DOUT=3, CLK=2
 *   - LCD I2C 16x2               → SDA=A4, SCL=A5 (Uno standard)
 *   - MFRC522 RFID reader        → SS=10, RST=9, SPI (MOSI=11, MISO=12, SCK=13)
 *
 * Serial protocol (9600 baud, with Qt host):
 *   ← Receives:  "LINE1:<text>\nLINE2:<text>\n"  → affiche sur LCD
 *   ← Receives:  "t"                             → tare la balance
 *   ← Receives:  "READ_RFID\n"                   → scan carte RFID (10s timeout)
 *   → Sends:     "WEIGHT:<val>\n"                → poids en grammes (chaque 1s)
 *   → Sends:     "TARE_OK\n"                     → confirmation tare
 *   → Sends:     "UID:<XX:XX:XX:XX>\n"           → UID carte lue
 *   → Sends:     "NO_CARD\n"                     → aucune carte dans le délai
 *   → Sends:     "READY\n"                       → au démarrage
 */

#include "HX711.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>

// ── Pin definitions ───────────────────────────────────────────────────────────
#define DOUT_PIN  3
#define CLK_PIN   2
#define SS_PIN    10
#define RST_PIN   9

// ── Config ────────────────────────────────────────────────────────────────────
#define SEUIL_DETECTION  100.0   // grammes
#define WEIGHT_INTERVAL  1000    // ms entre chaque lecture poids
#define LCD_COLS         16
#define CALIB_FACTOR    -1080.0   // à ajuster selon votre calibration
#define RFID_TIMEOUT     10000   // ms

// ── Objects ───────────────────────────────────────────────────────────────────
HX711             balance;
LiquidCrystal_I2C lcd(0x27, LCD_COLS, 2);
MFRC522           mfrc522(SS_PIN, RST_PIN);

// ── State ─────────────────────────────────────────────────────────────────────
String        inputBuffer    = "";
unsigned long lastWeightTime = 0;

// ── Helpers ───────────────────────────────────────────────────────────────────

String padLine(const String& s)
{
    String r = s;
    if (r.length() > LCD_COLS) r = r.substring(0, LCD_COLS);
    while ((int)r.length() < LCD_COLS) r += ' ';
    return r;
}

int countNewlines(const String& s)
{
    int n = 0;
    for (int i = 0; i < (int)s.length(); i++)
        if (s[i] == '\n') n++;
    return n;
}

void processLCDMessage(const String& msg)
{
    String line1 = "", line2 = "";
    int start = 0;

    while (start < (int)msg.length()) {
        int end = msg.indexOf('\n', start);
        if (end == -1) break;

        String seg = msg.substring(start, end);
        seg.trim();

        if (seg.startsWith("LINE1:"))      line1 = seg.substring(6);
        else if (seg.startsWith("LINE2:")) line2 = seg.substring(6);

        start = end + 1;
    }

    if (line1.length() > 0 || line2.length() > 0) {
        lcd.setCursor(0, 0); lcd.print(padLine(line1));
        lcd.setCursor(0, 1); lcd.print(padLine(line2));
    }
}

// Blocking RFID scan with timeout — called only when Qt requests it
void readRFIDCard()
{
    lcd.setCursor(0, 0); lcd.print(padLine("Scan carte..."));
    lcd.setCursor(0, 1); lcd.print(padLine("Approchez..."));

    unsigned long start = millis();
    bool found = false;

    while (millis() - start < RFID_TIMEOUT) {
        if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
            found = true;
            break;
        }
        delay(50);
    }

    if (found) {
        String uid = "";
        for (byte i = 0; i < mfrc522.uid.size; i++) {
            if (i > 0) uid += ":";
            if (mfrc522.uid.uidByte[i] < 0x10) uid += "0";
            uid += String(mfrc522.uid.uidByte[i], HEX);
        }
        uid.toUpperCase();

        Serial.println("UID:" + uid);

        MFRC522::PICC_Type t = mfrc522.PICC_GetType(mfrc522.uid.sak);
        Serial.println("CARD_TYPE:" + String(mfrc522.PICC_GetTypeName(t)));

        lcd.setCursor(0, 0); lcd.print(padLine("Carte OK"));
        lcd.setCursor(0, 1); lcd.print(padLine(uid));

        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
    } else {
        Serial.println("NO_CARD");
        lcd.setCursor(0, 0); lcd.print(padLine("Aucune carte"));
        lcd.setCursor(0, 1); lcd.print(padLine("Timeout 10s"));
    }

    delay(1500);
    lcd.setCursor(0, 0); lcd.print(padLine("WoodFlow"));
    lcd.setCursor(0, 1); lcd.print(padLine("En attente..."));
}

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup()
{
    Serial.begin(9600);

    // HX711
    balance.begin(DOUT_PIN, CLK_PIN);
    balance.set_scale(CALIB_FACTOR);
    balance.tare();

    // LCD
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0); lcd.print(padLine("WoodFlow"));
    lcd.setCursor(0, 1); lcd.print(padLine("Calibration..."));
    delay(1000);

    // RFID
    SPI.begin();
    mfrc522.PCD_Init();

    byte version = mfrc522.PCD_ReadRegister(MFRC522::VersionReg);
    if (version == 0x00 || version == 0xFF) {
        Serial.println("ERROR:RFID_NOT_FOUND");
        lcd.setCursor(0, 1); lcd.print(padLine("RFID erreur!"));
        delay(2000);
    }

    lcd.setCursor(0, 1); lcd.print(padLine("En attente..."));
    Serial.println("READY");
}

// ── Main loop ─────────────────────────────────────────────────────────────────
void loop()
{
    // ── 1. Read incoming Serial (commands from Qt) ────────────────────────────
    while (Serial.available() > 0) {
        char c = (char)Serial.read();

        // Single-char tare: 't' with empty buffer
        if (c == 't' && inputBuffer.length() == 0) {
            balance.tare();
            Serial.println("TARE_OK");
            lcd.setCursor(0, 1); lcd.print(padLine("Tare effectuee"));
            delay(600);
            lcd.setCursor(0, 1); lcd.print(padLine("En attente..."));
            continue;
        }

        inputBuffer += c;

        // "READ_RFID\n" — single-line command
        if (c == '\n') {
            String cmd = inputBuffer;
            cmd.trim();

            if (cmd == "READ_RFID") {
                inputBuffer = "";
                readRFIDCard();
                continue;
            }
        }

        // LCD two-line message: wait for LINE1 + LINE2 (2 newlines)
        if (countNewlines(inputBuffer) >= 2) {
            processLCDMessage(inputBuffer);
            inputBuffer = "";
        }

        // Safety flush
        if (inputBuffer.length() > 120) inputBuffer = "";
    }

    // ── 2. Send weight reading every WEIGHT_INTERVAL ms ──────────────────────
    unsigned long now = millis();
    if (now - lastWeightTime >= WEIGHT_INTERVAL) {
        lastWeightTime = now;

        if (balance.is_ready()) {
            float poids = balance.get_units(10);
            if (poids < 0) poids = 0;

            Serial.print("WEIGHT:");
            Serial.println(poids, 2);
        }
    }
}
