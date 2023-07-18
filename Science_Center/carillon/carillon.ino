
#include "USB.h"
#include "USBHIDKeyboard.h"
#include "SPI.h"
#include "FT6236.h"
#include "SPI_9488.h"
#include "Button.h"
#include <WiFi.h>
#include <time.h>

#define SSID "Makerfabs"
#define PWD "20160704"

#define I2C_SCL 39
#define I2C_SDA 38

#define COLOR_BACKGROUND TFT_BLACK
#define COLOR_BUTTON TFT_BLACK
#define COLOR_BUTTON_P 0x4BAF
#define COLOR_TEXT TFT_WHITE
#define COLOR_LINE TFT_WHITE
#define COLOR_SHADOW 0x4BAF

#define BUTTON_POS_X 10
#define BUTTON_POS_Y 90

#define BUTTON_DELAY 150

#define BUTTON_COUNT_M 1
#define BUTTON_COUNT_P1 5
#define BUTTON_COUNT_P2 6
#define BUTTON_COUNT_P3 5

//Pin defintions
#define PIN1 16
#define PIN2 8
#define PIN3 46
#define PIN4 9
#define PIN5 17
#define PIN6 11

//Relay delay
#define PRESSED_DELAY 200 //in milliseconds

LGFX lcd;
USBHIDKeyboard Keyboard;

int pos[2] = {0, 0};

void setup()
{
    Serial.begin(115200);
    Serial.println("Keyboard begin");

    lcd_init();
    Keyboard.begin();
    USB.begin();

    main_page();
}

void loop()
{
}

// Pages

void main_page(){
    Button b[BUTTON_COUNT_M];

    String b_list[BUTTON_COUNT_M] = {"Carillon"};

    // Button set
    for (int i = 0; i < BUTTON_COUNT_M; i++){

        b[i].set(BUTTON_POS_X, BUTTON_POS_Y + 80 * i, 200, 60, "NULL", ENABLE);
        b[i].setText(b_list[i]);
        b[i].setValue(i);

        drawButton(b[i]);
    }

    lcd.setTextSize(8);
    lcd.setTextColor(0xFE07);
    lcd.setCursor(10, 360);
    lcd.print("SPI");

    while (1){
        ft6236_pos(pos);

        for (int i = 0; i < BUTTON_COUNT_P1; i++){
            int button_value = UNABLE;
            if ((button_value = b[i].checkTouch(pos[0], pos[1])) != UNABLE){

                Serial.printf("Pos is :%d,%d\n", pos[0], pos[1]);
                Serial.printf("Value is :%d\n", button_value);
                Serial.printf("Text is :");
                Serial.println(b[i].getText());

                drawButton_p(b[i]);
                delay(BUTTON_DELAY);
                drawButton(b[i]);

                page_switch(0); //switch to the the first defined page

                delay(200);
            }
        }
    }
}

void page1(){
    Button b[BUTTON_COUNT_P2];

    String b_list[BUTTON_COUNT_P2] = {"1", "2", "3", "4", "5", "6"};

    clean_button();

    // Button set
    for (int i = 0; i < BUTTON_COUNT_P2; i++){

        b[i].set(BUTTON_POS_X + i % 3 * 100, BUTTON_POS_Y + i / 3 * 90, 80, 80, "NULL", ENABLE);
        b[i].setText(b_list[i]);
        b[i].setValue(i);
        drawButton(b[i]);
    }

    while (1){
        ft6236_pos(pos);
        for (int i = 0; i < BUTTON_COUNT_P2; i++){
            int button_value = UNABLE;
            if ((button_value = b[i].checkTouch(pos[0], pos[1])) != UNABLE){
                Serial.printf("%d\n", button_value);
                drawButton_p(b[i]);
                delay(BUTTON_DELAY);
                drawButton(b[i]);
                key_input_1(button_value);
                delay(200);
            }
        }
    }
}

void page2(){
//    placeholder
}

void page3(){
//    placeholder
}

// Hardware init
void lcd_init(){
    // lcd init
    lcd.init();
    lcd.fillScreen(COLOR_BACKGROUND);
    lcd.setTextColor(COLOR_TEXT);
    lcd.setTextSize(2);
    lcd.setCursor(10, 10);
    lcd.print("Makerfabs ESP32-S3");
    lcd.setCursor(10, 26);
    lcd.print("SPI lcd with Touch");
    lcd.setCursor(10, 42);
    lcd.print("Carillon Demo");

    // I2C init
    Wire.begin(I2C_SDA, I2C_SCL);
    byte error, address;

    Wire.beginTransmission(TOUCH_I2C_ADD);
    error = Wire.endTransmission();

    pinMode(PIN1, OUTPUT);
    pinMode(PIN2, OUTPUT);
    pinMode(PIN3, OUTPUT);
    pinMode(PIN4, OUTPUT);
    pinMode(PIN5, OUTPUT);
    pinMode(PIN6, OUTPUT);
    
    if (error == 0){
        Serial.print("I2C device found at address 0x");
        Serial.print(TOUCH_I2C_ADD, HEX);
        Serial.println("  !");
    }else{
        Serial.print("Unknown error at address 0x");
        Serial.println(TOUCH_I2C_ADD, HEX);
    }
}

// Draw button and shadow

void drawButton(Button b){
    int b_x;
    int b_y;
    int b_w;
    int b_h;
    int shadow_len = 4;
    String text;
    int textSize;

    b.getFoDraw(&b_x, &b_y, &b_w, &b_h, &text, &textSize);

    lcd.fillRect(b_x, b_y, b_w, b_h, COLOR_BUTTON);
    lcd.drawRect(b_x, b_y, b_w, b_h, COLOR_LINE);
    lcd.setCursor(b_x + 20, b_y + 20);
    lcd.setTextColor(COLOR_TEXT);
    lcd.setTextSize(textSize);
    lcd.print(text);

    // Add button shadow
    if (b.getValue() != UNABLE){
        lcd.fillRect(b_x + shadow_len, b_y + b_h, b_w, shadow_len, COLOR_SHADOW);
        lcd.fillRect(b_x + b_w, b_y + shadow_len, shadow_len, b_h, COLOR_SHADOW);
    }
}

void drawButton_p(Button b){
    int b_x;
    int b_y;
    int b_w;
    int b_h;
    int shadow_len = 4;
    String text;
    int textSize;

    b.getFoDraw(&b_x, &b_y, &b_w, &b_h, &text, &textSize);

    lcd.fillRect(b_x, b_y, b_w + shadow_len, b_h + shadow_len, COLOR_BACKGROUND);

    lcd.fillRect(b_x + shadow_len, b_y + shadow_len, b_w, b_h, COLOR_BUTTON_P);
    lcd.drawRect(b_x + shadow_len, b_y + shadow_len, b_w, b_h, COLOR_LINE);
    lcd.setCursor(b_x + 20, b_y + 20);
    lcd.setTextColor(COLOR_TEXT);
    lcd.setTextSize(textSize);
    lcd.print(text);
}

void clean_button(){
    lcd.fillRect(BUTTON_POS_X, BUTTON_POS_Y, 319 - BUTTON_POS_X, 479 - BUTTON_POS_Y, COLOR_BACKGROUND);
}


// Button Command
void page_switch(int page){
    switch (page){
    case 0:
        page1();
        break;
    case 1:
        page2();
        break;
    case 2:
        page3();
        break;

    defualt:
        break;
    }
    delay(100);
}

void play(int pin){
  digitalWrite(pin, HIGH);
  delay(PRESSED_DELAY); 
  digitalWrite(pin, LOW);
}

void key_input_1(int value){
    if(value < 0 || value > BUTTON_COUNT_P2){
      Serial.println("Invalid button value");
    }else{
      switch(value){
        case 0:
          play(PIN1);
          break;
        case 1:
          play(PIN2);
          break;
        case 2:
          play(PIN3);
          break;
        case 3:
          play(PIN4);
          break;
        case 4:
          play(PIN5);
          break;
        case 5:
          play(PIN6);
          break;
      }
    }
    delay(100);
}

void key_input_2(int value){
//    placeholder
}

void key_input_3(int value){
//    placeholder
}
