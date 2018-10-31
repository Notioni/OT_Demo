#include <stdio.h>
#include "stm32l4xx_hal.h"
#include "atdemo.h"
#include <k_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern uart_dev_t brd_uart3_dev;
//extern uart_dev_t uart_0;

//#define os_uart uart_0
#define wifi_uart brd_uart3_dev

#if defined(NB_MOUDLE) || defined(LORA_MODULE)
extern uart_dev_t brd_uart2_dev;
// extern uart_dev_t brd_uart4_dev;
#define lora_uart brd_uart2_dev

#define LORA_SLEEP_TIME    10
sys_time_t lora_cmd_timer = 0;  // record the time of executing last commond

#define LORA_DEFAULT_RECEIVE_COUNT  10

// extern uart_dev_t brd_uart2_dev;
#define nb_uart brd_uart2_dev

enum at_moudle_type_e {
  AT_MOUDLE_WIFI = 0,
  AT_MOUDLE_LORA,
  AT_MOUDLE_NB
};

static uint8_t at_moudle_type = AT_MOUDLE_WIFI;

#define UART_RECEIVE_DISABLE   0
#define UART_RECEIVE_ENABLE    1
static uint8_t uart_receive_flag = UART_RECEIVE_ENABLE;
#endif

#define MAX_CMD_LEN 512
#define MAX_OUT_LEN 2000
static aos_mutex_t at_mutex;

static int atcmd_init_mutex()
{
    if (0 != aos_mutex_new(&at_mutex)) {
        LOGE("atdemo", "Creating mutex failed");
        return HAL_ERROR;
    }

    return HAL_OK;
}

static int at_test(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char Recv_ch;
	uint8_t time_out = 0;

	memset(pOutBuffer, 0, OutLength);
	hal_uart_send(&wifi_uart, (void *)"AT+TEST", strlen("AT+TEST"), 30000);
	ret_val = hal_uart_send(&wifi_uart, (void *)"\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 3000);
			if(ret_val != HAL_OK){
				time_out++;
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		time_out = 0;
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 4){
				if(strstr(pOutBuffer, "\r\nERROR\r\n") || strstr(pOutBuffer, "\r\nOK\r\n"))
					break;
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int handle_at(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char Recv_ch;
	uint8_t time_out = 0;

	memset(pOutBuffer, 0, OutLength);
	hal_uart_send(&wifi_uart, (void *)"AT", strlen("AT"), 3000);
	ret_val = hal_uart_send(&wifi_uart, (void *)"\r", 1, 3000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 1000);
			if(ret_val != HAL_OK){
				time_out++;
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/100);
			}
		}while(recv_size != 1);
		time_out = 0;
		if(Recv_ch == '\0'){
			continue;
		}
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 4){
			if(strstr(pOutBuffer, "ERROR\r\n") || strstr(pOutBuffer, "OK\r\n"))
				break;
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_get_at_verion_old(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char Recv_ch;
	uint8_t time_out = 0, respone_start = 0;

	memset(pOutBuffer, 0, OutLength);
	hal_uart_send(&wifi_uart, (void *)"AT+GETATVERSION", strlen("AT+GETATVERSION"), 30000);
	ret_val = hal_uart_send(&wifi_uart, (void *)"\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 3000);
			if(ret_val != HAL_OK){
				time_out++;
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		time_out = 0;
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 9){
			if(strstr(pOutBuffer, "\r\nERROR\r\n"))
				break;
			else{
				if(strstr(pOutBuffer, "AT_VERSION")   && (respone_start == 0) ){
					respone_start = 1;
					recv_size_t = 0;
					memset(pOutBuffer, 0, OutLength);
					strcat(pOutBuffer, "AT_VERSION");
					recv_size_t = strlen("AT_VERSION");
				}
				else if(respone_start){
					if(strstr(pOutBuffer, "\r\n"))
						break;
				}
			}
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_get_at_verion(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char Recv_ch;
	uint8_t time_out = 0, respone_start = 0;

	memset(pOutBuffer, 0, OutLength);
	hal_uart_send(&wifi_uart, (void *)"AT+GETATVERSION?", strlen("AT+GETATVERSION?"), 30000);
	ret_val = hal_uart_send(&wifi_uart, (void *)"\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 3000);
			if(ret_val != HAL_OK){
				time_out++;
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		time_out = 0;
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 9){
			if(strstr(pOutBuffer, "\r\nERROR\r\n"))
				break;
			else{
				if(strstr(pOutBuffer, "AT_VERSION")   && (respone_start == 0) ){
					respone_start = 1;
					recv_size_t = 0;
					memset(pOutBuffer, 0, OutLength);
					strcat(pOutBuffer, "AT_VERSION");
					recv_size_t = strlen("AT_VERSION");
				}
				else if(respone_start){
					if(strstr(pOutBuffer, "\r\nOK\r\n"))
						break;
				}
			}
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_version(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char Recv_ch;
	uint8_t time_out = 0, respone_start = 0;

	memset(pOutBuffer, 0, OutLength);
	hal_uart_send(&wifi_uart, (void *)"AT+FWVER?", strlen("AT+FWVER?"), 30000);
	ret_val = hal_uart_send(&wifi_uart, (void *)"\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 3000);
			if(ret_val != HAL_OK){
				time_out++;
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		time_out = 0;
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 6){
			if(strstr(pOutBuffer, "\r\nERROR\r\n"))
				break;
			else{
				if(strstr(pOutBuffer, "+FWVER")   && (respone_start == 0) ){
					respone_start = 1;
					memset(pOutBuffer, 0, OutLength);
					strcat(pOutBuffer, "+FWVER");
					recv_size_t = strlen("+FWVER");
				}
				else if(respone_start){
					if(strstr(pOutBuffer, "\r\nOK\r\n"))
						break;
				}
			}
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_system_run_time_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char Recv_ch;
	uint8_t time_out = 0, respone_start = 0;

	memset(pOutBuffer, 0, OutLength);
	hal_uart_send(&wifi_uart, (void *)"AT+SYSTIME?", strlen("AT+SYSTIME?"), 30000);
	ret_val = hal_uart_send(&wifi_uart, (void *)"\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 3000);
			if(ret_val != HAL_OK){
				time_out++;
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		time_out = 0;
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 8){
			if(strstr(pOutBuffer, "\r\nERROR\r\n"))
				break;
			else{
				if(strstr(pOutBuffer, "+SYSTIME")   && (respone_start == 0) ){
					respone_start = 1;
					memset(pOutBuffer, 0, OutLength);
					strcat(pOutBuffer, "+SYSTIME");
					recv_size_t = strlen("+SYSTIME");
				}
				else if(respone_start){
					if(strstr(pOutBuffer, "\r\nOK\r\n"))
						break;
				}
			}
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_system_memory_free_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char Recv_ch;
	uint8_t time_out = 0, respone_start = 0;

	memset(pOutBuffer, 0, OutLength);
	hal_uart_send(&wifi_uart, (void *)"AT+MEMFREE?", strlen("AT+MEMFREE?"), 30000);
	ret_val = hal_uart_send(&wifi_uart, (void *)"\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 3000);
			if(ret_val != HAL_OK){
				time_out++;
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		time_out = 0;
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 8){
			if(strstr(pOutBuffer, "\r\nERROR\r\n"))
				break;
			else{
				if(strstr(pOutBuffer, "+MEMFREE?")   && (respone_start == 0) ){
					respone_start = 1;
					memset(pOutBuffer, 0, OutLength);
					strcat(pOutBuffer, "+MEMFREE?");
					recv_size_t = strlen("+MEMFREE?");
				}
				else if(respone_start){
					if(strstr(pOutBuffer, "\r\nOK\r\n"))
						break;
				}
			}
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_reboot_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char Recv_ch;
	uint8_t time_out = 0, flag_zore = 0;

	memset(pOutBuffer, 0, OutLength);
	hal_uart_send(&wifi_uart, (void *)"AT+REBOOT", strlen("AT+REBOOT"), 30000);
	ret_val = hal_uart_send(&wifi_uart, (void *)"\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 3000);
			if(ret_val != HAL_OK){
				time_out++;
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		time_out = 0;
		if(Recv_ch == '\0'){
			if(flag_zore == 0){
				flag_zore = 1;
				continue;
			}
			else
				break;
		}
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 6){
			if(strstr(pOutBuffer, "\r\nOK\r\n")){
				if(flag_zore == 0)
				break;
			}
			else if(strstr(pOutBuffer, "\r\nERROR\r\n")){
				if(flag_zore == 0)
					break;
			}
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_recover_factory_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char Recv_ch;
	uint8_t time_out = 0, flag_zore = 0;

	memset(pOutBuffer, 0, OutLength);
	hal_uart_send(&wifi_uart, (void *)"AT+FACTORY", strlen("AT+FACTORY"), 30000);
	ret_val = hal_uart_send(&wifi_uart, (void *)"\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 3000);
			if(ret_val != HAL_OK){
				time_out++;
				if(time_out >= 30){
					printf("AT+FACTORY timeout\n");
					return HAL_ERROR;
				}
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		time_out = 0;
		if(Recv_ch == '\0'){
			if(flag_zore == 0){
				flag_zore = 1;
				continue;
			}
			else
				break;
		}
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 6){
			if(strstr(pOutBuffer, "\r\nOK\r\n")){
				if(flag_zore == 0)
					break;
			}
			else if(strstr(pOutBuffer, "\r\nERROR\r\n")){
				if(flag_zore == 0)
					break;
			}
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_refer_flash_lock_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char Recv_ch;
	uint8_t time_out = 0, respone_start = 0;

	memset(pOutBuffer, 0, OutLength);
	hal_uart_send(&wifi_uart, (void *)"AT+FLASHLOCK?", strlen("AT+FLASHLOCK?"), 30000);
	ret_val = hal_uart_send(&wifi_uart, (void *)"\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 3000);
			if(ret_val != HAL_OK){
				time_out++;
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		time_out = 0;
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 8){
			if(strstr(pOutBuffer, "\r\nERROR\r\n"))
				break;
			else{
				if(strstr(pOutBuffer, "+FLASHLOCK")   && (respone_start == 0) ){
					respone_start = 1;
					memset(pOutBuffer, 0, OutLength);
					strcat(pOutBuffer, "+FLASHLOCK");
					recv_size_t = strlen("+FLASHLOCK");
				}
				else if(respone_start){
					if(strstr(pOutBuffer, "\r\nOK\r\n"))
						break;
				}
			}
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_set_flash_lock_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char send_cmd[128] = "AT+FLASHLOCK=";
	char Recv_ch;
	uint8_t time_out = 0;

	if(OutLength < 200)
		return HAL_ERROR;

	strcat(send_cmd, PInBuffer);
	memset(pOutBuffer, 0, OutLength);
	ret_val = hal_uart_send(&wifi_uart, send_cmd, strlen(send_cmd), 30000);
	ret_val = hal_uart_send(&wifi_uart, "\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 30000);
			if(ret_val != HAL_OK){
				time_out++;
				//printf("xiehj : time_out = %d\n", time_out);
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 6){
			if(strstr(pOutBuffer, "\r\nERROR\r\n") || strstr(pOutBuffer, "\r\nOK\r\n"))
					break;
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_refer_wifi_event_notification_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char Recv_ch;
	uint8_t time_out = 0, respone_start = 0;

	memset(pOutBuffer, 0, OutLength);
	hal_uart_send(&wifi_uart, (void *)"AT+WEVENT?", strlen("AT+WEVENT?"), 30000);
	ret_val = hal_uart_send(&wifi_uart, (void *)"\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 3000);
			if(ret_val != HAL_OK){
				time_out++;
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		time_out = 0;
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 8){
			if(strstr(pOutBuffer, "\r\nERROR\r\n"))
				break;
			else{
				if(strstr(pOutBuffer, "+WEVENT")   && (respone_start == 0) ){
					respone_start = 1;
					memset(pOutBuffer, 0, OutLength);
					strcat(pOutBuffer, "+WEVENT");
					recv_size_t = strlen("+WEVENT");
				}
				else if(respone_start){
					if(strstr(pOutBuffer, "\r\nOK\r\n"))
						break;
				}
			}
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_set_wifi_event_notification_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char send_cmd[128] = "AT+WEVENT=";
	char Recv_ch;
	uint8_t time_out = 0;

	if(OutLength < 200)
		return HAL_ERROR;

	strcat(send_cmd, PInBuffer);
	memset(pOutBuffer, 0, OutLength);
	ret_val = hal_uart_send(&wifi_uart, send_cmd, strlen(send_cmd), 30000);
	ret_val = hal_uart_send(&wifi_uart, "\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 30000);
			if(ret_val != HAL_OK){
				time_out++;
				//printf("xiehj : time_out = %d\n", time_out);
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 6){
			if(strstr(pOutBuffer, "\r\nERROR\r\n") || strstr(pOutBuffer, "\r\nOK\r\n"))
					break;
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_refer_wifi_power_save_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char Recv_ch;
	uint8_t time_out = 0, respone_start = 0;

	memset(pOutBuffer, 0, OutLength);
	hal_uart_send(&wifi_uart, (void *)"AT+WLPC?", strlen("AT+WLPC?"), 30000);
	ret_val = hal_uart_send(&wifi_uart, (void *)"\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 3000);
			if(ret_val != HAL_OK){
				time_out++;
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		time_out = 0;
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 5){
			if(strstr(pOutBuffer, "\r\nERROR\r\n"))
				break;
			else{
				if(strstr(pOutBuffer, "+WLPC")   && (respone_start == 0) ){
					respone_start = 1;
					memset(pOutBuffer, 0, OutLength);
					strcat(pOutBuffer, "+WLPC");
					recv_size_t = strlen("+WLPC");
				}
				else if(respone_start){
					if(strstr(pOutBuffer, "\r\nOK\r\n"))
						break;
				}
			}
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_set_wifi_power_save_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char send_cmd[128] = "AT+WLPC=";
	char Recv_ch;
	uint8_t time_out = 0;

	if(OutLength < 200)
		return HAL_ERROR;

	strcat(send_cmd, PInBuffer);
	memset(pOutBuffer, 0, OutLength);
	ret_val = hal_uart_send(&wifi_uart, send_cmd, strlen(send_cmd), 30000);
	ret_val = hal_uart_send(&wifi_uart, "\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 30000);
			if(ret_val != HAL_OK){
				time_out++;
				//printf("xiehj : time_out = %d\n", time_out);
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 6){
			if(strstr(pOutBuffer, "\r\nERROR\r\n") || strstr(pOutBuffer, "\r\nOK\r\n"))
					break;
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_refer_uart_info_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char Recv_ch;
	uint8_t time_out = 0, respone_start = 0;

	memset(pOutBuffer, 0, OutLength);
	hal_uart_send(&wifi_uart, (void *)"AT+UART?", strlen("AT+UART?"), 30000);
	ret_val = hal_uart_send(&wifi_uart, (void *)"\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 3000);
			if(ret_val != HAL_OK){
				time_out++;
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		time_out = 0;
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 5){
			if(strstr(pOutBuffer, "\r\nERROR\r\n"))
				break;
			else{
				if(strstr(pOutBuffer, "+UART")   && (respone_start == 0) ){
					respone_start = 1;
					memset(pOutBuffer, 0, OutLength);
					strcat(pOutBuffer, "+UART");
					recv_size_t = strlen("+UART");
				}
				else if(respone_start){
					if(strstr(pOutBuffer, "\r\nOK\r\n"))
						break;
				}
			}
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_set_uart_info_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char send_cmd[128] = "AT+UART=";
	char Recv_ch;
	uint8_t time_out = 0;

	if(OutLength < 200)
		return HAL_ERROR;

	strcat(send_cmd, PInBuffer);
	memset(pOutBuffer, 0, OutLength);
	ret_val = hal_uart_send(&wifi_uart, send_cmd, strlen(send_cmd), 30000);
	ret_val = hal_uart_send(&wifi_uart, "\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 30000);
			if(ret_val != HAL_OK){
				time_out++;
				//printf("xiehj : time_out = %d\n", time_out);
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 6){
			if(strstr(pOutBuffer, "\r\nERROR\r\n") || strstr(pOutBuffer, "\r\nOK\r\n"))
					break;
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_refer_uart_fomat_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char Recv_ch;
	uint8_t time_out = 0, respone_start = 0;

	memset(pOutBuffer, 0, OutLength);
	hal_uart_send(&wifi_uart, (void *)"AT+UARTFOMAT?", strlen("AT+UARTFOMAT?"), 30000);
	ret_val = hal_uart_send(&wifi_uart, (void *)"\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 3000);
			if(ret_val != HAL_OK){
				time_out++;
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		time_out = 0;
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 8){
			if(strstr(pOutBuffer, "\r\nERROR\r\n"))
				break;
			else{
				if(strstr(pOutBuffer, "+UARTFOMAT")   && (respone_start == 0) ){
					respone_start = 1;
					memset(pOutBuffer, 0, OutLength);
					strcat(pOutBuffer, "+UARTFOMAT");
					recv_size_t = strlen("+UARTFOMAT");
				}
				else if(respone_start){
					if(strstr(pOutBuffer, "\r\nOK\r\n"))
						break;
				}
			}
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_set_uart_fomat_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char send_cmd[128] = "AT+UARTFOMAT=";
	char Recv_ch;
	uint8_t time_out = 0;

	if(OutLength < 200)
		return HAL_ERROR;

	strcat(send_cmd, PInBuffer);
	memset(pOutBuffer, 0, OutLength);
	ret_val = hal_uart_send(&wifi_uart, send_cmd, strlen(send_cmd), 30000);
	ret_val = hal_uart_send(&wifi_uart, "\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 30000);
			if(ret_val != HAL_OK){
				time_out++;
				//printf("xiehj : time_out = %d\n", time_out);
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 6){
			if(strstr(pOutBuffer, "\r\nERROR\r\n") || strstr(pOutBuffer, "\r\nOK\r\n"))
					break;
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_refer_uart_echo_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char Recv_ch;
	uint8_t time_out = 0, respone_start = 0;

	memset(pOutBuffer, 0, OutLength);
	hal_uart_send(&wifi_uart, (void *)"AT+UARTE?", strlen("AT+UARTE?"), 30000);
	ret_val = hal_uart_send(&wifi_uart, (void *)"\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 3000);
			if(ret_val != HAL_OK){
				time_out++;
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		time_out = 0;
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 6){
			if(strstr(pOutBuffer, "\r\nERROR\r\n"))
				break;
			else{
				if(strstr(pOutBuffer, "+UARTE")   && (respone_start == 0) ){
					respone_start = 1;
					memset(pOutBuffer, 0, OutLength);
					strcat(pOutBuffer, "+UARTE");
					recv_size_t = strlen("+UARTE");
				}
				else if(respone_start){
					if(strstr(pOutBuffer, "\r\nOK\r\n"))
						break;
				}
			}
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_set_uart_echo_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char send_cmd[128] = "AT+UARTE=";
	char Recv_ch;
	uint8_t time_out = 0;

	if(OutLength < 200)
		return HAL_ERROR;

	strcat(send_cmd, PInBuffer);
	memset(pOutBuffer, 0, OutLength);
	ret_val = hal_uart_send(&wifi_uart, send_cmd, strlen(send_cmd), 30000);
	ret_val = hal_uart_send(&wifi_uart, "\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 30000);
			if(ret_val != HAL_OK){
				time_out++;
				//printf("xiehj : time_out = %d\n", time_out);
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 6){
			if(strstr(pOutBuffer, "\r\nERROR\r\n") || strstr(pOutBuffer, "\r\nOK\r\n"))
					break;
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_wifi_firmware_version_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char Recv_ch;
	uint8_t time_out = 0, respone_start = 0;

	memset(pOutBuffer, 0, OutLength);
	hal_uart_send(&wifi_uart, (void *)"AT+WFVER", strlen("AT+WFVER"), 30000);
	ret_val = hal_uart_send(&wifi_uart, (void *)"\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 3000);
			if(ret_val != HAL_OK){
				time_out++;
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		time_out = 0;
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 6){
			if(strstr(pOutBuffer, "\r\nERROR\r\n"))
				break;
			else{
				if(strstr(pOutBuffer, "+WFVER")   && (respone_start == 0) ){
					respone_start = 1;
					memset(pOutBuffer, 0, OutLength);
					strcat(pOutBuffer, "+WFVER");
					recv_size_t = strlen("+WFVER");
				}
				else if(respone_start){
					if(strstr(pOutBuffer, "\r\nOK\r\n"))
						break;
				}
			}
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_wl_mac_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char Recv_ch;
	uint8_t time_out = 0, respone_start = 0;

	memset(pOutBuffer, 0, OutLength);
	hal_uart_send(&wifi_uart, (void *)"AT+WMAC?", strlen("AT+WMAC?"), 30000);
	ret_val = hal_uart_send(&wifi_uart, (void *)"\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 3000);
			if(ret_val != HAL_OK){
				time_out++;
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		time_out = 0;
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 5){
			if(strstr(pOutBuffer, "\r\nERROR\r\n"))
				break;
			else{
				if(strstr(pOutBuffer, "+WMAC")   && (respone_start == 0) ){
					respone_start = 1;
					memset(pOutBuffer, 0, OutLength);
					strcat(pOutBuffer, "+WMAC");
					recv_size_t = strlen("+WMAC");
				}
				else if(respone_start){
					if(strstr(pOutBuffer, "\r\nOK\r\n"))
						break;
				}
			}
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}


static int at_refer_wifi_scan_option_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char Recv_ch;
	uint8_t time_out = 0, respone_start = 0;

	memset(pOutBuffer, 0, OutLength);
	hal_uart_send(&wifi_uart, (void *)"AT+WSCANOPT?", strlen("AT+WSCANOPT?"), 30000);
	ret_val = hal_uart_send(&wifi_uart, (void *)"\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 3000);
			if(ret_val != HAL_OK){
				time_out++;
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		time_out = 0;
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 9){
			if(strstr(pOutBuffer, "\r\nERROR\r\n"))
				break;
			else{
				if(strstr(pOutBuffer, "+WSCANOPT")   && (respone_start == 0) ){
					respone_start = 1;
					memset(pOutBuffer, 0, OutLength);
					strcat(pOutBuffer, "+WSCANOPT");
					recv_size_t = strlen("+WSCANOPT");
				}
				else if(respone_start){
					if(strstr(pOutBuffer, "\r\nOK\r\n"))
						break;
				}
			}
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_set_wifi_scan_option_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char send_cmd[128] = "AT+WSCANOPT=";
	char Recv_ch;
	uint8_t time_out = 0;

	if(OutLength < 200)
		return HAL_ERROR;

	strcat(send_cmd, PInBuffer);
	memset(pOutBuffer, 0, OutLength);
	ret_val = hal_uart_send(&wifi_uart, send_cmd, strlen(send_cmd), 30000);
	ret_val = hal_uart_send(&wifi_uart, "\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 30000);
			if(ret_val != HAL_OK){
				time_out++;
				//printf("xiehj : time_out = %d\n", time_out);
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 6){
			if(strstr(pOutBuffer, "\r\nERROR\r\n") || strstr(pOutBuffer, "\r\nOK\r\n"))
					break;
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_wl_scan_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char Recv_ch;
	uint8_t time_out = 0, respone_start = 0;

	memset(pOutBuffer, 0, OutLength);
	hal_uart_send(&wifi_uart, (void *)"AT+WSCAN", strlen("AT+WSCAN"), 30000);
	ret_val = hal_uart_send(&wifi_uart, (void *)"\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 3000);
			if(ret_val != HAL_OK){
				time_out++;
				if(time_out >= 20)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND);
			}
		}while(recv_size != 1);
		time_out = 0;
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 5){
			if(strstr(pOutBuffer, "\r\nERROR\r\n"))
				break;
			else{
				if(strstr(pOutBuffer, "+SCAN")   && (respone_start == 0) ){
					respone_start = 1;
					memset(pOutBuffer, 0, OutLength);
					strcat(pOutBuffer, "+SCAN");
					recv_size_t = strlen("+SCAN");
				}
				else if(respone_start){
					if(strstr(pOutBuffer, "\r\nOK\r\n"))
						break;
				}
			}
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_refer_wifi_dhcp_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char Recv_ch;
	uint8_t time_out = 0, respone_start = 0;

	memset(pOutBuffer, 0, OutLength);
	hal_uart_send(&wifi_uart, (void *)"AT+WDHCP?", strlen("AT+WDHCP?"), 30000);
	ret_val = hal_uart_send(&wifi_uart, (void *)"\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 3000);
			if(ret_val != HAL_OK){
				time_out++;
				if(time_out >= 20)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		time_out = 0;
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 5){
			if(strstr(pOutBuffer, "\r\nERROR\r\n"))
				break;
			else{
				if(strstr(pOutBuffer, "+WDHCP")   && (respone_start == 0) ){
					respone_start = 1;
					memset(pOutBuffer, 0, OutLength);
					strcat(pOutBuffer, "+WDHCP");
					recv_size_t = strlen("+WDHCP");
				}
				else if(respone_start){
					if(strstr(pOutBuffer, "\r\nOK\r\n"))
						break;
				}
			}
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_set_wifi_dhcp_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char send_cmd[128] = "AT+WDHCP=";
	char Recv_ch;
	uint8_t time_out = 0;

	if(OutLength < 200)
		return HAL_ERROR;

	strcat(send_cmd, PInBuffer);
	memset(pOutBuffer, 0, OutLength);
	ret_val = hal_uart_send(&wifi_uart, send_cmd, strlen(send_cmd), 30000);
	ret_val = hal_uart_send(&wifi_uart, "\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 30000);
			if(ret_val != HAL_OK){
				time_out++;
				//printf("xiehj : time_out = %d\n", time_out);
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 6){
			if(strstr(pOutBuffer, "\r\nERROR\r\n") || strstr(pOutBuffer, "\r\nOK\r\n"))
					break;
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_refer_wifi_ap_ip_mask_gate_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char Recv_ch;
	uint8_t time_out = 0, respone_start = 0;

	memset(pOutBuffer, 0, OutLength);
	hal_uart_send(&wifi_uart, (void *)"AT+WSAPIP?", strlen("AT+WSAPIP?"), 30000);
	ret_val = hal_uart_send(&wifi_uart, (void *)"\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 3000);
			if(ret_val != HAL_OK){
				time_out++;
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		time_out = 0;
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 6){
			if(strstr(pOutBuffer, "\r\nERROR\r\n"))
				break;
			else{
				if(strstr(pOutBuffer, "+WSAPIP")   && (respone_start == 0) ){
					respone_start = 1;
					memset(pOutBuffer, 0, OutLength);
					strcat(pOutBuffer, "+WSAPIP");
					recv_size_t = strlen("+WSAPIP");
				}
				else if(respone_start){
					if(strstr(pOutBuffer, "\r\nOK\r\n"))
						break;
				}
			}
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_set_wifi_ap_ip_mask_gate_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char send_cmd[128] = "AT+WSAPIP=";
	char Recv_ch;
	uint8_t time_out = 0;

	if(OutLength < 200)
		return HAL_ERROR;

	strcat(send_cmd, PInBuffer);
	memset(pOutBuffer, 0, OutLength);
	ret_val = hal_uart_send(&wifi_uart, send_cmd, strlen(send_cmd), 30000);
	ret_val = hal_uart_send(&wifi_uart, "\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 30000);
			if(ret_val != HAL_OK){
				time_out++;
				//printf("xiehj : time_out = %d\n", time_out);
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 6){
			if(strstr(pOutBuffer, "\r\nERROR\r\n") || strstr(pOutBuffer, "\r\nOK\r\n"))
					break;
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_refer_wifi_ap_info_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char Recv_ch;
	uint8_t time_out = 0, respone_start = 0;

	memset(pOutBuffer, 0, OutLength);
	hal_uart_send(&wifi_uart, (void *)"AT+WSAP?", strlen("AT+WSAP?"), 30000);
	ret_val = hal_uart_send(&wifi_uart, (void *)"\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 3000);
			if(ret_val != HAL_OK){
				time_out++;
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		time_out = 0;
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 5){
			if(strstr(pOutBuffer, "\r\nERROR\r\n"))
				break;
			else{
				if(strstr(pOutBuffer, "+WSAP")   && (respone_start == 0) ){
					respone_start = 1;
					memset(pOutBuffer, 0, OutLength);
					strcat(pOutBuffer, "+WSAP");
					recv_size_t = strlen("+WSAP");
				}
				else if(respone_start){
					if(strstr(pOutBuffer, "\r\nOK\r\n"))
						break;
				}
			}
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_set_wifi_ap_info_start_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char send_cmd[128] = "AT+WSAP=";
	char Recv_ch;
	uint8_t time_out = 0;

	if(OutLength < 200)
		return HAL_ERROR;

	strcat(send_cmd, PInBuffer);
	memset(pOutBuffer, 0, OutLength);
	ret_val = hal_uart_send(&wifi_uart, send_cmd, strlen(send_cmd), 30000);
	ret_val = hal_uart_send(&wifi_uart, "\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 30000);
			if(ret_val != HAL_OK){
				time_out++;
				//printf("xiehj : time_out = %d\n", time_out);
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 6){
			if(strstr(pOutBuffer, "\r\nERROR\r\n") || strstr(pOutBuffer, "\r\nOK\r\n"))
					break;
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_wifi_ap_quit_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char send_cmd[128] = "AT+WSAPQ";
	char Recv_ch;
	uint8_t time_out = 0;

	if(OutLength < 200)
		return HAL_ERROR;

	strcat(send_cmd, PInBuffer);
	memset(pOutBuffer, 0, OutLength);
	ret_val = hal_uart_send(&wifi_uart, send_cmd, strlen(send_cmd), 30000);
	ret_val = hal_uart_send(&wifi_uart, "\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 30000);
			if(ret_val != HAL_OK){
				time_out++;
				//printf("xiehj : time_out = %d\n", time_out);
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 6){
			if(strstr(pOutBuffer, "\r\nERROR\r\n") || strstr(pOutBuffer, "\r\nOK\r\n"))
					break;
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_get_ap_current_status_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char Recv_ch;
	uint8_t time_out = 0, respone_start = 0;

	memset(pOutBuffer, 0, OutLength);
	hal_uart_send(&wifi_uart, (void *)"AT+WSAPS", strlen("AT+WSAPS"), 30000);
	ret_val = hal_uart_send(&wifi_uart, (void *)"\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 3000);
			if(ret_val != HAL_OK){
				time_out++;
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		time_out = 0;
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 5){
			if(strstr(pOutBuffer, "\r\nERROR\r\n"))
				break;
			else{
				if(strstr(pOutBuffer, "state")   && (respone_start == 0) ){
					respone_start = 1;
					memset(pOutBuffer, 0, OutLength);
					strcat(pOutBuffer, "state");
					recv_size_t = strlen("state");
				}
				else if(respone_start){
					if(strstr(pOutBuffer, "\r\nOK\r\n"))
						break;
				}
			}
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_refer_wifi_sta_ip_mask_gate_dns_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char Recv_ch;
	uint8_t time_out = 0, respone_start = 0;

	memset(pOutBuffer, 0, OutLength);
	hal_uart_send(&wifi_uart, (void *)"AT+WJAPIP?", strlen("AT+WJAPIP?"), 30000);
	ret_val = hal_uart_send(&wifi_uart, (void *)"\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 3000);
			if(ret_val != HAL_OK){
				time_out++;
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		time_out = 0;
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 8){
			if(strstr(pOutBuffer, "\r\nERROR\r\n"))
				break;
			else{
				if(strstr(pOutBuffer, "+WJAPIP?")   && (respone_start == 0) ){
					respone_start = 1;
					memset(pOutBuffer, 0, OutLength);
					strcat(pOutBuffer, "+WJAPIP?");
					recv_size_t = strlen("+WJAPIP?");
				}
				else if(respone_start){
					if(strstr(pOutBuffer, "\r\nOK\r\n"))
						break;
				}
			}
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_set_wifi_sta_ip_mask_gate_dns_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char send_cmd[128] = "AT+WJAPIP=";
	char Recv_ch;
	uint8_t time_out = 0;

	if(OutLength < 200)
		return HAL_ERROR;

	strcat(send_cmd, PInBuffer);
	memset(pOutBuffer, 0, OutLength);
	ret_val = hal_uart_send(&wifi_uart, send_cmd, strlen(send_cmd), 30000);
	ret_val = hal_uart_send(&wifi_uart, "\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 30000);
			if(ret_val != HAL_OK){
				time_out++;
				//printf("xiehj : time_out = %d\n", time_out);
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 6){
			if(strstr(pOutBuffer, "\r\nERROR\r\n") || strstr(pOutBuffer, "\r\nOK\r\n"))
					break;
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_refer_wifi_sta_info_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char Recv_ch;
	uint8_t time_out = 0, respone_start = 0;

	memset(pOutBuffer, 0, OutLength);
	hal_uart_send(&wifi_uart, (void *)"AT+WJAP?", strlen("AT+WJAP?"), 30000);
	ret_val = hal_uart_send(&wifi_uart, (void *)"\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 3000);
			if(ret_val != HAL_OK){
				time_out++;
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		time_out = 0;
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 5){
			if(strstr(pOutBuffer, "\r\nERROR\r\n"))
				break;
			else{
				if(strstr(pOutBuffer, "+WJAP")   && (respone_start == 0) ){
					respone_start = 1;
					memset(pOutBuffer, 0, OutLength);
					strcat(pOutBuffer, "+WJAP");
					recv_size_t = strlen("+WJAP");
				}
				else if(respone_start){
					if(strstr(pOutBuffer, "\r\nOK\r\n"))
						break;
				}
			}
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_set_wifi_sta_info_start_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char send_cmd[128] = "AT+WJAP=";
	char Recv_ch;
	uint8_t time_out = 0;

	if(OutLength < 200)
		return HAL_ERROR;

	strcat(send_cmd, PInBuffer);
	memset(pOutBuffer, 0, OutLength);
	ret_val = hal_uart_send(&wifi_uart, send_cmd, strlen(send_cmd), 30000);
	ret_val = hal_uart_send(&wifi_uart, "\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 30000);
			if(ret_val != HAL_OK){
				time_out++;
				//printf("xiehj : time_out = %d\n", time_out);
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 6){
			if(strstr(pOutBuffer, "\r\nERROR\r\n") || strstr(pOutBuffer, "\r\nOK\r\n"))
					break;
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_wifi_sta_quit_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char send_cmd[128] = "AT+WJAPQ";
	char Recv_ch;
	uint8_t time_out = 0;

	if(OutLength < 200)
		return HAL_ERROR;

	strcat(send_cmd, PInBuffer);
	memset(pOutBuffer, 0, OutLength);
	ret_val = hal_uart_send(&wifi_uart, send_cmd, strlen(send_cmd), 30000);
	ret_val = hal_uart_send(&wifi_uart, "\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 30000);
			if(ret_val != HAL_OK){
				time_out++;
				//printf("xiehj : time_out = %d\n", time_out);
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND);
			}
		}while(recv_size != 1);
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 6){
			if(strstr(pOutBuffer, "\r\nERROR\r\n") || strstr(pOutBuffer, "\r\nOK\r\n"))
					break;
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_get_sta_current_status_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char Recv_ch;
	uint8_t time_out = 0, respone_start = 0;

	memset(pOutBuffer, 0, OutLength);
	hal_uart_send(&wifi_uart, (void *)"AT+WJAPS", strlen("AT+WJAPS"), 30000);
	ret_val = hal_uart_send(&wifi_uart, (void *)"\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 3000);
			if(ret_val != HAL_OK){
				time_out++;
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		time_out = 0;
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 5){
			if(strstr(pOutBuffer, "\r\nERROR\r\n"))
				break;
			else{
				if(strstr(pOutBuffer, "+WJAPS")   && (respone_start == 0) ){
					respone_start = 1;
					memset(pOutBuffer, 0, OutLength);
					strcat(pOutBuffer, "+WJAPS");
					recv_size_t = strlen("+WJAPS");
				}
				else if(respone_start){
					if(strstr(pOutBuffer, "\r\nOK\r\n"))
						break;
				}
			}
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_cip_send_raw_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char send_cmd[128] = "AT+CIPSENDRAW";
	char Recv_ch;
	uint8_t time_out = 0;

	if(OutLength < 200)
		return HAL_ERROR;

	strcat(send_cmd, PInBuffer);
	memset(pOutBuffer, 0, OutLength);
	ret_val = hal_uart_send(&wifi_uart, send_cmd, strlen(send_cmd), 30000);
	ret_val = hal_uart_send(&wifi_uart, "\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 30000);
			if(ret_val != HAL_OK){
				time_out++;
				//printf("xiehj : time_out = %d\n", time_out);
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 6){
			if(strstr(pOutBuffer, "\r\nERROR\r\n") || strstr(pOutBuffer, "\r\nOK\r\n"))
					break;
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_refer_cip_recv_cfg_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char Recv_ch;
	uint8_t time_out = 0, respone_start = 0;

	memset(pOutBuffer, 0, OutLength);
	hal_uart_send(&wifi_uart, (void *)"AT+CIPRECVCFG?", strlen("AT+CIPRECVCFG?"), 30000);
	ret_val = hal_uart_send(&wifi_uart, (void *)"\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 3000);
			if(ret_val != HAL_OK){
				time_out++;
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		time_out = 0;
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 9){
			if(strstr(pOutBuffer, "\r\nERROR\r\n"))
				break;
			else{
				if(strstr(pOutBuffer, "+CIPRECVCFG")   && (respone_start == 0) ){
					respone_start = 1;
					memset(pOutBuffer, 0, OutLength);
					strcat(pOutBuffer, "+CIPRECVCFG");
					recv_size_t = strlen("+CIPRECVCFG");
				}
				else if(respone_start){
					if(strstr(pOutBuffer, "\r\nOK\r\n"))
						break;
				}
			}
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_cip_recv_cfg_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char send_cmd[128] = "AT+CIPRECVCFG=";
	char Recv_ch;
	uint8_t time_out = 0;

	if(OutLength < 200)
		return HAL_ERROR;

	strcat(send_cmd, PInBuffer);
	memset(pOutBuffer, 0, OutLength);
	ret_val = hal_uart_send(&wifi_uart, send_cmd, strlen(send_cmd), 30000);
	ret_val = hal_uart_send(&wifi_uart, "\r", 1, 30000);
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 30000);
			if(ret_val != HAL_OK){
				time_out++;
				//printf("xiehj : time_out = %d\n", time_out);
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 6){
			if(strstr(pOutBuffer, "\r\nERROR\r\n") || strstr(pOutBuffer, "\r\nOK\r\n"))
					break;
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_fota_start_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char send_cmd[512] = {0};
	char Recv_ch, end_ch, ret_ch;
	uint8_t time_out = 0, respone_start = 0;

	if(OutLength < 200)
		return HAL_ERROR;

	strcat(send_cmd, "AT+FOTA=");
	strcat(send_cmd, PInBuffer);
	memset(pOutBuffer, 0, OutLength);
	ret_val = hal_uart_send(&wifi_uart, send_cmd, strlen(send_cmd), 30000);
	ret_val = hal_uart_send(&wifi_uart, "\r", 1, 30000);
	strcat(send_cmd, "\r\n");
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 30000);
			if(ret_val != HAL_OK){
				time_out++;
				if(time_out >= 180)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND);
			}
		}while(recv_size != 1);
		
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 9){
			if(strstr(pOutBuffer, "\r\nERROR\r\n"))
				break;
			else{
				if(strstr(pOutBuffer, "+FOTAEVENT")   && (respone_start == 0) ){
					respone_start = 1;
					memset(pOutBuffer, 0, OutLength);
					strcat(pOutBuffer, "+FOTAEVENT");
					recv_size_t = strlen("+FOTAEVENT");
				}
				else if(respone_start){
					if(strstr(pOutBuffer, "OK\r\n"))
						break;
				}
			}
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_fota_start(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char send_cmd[512] = {0};
	char Recv_ch, end_ch, ret_ch;
	uint8_t time_out = 0, respone_start = 0;

	if(OutLength < 200)
		return HAL_ERROR;

	strcat(send_cmd, "AT+FOTA=");
	strcat(send_cmd, PInBuffer);
	memset(pOutBuffer, 0, OutLength);
	ret_val = hal_uart_send(&wifi_uart, send_cmd, strlen(send_cmd), 30000);
	ret_val = hal_uart_send(&wifi_uart, "\r", 1, 30000);
	strcat(send_cmd, "\r\n");
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 30000);
			if(ret_val != HAL_OK){
				time_out++;
				if(time_out >= 180)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND);
			}
		}while(recv_size != 1);
		
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		if(recv_size_t >= 4){
			if(strstr(pOutBuffer, send_cmd)   && (respone_start == 0) ){
				respone_start = 1;
				recv_size_t = 0;
				memset(pOutBuffer, 0, OutLength);
			}
			else if(respone_start){
				if(strstr(pOutBuffer, "ERROR\r\n") || strstr(pOutBuffer, "OK\r\n"))
					break;
			}
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_set_common_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char send_cmd[128] = {0};
	char Recv_ch, end_ch, ret_ch;
	uint8_t time_out = 0, respone_start = 0;

	if(OutLength < 200)
		return HAL_ERROR;

	switch(id){
		case AT_CMD_AT_FLASHLOCK_SET:
			strcat(send_cmd, "AT+FLASHLOCK=");
			break;
		case AT_CMD_AT_WEVENT_SET:
			strcat(send_cmd, "AT+WEVENT=");
			break;
		case AT_CMD_AT_WLPC_SET:
			strcat(send_cmd, "AT+WLPC=");
			break;
		case AT_CMD_AT_UART_SET:
			strcat(send_cmd, "AT+UART=");
			break;
		case AT_CMD_AT_UARTFOMAT_SET:
			strcat(send_cmd, "AT+UARTFOMAT=");
			break;
		case AT_CMD_AT_UARTE_SET:
			strcat(send_cmd, "AT+UARTE=");
			break;
		case AT_CMD_AT_WSCANOPT_SET:
			strcat(send_cmd, "AT+WSCANOPT=");
			break;
		case AT_CMD_AT_WDHCP_SET:
			strcat(send_cmd, "AT+WDHCP=");
			break;
		case AT_CMD_AT_WSAPIP_SET:
			strcat(send_cmd, "AT+WSAPIP=");
			break;
		case AT_CMD_AT_WSAP_SET:
			strcat(send_cmd, "AT+WSAP=");
			break;
		case AT_CMD_AT_WJAPIP_SET:
			strcat(send_cmd, "AT+WJAPIP=");
			break;
		#if 0
		case AT_CMD_AT_SSLCERT_SET:
			strcat(send_cmd, "AT+SSLCERTSET=");
			break;
		case AT_CMD_AT_SSLCERT_GET:
			strcat(send_cmd, "AT+SSLCERTGET=");
			break;
		#endif
		case AT_CMD_AT_CIPRECVCFG_SET:
			strcat(send_cmd, "AT+CIPRECVCFG=");
			break;
		
	}

	strcat(send_cmd, PInBuffer);
	memset(pOutBuffer, 0, OutLength);
	ret_val = hal_uart_send(&wifi_uart, send_cmd, strlen(send_cmd), 30000);
	ret_val = hal_uart_send(&wifi_uart, "\r", 1, 30000);
	strcat(send_cmd, "\r\n");
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, (void *)&Recv_ch, 1, &recv_size, 30000);
			if(ret_val != HAL_OK){
				time_out++;
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		printf("Recv_ch = %c\n", Recv_ch);
		printf("recv_size_t = %d\n", recv_size_t);
		printf("pOutBuffer = %s\n", pOutBuffer);
		if(recv_size_t >= 4){
			if(strstr(pOutBuffer, send_cmd)   && (respone_start == 0) ){
				respone_start = 1;
				recv_size_t = 0;
				memset(pOutBuffer, 0, OutLength);
			}
			else if(respone_start){
				if(strstr(pOutBuffer, "ERROR\r\n") || strstr(pOutBuffer, "OK\r\n"))
					break;
			}
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

static int at_get_common_func(enum at_cmd_e id, char *PInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	int32_t ret_val = HAL_OK;
	uint32_t recv_size = 0, recv_size_t = 0;
	char send_cmd[50] = {0};
	char Recv_ch;
	uint8_t time_out = 0, respone_start = 0;

	switch(id){
		case AT_CMD_AT_TEST:
			strcat(send_cmd, "AT+TEST");
			break;
		case AT_CMD_AT:
			strcat(send_cmd, "AT");
			break;
		case AT_CMD_AT_FWVER:
			strcat(send_cmd, "AT+FWVER?");
			break;
		case AT_CMD_AT_SYSTIME:
			strcat(send_cmd, "AT+SYSTIME?");
			break;
		case AT_CMD_AT_MEMFREE:
			strcat(send_cmd, "AT+MEMFREE?");
			break;
		case AT_CMD_AT_REBOOT:
			strcat(send_cmd, "AT+REBOOT");
			break;
		case AT_CMD_AT_FACTORY:
			strcat(send_cmd, "AT+FACTORY");
			break;
		case AT_CMD_AT_FLASHLOCK_GET:
			strcat(send_cmd, "AT+FLASHLOCK?");
			break;
		case AT_CMD_AT_WEVENT_GET:
			strcat(send_cmd, "AT+WEVENT?");
			break;
		case AT_CMD_AT_WLPC_GET:
			strcat(send_cmd, "AT+WLPC?");
			break;
		case AT_CMD_AT_UART_GET:
			strcat(send_cmd, "AT+UART?");
			break;
		case AT_CMD_AT_UARTFOMAT_GET:
			strcat(send_cmd, "AT+UARTFOMAT?");
			break;
		case AT_CMD_AT_UARTE_GET:
			strcat(send_cmd, "AT+UARTE?");
			break;
		case AT_CMD_AT_WFVER:
			strcat(send_cmd, "AT+WFVER");
			break;
		case AT_CMD_AT_WMAC:
			strcat(send_cmd, "AT+WMAC?");
			break;
		case AT_CMD_AT_WSCANOPT_GET:
			strcat(send_cmd, "AT+WSCANOPT?");
			break;
		case AT_CMD_AT_WDHCP_GET:
			strcat(send_cmd, "AT+WDHCP?");
			break;
		case AT_CMD_AT_WSAPIP_GET:
			strcat(send_cmd, "AT+WSAPIP?");
			break;
		case AT_CMD_AT_WSAP_GET:
			strcat(send_cmd, "AT+WSAP?");
			break;
		case AT_CMD_AT_WSAPQ:
			strcat(send_cmd, "AT+WSAPQ");
			break;
		case AT_CMD_AT_WSAPS:
			strcat(send_cmd, "AT+WSAPS");
			break;
			
		case AT_CMD_AT_WJAPIP_GET:
			strcat(send_cmd, "AT+WJAPIP?");
			break;
		case AT_CMD_AT_WJAP_GET:
			strcat(send_cmd, "AT+WJAP?");
			break;
		case AT_CMD_AT_WJAPQ:
			strcat(send_cmd, "AT+WJAPQ");
			break;
		case AT_CMD_AT_WJAPS:
			strcat(send_cmd, "AT+WJAPS");
			break;
		#if 0
		case AT_CMD_AT_SSLCERT_GET:
			strcat(send_cmd, "AT+SSLCERTGET");
			break;
		case AT_CMD_AT_CIPDOMAIN:
			strcat(send_cmd, "AT+SSLCERTSET");
			break;
		case AT_CMD_AT_CIPAUTOCONN:
			strcat(send_cmd, "AT+CIPAUTOCONN");
			break;
		case AT_CMD_AT_CIPSSLOPT:
			strcat(send_cmd, "AT+CIPSSLOPT");
			break;
		case AT_CMD_AT_CIPSTART:
			strcat(send_cmd, "AT+CIPSTART");
			break;
		#endif
		case AT_CMD_AT_CIPSENDRAW:
			strcat(send_cmd, "AT+CIPSENDRAW");
			break;
		case AT_CMD_AT_CIPRECVCFG_GET:
			strcat(send_cmd, "AT+CIPRECVCFG?");
			break;
			
		
	}
	//strcat(send_cmd, "\r\n");

	memset(pOutBuffer, 0, OutLength);
	ret_val = hal_uart_send(&wifi_uart, send_cmd, strlen(send_cmd), 30000);
	ret_val = hal_uart_send(&wifi_uart, "\r", 1, 30000);
	strcat(send_cmd, "\r\n");
	if(ret_val != HAL_OK)
		return HAL_ERROR;
	while(1){
		do {
			ret_val = hal_uart_recv(&wifi_uart, &Recv_ch, 1, &recv_size, 30000);
			if(ret_val != HAL_OK){
				time_out++;
				if(time_out >= 10)
					return HAL_ERROR;
				krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			}
		}while(recv_size != 1);
		time_out = 0;
		if(Recv_ch == '\0'){
			recv_size_t = 0;
			memset(pOutBuffer, 0, OutLength);
			continue;
		}
		*(pOutBuffer + recv_size_t) = Recv_ch;
		recv_size_t++;
		printf("Recv_ch = %c\n", Recv_ch);
		printf("recv_size_t = %d\n", recv_size_t);
		printf("pOutBuffer = %s\n", pOutBuffer);
		if(recv_size_t >= 4){
			if(strstr(pOutBuffer, send_cmd)   && (respone_start == 0) ){
				respone_start = 1;
				recv_size_t = 0;
				memset(pOutBuffer, 0, OutLength);
			}
			else if(respone_start){
				if(strstr(pOutBuffer, "ERROR\r\n") || strstr(pOutBuffer, "OK\r\n"))
					break;
			}
		}
		if(recv_size_t >= OutLength){
			ret_val = HAL_ERROR;
			break;
		}
	}
	return ret_val;
}

#if defined(NB_MOUDLE) || defined(LORA_MODULE)

static void receive_message_from_uart(uart_dev_t *uart, char print_buff[], uint32_t buff_size)
{
  int32_t ret_val = HAL_OK;
  char Recv_ch;
  uint32_t recv_size = 0;

  while(1) {
    ret_val = HAL_UART_Receive_IT_Buf_Queue_1byte((UART_HandleTypeDef *)uart->priv, &Recv_ch);
    if (ret_val != HAL_OK) {
      if (recv_size) {
        printf("%s", print_buff);
        memset(print_buff, 0, recv_size);
      }
      return;
    }
    else {
      print_buff[recv_size++] = Recv_ch;
    }

    if (recv_size == buff_size) {
      print_buff[buff_size] = '\0';
      printf("%s \n", print_buff);
      memset(print_buff, 0, recv_size);
      recv_size = 0;
    }
  }
}

void uart_message_receive()
{
  char print_buff[MAX_CMD_LEN] = {0};

  while(1) {
      if (uart_receive_flag == UART_RECEIVE_ENABLE) {
        receive_message_from_uart(&lora_uart, print_buff, MAX_CMD_LEN - 1);
      }

      krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/5);
  }
}

static void awaken_lora_module()
{
  sys_time_t cur_time = krhino_sys_time_get();
  if (lora_cmd_timer > cur_time) {
    return;
  }
  else {
    lora_cmd_timer = cur_time + LORA_SLEEP_TIME * 1000;
  }

  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, GPIO_PIN_SET);
  krhino_task_sleep(1);
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, GPIO_PIN_RESET);
  krhino_task_sleep(1);
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, GPIO_PIN_SET);
}

// lora at commond entry
static void lora_at_common_func(const char * lora_cmd, int cmd_len)
{
	if(aos_mutex_lock(&at_mutex, AOS_WAIT_FOREVER)) {
		printf("at cmd busy\n");
	}

  if (!lora_cmd) {
    printf("%s(%d):lora_cmd is null.\n", __FUNCTION__, __LINE__);
    goto lora_out;
  }

  // awaken lora module
  awaken_lora_module();

	if(hal_uart_send(&lora_uart, lora_cmd, cmd_len, 300) != HAL_OK) {
    printf("%s(%d):send lora at commond fail \n", __FUNCTION__, __LINE__);
    goto lora_out;
  }

lora_out:
  aos_mutex_unlock(&at_mutex);
}

// nb at commond entry
static void nb_at_common_func(const char * nb_cmd, int cmd_len)
{
	if(aos_mutex_lock(&at_mutex, AOS_WAIT_FOREVER)) {
		printf("at cmd busy\n");
	}

  if (!nb_cmd) {
    printf("%s(%d):nb_cmd is null.\n", __FUNCTION__, __LINE__);
    goto nb_out;
  }

	if(hal_uart_send(&nb_uart, nb_cmd, cmd_len, 300) != HAL_OK) {
    printf("send nb at commond error\n");
  }

nb_out:
  aos_mutex_unlock(&at_mutex);
}

static int at_set_moudle_type(uint8_t new_type)
{
  if (new_type > AT_MOUDLE_NB)
    return HAL_ERROR;

  at_moudle_type = new_type;
  return HAL_OK;
}

static void deal_module_change_cmd(char* in_cmd)
{
  const char* module_get_str = "AT+MOUDLE?";   // string length 10
  const char* module_set_str = "AT+MOUDLE=1";  // string length 11
  int in_cmd_len = strlen(in_cmd);

  if (in_cmd_len == strlen(module_get_str)) {
    if (strncmp(in_cmd, module_get_str, in_cmd_len) == 0) {
      printf("AT+MOUDLE=%d\r\n", at_moudle_type);
      return;
    }

    printf("INPUT ERROR\r\n");
    return;
  }

  if (in_cmd_len == strlen(module_set_str)) {
    if (strncmp(in_cmd, module_set_str, in_cmd_len - 1) == 0) {
      uint8_t module_type = (uint8_t)(in_cmd[in_cmd_len - 1] - '0');
      if (at_set_moudle_type(module_type) == HAL_OK) {
        printf("OK\r\n");
        return;
      }

      printf("INPUT ERROR\r\n");
      return;
    }
  }
}

static int is_module_change_cmd(char* in_cmd)
{
  if (!in_cmd) {
    printf("%s(%d):in_cmd is null.\n", __FUNCTION__, __LINE__);
    return HAL_ERROR;
  }

  if (strncmp(in_cmd, "AT+MOUDLE", strlen("AT+MOUDLE")) == 0) {
    printf("\n");
    deal_module_change_cmd(in_cmd);
    return HAL_OK;
  }

  return HAL_ERROR;
}

static void nb_lora_pre_hand_cmd(char *buff_cmd, char *cmd)
{
  char*p = buff_cmd;
  int regulate_cmd = 1;
  
  while(*p) {
    if (regulate_cmd) {
      // delete blank space in at commond
      if (*p == ' ') {
        p++;
        continue;
      }

      // do not modify parameters
      if (*p == '=') {
        regulate_cmd = 0;
        *cmd++ = *p++;
        continue;
      }

      // upper to capital letter
      if (*p >= 'a' && *p <= 'z') {
        *cmd++ = *p++ - ('a' - 'A');
        continue;
      }
    }

    *cmd++ = *p++;
  }
  *cmd = '\0';
}

#endif

static const struct at_ap_command at_cmds_table[] = {
    { .id = AT_CMD_AT_TEST, .pre_cmd = "AT+TEST", .help = "AT+TEST", .function = at_test },
    { .id = AT_CMD_AT, .pre_cmd = "AT", .help = "AT", .function = handle_at },
    { .id = AT_CMD_AT_GETATVERSION, .pre_cmd = "AT+GETATVERSION?", 
    	.help = "AT+GETATVERSION?",.function = at_get_at_verion},
    { .id = AT_CMD_AT_GETATVERSION_V2, .pre_cmd = "AT+GETATVERSION", 
    	.help = "AT+GETATVERSION?",.function = at_get_at_verion_old},
    { .id = AT_CMD_AT_FWVER, .pre_cmd = "AT+FWVER?", .help = "AT+FWVER?",.function = at_version},
    { .id = AT_CMD_AT_SYSTIME, .pre_cmd = "AT+SYSTIME?", .help = "AT+SYSTIME?",.function = at_system_run_time_func},
    { .id = AT_CMD_AT_MEMFREE, .pre_cmd = "AT+MEMFREE?", .help = "AT+MEMFREE?",.function = at_system_memory_free_func},
    { .id = AT_CMD_AT_REBOOT, .pre_cmd = "AT+REBOOT", .help = "AT+REBOOT",.function = at_reboot_func},  
    { .id = AT_CMD_AT_FACTORY, .pre_cmd = "AT+FACTORY", .help = "AT+FACTORY",.function = at_recover_factory_func},
    {.id = AT_CMD_AT_FLASHLOCK_GET, .pre_cmd = "AT+FLASHLOCK?", .help = "AT+FLASHLOCK?",.function = at_refer_flash_lock_func}, 
    {.id = AT_CMD_AT_FLASHLOCK_SET, .pre_cmd = "AT+FLASHLOCK", .help = "AT+FLASHLOCK=<mode>",.function = at_set_flash_lock_func}, //at_set_flash_lock
    {.id = AT_CMD_AT_WEVENT_GET, .pre_cmd = "AT+WEVENT?", .help = "AT+WEVENT?",.function = at_refer_wifi_event_notification_func},
    {.id = AT_CMD_AT_WEVENT_SET, .pre_cmd = "AT+WEVENT", .help = "AT+WEVENT=<mode>",.function = at_set_wifi_event_notification_func}, //at_refer_wifi_event_notification
    {.id = AT_CMD_AT_WLPC_GET, .pre_cmd = "AT+WLPC?", .help = "AT+WLPC?",.function = at_refer_wifi_power_save_func},
    {.id = AT_CMD_AT_WLPC_SET, .pre_cmd = "AT+WLPC", .help = "AT+WLPC=<mode>",.function = at_set_wifi_power_save_func}, //at_set_wifi_power_save

    {.id = AT_CMD_AT_UART_GET, .pre_cmd = "AT+UART?", .help = "AT+UART?",.function = at_refer_uart_info_func}, 
    {.id = AT_CMD_AT_UART_SET, .pre_cmd = "AT+UART", 
    	.help = "AT+UART=<baud>,<bits>,<stpbit>,<parity>,<flw_ctl>",.function = at_set_uart_info_func}, 
    	
    {.id = AT_CMD_AT_UARTFOMAT_GET, .pre_cmd = "AT+UARTFOMAT?", .help = "AT+UARTFOMAT?",.function = at_refer_uart_fomat_func}, 
    {.id = AT_CMD_AT_UARTFOMAT_SET, .pre_cmd = "AT+UARTFOMAT", 
    	.help = "AT+UARTFOMAT=<length>,<time>",.function = at_set_uart_fomat_func},  //at_set_uart_fomat

    {.id = AT_CMD_AT_UARTE_GET, .pre_cmd = "AT+UARTE?", .help = "AT+UARTE?",.function = at_refer_uart_echo_func}, 
    {.id = AT_CMD_AT_UARTE_SET, .pre_cmd = "AT+UARTE", 
    	.help = "AT+UARTE=<OPTION>",.function = at_set_uart_echo_func}, //at_set_uart_echo

	//wifi config command
    {.id = AT_CMD_AT_WFVER, .pre_cmd = "AT+WFVER", .help = "AT+WFVER",.function = at_wifi_firmware_version_func}, 
    {.id = AT_CMD_AT_WMAC, .pre_cmd = "AT+WMAC?", .help = "AT+WMAC?",.function = at_wl_mac_func},
    { .id = AT_CMD_AT_WSCANOPT_GET, .pre_cmd = "AT+WSCANOPT?", .help = "AT+WSCANOPT?", .function = at_refer_wifi_scan_option_func }, 
    { .id = AT_CMD_AT_WSCANOPT_SET, .pre_cmd = "AT+WSCANOPT", 
    	.help = "AT+WSCANOPT=<OPTION>", .function =  at_set_wifi_scan_option_func},  //0,1
    { .id = AT_CMD_AT_WSCAN, .pre_cmd = "AT+WSCAN", .help = "AT+WSCAN", .function = at_wl_scan_func }, 
    { .id = AT_CMD_AT_WDHCP_GET, .pre_cmd = "AT+WDHCP?", .help = "AT+WDHCP?", .function = at_refer_wifi_dhcp_func }, 
    { .id = AT_CMD_AT_WDHCP_SET, .pre_cmd = "AT+WDHCP", .help = "AT+WDHCP=<option>", .function = at_set_wifi_dhcp_func }, 
    //AP
    { .id = AT_CMD_AT_WSAPIP_GET, .pre_cmd = "AT+WSAPIP?", .help = "AT+WSAPIP?", .function = at_refer_wifi_ap_ip_mask_gate_func }, 
    { .id = AT_CMD_AT_WSAPIP_SET, .pre_cmd = "AT+WSAPIP", .help = "AT+WSAPIP=<ip>,<mask>,<gate>", .function = at_set_wifi_ap_ip_mask_gate_func }, 
    { .id = AT_CMD_AT_WSAP_GET, .pre_cmd = "AT+WSAP?", .help = "AT+WSAP?", .function = at_refer_wifi_ap_info_func }, 
    { .id = AT_CMD_AT_WSAP_SET, .pre_cmd = "AT+WSAP", .help = "AT+WSAP=<ssid>,<psw>", .function = at_set_wifi_ap_info_start_func }, 
    { .id = AT_CMD_AT_WSAPQ, .pre_cmd = "AT+WSAPQ", .help = "AT+WSAPQ", .function = at_wifi_ap_quit_func }, 
    { .id = AT_CMD_AT_WSAPS, .pre_cmd = "AT+WSAPS", .help = "AT+WSAPS", .function = at_get_ap_current_status_func }, 
	//sta
    { .id = AT_CMD_AT_WJAPIP_GET, .pre_cmd = "AT+WJAPIP?", .help = "AT+WJAPIP?", .function = at_refer_wifi_sta_ip_mask_gate_dns_func }, 
    { .id = AT_CMD_AT_WJAPIP_SET, .pre_cmd = "AT+WJAPIP", .help = "AT+WJAPIP=<ip>,<mask>,<gate>[,<dns>]", .function = at_set_wifi_sta_ip_mask_gate_dns_func }, 
    { .id = AT_CMD_AT_WJAP_GET, .pre_cmd = "AT+WJAP?", .help = "AT+WJAP?", .function = at_refer_wifi_sta_info_func },
    { .id = AT_CMD_AT_WJAP_SET, .pre_cmd = "AT+WJAP", .help = "AT+WJAP=<ssid>,<psw>", .function =  at_set_wifi_sta_info_start_func},
    { .id = AT_CMD_AT_WJAPQ, .pre_cmd = "AT+WJAPQ", .help = "AT+WJAPQ", .function = at_wifi_sta_quit_func },  
    { .id = AT_CMD_AT_WJAPS, .pre_cmd = "AT+WJAPS", .help = "AT+WJAPS", .function = at_get_sta_current_status_func },
    //TCP/UDP
    { .id = AT_CMD_AT_SSLCERT_GET, .pre_cmd = "AT+SSLCERTGET", .help = "AT+SSLCERTGET=<type>", .function = NULL }, 
    { .id = AT_CMD_AT_SSLCERT_SET, .pre_cmd = "AT+SSLCERTSET", .help = "AT+SSLCERTSET=<type>", .function = NULL }, 
    { .id = AT_CMD_AT_CIPDOMAIN, .pre_cmd = "AT+CIPDOMAIN", .help = "AT+CIPDOMAIN=<domain>", .function = NULL }, 
    { .id = AT_CMD_AT_CIPAUTOCONN, .pre_cmd = "AT+CIPAUTOCONN", .help = "AT+CIPAUTOCONN=<id>[,option]", .function = NULL }, 
    { .id = AT_CMD_AT_CIPSSLOPT, .pre_cmd = "AT+CIPSSLOPT", .help = "AT+CIPSSLOPT=<id>,<isSSLRoot>,<isSSLClient>[,isSSLCrl]", .function = NULL }, 
    { .id = AT_CMD_AT_CIPSTART, .pre_cmd = "AT+CIPSTART", .help = "AT+CIPSTART=<id>,<type>,[domain],[remote_port],[local_port]", .function = NULL }, 
    { .id = AT_CMD_AT_CIPSTOP, .pre_cmd = "AT+CIPSTOP", .help = "AT+CIPSTOP=<id>[,<remote_port>]", .function = NULL }, 
    { .id = AT_CMD_AT_CIPSTATUS, .pre_cmd = "AT+CIPSTATUS", .help = "AT+CIPSTATUS=<id>", .function = NULL }, 
    { .id = AT_CMD_AT_CIPSEND, .pre_cmd = "AT+CIPSEND", .help = "AT+CIPSEND=<id>,[<remote_port>,]<data_length>", .function = NULL }, 
    { .id = AT_CMD_AT_CIPSENDRAW, .pre_cmd = "AT+CIPSENDRAW", .help = "AT+CIPSENDRAW", .function = at_cip_send_raw_func }, 
    { .id = AT_CMD_AT_CIPRECV, .pre_cmd = "AT+CIPRECV", .help = "AT+CIPRECV=<id>[,port]", .function = NULL }, 
    { .id = AT_CMD_AT_CIPRECVCFG_GET, .pre_cmd = "AT+CIPRECVCFG?", .help = "AT+CIPRECVCFG?", .function = at_refer_cip_recv_cfg_func },
    { .id = AT_CMD_AT_CIPRECVCFG_SET, .pre_cmd = "AT+CIPRECVCFG", .help = 	"AT+CIPRECVCFG=<recv mode>", .function = at_cip_recv_cfg_func },
    //FOTA
    { .id = AT_CMD_AT_FOTA, .pre_cmd = "AT+FOTA", .help = "AT+FOTA=<size>,<version>,<url>,<md5>", 
    		.function = at_fota_start_func },

    { .id = AT_CMD_MAX, .help = "end", .function = NULL },
};

uint32_t at_cmd_request(enum at_cmd_e request_id, char *pInBuffer, char *pOutBuffer, uint16_t OutLength)
{
	uint16_t cmd_index = 0; 
	uint8_t ret_val = HAL_OK;

	if(aos_mutex_lock(&at_mutex, AOS_WAIT_FOREVER))
	{
		printf("at cmd busy\n");
		return HAL_BUSY;
	}
	
	if(request_id >= AT_CMD_MAX || (pOutBuffer == NULL))
	{
		printf("request_id = %d not support\n", request_id);
		ret_val =  HAL_ERROR;
		goto out;
	}
	
	for(cmd_index = 0; ;cmd_index++){
		if(at_cmds_table[cmd_index].id == AT_CMD_MAX){
			printf("request_id = %d not support\n", request_id);
			ret_val =  HAL_ERROR;
			goto out;
		}
		
		if(request_id == at_cmds_table[cmd_index].id){
			 break;
		}	
	}
	if(at_cmds_table[cmd_index].function == NULL)
	{
		printf("cmd = %s not support\n", at_cmds_table[cmd_index].pre_cmd);
		ret_val =  HAL_ERROR;
	}
	else
		ret_val = at_cmds_table[cmd_index].function(request_id, pInBuffer, pOutBuffer, OutLength);
out:
	 aos_mutex_unlock(&at_mutex);
	return ret_val;
}

static void pre_hand_cmd(char *buff_cmd, char *cmd)
{
	char *p = buff_cmd;
	while(*p == ' ')
		p++;
	while((*p) && (*p != ' ')){
		*cmd++ = *p++;
	}
	*cmd = '\0';
}

static int look_up_cmd(char *buff, uint8_t *cmd_id)
{
	uint16_t cmd_index = 0; 
	uint8_t ret_val = HAL_OK;
	char pre_cmd[50] = {0};
	char len = 0;

	if((buff == NULL)  ||  strlen(buff) < 2){
		*cmd_id = 0xff;
		return HAL_ERROR; 
	}
	len = (strlen(buff) > sizeof(pre_cmd)) ? sizeof(pre_cmd) : strlen(buff);
	memcpy(pre_cmd, buff, len);

	len = 0;
	while(pre_cmd[len] && pre_cmd[len] != '=')
		len++;
  pre_cmd[len] = '\0';
	//printf("pre_cmd = %s\n", pre_cmd);
	for(cmd_index = 0; ;cmd_index++){
		if(at_cmds_table[cmd_index].id == AT_CMD_MAX){
			printf("cmd : %s not support\r\n", pre_cmd);
			return HAL_ERROR;
		}
		
		if(strcmp(pre_cmd, at_cmds_table[cmd_index].pre_cmd) == 0){	
			*cmd_id = at_cmds_table[cmd_index].id;
			break;
		}
	}

	return ret_val;
}

void wifi_cmd_task(void *arg)
{
	//struct nt_cli_command *command = NULL;
	uint8_t cmd_index = 0xff;
	uint32_t ret;
	int icnt;
	char pOutBuffer[MAX_OUT_LEN] = "OK";
	//char pInBuffer[100];
	char buff_cmd[MAX_CMD_LEN] = {0};
	char cmd[MAX_CMD_LEN] = {0};
	char gch;
	char AT_start = 0;
	char *pInBuffer = NULL;

	atcmd_init_mutex();
	
	printf("wifi_cmd_task\n");
	#if 0
	while(AT_start == 0){
		ret = hal_uart_send(&wifi_uart, (void *)"AT\r\n", strlen("AT\r\n"), 30000);
		if(ret == 0){
			do{
				ret = hal_uart_recv(&wifi_uart, (void *)&gch, 1, &recv_size, 3000);
				if(ret != HAL_OK){
					krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
				}
			}while(recv_size != 1);
			printf("gch = %c\r\n", gch);
		}
	}
	#endif
	while(1){
		cmd_index = 0xff;
		icnt = 0;
		memset(pOutBuffer, 0, sizeof(pOutBuffer));
		memset(buff_cmd, 0, sizeof(buff_cmd));
		#if 0
		while(1){
			gch = getchar();
			//printf("gch = 0x%x\n", gch);
			if(gch == 'A' && AT_start == 0){
				AT_start = 1;
				icnt = 0;
			}
			if(AT_start){
				if((gch == '\r') || (gch == '\n')){
					buff_cmd[icnt] = '\0';
					if(strlen(buff_cmd) >= 2)
						break;
					else{
						icnt = 0;
						AT_start = 0;
						printf("\r\ninput error\r\n");
						continue;
					}
				}
				else{
					buff_cmd[icnt++] = gch;
					if(icnt >= MAX_CMD_LEN){
						icnt = 0;
						AT_start = 0;
					}
						
				}
			}
		}
		#else
		while(1){
			gch = getchar();
			//printf("gch = 0x%x\n", gch);
			if(icnt >= MAX_CMD_LEN){
				printf("\r\ninput too long\r\n");
				icnt = 0;
			}
			if((gch == '\r') || (gch == '\n')){
				buff_cmd[icnt] = '\0';
				icnt = 0;
				if(strlen(buff_cmd) >= 2){
					break;
				}
				else{
					printf("\r\ninput error\r\n");
					continue;
				}	
			}
			else
				buff_cmd[icnt++] = gch;
		}
		#endif

#if defined(LORA_MODULE) || defined(NB_MOUDLE)
    memset(cmd, 0, MAX_CMD_LEN);
    nb_lora_pre_hand_cmd(buff_cmd, cmd);
    if (is_module_change_cmd(cmd) == HAL_OK) {
      continue;
    }

    if (at_moudle_type == AT_MOUDLE_WIFI) {
      ret = look_up_cmd(cmd, &cmd_index);
      if(ret != HAL_OK)
        continue;
      
      pInBuffer = strstr(cmd, "=");
      if (pInBuffer)
        pInBuffer++;
      printf("\n");
    }
    else {
      // regulate at commond
      icnt = 0; 
      while (icnt < MAX_CMD_LEN - 1) {
        if (cmd[icnt] == '\0') {
          cmd[icnt] = '\r';
          cmd[++icnt] = '\n';
          break;
        }
        icnt++;
      }

      // printf("%s(%d):cmd:%s\n", __FUNCTION__, __LINE__, cmd);
      printf("\n");
      if (at_moudle_type == AT_MOUDLE_NB) {
        // send and receive at response
        nb_at_common_func(cmd, icnt + 1);
      }
      else if (at_moudle_type == AT_MOUDLE_LORA) {
        lora_at_common_func(cmd, icnt + 1);
      }
      else {
        printf("moudle type error\n");
      }
    }

#else
		pre_hand_cmd(buff_cmd, cmd);
		ret = look_up_cmd(cmd, &cmd_index);
		if(ret != HAL_OK)
			continue;
		//printf("cmd_index = 0x%x\r\n", cmd_index);
		pInBuffer = strstr(cmd, "=") + 1;
		//printf("pInBuffer = %s\r\n", pInBuffer);
		printf("\r\n");
#endif

#if defined(LORA_MODULE) || defined(NB_MOUDLE)
    if (at_moudle_type == AT_MOUDLE_WIFI) {
#endif
  		ret = at_cmd_request((enum at_cmd_e)cmd_index, pInBuffer, pOutBuffer, 2000);
  		if(ret == HAL_OK)
  			//printf("ret = %d\r\n", ret);
  			printf("%s", pOutBuffer);
  		else
  			printf("AT cmd failed\r\n");
#if defined(LORA_MODULE) || defined(NB_MOUDLE)
    }
#endif
	}
}

