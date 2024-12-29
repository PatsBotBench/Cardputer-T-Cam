#include <Wire.h>
#include <Adafruit_AMG88xx.h>
#include <M5Unified.h>
#include "interpolation.h"

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 135
#define GRID_SIZE 8
#define INTERPOLATED_SIZE 16 // Interpolation for better visualization
#define SCALE_WIDTH 20       // Width of the temperature scale
#define SDA_PIN 2  // G2 (Cardputer SDA)
#define SCL_PIN 1  // G1 (Cardputer SCL)

// Configuration: Set to true for Fahrenheit, false for Celsius
const bool displayInFahrenheit = true;

Adafruit_AMG88xx amg;

// Convert Celsius to Fahrenheit
float toFahrenheit(float celsius) {
    return (celsius * 9.0 / 5.0) + 32.0;
}

// Map a value to a color gradient
uint16_t mapTemperatureToColor(float temp, float minTemp, float maxTemp) {
    float normalized = (temp - minTemp) / (maxTemp - minTemp);
    uint8_t red = constrain(255 * normalized, 0, 255);   // High temp -> Red
    uint8_t blue = constrain(255 * (1.0 - normalized), 0, 255); // Low temp -> Blue
    return M5.Lcd.color565(red, 0, blue);
}

void drawTemperatureScale(float minTemp, float maxTemp) {
    float tempStep = (maxTemp - minTemp) / SCREEN_HEIGHT;

    // Draw scale from top to bottom
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        float temp = maxTemp - y * tempStep; // Reverse the temperature gradient
        uint16_t color = mapTemperatureToColor(temp, minTemp, maxTemp);
        M5.Lcd.drawFastHLine(SCREEN_WIDTH - SCALE_WIDTH, y, SCALE_WIDTH, color);
    }

    // Add labels for min and max temperatures
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.setTextSize(2); // Increased font size for better readability
    M5.Lcd.setCursor(SCREEN_WIDTH - SCALE_WIDTH - 55, 0); // Adjusted starting position to the left
    M5.Lcd.printf("%.1f %s", displayInFahrenheit ? toFahrenheit(maxTemp) : maxTemp, displayInFahrenheit ? "F" : "C");
    M5.Lcd.setCursor(SCREEN_WIDTH - SCALE_WIDTH - 55, SCREEN_HEIGHT - 20); // Adjusted for bottom label
    M5.Lcd.printf("%.1f %s", displayInFahrenheit ? toFahrenheit(minTemp) : minTemp, displayInFahrenheit ? "F" : "C");
}

void setup() {
    M5.begin();
    Wire.begin(SDA_PIN, SCL_PIN);
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("Cardputer T-Cam");
    delay(5000);

    if (!amg.begin()) {
        M5.Lcd.println("AMG88xx not detected");
        while (true);
    }

    M5.Lcd.fillScreen(BLACK); // Clear the screen during setup

    // Draw static temperature scale background (only once)
    drawTemperatureScale(0, 100); // Initial range placeholder
}

void loop() {
    float pixels[GRID_SIZE * GRID_SIZE];
    float interpolated[INTERPOLATED_SIZE * INTERPOLATED_SIZE];
    float minTemp = 1000.0, maxTemp = -1000.0;

    amg.readPixels(pixels);

    // Find min and max temperatures
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
        if (pixels[i] < minTemp) minTemp = pixels[i];
        if (pixels[i] > maxTemp) maxTemp = pixels[i];
    }

    interpolate_image(
        pixels, GRID_SIZE, GRID_SIZE, 
        interpolated, INTERPOLATED_SIZE, INTERPOLATED_SIZE
    );

    // Update the grid
    for (int y = 0; y < INTERPOLATED_SIZE; y++) {
        for (int x = 0; x < INTERPOLATED_SIZE; x++) {
            float temp = interpolated[y * INTERPOLATED_SIZE + x];
            uint16_t color = mapTemperatureToColor(temp, minTemp, maxTemp);
            M5.Lcd.fillRect(
                x * (SCREEN_WIDTH - SCALE_WIDTH) / INTERPOLATED_SIZE,
                y * SCREEN_HEIGHT / INTERPOLATED_SIZE,
                (SCREEN_WIDTH - SCALE_WIDTH) / INTERPOLATED_SIZE,
                SCREEN_HEIGHT / INTERPOLATED_SIZE,
                color
            );
        }
    }

    // Update only the temperature scale when necessary
    drawTemperatureScale(minTemp, maxTemp);

    delay(1000);
}
