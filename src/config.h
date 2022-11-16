#define DELAY_INTERVAL 10   // seconds beetween channel cycling
#define DEVICE_NUMBER 2

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
    #define ESP_LABEL "Dev1"
#elif (DEVICE_NUMBER == 3 || DEVICE_NUMBER == 4)
    #define ESP_LABEL "Dev2"
#else
    #define ESP_LABEL "Dev3"
#endif