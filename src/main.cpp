#include <EtherCard.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>

DHT dht(2, DHT11);

#define PATH ""

byte mymac[] = {0x74, 0x69, 0x69, 0x2D, 0x30, 0x31};

char const website[] PROGMEM = "smart-room-api.herokuapp.com";

byte Ethernet::buffer[700];
uint32_t timer;
Stash stash;

void setup()
{
  Serial.begin(9600);
  Serial.println("\n[webClient]");
  dht.begin();

  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0)
    Serial.println("Failed to access Ethernet controller");
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
  ether.packetLoop(ether.packetReceive());

  if (millis() > timer)
  {
    timer = millis() + 10000;

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    byte sd = stash.create();
    stash.print("temp=");
    stash.print(t);
    stash.print("&humid=");
    stash.print(h);
    stash.save();

    Stash::prepare(PSTR("POST https://$F/$F HTTP/1.0"
                        "\r\n"
                        "Host: $F"
                        "\r\n"
                        "Content-Length: $D"
                        "\r\n"
                        "Content-Type: application/x-www-form-urlencoded"
                        "\r\n"
                        "\r\n"
                        "$H"),
                   website, PSTR(PATH), website, stash.size(), sd);

    ether.tcpSend();
  }
}