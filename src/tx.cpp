#include <Arduino.h>
#include "secrets.h"
#include <HardwareSerial.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "config.h"

#if (DEVICE_NUMBER == 2 || DEVICE_NUMBER == 4 || DEVICE_NUMBER == 6)
 
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

#if DEVICE_NUMBER == 2  // ESP32 DevKit V1
#define LED 2  
#define RX_pin 16
#define TX_pin 17

#else
#define LED 2
#define RX_pin 23
#define TX_pin 22

#endif

HardwareSerial SerialPort(2);
 
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

static void connectAWS();
static void publishMessage(String payload);
static void messageHandler(char* topic, byte* payload, unsigned int length);

volatile int interruptCounter;
hw_timer_t* timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onAlarm()
{
    portENTER_CRITICAL_ISR(&timerMux);
    interruptCounter++;
    portEXIT_CRITICAL_ISR(&timerMux);
}

/**
 * @brief Connects to WIFI and AWS IoT Core
 */
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

/**
 * @brief Publishes MQTT message to AWS IoT Core
 * 
 * @param payload message to be sent
 */
void publishMessage(String payload)
{
    Serial.print("Tx: ");
    Serial.println(payload);

    client.publish(AWS_IOT_PUBLISH_TOPIC, payload.c_str());
}

/**
 * @brief Callback for messages received from AWS IoT Core
 * 
 * @param topic MQTT topic
 * @param payload message
 * @param length message length 
 */
void messageHandler(char* topic, byte* payload, unsigned int length)
{
    Serial.print("incoming: ");
    Serial.println(topic);
    
    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload);
    const char* message = doc["message"];
    Serial.println(message);
}

/**
 * @brief Initialize Serial communication and AWS connection
 * 
 * @details Called on device boot
 */
void setup()
{
    Serial.begin(115200);
    SerialPort.begin(115200, SERIAL_8N1, RX_pin, TX_pin);   
    delay(10);

    pinMode(LED, OUTPUT);
    
    connectAWS();

    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onAlarm, true);
    timerAlarmWrite(timer, 1000000 * DELAY_INTERVAL, true); 
    timerAlarmEnable(timer);
}

/**
 * @brief Continuously read Serial data and publish if valid message is received
 * 
 * @details Enters loop after setup()
 */
void loop()
{
    if (interruptCounter > 0)
    {
        portENTER_CRITICAL(&timerMux);
        interruptCounter--;
        portEXIT_CRITICAL(&timerMux);

        timerStop(timer);
        Serial.println("================= DELAY TRIGGER ================");
        delay(DELAY_INTERVAL * 1000 * 6);
        timerStart(timer);
    }
    
    if (!client.connected())
    {
        Serial.println("AWS IoT Timeout!");


        while (!client.connect(THINGNAME))
        {
            Serial.print(".");
            delay(100);
        }
    }

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

    if (storedData[0] == '{' && storedData[storedData.length() - 1] == '}')
    {
        publishMessage(storedData);

        if (digitalRead(LED) == LOW)
        {
            digitalWrite(LED, HIGH);
        }
        else 
        {
            digitalWrite(LED, LOW);
        }
    }
    
    storedData = "";
}

#endif