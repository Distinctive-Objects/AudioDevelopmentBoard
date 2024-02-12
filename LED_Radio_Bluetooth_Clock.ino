#include <TFT_eSPI.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Audio.h>
#include "BluetoothA2DPSink.h"

// Radio settings - try this site for alternatiove streaming URLs: http://www.radiofeeds.co.uk or https://fmstream.org.  Use the streams that are .m3u for best results.
Audio audio;
const char ssid[] = "Your WIFI NAME";
const char password[] = "PASSWORD";
const char* radioURLs[] = {
    "hhttp://icecast.thisisdax.com/RadioXUKMP3.m3u",
    "http://media-ice.musicradio.com/ClassicFMMP3",
    "http://lstn.lv/bbc.m3u8?station=bbc_radio_one&bitrate=48000"
};
const char* stationNames[] = {"Radio X", "Classic FM", "BBC Radio One"};
const int numberOfStations = 3;
int currentStation = 0;

// TFT display and NeoPixel setup
TFT_eSPI tft = TFT_eSPI();   
Adafruit_NeoPixel strip = Adafruit_NeoPixel(8, 22, NEO_GRB + NEO_KHZ800);

// Joystick pins
const int joystickUpPin = 19;
const int joystickDownPin = 12;
const int joystickLeftPin = 14;
const int joystickRightPin = 13;
const int joystickCenterPin = 32;

// Potentiometer pin for volume control
const int potentiometerPin = 33;

bool inBluetoothMode = false;
bool isBluetoothInitialized = false;  // Declare the variable globally


// Menu options
const char* menuOptions[] = {"NeoPixel Control", "Radio", "Clock", "Bluetooth Speaker"};
const int numOfOptions = 4; // Update the number of menu options

// NeoPixel colors and TFT colors
uint32_t neoPixelColors[] = {
   strip.Color(255, 0, 0), // Red
    strip.Color(255, 165, 0), // Orange
    strip.Color(255, 255, 0), // Yellow
    strip.Color(0, 128, 0), // Green
    strip.Color(0, 0, 255), // Blue
    strip.Color(75, 0, 130), // Indigo
    strip.Color(128, 0, 128), // Violet
    strip.Color(255, 255, 255), // White
    strip.Color(0, 0, 0) // Black (Off)
};
uint16_t tftColors[] = {
    TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_GREEN, TFT_BLUE, TFT_PURPLE, TFT_MAGENTA, TFT_WHITE
};
const int numOfColors = 9;

// Global variables for menu navigation
int currentOption = 0;
int currentColor = 0;
bool inMenu = true;
bool inColorSelection = false;
bool inRadioMode = false;
bool inClockMode = false;

// screen timing variables
unsigned long previousMillis = 0;
const long interval = 1000; // Interval at which to refresh the screen (1000 milliseconds)





// NTP Client for clock mode
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000); // Change timezone offset as needed

// Global I2S pin configuration for Bluetooth
i2s_pin_config_t my_pin_config = {
    .bck_io_num = 26, // BCLK 
    .ws_io_num = 25,  // LRC pin 
    .data_out_num = 27, // Din pin
    .data_in_num = I2S_PIN_NO_CHANGE
};

BluetoothA2DPSink a2dp_sink;

void waitForResourceRelease(unsigned long delayTime) {
    unsigned long startTime = millis();
    while (millis() - startTime < delayTime) {
        // Implementation depends on your specific hardware and requirements
        // This is a placeholder for a delay, replace with actual checks if possible
        delay(50); 
    }
    Serial.println("Resource release wait completed.");
}

void initializeWiFi() {
    if (isBluetoothInitialized) {
        shutDownBluetooth();
    }

    // If issues persist, uncomment the next line to reset the ESP32
    //ESP.restart();

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected");
    // Rest of the WiFi initialization code...
}

void shutDownWiFi() {
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    Serial.println("WiFi turned off");
}

void initializeBluetooth() {
    if (WiFi.status() == WL_CONNECTED) {
        shutDownWiFi();
        waitForResourceRelease(500); // Wait for 500ms to allow WiFi to shut down completely
    }

    if (!isBluetoothInitialized) {
        Serial.println("Initializing Bluetooth...");
        a2dp_sink.set_pin_config(my_pin_config);
        a2dp_sink.start("MyMusic");
        isBluetoothInitialized = true;
        Serial.println("Bluetooth Initialized");
    } else {
        a2dp_sink.set_connected(true);
    }
}

void shutDownBluetooth() {
    if (isBluetoothInitialized) {
        a2dp_sink.end();
        isBluetoothInitialized = false;
        Serial.println("Bluetooth turned off");

        waitForResourceRelease(1000); // Increased delay to ensure resource release
    }
}

void setup() {

   Serial.begin(9600);

    // Initialize TFT display
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    
    // Initialize NeoPixel strip
    strip.begin();
    strip.show();

    // Initialize potentiometer pin
    pinMode(potentiometerPin, INPUT);

    // Initialize joystick pins
    pinMode(joystickUpPin, INPUT_PULLDOWN);
    pinMode(joystickDownPin, INPUT_PULLDOWN);
    pinMode(joystickLeftPin, INPUT_PULLDOWN);
    pinMode(joystickRightPin, INPUT_PULLDOWN);
    pinMode(joystickCenterPin, INPUT_PULLDOWN);

    // Draw the initial menu
    drawMenu();

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
}
Serial.println("\nWiFi Connected");


    // Set audio pinout and volume
    audio.setPinout(26, 25, 27);
    audio.setVolume(21);

    // Start NTP client
    timeClient.begin();
    timeClient.forceUpdate(); // Force NTP update if needed

    // A short delay to stabilize the startup
    delay(500);

   // initializeWiFi(); // Initialize WiFi by default

}


void loop() {
    if (inMenu) {
        checkJoystickMenu();
    } 
    else if (inColorSelection) {
        controlNeoPixel();
    } 
    else if (inRadioMode) {
        int potValue = analogRead(potentiometerPin);
        int volume = map(potValue, 0, 2048, 25, 0);
        audio.setVolume((volume));

        //Serial.print("Potentiometer Value: ");
        //Serial.print(potValue);
        //Serial.print(", Volume: ");
        //Serial.println(volume);


        if (digitalRead(joystickDownPin) == HIGH) {
            changeStation(1);
            delay(200);
        } else if (digitalRead(joystickUpPin) == HIGH) {
            changeStation(-1);
            delay(200);
        } else if (digitalRead(joystickLeftPin) == HIGH) {
          	ESP.restart();
        }

        audio.loop();
    } 
    else if (inClockMode) {
        timeClient.update();
         timeClient.forceUpdate(); // Force NTP update if needed
        displayTime();

        // Check for left joystick press to exit clock mode
        if (digitalRead(joystickLeftPin) == HIGH) {
            inClockMode = false;
            inMenu = true;
            drawMenu();
            delay(200); // Adding a delay to debounce the joystick button
        }
    }
    else if (inBluetoothMode) {
        bluetoothMode();
    }
}


void drawMenu() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(1); // Consider making text smaller if still too large

    int rectHeight = 25; // Reduce height of each menu rectangle
    int startYPos = 10; // Starting Y position for the first menu item

    for (int i = 0; i < numOfOptions; i++) {
        int yPos = startYPos + (rectHeight + 5) * i; // Adjust spacing

        if (i == currentOption) {
            tft.fillRoundRect(10, yPos, 140, rectHeight, 5, TFT_BLUE);
            tft.drawRoundRect(10, yPos, 140, rectHeight, 5, TFT_YELLOW);
            tft.setTextColor(TFT_WHITE, TFT_BLUE);
        } else {
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
        }
        tft.drawString(menuOptions[i], tft.width() / 2, yPos + rectHeight / 2, 2);
    }
}

void checkJoystickMenu() {
    if (digitalRead(joystickUpPin) == HIGH && currentOption > 0) {
        currentOption--;
        drawMenu();
        delay(200);
    } else if (digitalRead(joystickDownPin) == HIGH && currentOption < numOfOptions - 1) {
        currentOption++;
        drawMenu();
        delay(200);
    } else if (digitalRead(joystickCenterPin) == HIGH) {
        Serial.println("Joystick Center Pressed"); // Debugging
        switch (currentOption) {
            case 0:
                inMenu = false;
                inColorSelection = true;
                drawColorSelection();
                break;
            case 1:
                inMenu = false;
                inRadioMode = true;
                currentStation = 0;
                connectToStation(currentStation);
                drawRadioInterface();
                break;
            case 2:
                inMenu = false;
                inClockMode = true;
                if (isBluetoothInitialized) {
                a2dp_sink.end();
                isBluetoothInitialized = false;
                }
                break;
            case 3:
                Serial.println("Switching to Bluetooth Mode"); // Debugging
                inMenu = false;
                inBluetoothMode = true;
                bluetoothMode();
                break;
        }
    }
}


void controlNeoPixel() {
    // Modify this function to avoid resetting the strip when changing modes
    if (digitalRead(joystickLeftPin) == HIGH) {
        // Do not clear the strip here, just switch back to the menu
        inColorSelection = false;
        inMenu = true;
        drawMenu();
        delay(200);
        return;
    }

    if (digitalRead(joystickUpPin) == HIGH && currentColor > 0) {
        currentColor--;
        drawColorSelection();
        delay(200);
    } else if (digitalRead(joystickDownPin) == HIGH && currentColor < numOfColors - 1) {
        currentColor++;
        drawColorSelection();
        delay(200);
    } else if (digitalRead(joystickCenterPin) == HIGH) {
        for (int i = 0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, neoPixelColors[currentColor]);
        }
        strip.show();
        delay(200);
    }
}

void drawColorSelection() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    int rectHeight = tft.height() / numOfColors;
    int barWidth = 60;
    for (int i = 0; i < numOfColors; i++) {
        int barY = i * rectHeight;
        int yPos = barY + (rectHeight / 2);
        if (i == currentColor) {
            tft.fillRoundRect((tft.width() - barWidth) / 2, barY, barWidth, rectHeight, 5, tftColors[i]);
            tft.drawRoundRect((tft.width() - barWidth) / 2, barY, barWidth, rectHeight, 5, TFT_WHITE);
        } else {
            tft.fillRoundRect((tft.width() - barWidth) / 2, barY, barWidth, rectHeight, 5, tftColors[i]);
        }
        tft.setTextColor(TFT_WHITE, tftColors[i]);
        tft.drawString(" ", tft.width() / 2, yPos, 2);
    }
}

void drawRadioInterface() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    int rectHeight = 30;

    // Define a darker purple color for the background
    uint16_t darkerPurple = tft.color565(75, 0, 130); // Convert RGB to 16-bit color

    // Define the white color for the border
    uint16_t white = tft.color565(255, 255, 255); // Convert RGB to 16-bit color

    for (int i = 0; i < numberOfStations; i++) {
        int yPos = 30 * i + 35;
        if (i == currentStation) {
            // Highlight the selected station with a darker purple rectangle and white border
            tft.fillRoundRect(10, yPos - (rectHeight / 2), 140, rectHeight, 5, darkerPurple);
            tft.drawRoundRect(10, yPos - (rectHeight / 2), 140, rectHeight, 5, white);
            tft.setTextColor(TFT_WHITE, darkerPurple);
        } else {
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
        }
        tft.drawString(stationNames[i], tft.width() / 2, yPos, 2);
    }
}



void connectToStation(int stationIndex) {
    currentStation = stationIndex;
    audio.connecttohost(radioURLs[currentStation]);
    drawRadioInterface();
}

void changeStation(int direction) {
    currentStation += direction;
    if (currentStation >= numberOfStations) currentStation = 0;
    if (currentStation < 0) currentStation = numberOfStations - 1;
    connectToStation(currentStation);
}


void displayTime() {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        // Clear the screen
        tft.fillScreen(TFT_BLACK);

        // Set a background color for the time display area
        int rectX = 10;
        int rectY = (tft.height() / 2) - 20;
        int rectW = tft.width() - 20;
        int rectH = 40;
        tft.fillRoundRect(rectX, rectY, rectW, rectH, 8, TFT_NAVY); // Navy background

        // Set text properties
        tft.setTextColor(TFT_WHITE, TFT_NAVY); // White text on navy background
        tft.setTextSize(3); // Larger text size

        // Get the formatted time
        String formattedTime = timeClient.getFormattedTime();

        // Calculate the position to center the text
        int textWidth = tft.textWidth(formattedTime);
        int textX = (tft.width() - textWidth) / 2; // Center horizontally
        int textY = rectY + (rectH / 2) - 8; // Position vertically within the rectangle

        // Draw the time
        tft.setCursor(textX, textY);
        tft.println(formattedTime);

        // Decorative elements (e.g., lines, shapes, additional text)
        // Draw lines at the top and bottom of the rectangle
        tft.drawFastHLine(rectX, rectY, rectW, TFT_CYAN);
        tft.drawFastHLine(rectX, rectY + rectH, rectW, TFT_CYAN);
    }
}

void bluetoothMode() {
    Serial.println("Entering Bluetooth Mode");

    // Ensure that WiFi is turned off
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);

    // Initialize Bluetooth only if it hasn't been initialized before
    if (!isBluetoothInitialized) {
        Serial.println("Initializing Bluetooth...");
        a2dp_sink.set_pin_config(my_pin_config);
        a2dp_sink.start("MyMusic");
        isBluetoothInitialized = true;
        Serial.println("Bluetooth Initialized");
    } else {
        // Reconnect Bluetooth if already initialized
        a2dp_sink.set_connected(true);
    }

    displayBluetoothScreen();

    while (true) {
        if (digitalRead(joystickLeftPin) == HIGH) {
            ESP.restart();
        }
        delay(100);
    }
}

void displayBluetoothScreen() {
    // Define background and accent colors
    uint16_t bgColor = TFT_NAVY;
    uint16_t accentColor = TFT_CYAN;
    uint16_t textColor = TFT_WHITE;

    // Set the entire background
    tft.fillScreen(bgColor);

    // Screen dimensions and text positioning
    int screenWidth = 160; // Assuming a width of 160 pixels
    int screenHeight = 128; // Assuming a height of 128 pixels

    // Adjust text properties
    tft.setTextSize(2); // Medium text size for better readability
    tft.setTextColor(textColor, bgColor); // White text on navy background

    // Calculate text width and position for "Bluetooth Mode"
    String text = "Bluetooth\nMode";
    int textWidth = tft.textWidth("Bluetooth"); // Use the widest word to calculate width
    int textX = (screenWidth - textWidth) / 2; // Center text horizontally
    int titleYPos = 50; // Vertically centering the text block in the middle of the screen

    // Print "Bluetooth Mode" centered
    tft.setCursor(textX, titleYPos);
    tft.println("Bluetooth");
    tft.setCursor((screenWidth - tft.textWidth("Mode")) / 2, titleYPos + 20); // Adjust for "Mode" to be centered
    tft.println("Mode");

    // Draw a rounded rectangle around the text
    // Adjust the rectangle size based on text size and position
    int rectX = textX - 10; // 10 pixels padding on the left and right
    int rectY = titleYPos - 14; // 10 pixels padding on the top
    int rectWidth = textWidth + 20; // Text width plus left and right padding
    int rectHeight = 60; // Adjusted for the height of "Bluetooth\nMode" text block plus padding
    tft.drawRoundRect(rectX, rectY, rectWidth, rectHeight, 8, accentColor); // Draw rounded rectangle with 8px radius
}
