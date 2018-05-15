/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include "hal.h"
#include "soc_init.h"
#include "k_types.h"
#include "errno.h"
#include "hal_i2c_stm32l4.h"

/* Init and deInit function for i2c1 */
static void I2C1_Init(void);
static void I2C1_DeInit(void);
static void I2C1_MspInit(I2C_HandleTypeDef *hi2c);
static void I2C1_MspDeInit(I2C_HandleTypeDef *hi2c);

/* Init and deInit function for i2c2 */
static void I2C2_Init(void);
static void I2C2_DeInit(void);
static void I2C2_MspInit(I2C_HandleTypeDef *hi2c);
static void I2C2_MspDeInit(I2C_HandleTypeDef *hi2c);

/* Init and deInit function for i2c3 */
static void I2C3_Init(void);
static void I2C3_DeInit(void);
static void I2C3_MspInit(I2C_HandleTypeDef *hi2c);
static void I2C3_MspDeInit(I2C_HandleTypeDef *hi2c);

/* Init and deInit function for i2c4 */
static void I2C4_Init(void);
static void I2C4_DeInit(void);
static void I2C4_MspInit(I2C_HandleTypeDef *hi2c);
static void I2C4_MspDeInit(I2C_HandleTypeDef *hi2c);

/* handle for i2c */
static I2C_HandleTypeDef I2c1Handle = {0};
static I2C_HandleTypeDef I2c2Handle = {0};
static I2C_HandleTypeDef I2c3Handle = {0};
static I2C_HandleTypeDef I2c4Handle = {0};

int32_t hal_i2c_init(i2c_dev_t *i2c)
{
    int32_t ret = -1;

    if (i2c == NULL) {
       return -1;
    }

    switch (i2c->port) {
        case AOS_PORT_I2C1:
            I2C1_Init();
            i2c->priv = &I2c1Handle;
            ret = 0;
            break;
        case AOS_PORT_I2C2:
            I2C2_Init();
            i2c->priv = &I2c2Handle;
            ret = 0;
            break;
        case AOS_PORT_I2C3:
            I2C3_Init();
            i2c->priv = &I2c3Handle;
            ret = 0;
            break;
        case AOS_PORT_I2C4:
            I2C4_Init();
            i2c->priv = &I2c4Handle;
            ret = 0;
            break;
        default:
            break;
    }

    return ret;
}

int32_t hal_i2c_master_send(i2c_dev_t *i2c, uint16_t dev_addr, const uint8_t *data,
                            uint16_t size, uint32_t timeout)
{
    int ret = -1;

    if ((i2c != NULL) && (data != NULL)) {
        ret = HAL_I2C_Master_Transmit((I2C_HandleTypeDef*)(i2c->priv), dev_addr,
              (uint8_t *)data, size, timeout);
    }

    return ret;
}

int32_t hal_i2c_master_recv(i2c_dev_t *i2c, uint16_t dev_addr, uint8_t *data,
                            uint16_t size, uint32_t timeout)
{
    int ret = -1;

    if ((i2c != NULL) && (data != NULL)) {
        ret = HAL_I2C_Master_Receive((I2C_HandleTypeDef*)(i2c->priv), dev_addr,
              data, size, timeout);
    }

    return ret;
}

int32_t hal_i2c_slave_send(i2c_dev_t *i2c, const uint8_t *data, uint16_t size, uint32_t timeout)
{
    int ret = -1;

    if ((i2c != NULL) && (data != NULL)) {
        ret = HAL_I2C_Slave_Transmit((I2C_HandleTypeDef*)(i2c->priv), (uint8_t *)data,
              size, timeout);
    }

    return ret;
}

int32_t hal_i2c_slave_recv(i2c_dev_t *i2c, uint8_t *data, uint16_t size, uint32_t timeout)
{
    int ret = -1;

    if ((i2c != NULL) && (data != NULL)) {
        ret = HAL_I2C_Slave_Receive((I2C_HandleTypeDef*)(i2c->priv), data, size, timeout);
    }

    return ret;
}

int32_t hal_i2c_mem_write(i2c_dev_t *i2c, uint16_t dev_addr, uint16_t mem_addr,
                          uint16_t mem_addr_size, const uint8_t *data, uint16_t size,
                          uint32_t timeout)
{
    int ret = -1;

    if ((i2c != NULL) && (data != NULL)) {
        ret = HAL_I2C_Mem_Write((I2C_HandleTypeDef*)(i2c->priv), dev_addr, mem_addr,
              (uint16_t)mem_addr_size, (uint8_t *)data, size, timeout);
    }

    return ret;
};

int32_t hal_i2c_mem_read(i2c_dev_t *i2c, uint16_t dev_addr, uint16_t mem_addr,
                         uint16_t mem_addr_size, uint8_t *data, uint16_t size,
                         uint32_t timeout)
{
    int ret = -1;

    if ((i2c != NULL) && (data != NULL)) {
        ret = HAL_I2C_Mem_Read((I2C_HandleTypeDef*)(i2c->priv), dev_addr, mem_addr,
              (uint16_t)mem_addr_size, data, size, timeout);
    }

    return ret;
};

int32_t hal_i2c_finalize(i2c_dev_t *i2c)
{
    int32_t ret = -1;

    if (i2c == NULL) {
        return -1;
    }

    switch (i2c->port) {
        case AOS_PORT_I2C1:
            I2C1_DeInit();
            ret = 0;
            break;
        case AOS_PORT_I2C2:
            I2C2_DeInit();
            ret = 0;
            break;
        case AOS_PORT_I2C3:
            I2C3_DeInit();
            ret = 0;
            break;
        case AOS_PORT_I2C4:
            I2C4_DeInit();
            ret = 0;
            break;
        default:
            break;
    }

    return ret;
}

void I2C1_Init(void)
{
    if (HAL_I2C_GetState(&I2c1Handle) == HAL_I2C_STATE_RESET) {
        I2c1Handle.Instance              = I2C1_INSTANCE;
        I2c1Handle.Init.Timing           = I2C1_TIMING;
        I2c1Handle.Init.OwnAddress1      = I2C1_OWN_ADDRESS1;
        I2c1Handle.Init.AddressingMode   = I2C1_ADDRESSING_MODE;
        I2c1Handle.Init.DualAddressMode  = I2C1_DUAL_ADDRESS_MODE;
        I2c1Handle.Init.OwnAddress2      = I2C1_OWNADDRESS2;
        I2c1Handle.Init.GeneralCallMode  = I2C1_GENERAL_CALL_MODE;
        I2c1Handle.Init.NoStretchMode    = I2C1_NO_STRETCH_MODE;

        /* Init the I2C */
        I2C1_MspInit(&I2c1Handle);
        HAL_I2C_Init(&I2c1Handle);
    }
}

void I2C1_DeInit(void)
{
    if (HAL_I2C_GetState(&I2c1Handle) != HAL_I2C_STATE_RESET) {
        /* DeInit the I2C */
        HAL_I2C_DeInit(&I2c1Handle);
        I2C1_MspDeInit(&I2c1Handle);
    }
}

static void I2C1_MspInit(I2C_HandleTypeDef *hi2c)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_PeriphCLKInitTypeDef  RCC_PeriphCLKInitStruct;

    if (hi2c->Instance == I2C1_INSTANCE) {
        /*##-1- Configure the Discovery I2C1 clock source. The clock is derived from the SYSCLK #*/
        RCC_PeriphCLKInitStruct.PeriphClockSelection = I2C1_RCC_PERIPH_CLOCK_SELECTION;
        RCC_PeriphCLKInitStruct.I2c2ClockSelection = I2C1_RCC_CLOCK_SELECTION;
        HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);

        /* -2- Configure the GPIOs */
        /* Enable GPIO clock */
        I2C1_SDA_GPIO_CLK_ENABLE();
        I2C1_SCL_GPIO_CLK_ENABLE();

        /* Configure I2C Rx/Tx as alternate function  */
        GPIO_InitStructure.Pin       = I2C1_GPIO_SCL_PIN;
        GPIO_InitStructure.Mode      = I2C1_GPIO_MODE;
        GPIO_InitStructure.Pull      = I2C1_GPIO_PULL;
        GPIO_InitStructure.Speed     = I2C1_GPIO_SPEED;
        GPIO_InitStructure.Alternate = I2C1_GPIO_ALTERNATE;
        HAL_GPIO_Init(I2C1_GPIO_SCL_PORT, &GPIO_InitStructure);
        GPIO_InitStructure.Pin       = I2C1_GPIO_SDA_PIN;
        HAL_GPIO_Init(I2C1_GPIO_SDA_PORT, &GPIO_InitStructure);

        /* -3- Configure the Discovery I2C1 peripheral */
        /* Enable Discovery_I2C1 clock */
        I2C1_CLK_ENABLE();

        /* Force and release the I2C Peripheral Clock Reset */
        I2C1_FORCE_RESET();
        I2C1_RELEASE_RESET();

        /* Enable and set Discovery I2C1 Interrupt to the highest priority */
        HAL_NVIC_SetPriority(I2C1_EV_IRQn, I2C_IRQ_PRIORITY, 0);
        HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);

        /* Enable and set Discovery I2C1 Interrupt to the highest priority */
        HAL_NVIC_SetPriority(I2C1_ER_IRQn, I2C_IRQ_PRIORITY, 0);
        HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
    }
}

static void I2C1_MspDeInit(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1_INSTANCE) {
        /* -1- Unconfigure the GPIOs */
        /* Enable GPIO clock */
        I2C1_SDA_GPIO_CLK_ENABLE();
        I2C1_SCL_GPIO_CLK_ENABLE();

        /* Configure I2C Rx/Tx as alternate function  */
        HAL_GPIO_DeInit(I2C1_GPIO_SCL_PORT, I2C1_GPIO_SCL_PIN);
        HAL_GPIO_DeInit(I2C1_GPIO_SDA_PORT,  I2C1_GPIO_SDA_PIN);

        /* -2- Unconfigure the Discovery I2C1 peripheral */
        /* Force and release I2C Peripheral */
        I2C1_FORCE_RESET();
        I2C1_RELEASE_RESET();

        /* Disable Discovery I2C1 clock */
        I2C1_CLK_DISABLE();

        /* Disable Discovery I2C1 interrupts */
        HAL_NVIC_DisableIRQ(I2C1_EV_IRQn);
        HAL_NVIC_DisableIRQ(I2C1_ER_IRQn);
    }
}

void I2C2_Init(void)
{
    if (HAL_I2C_GetState(&I2c2Handle) == HAL_I2C_STATE_RESET) {
        I2c2Handle.Instance              = I2C2_INSTANCE;
        I2c2Handle.Init.Timing           = I2C2_TIMING;
        I2c2Handle.Init.OwnAddress1      = I2C2_OWN_ADDRESS1;
        I2c2Handle.Init.AddressingMode   = I2C2_ADDRESSING_MODE;
        I2c2Handle.Init.DualAddressMode  = I2C2_DUAL_ADDRESS_MODE;
        I2c2Handle.Init.OwnAddress2      = I2C2_OWNADDRESS2;
        I2c2Handle.Init.GeneralCallMode  = I2C2_GENERAL_CALL_MODE;
        I2c2Handle.Init.NoStretchMode    = I2C2_NO_STRETCH_MODE;

        /* Init the I2C */
        I2C2_MspInit(&I2c2Handle);
        HAL_I2C_Init(&I2c2Handle);
    }
}

void I2C2_DeInit(void)
{
    if (HAL_I2C_GetState(&I2c2Handle) != HAL_I2C_STATE_RESET) {
        /* DeInit the I2C */
        HAL_I2C_DeInit(&I2c2Handle);
        I2C2_MspDeInit(&I2c2Handle);
    }
}

static void I2C2_MspInit(I2C_HandleTypeDef *hi2c)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_PeriphCLKInitTypeDef  RCC_PeriphCLKInitStruct;

    if (hi2c->Instance == I2C2_INSTANCE) {
        /*##-1- Configure the Discovery I2C2 clock source. The clock is derived from the SYSCLK #*/
        RCC_PeriphCLKInitStruct.PeriphClockSelection = I2C2_RCC_PERIPH_CLOCK_SELECTION;
        RCC_PeriphCLKInitStruct.I2c2ClockSelection = I2C2_RCC_CLOCK_SELECTION;
        HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);

        /* -2- Configure the GPIOs */
        /* Enable GPIO clock */
        I2C2_SDA_GPIO_CLK_ENABLE();
        I2C2_SCL_GPIO_CLK_ENABLE();

        /* Configure I2C Rx/Tx as alternate function  */
        GPIO_InitStructure.Pin       = I2C2_GPIO_SCL_PIN;
        GPIO_InitStructure.Mode      = I2C2_GPIO_MODE;
        GPIO_InitStructure.Pull      = I2C2_GPIO_PULL;
        GPIO_InitStructure.Speed     = I2C2_GPIO_SPEED;
        GPIO_InitStructure.Alternate = I2C2_GPIO_ALTERNATE;
        HAL_GPIO_Init(I2C2_GPIO_SCL_PORT, &GPIO_InitStructure);
        GPIO_InitStructure.Pin       = I2C2_GPIO_SDA_PIN;
        HAL_GPIO_Init(I2C2_GPIO_SDA_PORT, &GPIO_InitStructure);

        /* -3- Configure the Discovery I2C2 peripheral */
        /* Enable Discovery_I2C2 clock */
        I2C2_CLK_ENABLE();

        /* Force and release the I2C Peripheral Clock Reset */
        I2C2_FORCE_RESET();
        I2C2_RELEASE_RESET();

        /* Enable and set Discovery I2C2 Interrupt to the highest priority */
        HAL_NVIC_SetPriority(I2C2_EV_IRQn, I2C_IRQ_PRIORITY, 0);
        HAL_NVIC_EnableIRQ(I2C2_EV_IRQn);

        /* Enable and set Discovery I2C2 Interrupt to the highest priority */
        HAL_NVIC_SetPriority(I2C2_ER_IRQn, I2C_IRQ_PRIORITY, 0);
        HAL_NVIC_EnableIRQ(I2C2_ER_IRQn);
    }
}

static void I2C2_MspDeInit(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C2_INSTANCE) {
        /* -1- Unconfigure the GPIOs */
        /* Enable GPIO clock */
        I2C2_SDA_GPIO_CLK_ENABLE();
        I2C2_SCL_GPIO_CLK_ENABLE();

        /* Configure I2C Rx/Tx as alternate function  */
        HAL_GPIO_DeInit(I2C2_GPIO_SCL_PORT, I2C2_GPIO_SCL_PIN);
        HAL_GPIO_DeInit(I2C2_GPIO_SDA_PORT,  I2C2_GPIO_SDA_PIN);

        /* -2- Unconfigure the Discovery I2C2 peripheral */
        /* Force and release I2C Peripheral */
        I2C2_FORCE_RESET();
        I2C2_RELEASE_RESET();

        /* Disable Discovery I2C2 clock */
        I2C2_CLK_DISABLE();

        /* Disable Discovery I2C2 interrupts */
        HAL_NVIC_DisableIRQ(I2C2_EV_IRQn);
        HAL_NVIC_DisableIRQ(I2C2_ER_IRQn);
    }
}

void I2C3_Init(void)
{
    if (HAL_I2C_GetState(&I2c3Handle) == HAL_I2C_STATE_RESET) {
        I2c3Handle.Instance              = I2C3_INSTANCE;
        I2c3Handle.Init.Timing           = I2C3_TIMING;
        I2c3Handle.Init.OwnAddress1      = I2C3_OWN_ADDRESS1;
        I2c3Handle.Init.AddressingMode   = I2C3_ADDRESSING_MODE;
        I2c3Handle.Init.DualAddressMode  = I2C3_DUAL_ADDRESS_MODE;
        I2c3Handle.Init.OwnAddress2      = I2C3_OWNADDRESS2;
        I2c3Handle.Init.GeneralCallMode  = I2C3_GENERAL_CALL_MODE;
        I2c3Handle.Init.NoStretchMode    = I2C3_NO_STRETCH_MODE;

        /* Init the I2C */
        I2C3_MspInit(&I2c3Handle);
        HAL_I2C_Init(&I2c3Handle);
    }
}

void I2C3_DeInit(void)
{
    if (HAL_I2C_GetState(&I2c3Handle) != HAL_I2C_STATE_RESET) {
        /* DeInit the I2C */
        HAL_I2C_DeInit(&I2c3Handle);
        I2C3_MspDeInit(&I2c3Handle);
    }
}

static void I2C3_MspInit(I2C_HandleTypeDef *hi2c)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_PeriphCLKInitTypeDef  RCC_PeriphCLKInitStruct;

    if (hi2c->Instance == I2C3_INSTANCE) {
        /*##-1- Configure the Discovery I2C3 clock source. The clock is derived from the SYSCLK #*/
        RCC_PeriphCLKInitStruct.PeriphClockSelection = I2C3_RCC_PERIPH_CLOCK_SELECTION;
        RCC_PeriphCLKInitStruct.I2c3ClockSelection = I2C3_RCC_CLOCK_SELECTION;
        HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);

        /* -2- Configure the GPIOs */
        /* Enable GPIO clock */
        I2C3_SDA_GPIO_CLK_ENABLE();
        I2C3_SCL_GPIO_CLK_ENABLE();

        /* Configure I2C Rx/Tx as alternate function  */
        GPIO_InitStructure.Pin       = I2C3_GPIO_SCL_PIN;
        GPIO_InitStructure.Mode      = I2C3_GPIO_MODE;
        GPIO_InitStructure.Pull      = I2C3_GPIO_PULL;
        GPIO_InitStructure.Speed     = I2C3_GPIO_SPEED;
        GPIO_InitStructure.Alternate = I2C3_GPIO_ALTERNATE;
        HAL_GPIO_Init(I2C3_GPIO_SCL_PORT, &GPIO_InitStructure);
        GPIO_InitStructure.Pin       = I2C3_GPIO_SDA_PIN;
        HAL_GPIO_Init(I2C3_GPIO_SDA_PORT, &GPIO_InitStructure);

        /* -3- Configure the Discovery I2C3 peripheral */
        /* Enable Discovery_I2C3 clock */
        I2C3_CLK_ENABLE();

        /* Force and release the I2C Peripheral Clock Reset */
        I2C3_FORCE_RESET();
        I2C3_RELEASE_RESET();

        /* Enable and set Discovery I2C3 Interrupt to the highest priority */
        HAL_NVIC_SetPriority(I2C3_EV_IRQn, I2C_IRQ_PRIORITY, 0);
        HAL_NVIC_EnableIRQ(I2C3_EV_IRQn);

        /* Enable and set Discovery I2C3 Interrupt to the highest priority */
        HAL_NVIC_SetPriority(I2C3_ER_IRQn, I2C_IRQ_PRIORITY, 0);
        HAL_NVIC_EnableIRQ(I2C3_ER_IRQn);
    }
}

static void I2C3_MspDeInit(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C3_INSTANCE) {
        /* -1- Unconfigure the GPIOs */
        /* Enable GPIO clock */
        I2C3_SDA_GPIO_CLK_ENABLE();
        I2C3_SCL_GPIO_CLK_ENABLE();

        /* Configure I2C Rx/Tx as alternate function  */
        HAL_GPIO_DeInit(I2C3_GPIO_SCL_PORT, I2C3_GPIO_SCL_PIN);
        HAL_GPIO_DeInit(I2C3_GPIO_SDA_PORT,  I2C3_GPIO_SDA_PIN);

        /* -2- Unconfigure the Discovery I2C3 peripheral */
        /* Force and release I2C Peripheral */
        I2C3_FORCE_RESET();
        I2C3_RELEASE_RESET();

        /* Disable Discovery I2C3 clock */
        I2C3_CLK_DISABLE();

        /* Disable Discovery I2C3 interrupts */
        HAL_NVIC_DisableIRQ(I2C3_EV_IRQn);
        HAL_NVIC_DisableIRQ(I2C3_ER_IRQn);
    }
}

void I2C4_Init(void)
{
    if (HAL_I2C_GetState(&I2c4Handle) == HAL_I2C_STATE_RESET) {
        I2c4Handle.Instance              = I2C4_INSTANCE;
        I2c4Handle.Init.Timing           = I2C4_TIMING;
        I2c4Handle.Init.OwnAddress1      = I2C4_OWN_ADDRESS1;
        I2c4Handle.Init.AddressingMode   = I2C4_ADDRESSING_MODE;
        I2c4Handle.Init.DualAddressMode  = I2C4_DUAL_ADDRESS_MODE;
        I2c4Handle.Init.OwnAddress2      = I2C4_OWNADDRESS2;
        I2c4Handle.Init.GeneralCallMode  = I2C4_GENERAL_CALL_MODE;
        I2c4Handle.Init.NoStretchMode    = I2C4_NO_STRETCH_MODE;

        /* Init the I2C */
        I2C4_MspInit(&I2c4Handle);
        HAL_I2C_Init(&I2c4Handle);
    }
}

void I2C4_DeInit(void)
{
    if (HAL_I2C_GetState(&I2c4Handle) != HAL_I2C_STATE_RESET) {
        /* DeInit the I2C */
        HAL_I2C_DeInit(&I2c4Handle);
        I2C4_MspDeInit(&I2c4Handle);
    }
}

static void I2C4_MspInit(I2C_HandleTypeDef *hi2c)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_PeriphCLKInitTypeDef  RCC_PeriphCLKInitStruct;

    if (hi2c->Instance == I2C4_INSTANCE) {
        /*##-1- Configure the Discovery I2C4 clock source. The clock is derived from the SYSCLK #*/
        RCC_PeriphCLKInitStruct.PeriphClockSelection = I2C4_RCC_PERIPH_CLOCK_SELECTION;
        RCC_PeriphCLKInitStruct.I2c4ClockSelection = I2C4_RCC_CLOCK_SELECTION;
        HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);

        /* -2- Configure the GPIOs */
        /* Enable GPIO clock */
        I2C4_SDA_GPIO_CLK_ENABLE();
        I2C4_SCL_GPIO_CLK_ENABLE();

        /* Configure I2C Rx/Tx as alternate function  */
        GPIO_InitStructure.Pin       = I2C4_GPIO_SCL_PIN;
        GPIO_InitStructure.Mode      = I2C4_GPIO_MODE;
        GPIO_InitStructure.Pull      = I2C4_GPIO_PULL;
        GPIO_InitStructure.Speed     = I2C4_GPIO_SPEED;
        GPIO_InitStructure.Alternate = I2C4_GPIO_ALTERNATE;
        HAL_GPIO_Init(I2C4_GPIO_SCL_PORT, &GPIO_InitStructure);
        GPIO_InitStructure.Pin       = I2C4_GPIO_SDA_PIN;
        HAL_GPIO_Init(I2C4_GPIO_SDA_PORT, &GPIO_InitStructure);

        /* -3- Configure the Discovery I2C4 peripheral */
        /* Enable Discovery_I2C4 clock */
        I2C4_CLK_ENABLE();

        /* Force and release the I2C Peripheral Clock Reset */
        I2C4_FORCE_RESET();
        I2C4_RELEASE_RESET();

        /* Enable and set Discovery I2C4 Interrupt to the highest priority */
        HAL_NVIC_SetPriority(I2C4_EV_IRQn, I2C_IRQ_PRIORITY, 0);
        HAL_NVIC_EnableIRQ(I2C4_EV_IRQn);

        /* Enable and set Discovery I2C4 Interrupt to the highest priority */
        HAL_NVIC_SetPriority(I2C4_ER_IRQn, I2C_IRQ_PRIORITY, 0);
        HAL_NVIC_EnableIRQ(I2C4_ER_IRQn);
    }
}

static void I2C4_MspDeInit(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C4_INSTANCE) {
        /* -1- Unconfigure the GPIOs */
        /* Enable GPIO clock */
        I2C4_SDA_GPIO_CLK_ENABLE();
        I2C4_SCL_GPIO_CLK_ENABLE();

        /* Configure I2C Rx/Tx as alternate function  */
        HAL_GPIO_DeInit(I2C4_GPIO_SCL_PORT, I2C4_GPIO_SCL_PIN);
        HAL_GPIO_DeInit(I2C4_GPIO_SDA_PORT,  I2C4_GPIO_SDA_PIN);

        /* -2- Unconfigure the Discovery I2C4 peripheral */
        /* Force and release I2C Peripheral */
        I2C4_FORCE_RESET();
        I2C4_RELEASE_RESET();

        /* Disable Discovery I2C4 clock */
        I2C4_CLK_DISABLE();

        /* Disable Discovery I2C4 interrupts */
        HAL_NVIC_DisableIRQ(I2C4_EV_IRQn);
        HAL_NVIC_DisableIRQ(I2C4_ER_IRQn);
    }
}
