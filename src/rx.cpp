#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <HardwareSerial.h>
#include "esp_wifi_types.h"
#include "esp_wifi.h"
#include "esp_err.h"
#include "config.h"

#define WIFI_CHANNEL_SWITCH_INTERVAL  (500)
#define WIFI_CHANNEL_MAX               (11) // US has 11 channels

#define PROBE_REQ_SUBTYPE 0b0100

#define RX_pin 23
#define TX_pin 22

HardwareSerial SerialPort(2);
 
uint8_t channel = 1; // spends twice as long on channel 1 in the beginning

const String id = WiFi.macAddress();

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

static void trace_wifi_init();
static void processMetadata(wifi_promiscuous_pkt_t *packet);
static void wifi_packet_handler(void *packet_buff, wifi_promiscuous_pkt_type_t packet_type); // wifi_promiscuous_cb_t params
static void wifi_set_channel(uint8_t wifi_channel);

void trace_wifi_init()
{
    wifi_init_config_t default_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&default_config)); // init wifi resources
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA)); // set wifi station mode
    ESP_ERROR_CHECK(esp_wifi_start()); // start station
    wifi_set_channel(1); // set wifi channel to 1
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

    printf("CHAN=%02d, RSSI=%02d,"
        " Request MAC=%02x:%02x:%02x:%02x:%02x:%02x\n",
        packet->rx_ctrl.channel,
        packet->rx_ctrl.rssi,
        header->addr2[0],header->addr2[1],header->addr2[2],
        header->addr2[3],header->addr2[4],header->addr2[5]
    );

    StaticJsonDocument<200> doc;
    // doc["chan"] = packet->rx_ctrl.channel;
    doc["mac"] = String(header->addr2[0], 16) + ":" + String(header->addr2[1], 16) + ":"
               + String(header->addr2[2], 16) + ":" + String(header->addr2[3], 16) + ":"
               + String(header->addr2[4], 16) + ":" + String(header->addr2[5], 16);
    doc["rssi"] = String(packet->rx_ctrl.rssi);;
    doc["label"] = ESP_LABEL;
    doc["id"] = id;
    char jsonBuffer[512];
    serializeJson(doc, jsonBuffer); 

    SerialPort.println(jsonBuffer);
}

void setup()
{
    Serial.begin(115200);
    SerialPort.begin(115200, SERIAL_8N1, RX_pin, TX_pin);   
    delay(10);
    
    trace_wifi_init();
}

void loop()
{
    vTaskDelay(WIFI_CHANNEL_SWITCH_INTERVAL / portTICK_PERIOD_MS); 
    wifi_set_channel(channel); 
    channel = (channel % WIFI_CHANNEL_MAX) + 1;
}