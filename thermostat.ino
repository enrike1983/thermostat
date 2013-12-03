#include <Wire.h>  // Comes with Arduino IDE

//LCD, ETHERNET, DHT11
#include <LiquidCrystal_I2C.h>
#include "dht11.h"
#include <EtherCard.h>

dht11 DHT11;

#define DHT11PIN 9

// set the LCD address to 0x27 for a 20 chars 4 line display
// Set the pins on the I2C chip used for LCD connections:
// addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

// ethernet interface mac address
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
// ethernet interface ip address
//static byte myip[] = { 10,10,9,69 };
// gateway ip address
//static byte gwip[] = { 192,168,1,34 };
// remote website ip address and port
//static byte hisip[] = { 192,168,1,34 };

// remote website name
char website[] PROGMEM = "http://www.robertcasanova.it";

byte Ethernet::buffer[700];
char line_buf[150];  

static uint32_t timer;

// called when the client request is complete
static void my_result_cb (byte status, word off, word len) {
  Serial.println(">>>");
  Ethernet::buffer[off+300] = 0;
  Serial.print((const char*) Ethernet::buffer + off);
  Serial.println("...");
}

void setup()  
{
  Serial.begin(9600);
  //initialize the lcd for 20 chars 4 lines, turn on backlight  
  lcd.begin(20,4);         
  
  // ------- Quick 3 blinks of backlight  -------------
  for(int i = 0; i< 3; i++)
  {
    lcd.backlight();
    delay(250);
    lcd.noBacklight();
    delay(250);
  }
  
  // finish with backlight on  
  lcd.backlight(); 
  
  lcd.setCursor(3,0);
  lcd.print("Termostato: ON!");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Temperatura");
  lcd.setCursor(0,1);
  lcd.print("Umidita'");  
  
  //ETHERNET
  Serial.println("\n[webClient]");

  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) 
    Serial.println( "Failed to access Ethernet controller");
  if (!ether.dhcpSetup())
    Serial.println("DHCP failed");

  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);  
  ether.printIp("DNS: ", ether.dnsip);  

  if (!ether.dnsLookup(website))
    Serial.println("DNS failed");
    
  ether.printIp("SRV: ", ether.hisip); 
}


void loop()
{
  //ethernet
  ether.packetLoop(ether.packetReceive());
  
  if (millis() > timer) {

    int chk = DHT11.read(DHT11PIN);

    int t = DHT11.temperature;
    int h = DHT11.humidity;

    // posiziono il cursore alla colonna 13 e riga 0
    lcd.setCursor(13, 0);
    lcd.print(t);
    lcd.setCursor(13, 1);
    lcd.print(h);
    lcd.print('%');
    
    timer = millis() + 5000;
    Serial.println();
    Serial.print("<<< REQ ");
    
    sprintf(line_buf,"?t=%d&h=%d", t, h);  
    ether.browseUrl(PSTR("/arduino"), line_buf, website, my_result_cb); 
  }

  //delay(2000);
}
