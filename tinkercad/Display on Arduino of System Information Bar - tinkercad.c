// Display on Arduino of System Information Bar - tinkercad.c
// v0.0.1 - 2018-10-07 - nelbren.com
#include <LiquidCrystal.h>

const char* host = "104.251.217.217";
const int   port = 80;
const char* uri = "/si.txt";
String httpPacket = "GET " + String(uri) + 
                    " HTTP/1.1\r\n" + 
                    "Host: " + String(host) + "\r\n" + 
                    "Connection: close\r\n\r\n";

const int errorPin = 13;
int n = 0;

const char* ssid = "Simulator Wifi";
const char *password = "";
const int rs = 2, en = 3, d0 = 4, d1 = 5, d2 = 6, d3 = 7, d4 = 8, d5 = 9, d6 = 10, d7 = 11;
LiquidCrystal lcd(rs, en, d0, d1, d2, d3, d4, d5, d6, d7);
const int lcd_cols = 16;
const int lcd_rows = 2;

int esp8266(String comm, int miliseconds, char *str) {
  int e = 0;
  Serial.println(comm);
  delay(miliseconds);
  if (!Serial.find(str)) {
    digitalWrite(errorPin, HIGH);
    e = 1;
  }
  return e;
}

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
}

void lcd_scroll(String msg) {
  lcd.setCursor(0,0);
  lcd.print(msg);
  delay(150);
  for (int p = 0; p < 24; p++) {
    lcd.scrollDisplayLeft();
    delay(150);
  }
  for (int p = 0; p < 24; p++) {
    lcd.scrollDisplayRight();
    delay(150);
  }  
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
      wc_x = msg2.length() - 1;
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
  Serial.begin(115200); 
  lcd.begin(lcd_cols, lcd_rows);
  pinMode(errorPin, OUTPUT);
  delay(100);
  esp8266("AT", 10, "OK");
  esp8266("AT+CWJAP=\"" + String(ssid) + "\",\"" + String(password) + "\"", 10, "OK");
  esp8266("AT+CIPSTART=\"TCP\",\"" + String(host) + "\"," + String(port), 50, "OK");
}

void display_error(String msg, int nn) {
  lcd_print(msg);
  digitalWrite(errorPin, HIGH);
  n = nn;
}

void read_stream() {
  unsigned long timeout = millis();
  while (Serial.available() == 0) {
    if (millis() - timeout > 5000) {
      display_error("Comm failed (2)!", 10);
      return;
    }
  }   
  if (Serial.find("\r\n\r\n")){
    delay(5);
    unsigned int i = 0;
    String message = "";
    while (i<60000) {
      if(Serial.available()) {
        message += (char)Serial.read();
        i=0;
      }
      i++;
    }
    //lcd_print(message, 0);
    lcd_scroll(message);
  } else {
    display_error("Comm failed (3)!", 15);
  }
}

void loop(void){
  wait_time();
  if ( n <= 0 ) {
    n = 120;
    esp8266("AT+CIPSEND=" + String(httpPacket.length()), 100, ">");
    int e = esp8266(httpPacket, 10, "SEND OK\r\n");
    if (e != 0) display_error("Comm failed (1)!", n);
    read_stream();
  } else {
    delay(1000);
  }
  n--;
}