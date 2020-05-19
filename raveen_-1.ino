
#include "ThingSpeak.h"
#include "WiFiEsp.h"
#include "secrets.h"
#include "EmonLib.h"                   // Include Emon Library
EnergyMonitor emon1;                   // Create an instance
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F, 16, 2);
double kilos = 0;
int peakPower = 0;
char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiEspClient  client;

// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(5, 3); // RX, TX
#define ESP_BAUDRATE  9600
#else
#define ESP_BAUDRATE  115200
#endif

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

// Initialize our values
int number1;
int number2;
int number3;
int number4;
String myStatus = "";

void setup() {
  //Initialize serial and wait for port to open
  Serial.begin(115200);  // Initialize serial
  lcd.begin();  
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("Smart pwr meter"));
  lcd.setCursor(0,1);
  lcd.print(F("  System init"));
  emon1.current(1, 66.6);
  // initialize serial for ESP module  
  setEspBaudRate(ESP_BAUDRATE);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo native USB port only
  }
 
  // initialize ESP module
  WiFi.init(&Serial1);

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println(F("WiFi shield not present"));
    // don't continue
    while (true);
  }
  Serial.println(F("found it!"));
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  delay(2000);
}

void loop() {

  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print(F("Attempting to connect to SSID: "));
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(F("."));
      delay(5000);     
    } 
    Serial.println(F("\nConnected."));
  }
double Irms = emon1.calcIrms(1480);  // Calculate Irms only
  double RMSPower = Irms*230.6 ;
  double RMSCurrent = -((Irms-516)*0.707)/11.8337;
  kilos = kilos + (RMSPower * (2.05/60/60/1000));    //Calculate kilowatt hours used
  Serial.print(RMSPower-20);         // Apparent power
  Serial.println(" ");
  //Serial.println(Irms);          // Irms
  lcd.clear();
  lcd.setCursor(0,0);           // Displays all current data
  lcd.print(Irms);
  lcd.print(F("A"));
  lcd.setCursor(10,0);
  lcd.print(RMSPower);
  lcd.print(F("W"));
  lcd.setCursor(0,1);
  lcd.print(kilos);
  lcd.print(F("H"));
  lcd.setCursor(10,1);
  lcd.print(RMSCurrent);
  lcd.print(F("T"));
  number1 = Irms;
  number2 = RMSPower;
  number3 = RMSCurrent;
  number4 = kilos;
  // set the fields with the values
  ThingSpeak.setField(1, number1);
  ThingSpeak.setField(2, number2);
  ThingSpeak.setField(3, number3);
  ThingSpeak.setField(4, number4);

  
  // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println(F("Channel update successful."));
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }

  delay(20000); // Wait 20 seconds to update the channel again
}

// This function attempts to set the ESP8266 baudrate. Boards with additional hardware serial ports
// can use 115200, otherwise software serial is limited to 19200.
void setEspBaudRate(unsigned long baudrate){
  long rates[6] = {115200,74880,57600,38400,19200,9600};

  Serial.print("Setting ESP8266 baudrate to ");
  Serial.print(baudrate);
  Serial.println("...");

  for(int i = 0; i < 6; i++){
    Serial1.begin(rates[i]);
    delay(100);
    Serial1.print("AT+UART_DEF=");
    Serial1.print(baudrate);
    Serial1.print(",8,1,0,0\r\n");
    delay(100);  
  }
    
  Serial1.begin(baudrate);
}
