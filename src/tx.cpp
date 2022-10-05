#include <Arduino.h>
#include "secrets.h"
#include <HardwareSerial.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
 
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

#define LED 2 // ESP32 DevKit V1 onboard LED
#define RX_pin 16
#define TX_pin 17

HardwareSerial SerialPort(2);
 
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

static void connectAWS();
static void publishMessage(String payload);
static void messageHandler(char* topic, byte* payload, unsigned int length);

void connectAWS()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.println("Connecting to Wi-Fi");

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    // Configure WiFiClientSecure to use the AWS IoT device credentials
    net.setCACert(AWS_CERT_CA);
    net.setCertificate(AWS_CERT_CRT);
    net.setPrivateKey(AWS_CERT_PRIVATE);

    // Connect to the MQTT broker on the AWS endpoint we defined earlier
    client.setServer(AWS_IOT_ENDPOINT, 8883);

    // Create a message handler
    client.setCallback(messageHandler);

    Serial.println("Connecting to AWS IOT");

    while (!client.connect(THINGNAME))
    {
        Serial.print(".");
        delay(100);
    }

    if (!client.connected())
    {
        Serial.println("AWS IoT Timeout!");
        return;
    }

    // Subscribe to a topic
    client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

    Serial.println("AWS IoT Connected!");
}

void publishMessage(String payload)
{
    Serial.print("Rx: ");
    Serial.println(payload);

    client.publish(AWS_IOT_PUBLISH_TOPIC, payload.c_str());
}
 
void messageHandler(char* topic, byte* payload, unsigned int length)
{
    Serial.print("incoming: ");
    Serial.println(topic);
    
    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload);
    const char* message = doc["message"];
    Serial.println(message);
}

void setup()
{
    Serial.begin(115200);
    SerialPort.begin(115200, SERIAL_8N1, RX_pin, TX_pin);   
    delay(10);
    
    connectAWS();
}

void loop()
{
    String storedData = "";
    if (SerialPort.available()) 
    { 
        while (SerialPort.available())         
        {
            char inChar = SerialPort.read();
            
            if (inChar == '\r' || inChar == '\n')
            {
                while (SerialPort.available())
                {
                    SerialPort.read();
                }

                break;
            }

            storedData += inChar;
        }
    }

    // Serial.print(storedData);
    if (storedData[0] == '{' && storedData[storedData.length() - 1] == '}')
    {
        publishMessage(storedData);
    }
    storedData = "";
}