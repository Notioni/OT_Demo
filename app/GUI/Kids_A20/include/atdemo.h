#ifndef ATDEMO_H
#define ATDEMO_H

typedef signed char     int8_t;
typedef signed short    int16_t;
typedef signed int     int32_t;
typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

enum at_cmd_e {
    AT_CMD_AT_TEST = 0, 
    AT_CMD_AT,  
    AT_CMD_AT_GETATVERSION,
    
    AT_CMD_AT_FWVER = 3,
    AT_CMD_AT_SYSTIME,
    AT_CMD_AT_MEMFREE,
    AT_CMD_AT_FACTORY,
    AT_CMD_AT_REBOOT,
    AT_CMD_AT_FLASHLOCK_GET,
    AT_CMD_AT_FLASHLOCK_SET,
    AT_CMD_AT_WEVENT_GET,
    AT_CMD_AT_WEVENT_SET,
    AT_CMD_AT_WLPC_GET,
    AT_CMD_AT_WLPC_SET,

    AT_CMD_AT_UART_GET = 14,
    AT_CMD_AT_UART_SET,
    AT_CMD_AT_UARTFOMAT_GET,
    AT_CMD_AT_UARTFOMAT_SET,
    AT_CMD_AT_UARTE_GET,
    AT_CMD_AT_UARTE_SET,
	//wifi config command
    AT_CMD_AT_WFVER = 20,
    AT_CMD_AT_WMAC,
    AT_CMD_AT_WSCANOPT_GET,
    AT_CMD_AT_WSCANOPT_SET,
    AT_CMD_AT_WSCAN,
    AT_CMD_AT_WDHCP_GET,
    AT_CMD_AT_WDHCP_SET,
    //AP
    AT_CMD_AT_WSAPIP_GET = 27,
    AT_CMD_AT_WSAPIP_SET,
    AT_CMD_AT_WSAP_GET,
    AT_CMD_AT_WSAP_SET,
    AT_CMD_AT_WSAPQ,
    AT_CMD_AT_WSAPS = 32,
    //sta
    AT_CMD_AT_WJAPIP_GET = 33,
    AT_CMD_AT_WJAPIP_SET,
    AT_CMD_AT_WJAP_GET = 35,
    AT_CMD_AT_WJAP_SET = 36,
    AT_CMD_AT_WJAPQ,
    AT_CMD_AT_WJAPS = 38,
    //TCP UDP
    AT_CMD_AT_SSLCERT_GET =39,
    AT_CMD_AT_SSLCERT_SET,
    AT_CMD_AT_CIPDOMAIN,
    AT_CMD_AT_CIPAUTOCONN,
    AT_CMD_AT_CIPSSLOPT,
    AT_CMD_AT_CIPSTART,
    AT_CMD_AT_CIPSTOP,
    AT_CMD_AT_CIPSTATUS,
    AT_CMD_AT_CIPSEND,
    AT_CMD_AT_CIPSENDRAW,
    AT_CMD_AT_CIPRECV,
    AT_CMD_AT_CIPRECVCFG_GET,
    AT_CMD_AT_CIPRECVCFG_SET,
     //FOTA
    AT_CMD_AT_FOTA,

#ifdef LORA_MODULE
    // LORA
    AT_CMD_AT_,
    AT_CMD_AT_RESET,
    AT_CMD_AT_SAVE,
    AT_CMD_AT_VERS_GET,
    AT_CMD_AT_BAUD_GET,
    AT_CMD_AT_BAUD_SET,
    AT_CMD_AT_CONFIRM_GET,
    AT_CMD_AT_CONFIRM_SET,
    AT_CMD_AT_NBTRIALS_GET,
    AT_CMD_AT_NBTRIALS_SET,
    AT_CMD_AT_ADR_GET,
    AT_CMD_AT_ADR_SET,
    AT_CMD_AT_CLASS_GET,
    AT_CMD_AT_CLASS_SET,
    AT_CMD_AT_PORT_GET,
    AT_CMD_AT_PORT_SET,
    AT_CMD_AT_LINK,
    AT_CMD_AT_TIME,
    AT_CMD_AT_TIME_GET,
    AT_CMD_AT_SIGNAL_GET,
    AT_CMD_AT_DATARATE_GET,
    AT_CMD_AT_DATARATE_SET,
    AT_CMD_AT_SEND_SET,
    AT_CMD_AT_SENDCK_SET,
    AT_CMD_AT_JOIN_GET,
    AT_CMD_AT_JOIN_SET,
    AT_CMD_AT_SLEEP_SET,
    AT_CMD_AT_HEART,
    AT_CMD_AT_HEART_GET,
    AT_CMD_AT_HEART_SET,
    AT_CMD_AT_CHANNEL_GET,
    AT_CMD_AT_CHANNEL_SET,
    AT_CMD_AT_DEVEUI_GET,
    AT_CMD_AT_APPEUI_GET,
    AT_CMD_AT_APPEUI_SET,
    AT_CMD_AT_APPKEY_GET,
    AT_CMD_AT_APPKEY_SET,
    AT_CMD_AT_POWER_GET,
    AT_CMD_AT_POWER_SET,
    AT_CMD_AT_AUTO_GET,
    AT_CMD_AT_AUTO_SET,
#endif

    AT_CMD_MAX = 0xff   
};

struct at_ap_command {
    enum at_cmd_e id;
    const char *pre_cmd;
    const char *help;
    int (*function)(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, unsigned short OutLength);
};

uint32_t at_cmd_request(enum at_cmd_e request_id, char *pInBuffer, char *pOutBuffer, uint16_t OutLength);

#endif  /* Avoid multiple inclusion */

