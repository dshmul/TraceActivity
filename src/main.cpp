#include <Arduino.h>
#include <ctime>
#include "esp_wifi_types.h"
#include "esp_wifi.h"
#include "esp_err.h"

#define PROBE_REQ_SUBTYPE 0b0100

// recycled
#define WIFI_CHANNEL_SWITCH_INTERVAL  (500)
#define WIFI_CHANNEL_MAX               (11) // US has 11 channels
 
uint8_t channel = 1;

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

void wifi_init()
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
}

void setup()
{
    Serial.begin(115200);
    delay(10);
    
    wifi_init();
}

void loop()
{
    vTaskDelay(WIFI_CHANNEL_SWITCH_INTERVAL / portTICK_PERIOD_MS); 
    wifi_set_channel(channel); 
    channel = (channel % WIFI_CHANNEL_MAX) + 1;
}