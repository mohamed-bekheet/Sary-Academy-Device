#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <ESPConnect.h>
#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h> //for mp3 module
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

AsyncWebServer server(80);//for auto connect function

//uncomment one only to choose which device you need to WORK

#define ATTENDANCE
//#define KIDSAREA

//new smd board
#define PIN_LED 10

#define SS_PIN D8  //D8
#define RST_PIN D4 //D4

#define button_pin 16

#define mp3_rx 5
#define mp3_tx 4

/*
 * //old board
#define PIN_LED D3 // On Trinket or Gemma, suggest changing this to 1
#define SS_PIN D8//D8
#define RST_PIN D4//D4
#define button_pin D0
#define mp3_rx D1
#define mp3_tx D0
*/

#define cmd 3
#define par1 5
#define par2 6
#define checkSumM 7
#define checkSumL 8

#ifdef ATTENDANCE
const char *url = "http://192.168.1.201/sary-system/api-attendances.php";
const char *api_key = "42f12f-b368be-e7fbb4-de0036-56975d";
#endif

#ifdef KIDSAREA
const char *url = "http://192.168.1.200/sary-system/api.php";
const char *api_key = "f13b5611-170a-4174-9dfb-f5d68dbde960";
#endif

int re_tries = 5;
int timesouthttp = 5000; //5s

SoftwareSerial mp3Ser(mp3_tx, mp3_rx, false); //SoftwareSerial(rxPin, txPin, inverse_logic, buffer size);
//start byte,version,Length,CMD,Feedback,Para_MSB,Para_LSB,checksum_MSByte,checksum_MSByte,End byte
char stack[10] = {0x7E, 0xFF, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF};

Adafruit_NeoPixel pixels(1, PIN_LED, NEO_GRB + NEO_KHZ800);

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;

String CardID = "1";
String button_state = "released";
//18059529

void card_reader_begin(void)
{
  SPI.begin();     // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
}

void sendStack(uint8_t command, uint8_t parameter1, uint8_t parameter2)
{
  uint16_t checkSum = (0xFFFF - (0x105 + command + parameter1 + parameter2) + 1);
  uint8_t sumL = (uint8_t)checkSum; //most sig..
  uint8_t sumM = (checkSum >> 8);   //least siginificant
  stack[cmd] = command;
  stack[par1] = parameter1;
  stack[par2] = parameter2;
  stack[checkSumM] = sumM;
  stack[checkSumL] = sumL;
  for (int i = 0; i < 10; i++)
  {
    mp3Ser.write(stack[i]);
  }
  //unsigned int n = USART_Read();
}
void playTrack(uint8_t folder, uint8_t track)
{
  sendStack(0x0F, folder, track);
}
void set_volume(uint8_t vol)
{
  sendStack(0x06, 0x00, vol);
}
void play_random(void)
{
  sendStack(0x18, 0x00, 0x00);
}
void Handle_msg(String msg)
{
  if (msg == "This User is Registerd Before")
    playTrack(01, 0x001);
  else if (msg == "the card has been added")
    playTrack(01, 0x002);
  else if (msg == "Timer Started")
    playTrack(01, 0x003);
  else if (msg == "Timer Stopped")
    playTrack(01, 0x004);
  else if (msg == "Recharge your card")
    playTrack(01, 0x005);
  else if (msg == "This Card has not registered before")
    playTrack(01, 0x006);
  delay(3000);
}
void Handle_msg_att(String msg)
{
  if (msg == "This User is Registerd Before")
    playTrack(01, 0x001);
  else if (msg == "the card has been added")
    playTrack(01, 0x002);
  else if (msg == "Logged IN")
    playTrack(01, 0x00A); //logged in succ
  else if (msg == "Logged OUT")
    playTrack(01, 0x00B); //logged out succ
  else if (msg == "Please create this user first")
    playTrack(01, 0x006);
  else if (msg == "Please enter the card type")
    playTrack(01, 0x01B); //fill card details
  else if (msg == "This Card has not registered before")
    playTrack(01, 0x006);
  delay(3000);
}

void setup()
{
  delay(100);
  Serial.begin(115200);
  mp3Ser.begin(9600);
  delay(500);
  Serial.println("ESP powered on");
  pixels.begin();
  delay(100);
  pixels.clear(); // Set all pixel colors to 'off'
  pixels.setPixelColor(0, pixels.Color(255, 0, 0));
  pixels.show(); //RED
  set_volume(30);
  delay(200);
  playTrack(01, 0x008); //wifi is not connected
  delay(2000);
  playTrack(01, 0x015); //connect to AP SARY ACADEMY DEVICE
  delay(1000);
  // WiFiManager wifiManager;

  // wifiManager.resetSettings(); // Uncomment and run it once, if you want to erase all the stored information

  // IPAddress local_ip(192, 168, 1, 1);
  // IPAddress gateway(192, 168, 11, 1);
  // IPAddress netmask(255, 255, 255, 0);
  // WiFi.softAPConfig(local_ip, gateway, netmask);
  // //wifiManager.setConnectTimeout(30);
  // wifiManager.autoConnect("Sary Academy DEVICE");
  //  WiFi.mode(WIFI_STA);
  // WiFi.begin("mohamed", "mohamed2024");
  // WiFi.setAutoReconnect(true);
  // int wifi_retry = 50;
  // while (WiFi.status() != WL_CONNECTED)
  // {
  //   delay(200);
  //   Serial.print('.');
  //   if (--wifi_retry == 0)
  //   {
  //     Serial.println("failed to connect");
  //     break;
  //   }
  // }

  /*
    AutoConnect AP
    Configure SSID and password for Captive Portal
  */
  ESPConnect.autoConnect("SaryDevice");

  /* 
    Begin connecting to previous WiFi
    or start autoConnect AP if unable to connect
  */
  if (ESPConnect.begin(&server))
  {
    Serial.println("Connected to WiFi");
    Serial.println("IPAddress: " + WiFi.localIP().toString());
  }
  else
  {
    Serial.println("Failed to connect to WiFi");
  }

  server.on("/", HTTP_GET, [&](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", "Hello from ESP"); });

  server.begin();

  Serial.println("Connected.");
  /**/
  delay(7000);
  playTrack(01, 0x018); //wifi connected
  pixels.setPixelColor(0, pixels.Color(0, 0, 255));
  pixels.show();
  delay(2000);

  card_reader_begin();
  Serial.println(F("Start Scanning Any Card..."));
  playTrack(01, 0x007); //wlecome to sary academy
  delay(2000);
}

void loop()
{
  // if (digitalRead(button_pin))
  //   return;
  // delay(1000);
  // pixels.setPixelColor(0, pixels.Color(255, 0, 0));
  // pixels.show(); //RED
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Wifi disconnected");
    playTrack(01, 0x008); //wifi is not connected
    // retry to connect to wifi
    WiFi.mode(WIFI_STA);
    WiFi.begin();
    int wifi_retry = 50;
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(200);
      Serial.print('.');
      if (--wifi_retry == 0)
      {
        Serial.println("failed to reconnect");
        ESP.reset(); // last resort is to just reset
      }
    }
  }

  if (!digitalRead(button_pin))
  {
    button_state = "pressed";
    pixels.setPixelColor(0, pixels.Color(255, 100, 0));
    pixels.show();        //orange
    playTrack(01, 0x01A); //swap ur card
    delay(600);
  }

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!rfid.PICC_IsNewCardPresent())
    return;
  if (!rfid.PICC_ReadCardSerial())
    return;
  pixels.setPixelColor(0, pixels.Color(255, 255, 0));
  pixels.show(); // yellow
//WHEN CARD DETECTED
#ifdef ATTENDANCE
  if (button_state != "pressed")
    playTrack(01, 0x012); //please be patient and wait for the response
#endif
#ifdef KIDSAREA
  if (button_state != "pressed")
    playTrack(01, 0x013); //hello our future hero
#endif

  Serial.println(F("A new card has been detected."));
  CardID = "";
  for (byte i = 0; i < 4; i++)
  {
    CardID += String(rfid.uid.uidByte[i], HEX);
  }
  Serial.print(F("CARD:"));
  Serial.println(CardID);
  delay(2000);

  //http request
  re_tries = 3;
  if (WiFi.status() == WL_CONNECTED)
  {

    int http_response_code = -1;
    String payload;
    while (re_tries > 0)
    {
      WiFiClient client;
      HTTPClient http;
      http.setTimeout(2000);
      http.begin(client, url);
      http.setReuse(false); //important
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      String data_sent = "key=" + String(api_key) + "&code=" + CardID + "&tag=" + button_state + "";
      http_response_code = http.POST(data_sent);
      payload = http.getString();
      Serial.println("httpRequestData: ");
      Serial.print(url);
      Serial.println(data_sent);
      Serial.print("Code:");
      Serial.println(http_response_code);
      Serial.print("msg:");
      Serial.println(payload);
      re_tries--;
      if (http_response_code > 0)
        re_tries = 0; //terminate
    }
    if (http_response_code > 0)
    {
      pixels.setPixelColor(0, pixels.Color(0, 255, 0));
      pixels.show(); //GREEN
      Serial.println("Succesful posting");
      re_tries = 0;

#ifdef ATTENDANCE
      Handle_msg_att(payload); //mp3
#endif
#ifdef KIDSAREA
      Handle_msg(payload); //mp3
#endif

      button_state = "released";
    }
    else
    {
      Serial.println("Posting Failed");
      pixels.setPixelColor(0, pixels.Color(255, 0, 0));
      pixels.show();        //RED (error)
      playTrack(01, 0x009); //connection faild
      delay(1000);
    }
    button_state = "released";
  }
  else
  {
    Serial.println("Wifi disconnected");
    playTrack(01, 0x008); //wifi is not connected
    // retry to connect to wifi
    WiFi.mode(WIFI_STA);
    WiFi.begin();
    int wifi_retry = 50;
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(200);
      Serial.print('.');
      if (--wifi_retry == 0)
      {
        Serial.println("failed to reconnect");
        ESP.reset(); // last resort is to just reset
      }
    }
  }
  delay(1000);
  // Halt PICC
  rfid.PICC_HaltA();
  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
  pixels.setPixelColor(0, pixels.Color(167, 0, 255));
  pixels.show(); //purple (normal mode)
}
