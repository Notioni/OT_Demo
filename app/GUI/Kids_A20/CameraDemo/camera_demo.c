#include <k_api.h>
#include <stdio.h>
#include <stdlib.h>
#include "stm32l4xx_hal.h"
#include "gc0329.h"
#include "camera_demo.h"
#include "fatfs.h"
#include "GUIDEMO.h"

//#define ST7789_LCD_PIXEL_WIDTH ((uint16_t)240)
//#define ST7789_LCD_PIXEL_HEIGHT ((uint16_t)240)
#define RGB565_TO_R(pixel)  ((pixel & 0x1f) << 3)
#define RGB565_TO_G(pixel)  ((pixel & 0x7e0) >> 3)
#define RGB565_TO_B(pixel)  ((pixel & 0xf800) >> 8)
 
CAMERA_DrvTypeDef *camera_drv;
DCMI_HandleTypeDef *phdcmi;
uint8_t camera_dis_on = 0;
uint8_t sd_on = 0;
uint16_t pBuffer[ST7789_WIDTH * ST7789_HEIGHT];
uint8_t pBuffer_t[3 * ST7789_WIDTH];
uint32_t counter = 0;
extern void camera_dispaly(uint16_t *data, uint32_t pixel_num);
extern void BSP_LCD_Clear(uint16_t Color);

void init_dcmi_dir(void)
{
    int ret;
    aos_dir_t *dp = (aos_dir_t *)aos_opendir("/sdcard/DCMI");
    if (!dp) {
        ret = aos_mkdir("/sdcard/DCMI");
        printf("aos_mkdir , ret = %d\n", ret);
    } else {
        ret = aos_closedir(dp);
        printf("aos_closedir , ret = %d\n", ret);
    }
}

static void get_images_index(uint32_t base_index)
{
	uint8_t str[30];
	uint32_t index = 0;
	struct stat st;
	int ret;

	for(index = base_index;; index++){
		memset(str, 0, sizeof(str));
		sprintf((char *)str, "/sdcard/DCMI/image_%lu.bmp", index);
		ret = aos_stat((char *)str, &st);
		if(ret < 0){
			counter = index;
			break;
		}
	}
}

void camera_open(void)
{
	HAL_DCMI_Start_DMA(phdcmi, DCMI_MODE_CONTINUOUS,  (uint32_t)pBuffer , (ST7789_WIDTH* ST7789_HEIGHT)/2 );
}

void camera_close(void)
{
	HAL_DCMI_Stop(phdcmi);
}

void CAMERA_Init(uint32_t Resolution)
{
	camera_drv = &gc0329_drv;
	phdcmi = &hdcmi_handle;
	
	 /* Camera Module Initialization via I2C to the wanted 'Resolution' */
    if (Resolution == CAMERA_R640x480)
    {
      /* For 240x240 resolution, the OV9655 sensor is set to QVGA resolution
       * as OV9655 doesn't supports 240x240  resolution,
       * then DCMI is configured to output a 240x240 cropped window */
      camera_drv->Init(GC0329_I2CADDR, CAMERA_R640x480);


      HAL_DCMI_ConfigCROP(phdcmi,
                          150,                 /* Crop in the middle of the VGA picture */
                          120,                 /* Same height (same number of lines: no need to crop vertically) */
                          (ST7789_WIDTH * 2) - 1,     /* 2 pixels clock needed to capture one pixel */
                          (ST7789_HEIGHT * 1) - 1);    /* All 240 lines are captured */
      HAL_DCMI_EnableCROP(phdcmi);
    }
}

void CameraDEMO_Main(void)
{
	uint8_t  sensor_id;
	HAL_StatusTypeDef hal_status = HAL_OK;
	
	printf("CameraDEMO_Main\n");
	gc0329_power_onoff(1);
	sensor_id = gc0329_ReadID();
	printf("sensor_id = 0x%x\n", sensor_id);
	//gc0329_power_onoff(0);
	init_dcmi_dir();
	get_images_index(counter);
	if(sensor_id == GC0329_ID){
		CAMERA_Init(CAMERA_R640x480);

		/* Wait 1s to let auto-loops in the camera module converge and lead to correct exposure */
 		// HAL_Delay(1000);
  		 krhino_task_sleep(krhino_ms_to_ticks(1000));
		  /*##-4- Camera Continuous capture start in QVGA resolution ############################*/
		  /* Disable unwanted HSYNC (IT_LINE)/VSYNC interrupts */
		  __HAL_DCMI_DISABLE_IT(phdcmi, DCMI_IT_LINE | DCMI_IT_VSYNC);

		  /* LCD size is 240 x 240 and format is RGB565 i.e. 16 bpp or 2 bytes/pixel. 
		     The LCD frame size is therefore 240 * 240 half-words of (240*240)/2 32-bit long words . 
		     Since the DMA associated to DCMI IP is configured in  BSP_CAMERA_MspInit() of stm32l496g_discovery_camera.c file
		     with words alignment, the last parameter of HAL_DCMI_Start_DMA is set to:
		      (ST7789H2_LCD_PIXEL_WIDTH*ST7789H2_LCD_PIXEL_HEIGHT)/2, that is 240 * 240 / 2
		   */   
		  //hal_status = HAL_DCMI_Start_DMA(phdcmi, DCMI_MODE_CONTINUOUS,  (uint32_t)pBuffer , (ST7789_WIDTH* ST7789_HEIGHT)/2 );
		 // OnError_Handler(hal_status != HAL_OK); 
		 camera_close();
	}	
}

uint8_t read_camera_flag = 1;
extern int   key_a_flag;
void GC0329_CAMERA_FrameEventCallback(void)
{
	//if(camera_dis_on){
		HAL_DCMI_Suspend(phdcmi);
		camera_dispaly(pBuffer, (ST7789_WIDTH* ST7789_HEIGHT));
		if(read_camera_flag && key_a_flag){
			read_camera_flag = 0;
			//memcpy(pBuffer_t, pBuffer, sizeof(pBuffer));
		}
		HAL_DCMI_Resume(phdcmi);
	//}
}

/**
  * @brief  Frame event callback
  * @param  hdcmi: pointer to the DCMI handle
  * @retval None
  */
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
  GC0329_CAMERA_FrameEventCallback();
}

static uint8_t SavePicture(void)
{
	uint8_t x,y;
	uint8_t nHeigh = ST7789_HEIGHT;
	uint8_t nWidth = ST7789_WIDTH;
	uint16_t tmp_data, bm;
	uint32_t pixel_index;
 	BmpHead bmpheaher;
	InfoHead infoheader;
	uint8_t str[30];
	int fd, ret = HAL_OK;

	bm = 0x4d42; //"BM"
	bmpheaher.imageSize = nHeigh * nHeigh * 3 + 54;
	bmpheaher.blank = 0;
	bmpheaher.startPosition = 54;

	infoheader.Length = 40;
	infoheader.height = ST7789_HEIGHT;
	infoheader.width = ST7789_WIDTH;
	infoheader.colorPlane = 1;
	infoheader.bitColor = 24;
	infoheader.zipFormat = 0;
	infoheader.realSize = nHeigh * nHeigh * 4;
	infoheader.xPels = 0;
	infoheader.yPels = 0;
	infoheader.colorUse = 0;
	infoheader.colorImportant = 0;

	get_images_index(counter);
	sprintf((char *)str, "/sdcard/DCMI/image_%lu.bmp", counter);

	fd = aos_open((const char*)str, O_RDWR | O_CREAT | O_TRUNC);
	if(fd < 0){
		printf("aos_open %s failed\n", str);
		return HAL_ERROR;
		
	}
	ret = aos_write(fd, &bm, sizeof(bm));
	if(ret != sizeof(bm)){
		printf("aos_write bm failed\n");
		ret = HAL_ERROR;
		goto end;
	}
	
	ret = aos_write(fd, &bmpheaher, sizeof(bmpheaher));
	if(ret != sizeof(bmpheaher)){
		printf("aos_write bmpheaher failed\n");
		ret = HAL_ERROR;
		goto end;
	}
	ret = aos_write(fd, &infoheader, sizeof(infoheader));
	if(ret != sizeof(infoheader)){
		printf("aos_write infoheader failed\n");
		ret = HAL_ERROR;
		goto end;
	}

	for(x = 0; x < nHeigh; x++){
		pixel_index = (nHeigh - 1 - x) * nWidth;
		for(y = 0; y < nWidth; y++){
			tmp_data = ((pBuffer[pixel_index + y] >> 8) & 0xff) | ((pBuffer[pixel_index + y] << 8) & 0xff00);
			pBuffer_t[3 * y] = RGB565_TO_R(tmp_data);
			pBuffer_t[3 * y +1] = RGB565_TO_G(tmp_data);
			pBuffer_t[3 * y +2] = RGB565_TO_B(tmp_data);
		}
		ret = aos_write(fd, pBuffer_t, sizeof(pBuffer_t));
		if(ret != sizeof(pBuffer_t)){
			printf("aos_write nHeigh = %d  failed\n", x);
			ret = HAL_ERROR;
			goto end;
		}
	}
	ret = HAL_OK;
	//printf("saved image_%lu.bmp\n", counter++);
	GUIDEMO_DrawBk(1);
	GUI_SetColor(GUI_BLACK);
	GUIDEMO_DrawBk(1);

	// set font
	GUI_SetColor(GUI_WHITE);
  	GUI_SetFont(&GUI_Font20_ASCII);
	GUI_DispStringAt("Saved",  100, 100);

end:
	aos_close(fd);
	if(ret != HAL_OK)
		aos_unlink((const char*)str);
	else{
		krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND);	
		GUI_Exit();
		GUI_Init();
	}
	//printf("ret = %d\n", ret);
	return ret;
}

void camera_to_sd(void)
{
	int times;
	if(read_camera_flag || sd_on == 0)
		return;
	if(key_a_flag == 0)
		return;
	HAL_DCMI_Suspend(phdcmi);
	times = 0;
	while(times++ < 10){
		if(SavePicture() == HAL_OK)
			break;
		else
			krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);	
	}
	HAL_DCMI_Resume(phdcmi);
	key_a_flag = 0;
	read_camera_flag = 1;
}

