// Display on Arduino of System Information Bar - WemosD1R2.c
// v0.0.1 - 2018-10-07 - nelbren.com

#include <ESP8266WiFi.h>
#include "LiquidCrystal_I2C.h"

const char* host = "104.251.217.217";
const int   port = 80;
const char* uri = "/si.txt";
String httpPacket = "GET " + String(uri) + 
                    " HTTP/1.1\r\n" + 
                    "Host: " + String(host) + "\r\n" + 
                    "Connection: close\r\n\r\n";

int n = 0;

const char* ssid = "CHANGE-TO-YOUR-SSID";
const char* password = "CHANGE-TO-YOUR-PASSWORD";
const int addr = 0x27, en = 2, rw = 1,rs = 0, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal_I2C lcd(addr, en, rw, rs, d4, d5, d6, d7);
const int lcd_cols = 20;
const int lcd_rows = 4;

void wait_time() {
  String msg = "";
  char buffer[10];
  lcd.noBlink();
  sprintf(buffer, "%03d", n);
  msg.concat(buffer);
  lcd.setCursor(lcd_cols - 3, lcd_rows - 1);
  lcd.print(buffer);
  lcd.setCursor(lcd_cols - 1, lcd_rows - 1);
  lcd.blink();
  Serial.print(buffer); 
  Serial.print(",");
}

void lcd_print(String msg, int r = 0) {
  String msg2;
  int wc_x, wc_y;
  int l = msg.length();
  lcd.setCursor(0, r);
  if (l > lcd_cols) {
    if (r >= lcd_rows - 1) {
      msg2 = msg.substring(0, lcd_cols - 3) + "...";     
      lcd.print(msg2);
      wc_x = msg2.length()-1;
      wc_y = r;
    } else {
      lcd.print(msg.substring(0, lcd_cols));
      wc_x = lcd_cols - 1;
      wc_y = r + 1;
      lcd_print(msg.substring(lcd_cols), r + 1);
    }
  } else {
    lcd.print(msg);
    wc_x = msg.length() - 1;
    wc_y = r;
  }
}

void setup(void) {
  String msg = "My IP: ";
  Serial.begin(115200); 
  lcd.begin(lcd_cols, lcd_rows);
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(LOW);  
  lcd.home();  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); }
  IPAddress myIP = WiFi.localIP();
  msg.concat(myIP.toString());
  lcd.clear();
  lcd_print(msg);
  Serial.println(msg);
}

void display_error(String msg, int nn) {
  lcd_print(msg);
  lcd.setBacklight(HIGH);
  n = nn;
}

void read_stream(WiFiClient client) {
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      display_error("Comm failed (2)!", 10);
      client.stop();
      return;
    }
  }   
  if (client.find("\r\n\r\n")){
    delay(5);
    unsigned int i = 0;
    String message = "";
    while (i<60000) {
      if(client.available()) {
        char c = client.read();
        if (c != '\r' && c != '\n') message += c;
        i=0;
      }
      i++;
    }
    Serial.println("\r\n[" + message + "]");
    lcd.setBacklight(HIGH);
    lcd_print(message, 0);
    delay(500);
    if (message.indexOf("*") == -1) lcd.setBacklight(LOW);
    delay(500);
  } else {
    display_error("Comm failed (3)!", 15);
  }
}

void loop(void){
  wait_time();
  if ( n <= 0 ) {
    WiFiClient client;
    n = 120;
    if (!client.connect(host, port)) {
      display_error("Comm failed (1)!", 5);
      return;
    }  
    client.print(httpPacket);
    read_stream(client);
  } else {
    delay(1000);
  }
  n--;
}
