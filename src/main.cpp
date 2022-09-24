#include <Arduino.h>
#include <ctime>
#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include "esp_wifi_types.h"
#include "esp_wifi.h"
#include "esp_err.h"

#define PROBE_REQ_SUBTYPE 0b0100
 
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

// recycled
#define WIFI_CHANNEL_SWITCH_INTERVAL  (500)
#define WIFI_CHANNEL_MAX               (11) // US has 11 channels
 
uint8_t channel = 1;

String rssi = "";
String mac = "";
String timestamp = "";

String label = "daniel";
String id = WiFi.macAddress();

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

// recycled (https://blog.podkalicki.com/esp32-wifi-sniffer/)
typedef struct {
  unsigned frame_ctrl:16;
  unsigned duration_id:16;
  uint8_t addr1[6]; /* receiver address */
  uint8_t addr2[6]; /* sender address */
  uint8_t addr3[6]; /* filtering address */
  unsigned sequence_ctrl:16;
  uint8_t addr4[6]; /* optional */
} wifi_ieee80211_mac_hdr_t;

typedef struct {
  wifi_ieee80211_mac_hdr_t header;
  uint8_t payload[0]; /* network data ended with 4 bytes csum (CRC32) */
} wifi_ieee80211_packet_t;

static void wifi_init();
static void processMetadata(wifi_promiscuous_pkt_t *packet);
static void wifi_packet_handler(void *packet_buff, wifi_promiscuous_pkt_type_t packet_type); // wifi_promiscuous_cb_t params
static void wifi_set_channel(uint8_t wifi_channel);

static void connectAWS();
static void publishMessage();
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

void wifi_init()
{
    wifi_init_config_t default_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&default_config)); // init wifi resources
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA)); // set wifi station mode
    ESP_ERROR_CHECK(esp_wifi_start()); // start station
    // wifi_set_channel(1); // set wifi channel to 1
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true)); // enter promiscuous mode  
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(&wifi_packet_handler)); // set packet handler callback
}

void wifi_set_channel(uint8_t channel)
{
    ESP_ERROR_CHECK(esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE));
}

void wifi_packet_handler(void *packet_buff, wifi_promiscuous_pkt_type_t packet_type)
{
    wifi_promiscuous_pkt_t *packet = (wifi_promiscuous_pkt_t *)packet_buff;
    wifi_ieee80211_packet_t *payload = (wifi_ieee80211_packet_t *)packet->payload;
    wifi_ieee80211_mac_hdr_t *header = &payload->header;
    uint16_t frameCtrl = header->frame_ctrl;
    uint16_t packet_subtype = (frameCtrl & 0b0000000011110000) >> 4; // little endian
    
    if (packet_type != WIFI_PKT_MGMT && packet_subtype != PROBE_REQ_SUBTYPE)
    {
        return;
    }

    processMetadata(packet);
}

static void processMetadata(wifi_promiscuous_pkt_t *packet)
{
    wifi_ieee80211_packet_t *payload = (wifi_ieee80211_packet_t *)packet->payload;
    wifi_ieee80211_mac_hdr_t *header = &payload->header;

    time_t ttime = time(0);    

    printf("CHAN=%02d, RSSI=%02d,"
        " Request MAC=%02x:%02x:%02x:%02x:%02x:%02x," 
        " Timestamp=%s",
        packet->rx_ctrl.channel,
        packet->rx_ctrl.rssi,
        header->addr2[0],header->addr2[1],header->addr2[2],
        header->addr2[3],header->addr2[4],header->addr2[5],
        ctime(&ttime)
    );

    rssi = String(packet->rx_ctrl.rssi);
    mac = String(header->addr2[0], 16) + ":" + String(header->addr2[1], 16) + ":"
        + String(header->addr2[2], 16) + ":" + String(header->addr2[3], 16) + ":"
        + String(header->addr2[4], 16) + ":" + String(header->addr2[5], 16);
    timestamp = String(ctime(&ttime));

    publishMessage();
    client.loop();
}

void publishMessage()
{
    StaticJsonDocument<200> doc;
    doc["mac"] = mac;
    doc["rssi"] = rssi;
    doc["timestamp"] = timestamp;
    doc["label"] = label;
    doc["id"] = id;
    char jsonBuffer[512];
    serializeJson(doc, jsonBuffer); // print to client
    
    client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
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
    delay(10);
    
    connectAWS();
    wifi_init();
}

void loop()
{
    // send message to AWS
    // publishMessage();
    // client.loop();

    // vTaskDelay(WIFI_CHANNEL_SWITCH_INTERVAL / portTICK_PERIOD_MS); 
    // wifi_set_channel(channel); 
    // channel = (channel % WIFI_CHANNEL_MAX) + 1;
}