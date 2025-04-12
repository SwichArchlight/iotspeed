#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>

int ena = 14;
int in1 = 26;
int in2 = 27;
int lastdirection = 1; //default putar ke kanan
int speeds = 0;

const char* ssid = "Cobi";          // SSID wifi
const char* password = "SYIFA123";  // password wifi

#define BOTtoken "7867060153:AAEiwOtbDeJZ6iytLeNyVZD6cvOu7JvhM5c"  // Token Bot telegram
#define CHAT_ID "8190671274"                                       // Id user yang menggunakan
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

LiquidCrystal_I2C lcd(0x27,16,2);

void SetWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());
}

void speed(int speed){
  int speeds = map(speed, 0, 100, 0, 255);
  analogWrite(ena, speeds);
  Serial.println(speed);
}

void Arah(int arah){
  if(arah){
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  lastdirection = 1;  
  }
  else{
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  lastdirection = 0;  
}
}

void status(String ids){
  int ins1 = digitalRead(in1);
  int ins2 = digitalRead(in2);
  String spd = String(speeds);
  String pesan="Status Dinamo Listrik sekarang : \n";
  pesan += "ON / OFF  : ";
  if(ins2 == HIGH || ins1 == HIGH){
    pesan += "ON\n";
  }
  else{
    pesan += "OFF\n";
  }
  pesan += "Direction : ";
  if(lastdirection == 1){
    pesan += "Kanan\n";
  }
  else if (lastdirection == 0) {
    pesan += "Kiri\n";
  }
  pesan += "Kecepatan : " + spd;
  bot.sendMessage(ids,pesan,"");
}

void lcdUpdate(){
  int ins1 = digitalRead(in1);
  int ins2 = digitalRead(in2);
  String status;

  lcd.clear();

  if(ins2 == HIGH || ins1 == HIGH){
    status = "ON";
  }
  else{
    status = "OFF";
  }

  lcd.setCursor(0, 0);
  lcd.print("Status Motor:");
  lcd.print(status);

  lcd.setCursor(0, 1);
  lcd.print("Speed : ");
  lcd.print(speeds); // pastikan speeds = global
}


void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    String text = bot.messages[i].text;
    Serial.println(text);
    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String control = "Selamat Datang, " + from_name + ".\n";
      control += "Gunakan perintah ini untuk mengontrol motor.\n\n";
      control += "/speed <angka 0 - 100> untuk mengatur kecepatan motor\n";
      control += "/direction <kanan / kiri>untuk mengontrol arah motor\n";
      control += "/status untuk melihat status motor\n";
      control += "/onoff <on/off> untuk menyalakan atau mematikan motor\n";
      bot.sendMessage(chat_id, control, "");
    } 
    else if (text.startsWith("/speed")) {
      String speedStr = text.substring(7); 
      speedStr.trim();
      speeds = speedStr.toInt();
      speed(speeds);
      if (speedStr.length() > 0 && speedStr.toInt() >= 0 && speedStr.toInt() <= 100) {
        bot.sendMessage(chat_id, "Kecepatan motor diatur ke " + String(speeds));
      } else {
        bot.sendMessage(chat_id, "Format salah! Gunakan: /speed <angka> (1-100)");
      }
    }
    else if (text.startsWith("/direction")) {
      String arah = text.substring(11); 
      arah.trim();
      if (arah == "kanan") {
        Arah(1);
        bot.sendMessage(chat_id, "Putar ke kanan");
      } 
      else if(arah == "kiri"){
        Arah(0);
        bot.sendMessage(chat_id, "putar ke kiri");
      }
      else{
        bot.sendMessage(chat_id, "Format salah! Gunakan: kanan / kiri");
      }
    }
    else if (text.startsWith("/onoff")) {
      String onoff = text.substring(6); 
      onoff.trim();
      if (onoff == "on") {
        if (lastdirection == 1)
        {
          Arah(1);
        }
        else{
          Arah(0);
        }
        bot.sendMessage(chat_id, "hidup");
      } 
      else if(onoff == "off"){
        digitalWrite(in1, LOW);
        digitalWrite(in2, LOW);
        bot.sendMessage(chat_id, "mati");
      }
      else {
        bot.sendMessage(chat_id, "Format salah! Gunakan: on / off");
      }
    }
    else if(text.startsWith("/status")){
      status(chat_id);
    }
  }
} 

void setup() {
  Serial.begin(115200);
  SetWifi();
  lcd.init();
  lcd.backlight();
  pinMode(ena, OUTPUT);
  pinMode(in1, OUTPUT); 
  pinMode(in2, OUTPUT);
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
}

void loop() {
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
    lcdUpdate();
  }
  
}
