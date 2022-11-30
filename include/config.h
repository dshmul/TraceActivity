#define DELAY_INTERVAL 60   // seconds beetween channel cycling
#define CAPTURE_INTERVAL 10 // length of channel sweeping
#define DEVICE_NUMBER 6

/*
    DEVICE_NUMBER Table
    1 = Blue Box RX
    2 = Blue Box TX
    3 = Red Box RX
    4 = Red Box TX
    5 = Green Box RX
    6 = Green Box TX
*/
#if (DEVICE_NUMBER == 1 || DEVICE_NUMBER == 2)
    #define ESP_LABEL "Alight"
#elif (DEVICE_NUMBER == 3 || DEVICE_NUMBER == 4)
    #define ESP_LABEL "2nd Ave"
#else
    #define ESP_LABEL "Lab Hotspot"
#endif