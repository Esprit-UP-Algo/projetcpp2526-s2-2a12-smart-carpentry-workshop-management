
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ── LCD setup ────────────────────────────────────────────────────────────────
// Parameters: I2C address, columns, rows
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ── Constants ────────────────────────────────────────────────────────────────
const int LCD_COLS = 16;

// ── State ────────────────────────────────────────────────────────────────────
String inputBuffer = "";

// ── Setup ────────────────────────────────────────────────────────────────────
void setup()
{
    Serial.begin(9600);

    lcd.init();          // Initialize the I2C LCD
    lcd.backlight();     // Turn backlight on

    lcd.setCursor(0, 0);
    lcd.print("WoodFlow");
    lcd.setCursor(0, 1);
    lcd.print("En attente...   ");
}

// ── Helpers ──────────────────────────────────────────────────────────────────

// Pad or truncate string to exactly 16 chars so previous text is overwritten
String padLine(const String& s)
{
    String result = s;
    if (result.length() > LCD_COLS)
        result = result.substring(0, LCD_COLS);
    while ((int)result.length() < LCD_COLS)
        result += ' ';
    return result;
}

// Extract value from "KEY:VALUE" line
String extractValue(const String& line, const String& prefix)
{
    if (line.startsWith(prefix))
        return line.substring(prefix.length());
    return "";
}

// Parse full message and write to LCD
void processMessage(const String& msg)
{
    String line1 = "";
    String line2 = "";

    int start = 0;
    while (start < (int)msg.length()) {
        int end = msg.indexOf('\n', start);
        if (end == -1) break;

        String segment = msg.substring(start, end);
        segment.trim();

        String v1 = extractValue(segment, "LINE1:");
        String v2 = extractValue(segment, "LINE2:");

        if (v1.length() > 0) line1 = v1;
        if (v2.length() > 0) line2 = v2;

        start = end + 1;
    }

    if (line1.length() > 0 || line2.length() > 0) {
        lcd.setCursor(0, 0);
        lcd.print(padLine(line1));
        lcd.setCursor(0, 1);
        lcd.print(padLine(line2));
    }
}

// ── Main loop ────────────────────────────────────────────────────────────────
void loop()
{
    while (Serial.available() > 0) {
        char c = (char)Serial.read();
        inputBuffer += c;

        // Count newlines — a complete message has 2 (\n after LINE1 and LINE2)
        int newlineCount = 0;
        for (int i = 0; i < (int)inputBuffer.length(); i++)
            if (inputBuffer[i] == '\n') newlineCount++;

        if (newlineCount >= 2) {
            processMessage(inputBuffer);
            inputBuffer = "";
        }

        // Safety flush if buffer grows too large
        if (inputBuffer.length() > 120)
            inputBuffer = "";
    }
}
