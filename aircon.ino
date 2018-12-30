#include <DHT11.h>    //라이브러리 불러옴
#include <IRremote.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>// used to couumicate via the I2C bus 

int pin = 5;          //Signal 이 연결된 아두이노의 핀번호
int RECV_PIN = 4;
bool airconstatus;
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x48 };
IPAddress ip(192, 168, 35, 148); // IP address, may need to change depending on network
IPAddress gateway(192,168, 35, 1);
EthernetServer server(80); // create a server at port 80
DHT11 dht11(pin);
IRrecv irrecv(RECV_PIN);
IRsend irsend;
decode_results results;
unsigned int  ondata[59] = {8900, 4150, 450, 1650, 400, 600, 450, 600, 450, 550, 500, 1600, 450, 550, 500, 550, 450, 600, 400, 600, 450, 600, 450, 600, 450, 600, 400, 600, 450, 600, 450, 550, 450, 600, 450, 1650, 400, 600, 450, 600, 450, 1650, 400, 600, 450, 1650, 400, 600, 450, 550, 500, 1600, 400, 1650, 450, 600, 400, 1650, 450}; // UNKNOWN F6D13AE8
unsigned int  offdata[59] = {8800, 4200, 450, 1600, 500, 550, 450, 600, 450, 550, 500, 1600, 450, 600, 450, 550, 450, 600, 450, 1600, 450, 1600, 500, 550, 450, 600, 450, 550, 500, 550, 450, 600, 450, 550, 500, 550, 450, 600, 450, 550, 450, 600, 450, 600, 450, 1600, 450, 600, 400, 1650, 450, 550, 500, 550, 450, 600, 450, 1600, 450}; // UNKNOWN 1035C9DA
unsigned int  twentyfourdata[59] = {8800, 4200, 550, 1550, 500, 550, 450, 600, 450, 550, 500, 1600, 450, 550, 500, 550, 500, 550, 450, 550, 500, 550, 450, 600, 450, 550, 450, 1600, 500, 550, 450, 600, 450, 600, 450, 1600, 500, 500, 500, 550, 500, 1550, 500, 600, 450, 1550, 500, 550, 450, 600, 450, 550, 500, 1600, 450, 550, 500, 1600, 450}; // UNKNOWN C4DB2A22
void setup()
{
  Ethernet.begin(mac, ip, gateway); // initialize Ethernet device
  server.begin(); // start to listen for clients
  Serial.begin(9600); //통신속도 설정
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  irrecv.enableIRIn();
  airconstatus = false;
}


int on() {
  Serial.println("aircon on!");
  airconstatus = true;
  irsend.sendRaw(ondata, 59, 38);
  delay(1000);
  irsend.sendRaw(twentyfourdata, 59, 38);
  return 0;
}
int off() {
  Serial.println("aircon off!");
  airconstatus = false;
  irsend.sendRaw(offdata, 59, 38);
  delay(1000);
  return 0;
}

int check() {
  float temp, humi;
  dht11.read(humi, temp); //온도, 습도 읽어와서 표시
  Serial.print("temperature:");
  Serial.print(temp);
  Serial.print(" humidity:");
  Serial.print(humi);
  Serial.println();
  if (temp >= 25.5f && airconstatus == false) {
    on();
  }
  else if (temp < 25.0f && airconstatus == true) {
    off();
  }
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println(F("HTTP/1.1 200 OK"));
          client.println(F("Content-Type: text/html"));
          client.println(F("Connection: close"));
          client.println(F("Refresh: 5"));
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<head>");
          client.println(F("<title>temp and humi of my room</title>"));
          client.println("</head>");
          client.println("<body>");
          // output the value from the BME280 sensor
          client.print(F("Temperature is "));
          client.print(temp);
          client.println("&nbsp;&#8451;<br><br>");
          client.print("Humidity is ");
          client.print(humi);
          client.println(" %<br><br>");
          client.println("</body>");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
  }

  // give the web browser time to receive the data
  delay(1);
  // close the connection:
  client.stop();
  // Serial.println("client disconnected");
  return 0;
}


int IRsensorRecieve() {
  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    irrecv.resume();
    return 0;
  }
}

void loop()
{
  check();
  delay(5000);
}

