/**
  ******************************************************************************
  * @file    stm32l4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
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
#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"
#include "stm32l4xx_it.h"
#include "k_api.h"
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern UART_HandleTypeDef lpuart1_handle;
extern UART_HandleTypeDef uart2_handle;
extern UART_HandleTypeDef uart3_handle;
extern DCMI_HandleTypeDef hdcmi_handle;
extern SD_HandleTypeDef sd_handle;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim16;
extern DMA_HandleTypeDef hdma_sai2_a;
extern SAI_HandleTypeDef hsai_BlockA2;

/******************************************************************************/
/*            Cortex-M4 Processor Interruption and Exception Handlers         */ 
/******************************************************************************/

/**
* @brief This function handles Non maskable Interrupt.
*/
void NMI_Handler(void)
{

}

/**
* @brief This function handles Hard fault interrupt.
*/
void HardFault_Handler(void)
{
  while (1)
  {
  }
}

/**
* @brief This function handles Memory management fault.
*/
void MemManage_Handler(void)
{
  while (1)
  {
  }
}

/**
* @brief This function handles Prefetch fault, memory access fault.
*/
void BusFault_Handler(void)
{
  while (1)
  {
  }
}

/**
* @brief This function handles Undefined instruction or illegal state.
*/
void UsageFault_Handler(void)
{
  while (1)
  {
  }
}

/**
* @brief This function handles System service call via SWI instruction.
*/
void SVC_Handler(void)
{

}

/**
* @brief This function handles Debug monitor.
*/
void DebugMon_Handler(void)
{

}

/**
* @brief This function handles System tick timer.
*/
void SysTick_Handler(void)
{
  HAL_IncTick();
  krhino_intrpt_enter();
  krhino_tick_proc();
  krhino_intrpt_exit();
  //HAL_SYSTICK_IRQHandler();
}

/******************************************************************************/
/* STM32L4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32l4xx.s).                    */
/******************************************************************************/

/**
* @brief This function handles LPUART1 global interrupt.
*/
void LPUART1_IRQHandler(void)
{
  krhino_intrpt_enter();
  HAL_UART_IRQHandler(&lpuart1_handle);
  krhino_intrpt_exit();
}

/**
* @brief This function handles USART2 global interrupt.
*/
void USART2_IRQHandler(void)
{
  krhino_intrpt_enter();
  HAL_UART_IRQHandler(&uart2_handle);
  krhino_intrpt_exit();
}

/**
* @brief This function handles USART3 global interrupt.
*/
void USART3_IRQHandler(void)
{
  krhino_intrpt_enter();
  HAL_UART_IRQHandler(&uart3_handle);
  krhino_intrpt_exit();
}

/**
* @brief This function handles EXTI line[9:5] interrupts.
*/
void EXTI9_5_IRQHandler(void)
{
  krhino_intrpt_enter();
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);
  krhino_intrpt_exit();
}

/**
* @brief This function handles EXTI line[15:10] interrupts.
*/
void EXTI15_10_IRQHandler(void)
{
  krhino_intrpt_enter();
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
  krhino_intrpt_exit();
}

/**
* @brief This function handles DMA2 channel6 global interrupt.
*/
void DMA2_Channel6_IRQHandler(void)
{
  krhino_intrpt_enter();
  HAL_DMA_IRQHandler(hdcmi_handle.DMA_Handle);
  krhino_intrpt_exit();
}

/**
* @brief This function handles DCMI global interrupt.
*/
void DCMI_IRQHandler(void)
{
  krhino_intrpt_enter();
  HAL_DCMI_IRQHandler(&hdcmi_handle);
  krhino_intrpt_exit();
}

void SDMMC1_IRQHandler(void)
{
  krhino_intrpt_enter();
  HAL_SD_IRQHandler(&sd_handle);
  krhino_intrpt_exit();
}

/**
* @brief This function handles TIM1 update interrupt and TIM16 global interrupt.
*/
void TIM1_UP_TIM16_IRQHandler(void)
{
  /* This interrupt is time critical for IRDA */
  //krhino_intrpt_enter();
  HAL_TIM_IRQHandler(&htim1);
  HAL_TIM_IRQHandler(&htim16);
  //krhino_intrpt_exit();
}

/**
* @brief This function handles TIM1 capture compare interrupt.
*/
void TIM1_CC_IRQHandler(void)
{
  /* This interrupt is time critical for IRDA */
  //krhino_intrpt_enter();
  HAL_TIM_IRQHandler(&htim1);
  //krhino_intrpt_exit();
}

/**
* @brief This function handles DMA1 channel6 global interrupt.
*/
void DMA1_Channel6_IRQHandler(void)
{
  krhino_intrpt_enter();
  HAL_DMA_IRQHandler(&hdma_sai2_a);
  krhino_intrpt_exit();
}

/**
* @brief This function handles SAI2 global interrupt.
*/
void SAI2_IRQHandler(void)
{
  krhino_intrpt_enter();
  HAL_SAI_IRQHandler(&hsai_BlockA2);
  krhino_intrpt_exit();
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
