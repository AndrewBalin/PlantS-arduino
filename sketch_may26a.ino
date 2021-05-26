#include <ArduinoJson.h> 
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <Stepper.h> 

ESP8266WiFiMulti WiFiMulti;
LiquidCrystal_I2C lcd(0x3F, 20, 4);
Stepper XStepper(4, 5, 4, 0, 2);
Stepper YStepper(4, 15, 13, 12, 14);

const int humAnalog = A0; //подключение аналогового пина (влажность)

int hum = 0;

int x = 0;
int y = 0;
int i = 1;

String motion="0";
boolean state = false;

String host = "http://f0535729.xsph.ru/info.json";
String sending = "http://f0535729.xsph.ru/post.php";

void setup() {
  

  Serial.begin(9600);
  XStepper.setSpeed(60);
  YStepper.setSpeed(60);
  lcd.begin();
  lcd.backlight();
  lcd.print("Hello, World!");
    for(uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }
    WiFiMulti.addAP("Viktor", "samsonov");
}


String Get(String fin){           // парсинг
   
   HTTPClient http;
   http.begin(host);
   int httpCode = http.GET();
   
   if(httpCode == 200) {
       String payload = http.getString();
       const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
       DynamicJsonBuffer jsonBuffer(capacity);
       JsonObject& root = jsonBuffer.parseObject(payload);
       return root[fin].as<char*>();
   }
   else {
       Serial.println("Файл не найден");
   }   
}

void shipment(int b, int c) {
  
    HTTPClient http;
    String postData = ""; 
    String address = sending + "?b=" + b + "&c=" + c; 
    http.begin(address);  
    http.addHeader("Content-Type", "application/x-www-form-urlencoded"); 
    auto httpCode = http.POST(postData);  
    http.end(); //Close connection Serial.println(); 

}

void print_lcd(int i, int x, int y, String text) {
  if(i == 0){
    lcd.clear();
  }
  lcd.setCursor(x, y);
  lcd.print(text);
}

int humidity() {
  
  Serial.println(analogRead(humAnalog));
  hum = map(constrain(analogRead(humAnalog), 240, 540), 240, 540, 100, 0); 
  
  return hum;
}

void shag(int x, int y) {
   YStepper.step(y);
   XStepper.step(x);  
   i++;
}

void light(String off) {
   if(off == "1") {
      digitalWrite(1, HIGH);
   }
   else {
      digitalWrite(1, LOW); 
   }
}

void loop() {
  
    // ожидание WiFi соединения
    if((WiFiMulti.run() == WL_CONNECTED)) {
      // парсинг
      Serial.println(Get("state"));
      
      if(Get("State") == "1") {
        state = true;
      }
      else if(Get("motion") != "0" and  Get("State") != "1") {
        motion = Get("motion");
      }
      if(Get("light") == "1") {
        light(Get("light"));
      }
      
    }    
    else {
        Serial.println("Нет доступа к Wi-Fi");
        print_lcd(0, 1, 0, "Нет доступа к Wi-Fi");
    }


    shipment(humidity(), 1);

    
    if(state == "1"){
      if(i != 11) {
        x = x * -1;
        y = 0;
      }
      else if(i == 11) {
        x = x * -1;
        y = 1;
        i = 0;
        a++;
      }
      shag(x, y);
    }
    else if(state != "1" and motion != "0") {
      switch(motion) {
        case "a":
          shag(0, -1);
          Serial.println("Вперёд");
          break;
        case "b":
          shag(0, 1);
          Serial.println("Назад");
          break;
        case "l":
          shag(1, 0);
          Serial.println("Влево");
          break;
        case "r":
          shag(-1, 0);
          Serial.println("Вправо");
          break;
      }
    }
    
       
    delay(1000);
    
    
}
