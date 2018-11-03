/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "soc_init.h"
#include "k_api.h"
#include "stm32l4xx_hal.h"
#include "GUI_init.h"
#include "drv_baro_bosch_bmp280.h"
#include "drv_als_liteon_ltr553.h"
#include "drv_ps_liteon_ltr553.h"
#include "drv_humi_sensirion_shtc1.h"
#include "drv_acc_st_lsm6dsl.h"
#include "drv_gyro_st_lsm6dsl.h"
#include "drv_temp_st_lsm6dsl.h"
#include "drv_mag_memsic_mmc3680kj.h"
#include "drv_temp_memsic_mmc3680kj.h"
#include "irda.h"

#if defined (__CC_ARM) && defined(__MICROLIB)
void __aeabi_assert(const char *expr, const char *file, int line)
{
    while (1);
}
#endif

#if defined (__CC_ARM) && defined(__MICROLIB)
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#define GETCHAR_PROTOTYPE int fgetc(FILE *f)
#elif defined(__ICCARM__)
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#define GETCHAR_PROTOTYPE int fgetc(FILE *f)
#else
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#define GETCHAR_PROTOTYPE int __io_getchar(void)
#endif /* defined (__CC_ARM) && defined(__MICROLIB) */


/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
#ifndef ALIOS_HAL
UART_HandleTypeDef hlpuart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
I2C_HandleTypeDef hi2c2;
I2C_HandleTypeDef hi2c3;
I2C_HandleTypeDef hi2c4;
#endif
SPI_HandleTypeDef hspi1;
SAI_HandleTypeDef hsai_BlockA2;
DMA_HandleTypeDef hdma_sai2_a;
ADC_HandleTypeDef hadc3;
//DCMI_HandleTypeDef hdcmi;
DCMI_HandleTypeDef hdcmi_handle;
CRC_HandleTypeDef hcrc;
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim16;
TIM_HandleTypeDef htim17;
SMARTCARD_HandleTypeDef hsmartcard2;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
#ifndef ALIOS_HAL
static void MX_GPIO_Init(void);
static void MX_LPUART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_I2C2_Init(void);
static void MX_I2C3_Init(void);
static void MX_I2C4_Init(void);
#else
static void brd_peri_init(void);
#endif
/*xiehj add*/
static void MX_DMA_Init(void);
static void MX_ADC3_Init(void);
static void MX_DCMI_Init(void);
static void MX_SAI2_Init(void);
static void MX_SDMMC1_Init(void);
static void MX_SPI1_Init(void);
static void MX_USB_OTG_FS_USB_Init(void);
static void MX_CRC_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM17_Init(void);
static void MX_TIM16_Init(void);
static void MX_IRTIM_Init(void);
static void Hal_Smartcard_Init(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

void stm32_soc_init(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
#ifdef ALIOS_HAL
  brd_peri_init();
#endif
#ifndef ALIOS_HAL
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_I2C3_Init();
  MX_LPUART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
#endif
  /*xiehj add DMA2 config*/
#if !defined(NB_MOUDLE) && !defined(LORA_MODULE)
  Hal_Smartcard_Init();
#endif
  MX_DMA_Init();
  MX_ADC3_Init();
  MX_DCMI_Init();
  MX_SAI2_Init();
  MX_SDMMC1_Init();
  MX_SPI1_Init();
  MX_USB_OTG_FS_USB_Init();
  MX_CRC_Init();
  MX_TIM1_Init();
  MX_TIM17_Init();
  MX_TIM16_Init();
  MX_IRTIM_Init();
	
  audio_init();
  isd9160_i2c_init();

  /* Initialize LCD and LEDs */
  BSP_GUI_init();
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Configure LSE Drive Capability 
    */
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
    /*xiehj add RCC_OSCILLATORTYPE_HSI48*/
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  /*xiehj add*/
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_USART3|RCC_PERIPHCLK_UART4
                              |RCC_PERIPHCLK_LPUART1|RCC_PERIPHCLK_SAI2
                              |RCC_PERIPHCLK_I2C2|RCC_PERIPHCLK_I2C3
                              |RCC_PERIPHCLK_I2C4|RCC_PERIPHCLK_USB
                              |RCC_PERIPHCLK_SDMMC1|RCC_PERIPHCLK_ADC;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
  PeriphClkInit.Uart4ClockSelection = RCC_UART4CLKSOURCE_PCLK1;
  PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;
  PeriphClkInit.I2c2ClockSelection = RCC_I2C2CLKSOURCE_PCLK1;
  PeriphClkInit.I2c3ClockSelection = RCC_I2C3CLKSOURCE_PCLK1;
  PeriphClkInit.I2c4ClockSelection = RCC_I2C4CLKSOURCE_PCLK1;
  PeriphClkInit.Sai2ClockSelection = RCC_SAI2CLKSOURCE_PLLSAI1;
  PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_PLLSAI1;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLLSAI1;
  PeriphClkInit.Sdmmc1ClockSelection = RCC_SDMMC1CLKSOURCE_PLLSAI1;
  PeriphClkInit.PLLSAI1.PLLSAI1Source = RCC_PLLSOURCE_MSI;
  PeriphClkInit.PLLSAI1.PLLSAI1M = 1;
  PeriphClkInit.PLLSAI1.PLLSAI1N = 24;
  PeriphClkInit.PLLSAI1.PLLSAI1P = RCC_PLLP_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1Q = RCC_PLLQ_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1R = RCC_PLLR_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_SAI1CLK|RCC_PLLSAI1_48M2CLK
                              |RCC_PLLSAI1_ADC1CLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  /*xiehj add*/
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI48, RCC_MCODIV_2);
  
    /**Configure the main internal regulator output voltage 
    */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/8/RHINO_CONFIG_TICKS_PER_SECOND);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK_DIV8);

    /**Enable MSI Auto calibration 
    */
  HAL_RCCEx_EnableMSIPLLMode();

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, TICK_INT_PRIORITY, 0);
}

#ifdef ALIOS_HAL
static gpio_irq_trigger_t mode_rising = IRQ_TRIGGER_RISING_EDGE;
static gpio_irq_trigger_t mode_falling = IRQ_TRIGGER_FALLING_EDGE;
static gpio_irq_trigger_t mode_both = IRQ_TRIGGER_BOTH_EDGES;
static uint8_t gpio_set = 1;
static uint8_t gpio_reset = 0;

gpio_dev_t brd_gpio_table[] = {
	{ALS_INT, IRQ_MODE, &mode_rising},
	{ALS_LED, OUTPUT_PUSH_PULL, &gpio_reset},
	{AUDIO_CTL, OUTPUT_PUSH_PULL, &gpio_reset},
	{AUDIO_RST, OUTPUT_PUSH_PULL, &gpio_set},
	{AUDIO_WU, OUTPUT_PUSH_PULL, &gpio_set},
	{CAM_PD, OUTPUT_PUSH_PULL, &gpio_set},
	{CAM_RST, OUTPUT_PUSH_PULL, &gpio_set},
	{COMPASS_LED, OUTPUT_PUSH_PULL, &gpio_reset},
	{GS_LED, OUTPUT_PUSH_PULL, &gpio_reset},
	{HTS_LED, OUTPUT_PUSH_PULL, &gpio_set},
	{KEY_1, IRQ_MODE, &mode_rising},
	{KEY_2, IRQ_MODE, &mode_rising},
	{KEY_3, IRQ_MODE, &mode_rising},
	{LCD_DCX, OUTPUT_PUSH_PULL, &gpio_set},
	{LCD_PWR, OUTPUT_PUSH_PULL, &gpio_reset},
	{LCD_RST, OUTPUT_PUSH_PULL, &gpio_set},
	{PCIE_RST, OUTPUT_PUSH_PULL, &gpio_set},
	{PS_LED, OUTPUT_PUSH_PULL, &gpio_set},
	//{SECURE_CLK, OUTPUT_PUSH_PULL, &gpio_set},
	//{SECURE_IO, OUTPUT_PUSH_PULL, &gpio_set},
	//{SECURE_RST, OUTPUT_PUSH_PULL, &gpio_set},
	{SIM_DET, INPUT_HIGH_IMPEDANCE, NULL},
	{USB_PCIE_SW, OUTPUT_PUSH_PULL, &gpio_set},
	{WIFI_RST, OUTPUT_PUSH_PULL, &gpio_set},
	{WIFI_WU, OUTPUT_PUSH_PULL, &gpio_set},
	#ifdef LORA_MODULE
	{ZIGBEE_INT, OUTPUT_PUSH_PULL, &gpio_set},
	#endif
  #if defined(NB_MOUDLE)
  // {ZIGBEE_INT, IRQ_MODE, &mode_rising},
	{ZIGBEE_RST, OUTPUT_PUSH_PULL, &gpio_set},
	#else
	{SE_RST, OUTPUT_PUSH_PULL, &gpio_reset},
	#endif
};

i2c_dev_t brd_i2c2_dev = {AOS_PORT_I2C2, {0}, NULL};
i2c_dev_t brd_i2c3_dev = {AOS_PORT_I2C3, {0}, NULL};
i2c_dev_t brd_i2c4_dev = {AOS_PORT_I2C4, {0}, NULL};
/* brd_lpuart1_dev compat for vfs.c which will be compiled in Travis Ci script */
uart_dev_t uart_0 = {
	PORT_LPUART1,
	{LPUART1_BANDRATE, DATA_WIDTH_8BIT, NO_PARITY, STOP_BITS_1, FLOW_CONTROL_DISABLED, MODE_TX_RX},
	NULL
};
uart_dev_t brd_uart2_dev = {
	PORT_UART2,
	{UART2_BANDRATE, DATA_WIDTH_8BIT, NO_PARITY, STOP_BITS_1, FLOW_CONTROL_DISABLED, MODE_TX_RX},
	NULL
};
uart_dev_t brd_uart3_dev = {
	PORT_UART3,
	{UART3_BANDRATE, DATA_WIDTH_8BIT, NO_PARITY, STOP_BITS_1, FLOW_CONTROL_DISABLED, MODE_TX_RX},
	NULL
};
uart_dev_t brd_uart4_dev = {
	PORT_UART4,
	{UART4_BANDRATE, DATA_WIDTH_8BIT, NO_PARITY, STOP_BITS_1, FLOW_CONTROL_DISABLED, MODE_TX_RX},
	NULL
};
static void brd_peri_init(void)
{
	int i;
	int gpcfg_num = sizeof(brd_gpio_table) / sizeof(brd_gpio_table[0]);

	for (i = 0; i < gpcfg_num; ++i) {
		hal_gpio_init(&brd_gpio_table[i]);
	}
	hal_uart_init(&uart_0);
  #if defined(LORA_MODULE) || defined(NB_MOUDLE)
	hal_uart_init(&brd_uart2_dev);
  #endif
	hal_uart_init(&brd_uart3_dev);
	hal_uart_init(&brd_uart4_dev);
	hal_i2c_init(&brd_i2c2_dev);
	hal_i2c_init(&brd_i2c3_dev);
	hal_i2c_init(&brd_i2c4_dev);
}
#endif

#ifndef ALIOS_HAL
/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
     PC8   ------> SDMMC1_D0
     PC9   ------> SDMMC1_D1
     PA8   ------> RCC_MCO
     PA10   ------> USB_OTG_FS_ID
     PA11   ------> USB_OTG_FS_DM
     PA12   ------> USB_OTG_FS_DP
     PC10   ------> SDMMC1_D2
     PC11   ------> SDMMC1_D3
     PC12   ------> SDMMC1_CK
     PD2   ------> SDMMC1_CMD
     PB9   ------> IR_OUT
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, GS_LED_Pin|CAM_PD_Pin|SECURE_CLK_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(PCIE_RST_GPIO_Port, PCIE_RST_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, SECURE_IO_Pin|SECURE_RST_Pin|LCD_DCX_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, WIFI_RST_Pin|WIFI_WU_Pin|LCD_RST_Pin|USB_PCIE_SW_Pin 
                          |ALS_LED_Pin|CAM_RST_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, LCD_PWR_Pin|SE_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, HTS_LED_Pin|PS_LED_Pin|COMPASS_LED_Pin|AUDIO_WU_Pin 
                          |AUDIO_RST_Pin|ZIGBEE_RST_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(AUDIO_CTL_GPIO_Port, AUDIO_CTL_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : SIM_DET_Pin */
  GPIO_InitStruct.Pin = SIM_DET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SIM_DET_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : GS_LED_Pin LCD_PWR_Pin CAM_PD_Pin SECURE_CLK_Pin */
  GPIO_InitStruct.Pin = GS_LED_Pin|LCD_PWR_Pin|CAM_PD_Pin|SECURE_CLK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : PCIE_RST_Pin */
  GPIO_InitStruct.Pin = PCIE_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(PCIE_RST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SECURE_IO_Pin SECURE_RST_Pin LCD_DCX_Pin */
  GPIO_InitStruct.Pin = SECURE_IO_Pin|SECURE_RST_Pin|LCD_DCX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : WIFI_RST_Pin WIFI_WU_Pin LCD_RST_Pin USB_PCIE_SW_Pin 
                           ALS_LED_Pin CAM_RST_Pin */
  GPIO_InitStruct.Pin = WIFI_RST_Pin|WIFI_WU_Pin|LCD_RST_Pin|USB_PCIE_SW_Pin 
                          |ALS_LED_Pin|CAM_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : ZIGBEE_INT_Pin KEY_3_Pin KEY_1_Pin KEY_2_Pin */
  GPIO_InitStruct.Pin = ZIGBEE_INT_Pin|KEY_3_Pin|KEY_1_Pin|KEY_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : HTS_LED_Pin PS_LED_Pin COMPASS_LED_Pin AUDIO_WU_Pin 
                           AUDIO_CTL_Pin AUDIO_RST_Pin ZIGBEE_RST_Pin */
  GPIO_InitStruct.Pin = HTS_LED_Pin|PS_LED_Pin|COMPASS_LED_Pin|AUDIO_WU_Pin 
                          |AUDIO_CTL_Pin|AUDIO_RST_Pin|ZIGBEE_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : PC8 PC9 PC10 PC11 
                           PC12 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11 
                          |GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA10 PA11 PA12 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : ALS_INT_Pin */
  GPIO_InitStruct.Pin = ALS_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ALS_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PD2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : IR_OUT_Pin */
  GPIO_InitStruct.Pin = IR_OUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF1_IR;
  HAL_GPIO_Init(IR_OUT_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* LPUART1 init function */
static void MX_LPUART1_UART_Init(void)
{

  hlpuart1.Instance = LPUART1;
  hlpuart1.Init.BaudRate = 115200;
  hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
  hlpuart1.Init.StopBits = UART_STOPBITS_1;
  hlpuart1.Init.Parity = UART_PARITY_NONE;
  hlpuart1.Init.Mode = UART_MODE_TX_RX;
  hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&hlpuart1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* USART2 init function */
static void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* USART3 init function */
static void MX_USART3_UART_Init(void)
{

  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* I2C2 init function */
static void MX_I2C2_Init(void)
{

  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x10909CEC;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Analogue filter 
    */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Digital filter 
    */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* I2C3 init function */
static void MX_I2C3_Init(void)
{

  hi2c3.Instance = I2C3;
  hi2c3.Init.Timing = 0x10909CEC;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Analogue filter 
    */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Digital filter 
    */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* I2C4 init function */
static void MX_I2C4_Init(void)
{

  hi2c4.Instance = I2C4;
  hi2c4.Init.Timing = 0x10909CEC;
  hi2c4.Init.OwnAddress1 = 0;
  hi2c4.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c4.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c4.Init.OwnAddress2 = 0;
  hi2c4.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c4.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c4.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Analogue filter 
    */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c4, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Digital filter 
    */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c4, 0) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}
#endif

static void Hal_Smartcard_Init(void)
{
  /* USART Clock set to 4 MHz (PCLK1 (80 MHz) / 20) => prescaler set to 10 */
  hsmartcard2.Instance = USART2;
  hsmartcard2.Init.BaudRate = 10752;//10752; /*4MHz / 372*/
  hsmartcard2.Init.WordLength = SMARTCARD_WORDLENGTH_9B;
  hsmartcard2.Init.StopBits = SMARTCARD_STOPBITS_1_5;
  hsmartcard2.Init.Parity = SMARTCARD_PARITY_EVEN;
  hsmartcard2.Init.Mode = SMARTCARD_MODE_TX_RX;
  hsmartcard2.Init.CLKPolarity = SMARTCARD_POLARITY_LOW;
  hsmartcard2.Init.CLKPhase = SMARTCARD_PHASE_1EDGE;
  hsmartcard2.Init.CLKLastBit = SMARTCARD_LASTBIT_ENABLE;
  hsmartcard2.Init.OneBitSampling = SMARTCARD_ONE_BIT_SAMPLE_DISABLE;
  hsmartcard2.Init.Prescaler = 10;
  hsmartcard2.Init.GuardTime = 1;
  hsmartcard2.Init.NACKEnable = SMARTCARD_NACK_ENABLE;
  hsmartcard2.Init.TimeOutEnable = SMARTCARD_TIMEOUT_DISABLE;
  hsmartcard2.Init.BlockLength = 0;
  hsmartcard2.Init.AutoRetryCount = 3;
  hsmartcard2.AdvancedInit.AdvFeatureInit = SMARTCARD_ADVFEATURE_NO_INIT;
  if (HAL_SMARTCARD_Init(&hsmartcard2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
    //return HAL_ERROR;
  }
  //printf("HAL_SMARTCARD_Init ok\n");
  //krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/100);
  //SC_Reset(GPIO_PIN_SET);
  
  //return HAL_OK;
}
/* ADC3 init function */
static void MX_ADC3_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Common config 
    */
  hadc3.Instance = ADC3;
  hadc3.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc3.Init.Resolution = ADC_RESOLUTION_12B;
  hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc3.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc3.Init.LowPowerAutoWait = DISABLE;
  hadc3.Init.ContinuousConvMode = DISABLE;
  hadc3.Init.NbrOfConversion = 1;
  hadc3.Init.DiscontinuousConvMode = DISABLE;
  hadc3.Init.NbrOfDiscConversion = 1;
  hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc3.Init.DMAContinuousRequests = DISABLE;
  hadc3.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc3.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_DIFFERENTIAL_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
  /* DMA2_Channel6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Channel6_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA2_Channel6_IRQn);

}

/* DCMI init function */
static void MX_DCMI_Init(void)
{
  DCMI_HandleTypeDef *phdcmi_handle;

  /* Get the DCMI handle structure */
  phdcmi_handle = &hdcmi_handle;

  phdcmi_handle->Instance = DCMI;
  phdcmi_handle->Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;
  phdcmi_handle->Init.PCKPolarity = DCMI_PCKPOLARITY_RISING;
  phdcmi_handle->Init.VSPolarity = DCMI_VSPOLARITY_LOW;
  //phdcmi_handle->Init.VSPolarity = DCMI_VSPOLARITY_HIGH;
  phdcmi_handle->Init.HSPolarity = DCMI_HSPOLARITY_LOW;
  //phdcmi_handle->Init.HSPolarity = DCMI_HSPOLARITY_HIGH;
  phdcmi_handle->Init.CaptureRate = DCMI_CR_ALL_FRAME;
  phdcmi_handle->Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
  phdcmi_handle->Init.JPEGMode = DCMI_JPEG_DISABLE;
  phdcmi_handle->Init.ByteSelectMode = DCMI_BSM_ALL;
  phdcmi_handle->Init.ByteSelectStart = DCMI_OEBS_ODD;
  phdcmi_handle->Init.LineSelectMode = DCMI_LSM_ALL;
  phdcmi_handle->Init.LineSelectStart = DCMI_OELS_ODD;

  if (HAL_DCMI_Init(phdcmi_handle) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* SAI2 init function */
static void MX_SAI2_Init(void)
{

  hsai_BlockA2.Instance = SAI2_Block_A;
  hsai_BlockA2.Init.Protocol = SAI_FREE_PROTOCOL;
  hsai_BlockA2.Init.AudioMode = SAI_MODEMASTER_RX;
  hsai_BlockA2.Init.DataSize = SAI_DATASIZE_16;
  hsai_BlockA2.Init.FirstBit = SAI_FIRSTBIT_MSB;
  hsai_BlockA2.Init.ClockStrobing = SAI_CLOCKSTROBING_RISINGEDGE;
  hsai_BlockA2.Init.Synchro = SAI_ASYNCHRONOUS;
  hsai_BlockA2.Init.OutputDrive = SAI_OUTPUTDRIVE_ENABLE;
  hsai_BlockA2.Init.NoDivider = SAI_MASTERDIVIDER_ENABLE;
  hsai_BlockA2.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_EMPTY;
  hsai_BlockA2.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_16K;
  hsai_BlockA2.Init.SynchroExt = SAI_SYNCEXT_DISABLE;
  hsai_BlockA2.Init.MonoStereoMode = SAI_STEREOMODE;
  hsai_BlockA2.Init.CompandingMode = SAI_NOCOMPANDING;
  hsai_BlockA2.FrameInit.FrameLength = 32;
  hsai_BlockA2.FrameInit.ActiveFrameLength = 16;
  hsai_BlockA2.FrameInit.FSDefinition = SAI_FS_CHANNEL_IDENTIFICATION;
  hsai_BlockA2.FrameInit.FSPolarity = SAI_FS_ACTIVE_LOW;
  hsai_BlockA2.FrameInit.FSOffset = SAI_FS_BEFOREFIRSTBIT;
  hsai_BlockA2.SlotInit.FirstBitOffset = 0;
  hsai_BlockA2.SlotInit.SlotSize = SAI_SLOTSIZE_DATASIZE;
  hsai_BlockA2.SlotInit.SlotNumber = 2;
  hsai_BlockA2.SlotInit.SlotActive = 0x0000FFFF;
  if (HAL_SAI_Init(&hsai_BlockA2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* SDMMC1 init function */
static void MX_SDMMC1_Init(void)
{

}

/* SPI1 init function */
static void MX_SPI1_Init(void)
{

  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_1LINE;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_HARD_OUTPUT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* USB_OTG_FS init function */
static void MX_USB_OTG_FS_USB_Init(void)
{

}

/* CRC init function */
static void MX_CRC_Init(void)
{

  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM1 init function */
static void MX_TIM1_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_SlaveConfigTypeDef sSlaveConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_IC_InitTypeDef sConfigIC;

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 79;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 0xffff;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_IC_Init(&htim1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
  sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;
  sSlaveConfig.TriggerPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
  sSlaveConfig.TriggerFilter = 0;
  if (HAL_TIM_SlaveConfigSynchronization(&htim1, &sSlaveConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim1, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_INDIRECTTI;
  if (HAL_TIM_IC_ConfigChannel(&htim1, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM16 init function */
static void MX_TIM16_Init(void)
{

  TIM_OC_InitTypeDef sConfigOC;
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;

  htim16.Instance = TIM16;
  htim16.Init.Prescaler = 0;
  htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim16.Init.Period = 0;
  htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim16.Init.RepetitionCounter = 0;
  htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim16) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_OC_Init(&htim16) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_TIMING;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_OC_ConfigChannel(&htim16, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim16, &sBreakDeadTimeConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM17 init function */
static void MX_TIM17_Init(void)
{

  TIM_OC_InitTypeDef sConfigOC;
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;

  htim17.Instance = TIM17;
  htim17.Init.Prescaler = 0;
  htim17.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim17.Init.Period = 2105;
  htim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim17.Init.RepetitionCounter = 0;
  htim17.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim17) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_OC_Init(&htim17) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 1052;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_OC_ConfigChannel(&htim17, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim17, &sBreakDeadTimeConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* IRTIM init function */
static void MX_IRTIM_Init(void)
{
	irda_init();
}

/* USER CODE BEGIN 4 */

int sensor_brd_init(void)
{
	int ret = 0;

	ret |= drv_baro_bosch_bmp280_init(&brd_i2c2_dev, BARO_BOSCH_BMP280_ADDR_HIGH);
	ret |= drv_als_liteon_ltr553_init(&brd_i2c2_dev);
	ret |= drv_ps_liteon_ltr553_init(&brd_i2c2_dev);
	ret |= drv_humi_sensirion_shtc1_init(&brd_i2c2_dev);
	ret |= drv_temp_sensirion_shtc1_init(&brd_i2c2_dev);
	ret |= drv_acc_st_lsm6dsl_init(&brd_i2c4_dev, ACC_ST_LSM6DSL_ADDR_HIGH);
	ret |= drv_gyro_st_lsm6dsl_init(&brd_i2c4_dev, GYRO_ST_LSM6DSL_ADDR_HIGH);
	ret |= drv_mag_memsic_mmc3680kj_init(&brd_i2c4_dev);

	return ret;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
  /*Configure the SysTick to have interrupt in 1ms time basis*/
  HAL_SYSTICK_Config(SystemCoreClock/RHINO_CONFIG_TICKS_PER_SECOND);

  /*Configure the SysTick IRQ priority */
  HAL_NVIC_SetPriority(SysTick_IRQn, TickPriority ,0U);

   /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
#ifdef ALIOS_HAL
  if (ch == '\n') {
    hal_uart_send(&console_uart, (void *)"\r", 1, 30000);
  }
  hal_uart_send(&console_uart, (void *)&ch, 1, 30000);
  if (ch == '\r') {
    hal_uart_send(&console_uart, (void *)"\n", 1, 30000);
  }
#else
  if (ch == '\n') {
    HAL_UART_Transmit(&hlpuart1, (void *)"\r", 1, 30000);
  }
  HAL_UART_Transmit(&hlpuart1, (uint8_t *)&ch, 1, 30000);
#endif
  return ch;
}

/**
  * @brief  Retargets the C library scanf function to the USART.
  * @param  None
  * @retval None
  */
GETCHAR_PROTOTYPE
{
  /* Place your implementation of fgetc here */
  /* e.g. readwrite a character to the USART2 and Loop until the end of transmission */
  uint8_t ch = 0;
#ifdef ALIOS_HAL
  uint32_t size = 0;
  do {
    hal_uart_recv(&console_uart, (void *)&ch, 1, &size, 30000);
  } while (size != 1);
#else
  HAL_UART_Receive(&hlpuart1, &ch, 1, 30000);
#endif
  return ch;
}

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
