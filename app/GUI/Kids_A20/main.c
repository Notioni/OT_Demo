/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <k_api.h>
#include <stdio.h>
#include <stdlib.h>
#include "stm32l4xx_hal.h"
#include "GUIDEMO.h"
#include <aos/aos.h>
#include <aos/uData.h>
#include "isd9160.h"
#include "irda.h"

#if defined(NB_MOUDLE) || defined(LORA_MODULE)
#include "atdemo.h"
#endif

#ifdef CONFIG_AOS_FATFS_SUPPORT_MMC
#include "fatfs.h"
static const char *g_string         = "Fatfs test string.";
static const char *g_filepath       = "/sdcard/test.txt";
static const char *g_dirpath        = "/sdcard/testDir";
static const char *g_dirtest_1      = "/sdcard/testDir/test_1.txt";
static const char *g_dirtest_2      = "/sdcard/testDir/test_2.txt";
static const char *g_dirtest_3      = "/sdcard/testDir/test_3.txt";
static const char *g_new_filepath   = "/sdcard/testDir/newname.txt";
#endif
#define DEMO_TASK_STACKSIZE    1024 //512*cpu_stack_t = 2048byte
#define DEMO_TASK_PRIORITY     20
#define DAEMON_TASK_STACKSIZE 1024 //512*cpu_stack_t = 2048byte
#define DAEMON_TASK_PRIORITY  21
#define UART_RECEIVE_TASK_PRIORITY 22
#define WIFICMD_TASK_PRIORITY  23

extern void wifi_cmd_task(void *arg);
static ktask_t demo_task_obj;
static ktask_t daemon_task_obj;
static ktask_t nt_task_obj;
static ktask_t uart_receive_task_obj;

cpu_stack_t demo_task_buf[DEMO_TASK_STACKSIZE];
cpu_stack_t nt_task_buf[DEMO_TASK_STACKSIZE];
cpu_stack_t daemon_task_buf[DAEMON_TASK_STACKSIZE];
cpu_stack_t uart_receive_task_buf[DEMO_TASK_STACKSIZE];

static kinit_t kinit;
extern int key_flag;
extern int key_a_flag;
extern uint8_t sd_on;
// static int old_key_flag;

int handing_shake()
{
       static sys_time_t last_time = 0;
       sys_time_t now_time = 0;
       int ret = 0;

       now_time = krhino_sys_time_get();
       if (now_time - last_time < 200) {
               ret = 1;
       }
       last_time = now_time;

       return ret;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	GUI_KEY_A10 key = 0;

  if (handing_shake())
    return;

	switch (GPIO_Pin) {
		case KEY_1_Pin:
			key = GUI_KEY_1;
      key_flag = GUI_DEMO_PAGE_INIT;
			break;
		case KEY_2_Pin:
			key = GUI_KEY_2;
      key_a_flag = 1;
			break;
		case KEY_3_Pin:
			key = GUI_KEY_3;
      ++key_flag;
			break;
		default:
			return;
	}
	GUI_StoreKeyMsg(key, 1);
	GUI_StoreKeyMsg(key, 0);
}

void uData_report_demo(input_event_t *event, void *priv_data)
{
    udata_pkg_t buf;
    if ((event == NULL)||(event->type != EV_UDATA)) {
        return;
    }

    if(event->code == CODE_UDATA_REPORT_PUBLISH){
        int ret = 0;
        ret = uData_report_publish(&buf);
        if(ret == 0){
            barometer_data_t* data = (barometer_data_t*)buf.payload;
            printf("uData_application::::::::::::::type = (%d)\n", buf.type);
            printf("uData_application::::::::::::::data = (%d)\n", data->p);
            printf("uData_application:::::::::timestamp = (%d)\n", data->timestamp);
        }
    }
}

int application_start(int argc, char *argv[])
{
    int ret = 0;

    aos_register_event_filter(EV_UDATA, uData_report_demo, NULL);

    ret = uData_subscribe(UDATA_SERVICE_BARO);
    if(ret != 0){
        printf("%s %s %s %d\n", uDATA_STR, __func__, ERROR_LINE, __LINE__);
        return -1;
    }
    aos_loop_run();

    return 0;
}
#ifdef CONFIG_AOS_FATFS_SUPPORT_MMC
void test_sd_case(void)
{
	int fd, ret;
	char readBuffer[32] = {0};
	printf(" test_sd_case\n");
	
	  /* Fatfs write test */
	printf(" Fatfs write test\n");
	fd = aos_open(g_filepath, O_RDWR | O_CREAT | O_TRUNC);
	 printf("aos_open , ret = %d\n", fd);
	 if (fd > 0) {
	        ret = aos_write(fd, g_string, strlen(g_string));
	        printf("aos_write , ret = %d\n", ret);
	        ret = aos_sync(fd);
	        printf("aos_sync , ret = %d\n", ret);
	        aos_close(fd);
     }

      /* Fatfs read test */
     printf(" Fatfs read test\n");
    fd = aos_open(g_filepath, O_RDONLY);
    if (fd > 0) {
        ret = aos_read(fd, readBuffer, sizeof(readBuffer));
        printf("aos_read , readBuffer = %s\n", readBuffer);
        aos_close(fd);      
    }

	/* Fatfs mkdir test */
	printf(" Fatfs mkdir test\n");
    aos_dir_t *dp = (aos_dir_t *)aos_opendir(g_dirpath);
    if (!dp) {
        ret = aos_mkdir(g_dirpath);
        printf("aos_mkdir , ret = %d\n", ret);
    } else {
        ret = aos_closedir(dp);
        printf("aos_closedir , ret = %d\n", ret);
    }

    /* Fatfs readdir test */
	printf(" Fatfs readdir test\n");
    fd = aos_open(g_dirtest_1, O_RDWR | O_CREAT | O_TRUNC);
    if (fd > 0)
        aos_close(fd);

    fd = aos_open(g_dirtest_2, O_RDWR | O_CREAT | O_TRUNC);
    if (fd > 0)
        aos_close(fd);

    fd = aos_open(g_dirtest_3, O_RDWR | O_CREAT | O_TRUNC);
    if (fd > 0)
        aos_close(fd);

    dp = (aos_dir_t *)aos_opendir(g_dirpath);
    if (dp) {
        aos_dirent_t *out_dirent;
        while(1) {
            out_dirent = (aos_dirent_t *)aos_readdir(dp);
            if (out_dirent == NULL)
                break;

            printf("file name is %s\n", out_dirent->d_name);            
        }
    }
    aos_closedir(dp);

     /* Fatfs rename test */
	 printf(" Fatfs rename test\n");
    ret = aos_rename(g_filepath, g_new_filepath);
    printf("aos_rename , ret = %d\n", ret);

    fd = aos_open(g_filepath, O_RDONLY);
    if (fd >= 0)
        aos_close(fd);

    fd = aos_open(g_new_filepath, O_RDONLY);
     printf("aos_open , ret = %d\n", fd);
    if (fd > 0)
        aos_close(fd);

    /* Fatfs unlink test */
    ret = aos_unlink(g_new_filepath);
   printf("aos_unlink , ret = %d\n", ret);

    fd = aos_open(g_new_filepath, O_RDONLY);
     printf("aos_open , ret = %d\n", fd);
    if (fd > 0)
        aos_close(fd);
}
#endif
#if !defined(NB_MOUDLE) && !defined(LORA_MODULE)
int test_se1(void)
{
	char test[2][20];
	int ret = 0;
	ret = DeviceOpen(test);
	if(ret == HAL_OK)
		printf("DeviceOpen ok\n");
	else
		printf("DeviceOpen error\n");

	return ret;
}
void test_id2(void)
{
	uint8_t retval = 0;
	uint8_t ins1[] = {0x00, 0xA4, 0x00, 0x04, 0x02, 0x3F, 0x00};
	uint8_t ins2[] = {0x00, 0xA4, 0x00, 0x04, 0x02, 0x7F, 0x40};
	uint8_t ins3[] = {0x00, 0xA4, 0x00, 0x04, 0x02, 0x6F, 0xF1};
	uint8_t ins4[] = {0x00, 0xB0, 0x00, 0x00, 0x19 };
	uint8_t ins_out[40] = {0};
	uint8_t ii;
	DeviceTransmit(NULL, ins1, sizeof(ins1), ins_out, sizeof(ins_out));
	DeviceTransmit(NULL, ins2, sizeof(ins2), ins_out, sizeof(ins_out));
	DeviceTransmit(NULL, ins3, sizeof(ins3), ins_out, sizeof(ins_out));
	retval = DeviceTransmit(NULL, ins4, sizeof(ins4), ins_out, sizeof(ins_out));
	if(retval){
		printf("ins2 cmd error\n");
		return;
	}
	for(ii = 0; ii < 0x19 + 2; ii++)
		printf("ins_out[%d] = 0x%x\n", ii, ins_out[ii]);
}

int test_se2(void)
{
	//uint8_t ins[] = {
	//	0x00, 0xA4, 0x04, 0x04, 0x10, 0xA0, 0x00, 0x00, 0x00, 0x30, 0x50, 0x00,
	//	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0x64, 0x32
	//};
	uint8_t retval = 0;
	uint8_t ins[] = {0x00, 0x36, 0x00, 0x00,  0x03, 0x41, 0x00, 0x41};
	uint8_t ins2[] = {0x00, 0xC0, 0x00, 0x00, 0x1D};
	uint8_t ins_out[31] = {0};
	/*
	uint8_t ins_resp[31] = {
		0x41, 0x30, 0x30, 0x33, 0x43, 0x46, 0x41, 0x39, 0x43, 0x35, 
		0x43, 0x44, 0x36, 0x46, 0x35, 0x30, 0x44, 0x45, 0x39, 0x43, 
		0x32, 0x45, 0x33, 0x30, 0x30, 0x18, 0x00, 0x05, 0xde, 0x90, 0x00
	};
	*/
	int32_t resp_len = 0;
	
	retval = DeviceTransmit(NULL, ins, sizeof(ins), ins_out, &resp_len);
	if(retval){
		printf("ins cmd error\n");
		return HAL_ERROR;
	}
	retval = DeviceTransmit(NULL, ins2, sizeof(ins2), ins_out, &resp_len);
	if(retval){
		printf("ins2 cmd error\n");
		return HAL_ERROR;
	}
	printf("resp_len = %d\n", resp_len);
	printf("ins_out[29] = 0x%x\n", ins_out[29]);
	printf("ins_out[30] = 0x%x\n", ins_out[30]);
	if(resp_len != (0x1D + 2) || ins_out[resp_len -2] != 0x90 || ins_out[resp_len -1] != 0x00){
		printf("cmd respone length or SW error\n");
		return HAL_ERROR;
	}
	return HAL_OK;
}

void test_se3(void)
{
	DeviceClose(NULL);
}

int test_se(void)
{
	int ret = 0;
	ret = test_se1();
	if(ret)
		return HAL_ERROR;
	ret = test_se2();
	if(ret)
		return HAL_ERROR;
	test_se3();
	printf("test se ok\n");
	return HAL_OK;
}

#endif
void demo_task(void *arg)
{
    int ret = 0;
    int count = 0;

    stm32_soc_init();

    kinit.argc = 0;
    kinit.argv = NULL; 
    kinit.cli_enable = 1;
    aos_kernel_init(&kinit);
#ifdef CONFIG_AOS_FATFS_SUPPORT_MMC	
	ret = fatfs_register();
	printf("reg_result = %d\n", ret);
	if(ret == 0)
		sd_on = 1;
#endif

    GUIDEMO_Main();

    while (1)
    {
        printf("hello world! count %d\n", count++);

        //sleep 1 second
        krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND);
    };
}

static void LED_test(void)
{
	hal_gpio_output_toggle(&brd_gpio_table[GPIO_ALS_LED]);
	hal_gpio_output_toggle(&brd_gpio_table[GPIO_GS_LED]);
	hal_gpio_output_toggle(&brd_gpio_table[GPIO_COMPASS_LED]);
}

void daemon_task(void *arg)
{
	static int test_flag = (5 << 1) - 1;
	krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND);
	while (1) {
		if (test_flag) {
			--test_flag;
			LED_test();
		}
		isd9160_loop_once();
		krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND);
	}
}

int main(void)
{
    krhino_init();
    krhino_task_create(&demo_task_obj, "demo_task", 0, DEMO_TASK_PRIORITY, 
        50, demo_task_buf, DEMO_TASK_STACKSIZE, demo_task, 1);

    krhino_task_create(&nt_task_obj, "wifi_cmd_task", 0,  WIFICMD_TASK_PRIORITY, 
        50, nt_task_buf, DEMO_TASK_STACKSIZE, wifi_cmd_task, 1);

    krhino_task_create(&daemon_task_obj, "daemon_task", 0, DAEMON_TASK_PRIORITY, 
        50, daemon_task_buf, DAEMON_TASK_STACKSIZE, daemon_task, 1);

#if defined(NB_MOUDLE) || defined(LORA_MODULE)
    krhino_task_create(&uart_receive_task_obj, "uart_receive_task", 0, UART_RECEIVE_TASK_PRIORITY, 
        50, uart_receive_task_buf, DEMO_TASK_STACKSIZE, uart_message_receive, 1);
#endif
	
    krhino_start();
    
    return 0;
}

