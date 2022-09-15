#include <Arduino.h>
#include "esp_wifi_types.h"
#include "esp_wifi.h"
#include "esp_err.h"
#include "WiFi.h"
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"

#define PROBE_REQ_SUBTYPE 0b0100

// recycled
#define WIFI_CHANNEL_SWITCH_INTERVAL  (500)
#define WIFI_CHANNEL_MAX               (11) // US has 11 channels

const char* ssid  = "SSID"; // replace with actual ssid
const char* password = "PASSWORD"; // replace with actual password

AsyncWebServer server(80);

String chan = "";
String rssi = "";
String mac = "";

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
static void server_init();
static void wifi_connect();

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
    WiFi.disconnect();
    ESP_ERROR_CHECK(esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE));
    WiFi.reconnect();
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

    chan = String(packet->rx_ctrl.channel);
    rssi = String(packet->rx_ctrl.rssi);
    mac = String(header->addr2[0], 16) + ":" + String(header->addr2[1], 16) + ":"
        + String(header->addr2[2], 16) + ":" + String(header->addr2[3], 16) + ":"
        + String(header->addr2[4], 16) + ":" + String(header->addr2[5], 16);
}

const char index_html[] = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP32 Web Server</h2>
  <p>
    <span class="labels">Channel</span> 
    <span id="channel">%CHANNEL%</span>
  </p>
  <p>
    <span class="labels">RSSI</span>
    <span id="rssi">%RSSI%</span>
  </p>
  <p>
    <span class="labels">Mac Address</span> 
    <span id="mac-address">%MAC_ADDRESS%</span>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("channel").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/channel", true);
  xhttp.send();
}, 10 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("rssi").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/rssi", true);
  xhttp.send();
}, 10 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("mac-address").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/mac-address", true);
  xhttp.send();
}, 10) ;

</script>
</html>)rawliteral";

String processor(const String& var){
    //Serial.println(var);
    if(var == "CHANNEL"){
        return chan;
    }
    else if(var == "RSSI"){
        return rssi;
    }
    else if(var == "MACADDRESS"){
        return mac;
    }
    return String();
}

static void server_init(){
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("Connecting to WiFi..");
        delay(1000);
    }
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html, processor);
    });
    server.on("/channel", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", chan.c_str());
    });
    server.on("/rssi", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", rssi.c_str());
    });
    server.on("/mac-address", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/plain", mac.c_str());
    });
    server.onNotFound([](AsyncWebServerRequest *request){
        request->send(404, "text/plain", "Not found");
    });
    server.begin();
}

void setup()
{
    Serial.begin(115200);
    delay(10);
    server_init();
    wifi_init();
}

void loop()
{
    vTaskDelay(WIFI_CHANNEL_SWITCH_INTERVAL / portTICK_PERIOD_MS); // block callback?
    wifi_set_channel(channel); 
    channel = (channel % WIFI_CHANNEL_MAX) + 1;
}
