#include <odroid_go.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>
#include "BluetoothSerial.h"
#include <WiFi.h>
#include <ESP32Ping.h>

#define BTSERIAL_NAME       "IOTDebugger"

#define WIFIAP_SSID         "IOTDebuggerAP"
#define WIFIAP_PW           "12345678"
#define WIFIAP_PORT         80

#define WIFI_SSID           "GlobeAtHome_27646"
#define WIFI_PW             "D78789C1"
#define TEST1_HOST          "data.sparkfun.com"
#define WIFI_PORT           80

#define WIFI_PING_HOST      "www.google.com"

#define RESISTANCE_NUM      2
#define DEFAULT_VREF        1100
#define NO_OF_SAMPLES       64

#define PIN_BLUE_LED        2
#define PWM_CHANNEL         1
#define PWM_12KHZ           12000
#define PWM_8BIT            8

#define DISPLAY_BLANK       0
#define DISPLAY_SPLASH      1
#define DISPLAY_DEBUG       2
#define DISPLAY_MAIN        3
#define DISPLAY_MENU        4
#define DISPLAY_BTMENU      5
#define DISPLAY_WIFIMENU    6
#define DISPLAY_WIFIAPMENU  7
#define DISPLAY_CUSTOM      8
#define DISPLAY_GPIO        9
#define DISPLAY_ABOUT       90
#define DISPLAY_TEST        99

#define LED_STAT_ON         0
#define LED_STAT_OFF        1
#define LED_STAT_FASTBLINK  2
#define LED_STAT_SLOWBLINK  3
#define LED_STAT_BEATING    4

#define BUT_DEBOUNCE        10
#define BUT_NONE            0
#define BUT_UP              1
#define BUT_DOWN            2
#define BUT_LEFT            3
#define BUT_RIGHT           4
#define BUT_A               5
#define BUT_B               6
#define BUT_MENU            7
#define BUT_VOL             8
#define BUT_SEL             9
#define BUT_START           10

#define GPIO_PIN2           18  //cannot use, shared with SPI lcd/sdcard
#define GPIO_PIN3           12
#define GPIO_PIN4           15
#define GPIO_PIN5           4
#define GPIO_PIN7           19  //cannot use, shared with SPI lcd/sdcard
#define GPIO_PIN8           23  //cannot use, shared with SPI lcd/sdcard

#define GPIO_LOW            0
#define GPIO_HIGH           1
#define GPIO_SLOWB          2
#define GPIO_FASTB          3


static esp_adc_cal_characteristics_t adc_chars;
BluetoothSerial serialBT;

const char *apSSID = WIFIAP_SSID;
const char *apPW = WIFIAP_PW;

WiFiServer server(WIFIAP_PORT);
int command1 = 0;
int command2 = 0;
int enable_wifiAP = 0;
int current_wifiAP_stat = 0;

const char *ssid = WIFI_SSID;
const char *pw = WIFI_PW;
const char *host = TEST1_HOST;
const char *streamId   = "....................";
const char *privateKey = "....................";
int enable_wifi = 0;
int current_wifi_stat = 0;
int is_wifi_connected = 0;
int wifi_data = 0;
int wifi_station_debounce = 0;
const char *ping_host = WIFI_PING_HOST;

int gpio2 = GPIO_LOW;   //conflict with spi, needs to fix
int gpio3 = GPIO_LOW;   //usable
int gpio4 = GPIO_LOW;   //usable
int gpio5 = GPIO_LOW;   //usable
int gpio7 = GPIO_LOW;   //conflict with spi, needs to fix
int gpio8 = GPIO_LOW;   //conflict with spi, needs to fix
int gpio_counter = 0;

String line1 = "";
String line2 = "";
String line3 = "";
String line4 = "";
String line5 = "";
int bluetooth_enable = 0;
int current_volume = 8;
unsigned int program_flow = 0;
double battery_level = 0;
int battery_debounce = 100;
int led_stat;
int led_brightness = 0;
int led_counter = 0;
int led_dimming = 0;
int current_display = DISPLAY_BLANK;
int x_cur = 0;
int y_cur = 0;
int but_debounce = 0;
int current_key = BUT_NONE;
int current_cursor = 0;
int ping_test = 0;
int ping_test_debounce = 0;

void push_line_message(String line_data)
{
  line5 = line4;
  line4 = line3;
  line3 = line2;
  line2 = line1;
  line1 = line_data;
}

void input_init(void)
{

}
int consume_input(void)
{
  int ret_val;

  ret_val = current_key;
  current_key = BUT_NONE;
  return ret_val;  
}
int input_handler(void)
{
  if(but_debounce > 0)
  {
    but_debounce--;
    return 0;
  }
    
  if (GO.BtnA.isPressed() == 1)
    current_key = BUT_A;
  if (GO.BtnB.isPressed() == 1)
    current_key = BUT_B;
  if (GO.BtnStart.isPressed() == 1)
    current_key = BUT_START;
  if (GO.BtnSelect.isPressed() == 1)
    current_key = BUT_SEL;
  if (GO.BtnVolume.isPressed() == 1)
    current_key = BUT_VOL;
  if (GO.BtnMenu.isPressed() == 1)
    current_key = BUT_MENU;
  if (GO.JOY_X.isAxisPressed() == 1)
    current_key = BUT_RIGHT;
  if (GO.JOY_X.isAxisPressed() == 2)
    current_key = BUT_LEFT;
  if (GO.JOY_Y.isAxisPressed() == 1)
    current_key = BUT_DOWN;
  if (GO.JOY_Y.isAxisPressed() == 2)
    current_key = BUT_UP;

  if(current_key != BUT_NONE)
    but_debounce = BUT_DEBOUNCE;

  if(but_debounce == BUT_DEBOUNCE)
  {
//    GO.Speaker.beep();
//    GO.Speaker.tone(3000,200);
    return 1;
  }
  else 
    return 0;
}

void set_display(int display)
{
  GO.lcd.clearDisplay();  
  current_display = display; 
}
void open_menu(int display)
{
  current_display = display; 
}
int display_service(void)
{
  switch(current_display)
  {
    case DISPLAY_BLANK:
      GO.lcd.setTextSize(1);
      GO.lcd.setTextColor(YELLOW, BLACK);
      GO.lcd.setCursor(0, 0);
      GO.lcd.print("Developed by IOTLaunchPad");
    break;
    case DISPLAY_SPLASH:
      GO.lcd.setTextSize(3);
      GO.lcd.setTextColor(GREEN, BLACK);
      GO.lcd.setCursor(50, 55);
      GO.lcd.print("IOT Debugger");
      GO.lcd.setCursor(40, 85);
      GO.lcd.print("Communication");
      GO.lcd.setCursor(120, 115);
      GO.lcd.print("Tool");
    break;
    case DISPLAY_DEBUG:
      GO.lcd.setTextSize(1);
      GO.lcd.setCursor(0, 0);
      GO.lcd.setTextColor(GREEN, BLACK);
      GO.lcd.println("/* Direction pad */");
      GO.lcd.printf("Joy-Y-Up: %s \n", (GO.JOY_Y.isAxisPressed() == 2) ? "X" : " ");
      GO.lcd.printf("Joy-Y-Down: %s \n", (GO.JOY_Y.isAxisPressed() == 1) ? "X" : " ");
      GO.lcd.printf("Joy-X-Left: %s \n", (GO.JOY_X.isAxisPressed() == 2) ? "X" : " ");
      GO.lcd.printf("Joy-X-Right: %s \n", (GO.JOY_X.isAxisPressed() == 1) ? "X" : " ");
      GO.lcd.println("");
      GO.lcd.println("/* Function key */");
      GO.lcd.printf("Volume: %s \n", (GO.BtnVolume.isPressed() == 1) ? "X" : " ");
      GO.lcd.printf("Select: %s \n", (GO.BtnSelect.isPressed() == 1) ? "X" : " ");
      GO.lcd.printf("Start: %s \n", (GO.BtnStart.isPressed() == 1) ? "X" : " ");
      GO.lcd.println("");
      GO.lcd.println("/* Actions */");
      GO.lcd.printf("B: %s \n", (GO.BtnB.isPressed() == 1) ? "X" : " ");
      GO.lcd.printf("A: %s \n", (GO.BtnA.isPressed() == 1) ? "X" : " ");
      GO.lcd.println("");
      GO.lcd.printf("Voltage: %1.3lf V\n", battery_level);
    break;
    case DISPLAY_MAIN:
      GO.lcd.setTextSize(2);
      GO.lcd.setTextColor(CYAN, BLACK);
      GO.lcd.setCursor(245, 0);
      GO.lcd.printf("%1.3lfV\n", battery_level);
      GO.lcd.setTextSize(2);
      GO.lcd.setTextColor(YELLOW, BLACK);
      GO.lcd.setCursor(0, 0);
      GO.lcd.printf("Welcome");
      if(current_cursor == 0) GO.lcd.setTextColor(YELLOW, RED);
      else GO.lcd.setTextColor(YELLOW, BLACK);
      GO.lcd.setCursor(20, 20);
      GO.lcd.printf("Bluetooth Tools");
      if(current_cursor == 1) GO.lcd.setTextColor(YELLOW, RED);
      else GO.lcd.setTextColor(YELLOW, BLACK);
      GO.lcd.setCursor(20, 40);
      GO.lcd.printf("Wifi Tools");
      if(current_cursor == 2) GO.lcd.setTextColor(YELLOW, RED);
      else GO.lcd.setTextColor(YELLOW, BLACK);
      GO.lcd.setCursor(20, 60);
      GO.lcd.printf("WifiAP Tools");
      if(current_cursor == 3) GO.lcd.setTextColor(YELLOW, RED);
      else GO.lcd.setTextColor(YELLOW, BLACK);
      GO.lcd.setCursor(20, 80);
      GO.lcd.printf("Custom Test");
      if(current_cursor == 4) GO.lcd.setTextColor(YELLOW, RED);
      else GO.lcd.setTextColor(YELLOW, BLACK);
      GO.lcd.setCursor(20, 100);
      GO.lcd.printf("GPIO Config");
      if(current_cursor == 5) GO.lcd.setTextColor(YELLOW, RED);
      else GO.lcd.setTextColor(YELLOW, BLACK);
      GO.lcd.setCursor(20, 120);
      GO.lcd.printf("About");
//      GO.lcd.setCursor(20, 120);
//      GO.lcd.printf("Cursor %d", current_cursor);
    break;            
    case DISPLAY_BTMENU:
      GO.lcd.setTextSize(2);
      GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(40, 20);
      GO.lcd.printf("Bluetooth Menu          ");
      if(current_cursor == 0) GO.lcd.setTextColor(ORANGE, RED);
      else GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(40, 40);
      GO.lcd.printf("  BT Enable        ");
      if(current_cursor == 1) GO.lcd.setTextColor(ORANGE, RED);
      else GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(40, 60);
      GO.lcd.printf("  BT Disable        ");
      if(current_cursor == 2) GO.lcd.setTextColor(ORANGE, RED);
      else GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(40, 80);
      GO.lcd.printf("  BT Send \"Hello World\"");
      if(current_cursor == 3) GO.lcd.setTextColor(ORANGE, RED);
      else GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(40, 100);
      GO.lcd.printf("  BT Send \"Command1\"");
      if(current_cursor == 4) GO.lcd.setTextColor(ORANGE, RED);
      else GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(40, 120);
      GO.lcd.printf("  BT Send \"Command2\"");
      GO.lcd.setTextSize(1);
      GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(20, 140);
      GO.lcd.printf("BluetoothRX          ");
      GO.lcd.setCursor(20, 150);
      GO.lcd.printf("[%s]", line1);GO.lcd.println("");
      GO.lcd.setCursor(20, 160);
      GO.lcd.printf("[%s]", line2);GO.lcd.println("");
      GO.lcd.setCursor(20, 170);
      GO.lcd.printf("[%s]", line3);GO.lcd.println("");
      GO.lcd.setCursor(20, 180);
      GO.lcd.printf("[%s]", line4);GO.lcd.println("");
      GO.lcd.setCursor(20, 190);
      GO.lcd.printf("[%s]", line5);GO.lcd.println("");
    break;    
    case DISPLAY_WIFIMENU:
      GO.lcd.setTextSize(2);
      GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(40, 20);
      GO.lcd.printf("Wifi Menu          "); 
      if(current_cursor == 0) GO.lcd.setTextColor(ORANGE, RED);
      else GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(40, 40);
      GO.lcd.printf("  WIFI Enable          ");
      if(current_cursor == 1) GO.lcd.setTextColor(ORANGE, RED);
      else GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(40, 60);
      GO.lcd.printf("  WIFI Disable         ");
      if(current_cursor == 2) GO.lcd.setTextColor(ORANGE, RED);
      else GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(40, 80);
      GO.lcd.printf("  WIFITest3          ");
      if(current_cursor == 3) GO.lcd.setTextColor(ORANGE, RED);
      else GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(40, 100);
      GO.lcd.printf("  WIFITest4          ");
      if(current_cursor == 4) GO.lcd.setTextColor(ORANGE, RED);
      else GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(40, 120);
      GO.lcd.printf("  WIFITest5          ");
      GO.lcd.setTextSize(1);
      GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(20, 140);
      if(enable_wifi == 0)
        GO.lcd.printf("Wifi is Disabled      ");
      else
      {
        if(is_wifi_connected == 1) 
        {
          GO.lcd.printf("Wifi is Enabled : Connected");      
        }
        else
          GO.lcd.printf("Wifi is Enabled       ");              
      }
      GO.lcd.setCursor(20, 150);
      GO.lcd.printf("[%s]", line1);GO.lcd.println("");
      GO.lcd.setCursor(20, 160);
      GO.lcd.printf("[%s]", line2);GO.lcd.println("");
      GO.lcd.setCursor(20, 170);
      GO.lcd.printf("[%s]", line3);GO.lcd.println("");
      GO.lcd.setCursor(20, 180);
      GO.lcd.printf("[%s]", line4);GO.lcd.println("");
      GO.lcd.setCursor(20, 190);
      GO.lcd.printf("[%s]", line5);GO.lcd.println("");
    break;    
    case DISPLAY_WIFIAPMENU:
      GO.lcd.setTextSize(2);
      GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(40, 20);
      GO.lcd.printf("Wifi AP Menu        "); 
      if(current_cursor == 0) GO.lcd.setTextColor(ORANGE, RED);
      else GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(40, 40);
      GO.lcd.printf("  WIFI AP Enable      ");
      if(current_cursor == 1) GO.lcd.setTextColor(ORANGE, RED);
      else GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(40, 60);
      GO.lcd.printf("  WIFI AP Disable     ");
      if(current_cursor == 2) GO.lcd.setTextColor(ORANGE, RED);
      else GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(40, 80);
      GO.lcd.printf("  Send Rest 1     ");
      if(current_cursor == 3) GO.lcd.setTextColor(ORANGE, RED);
      else GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(40, 100);
      GO.lcd.printf("  Send Rest 2     ");
      if(current_cursor == 4) GO.lcd.setTextColor(ORANGE, RED);
      else GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(40, 120);
      GO.lcd.printf("  Send Rest 3     ");
      GO.lcd.setTextSize(1);
      GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(20, 140);
      if(enable_wifiAP == 0)
        GO.lcd.printf("Wifi AP is Disabled      ");
      else
        GO.lcd.printf("Wifi AP is Enabled       ");
      GO.lcd.setCursor(20, 150);
      GO.lcd.printf("[%s]", line1);GO.lcd.println("");
      GO.lcd.setCursor(20, 160);
      GO.lcd.printf("[%s]", line2);GO.lcd.println("");
      GO.lcd.setCursor(20, 170);
      GO.lcd.printf("[%s]", line3);GO.lcd.println("");
      GO.lcd.setCursor(20, 180);
      GO.lcd.printf("[%s]", line4);GO.lcd.println("");
      GO.lcd.setCursor(20, 190);
      GO.lcd.printf("[%s]", line5);GO.lcd.println("");     
      GO.lcd.setCursor(20, 200);
      if(command1 == 0)        
        GO.lcd.printf("Command1 : X");
      else
        GO.lcd.printf("Command1 : O");
      GO.lcd.setCursor(20, 210);
      if(command2 == 0)
        GO.lcd.printf("Command2 : X");
      else
        GO.lcd.printf("Command2 : O");
    break;    
    case DISPLAY_CUSTOM:
      GO.lcd.setTextSize(2);
      GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(40, 20);
      GO.lcd.printf("Custom Test          ");
      if(current_cursor == 0) GO.lcd.setTextColor(ORANGE, RED);
      else GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(40, 40);
      if(led_stat == LED_STAT_OFF)
        GO.lcd.printf("  BLUE LED ON        ");      
      else 
        GO.lcd.printf("  BLUE LED OFF       ");      
      if(current_cursor == 1) GO.lcd.setTextColor(ORANGE, RED);
      else GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(40, 60);
      if(ping_test == 0)
        GO.lcd.printf("  Ping Test Enable   ");
      else
        GO.lcd.printf("  Ping Test Disable  ");
      if(current_cursor == 2) GO.lcd.setTextColor(ORANGE, RED);
      else GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(40, 80);
      GO.lcd.printf("  Test3          ");
      if(current_cursor == 3) GO.lcd.setTextColor(ORANGE, RED);
      else GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(40, 100);
      GO.lcd.printf("  Test4          ");
      if(current_cursor == 4) GO.lcd.setTextColor(ORANGE, RED);
      else GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(40, 120);
      GO.lcd.printf("  Test5          ");
      GO.lcd.setTextSize(1);
      GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(20, 140);
      if(is_wifi_connected == 1) 
      {
        GO.lcd.printf("Wifi is Enabled : Connected");      
      }
      else
        GO.lcd.printf("Wifi is Disabled           ");              
      GO.lcd.setCursor(20, 150);
      GO.lcd.printf("[%s]", line1);GO.lcd.println("");
      GO.lcd.setCursor(20, 160);
      GO.lcd.printf("[%s]", line2);GO.lcd.println("");
      GO.lcd.setCursor(20, 170);
      GO.lcd.printf("[%s]", line3);GO.lcd.println("");
      GO.lcd.setCursor(20, 180);
      GO.lcd.printf("[%s]", line4);GO.lcd.println("");
      GO.lcd.setCursor(20, 190);
      GO.lcd.printf("[%s]", line5);GO.lcd.println("");
    break;
    case DISPLAY_GPIO:
      GO.lcd.setTextSize(2);
      GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(40, 20);
      GO.lcd.printf("GPIO Config          ");
      if(current_cursor == 0) GO.lcd.setTextColor(ORANGE, RED);
      else GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(40, 40);
      if(gpio3 == GPIO_LOW)
        GO.lcd.printf("  GPIO3 LOW     ");      
      else if(gpio3 == GPIO_HIGH)
        GO.lcd.printf("  GPIO3 HIGH    ");      
      else if(gpio3 == GPIO_SLOWB)
        GO.lcd.printf("  GPIO3 SLOWB    ");      
      else
        GO.lcd.printf("  GPIO3 FASTB    ");      
      if(current_cursor == 1) GO.lcd.setTextColor(ORANGE, RED);
      else GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(40, 60);      
      if(gpio4 == GPIO_LOW)
        GO.lcd.printf("  GPIO4 LOW     ");      
      else if(gpio4 == GPIO_HIGH)
        GO.lcd.printf("  GPIO4 HIGH    ");      
      else if(gpio4 == GPIO_SLOWB)
        GO.lcd.printf("  GPIO4 SLOWB    ");      
      else
        GO.lcd.printf("  GPIO4 FASTB    ");      
      if(current_cursor == 2) GO.lcd.setTextColor(ORANGE, RED);
      else GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(40, 80);
      if(gpio5 == GPIO_LOW)
        GO.lcd.printf("  GPIO5 LOW     ");      
      else if(gpio5 == GPIO_HIGH)
        GO.lcd.printf("  GPIO5 HIGH    ");      
      else if(gpio5 == GPIO_SLOWB)
        GO.lcd.printf("  GPIO5 SLOWB    ");      
      else
        GO.lcd.printf("  GPIO5 FASTB    ");      
      if(current_cursor == 3) GO.lcd.setTextColor(ORANGE, RED);
      else GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(40, 100);
      GO.lcd.printf("  Test4          ");
      if(current_cursor == 4) GO.lcd.setTextColor(ORANGE, RED);
      else GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(40, 120);
      GO.lcd.printf("  Test5          ");
      GO.lcd.setTextSize(1);
      GO.lcd.setTextColor(ORANGE, BLACK);
      GO.lcd.setCursor(20, 140);
      GO.lcd.printf("GPIO Input Status      ");              
      GO.lcd.setCursor(20, 150);
      GO.lcd.printf("GPIO3 [%d] ", gpio3);
      if(gpio3 == GPIO_LOW) GO.lcd.printf("LOW");
      else if(gpio3 == GPIO_HIGH) GO.lcd.printf("HIGH");
      else if(gpio3 == GPIO_SLOWB) GO.lcd.printf("Slow Blinking");
      else  GO.lcd.printf("Fast Blinking");
      GO.lcd.setCursor(20, 160);
      GO.lcd.printf("GPIO4 [%d] ", gpio4);
      if(gpio4 == GPIO_LOW) GO.lcd.printf("LOW");
      else if(gpio4 == GPIO_HIGH) GO.lcd.printf("HIGH");
      else if(gpio4 == GPIO_SLOWB) GO.lcd.printf("Slow Blinking");
      else  GO.lcd.printf("Fast Blinking");
      GO.lcd.setCursor(20, 170);
      GO.lcd.printf("GPIO5 [%d] ", gpio5);
      if(gpio5 == GPIO_LOW) GO.lcd.printf("LOW");
      else if(gpio5 == GPIO_HIGH) GO.lcd.printf("HIGH");
      else if(gpio5 == GPIO_SLOWB) GO.lcd.printf("Slow Blinking");
      else  GO.lcd.printf("Fast Blinking");
    break;
    case DISPLAY_ABOUT:
      GO.lcd.setTextSize(1);
      GO.lcd.setTextColor(WHITE, BLACK);
      GO.lcd.setCursor(60, 55);
      GO.lcd.print("Created for testing ESP32\n");
      GO.lcd.setCursor(30, 65);
      GO.lcd.print("and familiarization for our upcoming\n");
      GO.lcd.setCursor(30, 75);
      GO.lcd.print("project, IOT Framework. Which enables all\n");
      GO.lcd.setCursor(30, 85);
      GO.lcd.print("devices to connect to our IOT Platform.\n");
      GO.lcd.setCursor(30, 95);
      GO.lcd.print("and creating a Machine Learning functionality\n");
      GO.lcd.setCursor(30, 105);
      GO.lcd.print("togther with centralized function\n");
    break;
    case DISPLAY_TEST:
      GO.lcd.setTextSize(3);
      GO.lcd.setTextColor(WHITE, BLACK);
      GO.lcd.setCursor(30, 55);
      GO.lcd.print("Time to create");
      GO.lcd.setCursor(30, 90);
      GO.lcd.print("my own product\n");
    break;

    default:
      ;     
  }
  return 0;
}

int process_data(void)
{
 
  return 0;
}

void set_led_stat(int stat)
{
  led_stat = stat;
  if(stat == LED_STAT_BEATING)
  {
    ledcAttachPin(PIN_BLUE_LED, PWM_CHANNEL); 
    ledcSetup(PWM_CHANNEL, PWM_12KHZ, PWM_8BIT);
  }
  else
  {
    ledcDetachPin(PIN_BLUE_LED);
    pinMode(PIN_BLUE_LED, OUTPUT);
  }
  led_counter = 0;
}
int led_service(void)
{
  switch(led_stat)
  {
    case LED_STAT_ON:
      digitalWrite(PIN_BLUE_LED, HIGH);
    break;
    case LED_STAT_OFF:
      digitalWrite(PIN_BLUE_LED, LOW);
    break;
    case LED_STAT_FASTBLINK:  
      if(led_counter == 25)
        digitalWrite(PIN_BLUE_LED, HIGH);
      else if(led_counter == 50)
      {
        digitalWrite(PIN_BLUE_LED, LOW);
        led_counter = 0;
      }
    break;
    case LED_STAT_SLOWBLINK:  
      if(led_counter == 100)
        digitalWrite(PIN_BLUE_LED, HIGH);
      else if(led_counter == 200)
      {
        digitalWrite(PIN_BLUE_LED, LOW);
        led_counter = 0;
      }
    break;
    case LED_STAT_BEATING:    
      if(led_counter == 5)
      {
        if(led_dimming == 0)
          led_brightness++;
        else
          led_brightness--;
        if(led_brightness >= 255)
          led_dimming = 1;
        else if(led_brightness == 0)
          led_dimming = 0;
        
        ledcWrite(PWM_CHANNEL, led_brightness);
        led_counter = 0;
      }
    break;
    default:
    ;
  }

  if(led_counter <= 255)
    led_counter++;
  else
    led_counter = 0;
  return 0;
}

void battery_level_init(void)
{
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
  esp_adc_cal_characterize(
    ADC_UNIT_1, 
    ADC_ATTEN_DB_11, 
    ADC_WIDTH_BIT_12, 
    DEFAULT_VREF, 
    &adc_chars);
}
double get_battery_level(void)
{
  return battery_level;
}
int battery_level_service(void)
{
  uint32_t adc_reading = 0;
  
  if(battery_debounce <= 100)
  {
    battery_debounce++;
    return 0;
  }
  else
    battery_debounce = 0;

  for(int i = 0; i < NO_OF_SAMPLES; i++)
  {
    adc_reading += adc1_get_raw((adc1_channel_t) ADC1_CHANNEL_0);
  }
  adc_reading /= NO_OF_SAMPLES;
  battery_level = (double) esp_adc_cal_raw_to_voltage(adc_reading, &adc_chars) *
    RESISTANCE_NUM / 1000;
  
  return 0;
}

void speaker_init(void)
{
    //volume is only applicable to music, still not sure the correct value
//  GO.Speaker.setVolume(current_volume);
//  GO.Speaker.playMusic(m5stack_startup_music, 25000);
}

void bluetooth_init(void)
{
//  serialBT.begin("BTSERIAL_NAME");
}
int bluetooth_service(void)
{
  String temp;
  if(serialBT.available())
  {
    //push message
    String line_ret = "Return : " + serialBT.read();
    push_line_message(line_ret);    
  }
}

void wifi_init(void)
{
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
}
int wifi_service(void)
{
  WiFiClient client;

  if(wifi_station_debounce<100)
  {
    wifi_station_debounce++;
    return 0;
  }
  else
    wifi_station_debounce = 0;
  
  if(enable_wifi == 1)
  {
    if(current_wifi_stat == 0)
    {
      WiFi.begin(ssid, pw);
      current_wifi_stat = 1;
    }
    if(WiFi.status() != WL_CONNECTED)
      return 0;
      is_wifi_connected = 1;
      push_line_message("connected");      
  }
  else if(enable_wifi == 0)
  {
    if(current_wifi_stat == 1)
    {
      ; //off wifi
    }
    return 0;
  }
  else
    return 0;
  
  push_line_message("sending");
  if(!client.connect(host, WIFI_PORT))
  {
    return 0;
  }

  String url = "/input/";
  url += streamId;
  url += "?private_key=";
  url += privateKey;
  url += "&value=";
  url += wifi_data;

  push_line_message("getting reply");  

  return 0;

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
    "Host: " + host + "\r\n" +
    "Connection: close\r\n\r\n");

  unsigned long timeout = millis();
  while(client.available() == 0) {
    if(millis() - timeout > 5000)
    {
      client.stop();
      return 0;
    }
  }

  String line;
  while(client.available())
  {
    line = client.readStringUntil('\r');    
  }
  push_line_message("received");

  return 0;
}
int wifiap_service(void)
{
  if(enable_wifiAP == 1)
  {
    if(current_wifiAP_stat == 0)
    {
      if(WiFi.softAP(apSSID, apPW))
        server.begin(); 
      current_wifiAP_stat = 1;
    }
  }
  else if(enable_wifiAP == 0)
  {
    if(current_wifiAP_stat == 1)
    {
//      server.disconnect();
      WiFi.softAPdisconnect(true);
      current_wifiAP_stat = 0;      
    }    
    return 0;
  }
  else
    return 0;
  
  WiFiClient client = server.available();
  String currentLine = "";
  char c;
  
  if(client)
  {
    currentLine = "";
    while(client.connected())
    {
      if(client.available())
      {
        c = client.read();
        if(c == '\n')
        {
          if(currentLine.length() == 0)
          {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            client.print("Click <a href=\"/H\">XXXX</a> Command1<br>");
            client.print("Click <a href=\"/L\">XXXX</a> Command2<br>");
                        
            client.println();
            break;
          }
          else
          {
            currentLine = "";
          }
        }
        else if(c != '\r')
        {
          currentLine += c;
        }
        if(currentLine.endsWith("GET /H"))
        {
          if(command1 == 0) command1 = 1;
          else  command1 = 0; 
          GO.Speaker.beep();
        }
        if(currentLine.endsWith("GET /L"))
        {
          if(command2 == 0) command2 = 1;
          else  command2 = 0;
          GO.Speaker.beep();
        }
      }
    }      
    client.stop();     
  }
  
  return 0;
}

int ping_test_service(void)
{
  #if 1
  bool ping_result;
  
  if(ping_test_debounce < 100)
  {
    ping_test_debounce++;
    return 0;
  }
  else 
    ping_test_debounce = 0;

  if(ping_test == 1)
  {
    if(current_wifi_stat == 0)
    {
      WiFi.begin(ssid, pw);
      current_wifi_stat = 1;
    }
    if(WiFi.status() != WL_CONNECTED)
      return 0;
    is_wifi_connected = 1;    
  }
  else if(ping_test == 0)
  {
    if(current_wifi_stat == 1)
    {
      WiFi.mode(WIFI_OFF);
      current_wifi_stat = 0;
      is_wifi_connected = 0;
    }
    return 0;
  }
  else
    return 0;

  ping_result = Ping.ping(ping_host, 1);
  if(ping_result)
  {
    push_line_message("Success");
    GO.Speaker.tone(3000,200);   
  }
  else
  {
    push_line_message("Fail   ");
    GO.Speaker.beep();
  }
  #endif
}

void gpio_init(void)
{
  pinMode(GPIO_PIN2, OUTPUT);
  pinMode(GPIO_PIN3, OUTPUT);
  pinMode(GPIO_PIN4, OUTPUT);
  pinMode(GPIO_PIN5, OUTPUT);
  pinMode(GPIO_PIN7, OUTPUT);
  pinMode(GPIO_PIN8, OUTPUT);
}
int gpio_test_service(void)
{
//  digitalWrite(GPIO_PIN2, LOW);
//  digitalWrite(GPIO_PIN3, LOW);
//  digitalWrite(GPIO_PIN4, LOW);
//  digitalWrite(GPIO_PIN5, LOW);
//  digitalWrite(GPIO_PIN7, LOW);
//  digitalWrite(GPIO_PIN8, LOW);
  if(gpio3 == GPIO_LOW)
    digitalWrite(GPIO_PIN3, LOW);
  if(gpio4 == GPIO_LOW)
    digitalWrite(GPIO_PIN4, LOW);
  if(gpio5 == GPIO_LOW)
    digitalWrite(GPIO_PIN5, LOW);
  if(gpio3 == GPIO_HIGH)
    digitalWrite(GPIO_PIN3, HIGH);
  if(gpio4 == GPIO_HIGH)
    digitalWrite(GPIO_PIN4, HIGH);
  if(gpio5 == GPIO_HIGH)
    digitalWrite(GPIO_PIN5, HIGH);    
  
  if(gpio3 == GPIO_SLOWB)
  {
    if(gpio_counter <= 200)
      digitalWrite(GPIO_PIN3, HIGH);
    if(gpio_counter <= 400)
      digitalWrite(GPIO_PIN3, LOW);
  }
  if(gpio4 == GPIO_SLOWB)
  {
    if(gpio_counter <= 200)
      digitalWrite(GPIO_PIN4, HIGH);
    if(gpio_counter <= 400)
      digitalWrite(GPIO_PIN4, LOW);
  }
  if(gpio5 == GPIO_SLOWB)
  {
    if(gpio_counter <= 200)
      digitalWrite(GPIO_PIN5, HIGH);
    if(gpio_counter <= 400)
      digitalWrite(GPIO_PIN5, LOW);
  }
  
  if(gpio3 == GPIO_FASTB)
  {
    if(gpio_counter % 100)
      digitalWrite(GPIO_PIN3, HIGH);
    else if(gpio_counter % 50)
      digitalWrite(GPIO_PIN3, LOW);
  }
  if(gpio4 == GPIO_FASTB)
  {
    if(gpio_counter % 100)
      digitalWrite(GPIO_PIN4, HIGH);
    else if(gpio_counter % 50)
      digitalWrite(GPIO_PIN4, LOW);
  }
  if(gpio5 == GPIO_FASTB)
  {
    if(gpio_counter % 100)
      digitalWrite(GPIO_PIN5, HIGH);
    else if(gpio_counter % 50)
      digitalWrite(GPIO_PIN5, LOW);
  }

  if(gpio_counter <= 400)
    gpio_counter++;
  else
    gpio_counter = 0;
}

int hardware_init(void)
{
  int i;
  input_init();

  //led init
  set_led_stat(LED_STAT_OFF);
  pinMode(PIN_BLUE_LED, OUTPUT);
  for(i = 0; i<5; i++)
  {
    digitalWrite(PIN_BLUE_LED, HIGH);
    delay(50);
    digitalWrite(PIN_BLUE_LED, LOW);
    delay(50);
  }

  battery_level_init();
  speaker_init();
  bluetooth_init();
  wifi_init();
  gpio_init();
  return 0;
}

void setup() {
  // put your setup code here, to run once:
  GO.begin();

  hardware_init();
  
  set_display(DISPLAY_BLANK);  
}

void loop() {
  // put your main code here, to run repeatedly:
  int temp;
  switch(current_display)
  {
    case DISPLAY_BLANK:
      if(program_flow == 500)
      {
        set_display(DISPLAY_SPLASH);        
        set_led_stat(LED_STAT_FASTBLINK);
        program_flow = 0;
      }
    break;
    case DISPLAY_SPLASH:
      if(program_flow == 100)
      {
        set_display(DISPLAY_MAIN);
        set_led_stat(LED_STAT_SLOWBLINK);
        program_flow = 0;
      }      
    break;
    case DISPLAY_DEBUG:
      if(consume_input() == BUT_MENU)
      {
        set_display(DISPLAY_MAIN);
        set_led_stat(LED_STAT_OFF);
      }
    break;
    case DISPLAY_MAIN:
      temp = consume_input();
      if(temp == BUT_MENU)
      {
        set_display(DISPLAY_DEBUG);
        set_led_stat(LED_STAT_FASTBLINK);
      }
      if(temp == BUT_UP)
        if(current_cursor > 0) current_cursor--;
      if(temp == BUT_DOWN)
        if(current_cursor < 5) current_cursor++;
      if(temp == BUT_A)
      {
        if(current_cursor == 0)
        {
          open_menu(DISPLAY_BTMENU);
          current_cursor = 0;
        }
        if(current_cursor == 1)
        {
          open_menu(DISPLAY_WIFIMENU);
          current_cursor = 0;
        }
        if(current_cursor == 2)
        {
          open_menu(DISPLAY_WIFIAPMENU);
          current_cursor = 0;
        }
        if(current_cursor == 3)
        {
          open_menu(DISPLAY_CUSTOM);
          current_cursor = 0;
        }
        if(current_cursor == 4)
        {
          open_menu(DISPLAY_GPIO);
          current_cursor = 0;
        }
        if(current_cursor == 5)
          set_display(DISPLAY_ABOUT);
      }
      if(temp == BUT_B)
        GO.Speaker.beep(); 
    break;        
    case DISPLAY_BTMENU:
      temp = consume_input();
      if(temp == BUT_UP)
        if(current_cursor > 0) current_cursor--;
      if(temp == BUT_DOWN)
        if(current_cursor < 4) current_cursor++;      
      if(temp == BUT_B)
      {
        set_display(DISPLAY_MAIN);
        current_cursor = 0;
      }
    break;
    case DISPLAY_WIFIMENU:
      temp = consume_input();
      if(temp == BUT_UP)
        if(current_cursor > 0) current_cursor--;
      if(temp == BUT_DOWN)
        if(current_cursor < 4) current_cursor++;      
      if(temp == BUT_A)
      {
        if(current_cursor == 0)
          enable_wifi = 1;
        if(current_cursor == 1)
          enable_wifi = 0;
      }
      if(temp == BUT_B)
      {
        set_display(DISPLAY_MAIN);
        current_cursor = 1;
      }
    break;
    case DISPLAY_WIFIAPMENU:
      temp = consume_input();
      if(temp == BUT_UP)
        if(current_cursor > 0) current_cursor--;
      if(temp == BUT_DOWN)
        if(current_cursor < 4) current_cursor++;      
      if(temp == BUT_A)
      {
        if(current_cursor == 0)
          enable_wifiAP = 1;
        if(current_cursor == 1)
          enable_wifiAP = 0;
      }
      if(temp == BUT_B)
      {
        set_display(DISPLAY_MAIN);
        current_cursor = 2;
      }
    break;
    case DISPLAY_CUSTOM:
      temp = consume_input();
      if(temp == BUT_UP)
        if(current_cursor > 0) current_cursor--;
      if(temp == BUT_DOWN)
        if(current_cursor < 4) current_cursor++;      
      if(temp == BUT_A)
      {
        if(current_cursor == 0)
        {
          if(led_stat == LED_STAT_OFF)
            set_led_stat(LED_STAT_ON);
          else
            set_led_stat(LED_STAT_OFF);
        }
        if(current_cursor == 1)
        {
          if(ping_test == 0)
            ping_test = 1;
          else
            ping_test = 0;
        }
      }                        
      if(temp == BUT_B)
      {
        set_display(DISPLAY_MAIN);
        current_cursor = 3;
      }
    break;    
    case DISPLAY_GPIO:
      temp = consume_input();
      if(temp == BUT_UP)
        if(current_cursor > 0) current_cursor--;
      if(temp == BUT_DOWN)
        if(current_cursor < 4) current_cursor++;      
      if(temp == BUT_A)
      {
        if(current_cursor == 0)
        {
          if(gpio3 != GPIO_FASTB)
            gpio3++;
          else
            gpio3 = GPIO_LOW;
        }
        if(current_cursor == 1)
        {
          if(gpio4 != GPIO_FASTB)
            gpio4++;
          else
            gpio4 = GPIO_LOW;
        }
        if(current_cursor == 2)
        {
          if(gpio5 != GPIO_FASTB)
            gpio5++;
          else
            gpio5 = GPIO_LOW;
        }
      }                        
      if(temp == BUT_B)
      {
        set_display(DISPLAY_MAIN);
        current_cursor = 4;
      }
    break;    
    case DISPLAY_ABOUT:
      if(consume_input() == BUT_B)
      {
        set_display(DISPLAY_MAIN);
        current_cursor = 5;
      }
    break;
    default:
    ;
  }

  input_handler();
  process_data();
  led_service();
  battery_level_service();
  bluetooth_service();
  wifi_service();
  wifiap_service();
  display_service();

  ping_test_service();   
  gpio_test_service();

  if(program_flow <= 100000)
    program_flow++;
  else
    program_flow = 0;

  GO.update();
}
