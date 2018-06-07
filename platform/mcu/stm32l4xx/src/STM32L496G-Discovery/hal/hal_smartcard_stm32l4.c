/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include "hal_smartcard_stm32l4.h"


SC_ATR g_card_atr;
uint8_t SC_ATR_Table[40] = {0};
static __IO uint8_t SCData = 0;
static uint32_t F_Table[16] = {372, 372, 558, 744, 1116, 1488, 1860, 0,
                               0, 512, 768, 1024, 1536, 2048, 0, 0};
static uint32_t D_Table[8] = {0, 1, 2, 4, 8, 16, 32, 64};
extern SMARTCARD_HandleTypeDef hsmartcard2;

void SC_Reset(int  ResetState);

void HAL_SMARTCARD_MspInit(SMARTCARD_HandleTypeDef* hsmartcard)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(hsmartcard->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();
  
    /**USART2 GPIO Configuration    
    PA2     ------> USART2_TX
    PD7     ------> USART2_CK 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
  }

}

void HAL_SMARTCARD_MspDeInit(SMARTCARD_HandleTypeDef* hsmartcard)
{

  if(hsmartcard->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();
  
    /**USART2 GPIO Configuration    
    PA2     ------> USART2_TX
    PD7     ------> USART2_CK 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2);

    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_7);

    /* USART2 interrupt DeInit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  }
}  

int32_t hal_smartcard_Init_r(void)
{
  /* USART Clock set to 4 MHz (PCLK1 (80 MHz) / 20) => prescaler set to 10 */
  hsmartcard2.Instance = USART2;
  hsmartcard2.Init.BaudRate = 10752; /*4MHz / 372*/
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
    //_Error_Handler(__FILE__, __LINE__);
    return HAL_ERROR;
  }
  //printf("HAL_SMARTCARD_Init ok\n");
  //krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/100);
  SC_Reset(GPIO_PIN_SET);
  
  return HAL_OK;
}

int32_t hal_smartcard_DeInit(void)
{
    int32_t ret = HAL_OK;

    hsmartcard2.Instance = USART2;

    if (HAL_SMARTCARD_DeInit(&hsmartcard2) != HAL_OK) {
        ret = HAL_ERROR;
    }

    return ret;
}

void SC_Reset(int ResetState)
{
  /* RST active high  GPIO_PIN_SET*/
  HAL_GPIO_WritePin(GPIOE, SE_RST_Pin, ResetState);
}

void SC_Reset_init(viod)
{
   GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SE_RST_GPIO_Port, SE_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : SE_RST_Pin */
  GPIO_InitStruct.Pin = SE_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SE_RST_GPIO_Port, &GPIO_InitStruct);
}

uint8_t SC_PTSConfig(void)
{
	uint32_t workingbaudrate = 0, apbclock = 0;
	uint8_t locData = 0, PPSConfirmStatus = 1;
	uint8_t SC_PPSS[4] = {0};
	uint8_t SC_PPSS_response[4] = {0};

	/* Reconfigure the USART Baud Rate -----------------------------------------*/
	apbclock = HAL_RCC_GetPCLK1Freq();
	apbclock /= ((USART2->GTPR & (uint16_t)0x00FF) * 2);

	if((g_card_atr.T0 & (uint8_t)0x10) == 0x10){
		if(g_card_atr.T[0] != 0x11){
			/* PPSS identifies the PPS request or responce and is equal to 0xFF */
			SC_PPSS[0] = 0xFF;
			/* PPS0 indicates by the bits b5, b6, b7 equal to 1 the presence of the optional
			bytes PPSI1, PPS2, PPS3 respectively */
			SC_PPSS[1] = 0x10;   /* only send PPS1 */
			/* PPS1 allows the interface device to propose value of F and D to the card */
			SC_PPSS[2] = g_card_atr.T[0]; 
			/* PCK check character */ 
			SC_PPSS[3] = (uint8_t)0xFF^(uint8_t)0x10^(uint8_t)g_card_atr.T[0];      
			//printf("send SC_PPSS cmd ...\n");
			//krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
			if(HAL_SMARTCARD_Transmit(&hsmartcard2, (uint8_t *)SC_PPSS, 4, SC_TRANSMIT_TIMEOUT) != HAL_OK)
			{
				Error_Handler();
			}

			
			/* PPSS response = PPSS request = 0xFF*/
			if(HAL_SMARTCARD_Receive(&hsmartcard2, (uint8_t *)SC_PPSS_response, 4, SC_RECEIVE_TIMEOUT) == HAL_OK)  
			{
				for(locData = 0; locData < 4; locData++)
					if(SC_PPSS_response[locData] != SC_PPSS[locData]){
						PPSConfirmStatus = 0x00;
						break;
					}
			}
			else
			{
				/*PPSS exchange unsuccessful */
				PPSConfirmStatus = 0x00;
			}

			/* PPS exchange successful */
			if(PPSConfirmStatus == 0x01)
			{
				workingbaudrate = apbclock * D_Table[(g_card_atr.T[0] & (uint8_t)0x0F)];
				workingbaudrate /= F_Table[((g_card_atr.T[0] >> 4) & (uint8_t)0x0F)];
				printf("workingbaudrate = %d\n", workingbaudrate);
				hsmartcard2.Instance = USART2;
				hsmartcard2.Init.BaudRate = workingbaudrate;
				hsmartcard2.Init.WordLength = SMARTCARD_WORDLENGTH_9B;
				hsmartcard2.Init.StopBits = SMARTCARD_STOPBITS_1_5;
				hsmartcard2.Init.Parity = SMARTCARD_PARITY_EVEN;
				hsmartcard2.Init.Mode = SMARTCARD_MODE_TX_RX;
				hsmartcard2.Init.CLKPolarity = SMARTCARD_POLARITY_LOW;
				hsmartcard2.Init.CLKPhase = SMARTCARD_PHASE_1EDGE;
				hsmartcard2.Init.CLKLastBit = SMARTCARD_LASTBIT_ENABLE;
				hsmartcard2.Init.Prescaler = 10;
				hsmartcard2.Init.GuardTime = 1;
				hsmartcard2.Init.NACKEnable = SMARTCARD_NACK_ENABLE;
				hsmartcard2.Init.OneBitSampling = SMARTCARD_ONE_BIT_SAMPLE_DISABLE;
				hsmartcard2.Init.TimeOutEnable = SMARTCARD_TIMEOUT_DISABLE;
				hsmartcard2.Init.BlockLength = 0;  /* T=1 not applicable */
				hsmartcard2.Init.AutoRetryCount = 3;        
				hsmartcard2.AdvancedInit.AdvFeatureInit = SMARTCARD_ADVFEATURE_NO_INIT;
				if(HAL_SMARTCARD_Init(&hsmartcard2) != HAL_OK)
				{
					Error_Handler();
				}
			}
			else{
				printf("PPSConfirmStatus is zero\n");
				return HAL_ERROR;
			}
		}
	}  
	return HAL_OK;
}

static int iso7816_get_atr(unsigned char *atr)
{
    unsigned char index = 0;
    unsigned char i = 0;
    unsigned char temp = 0;
    unsigned char h_len;
    unsigned char ret;
  

    /* get TS */
     ret = HAL_SMARTCARD_Receive(&hsmartcard2, &atr[i++], 1, SC_RECEIVE_TIMEOUT);
     //ret = HAL_SMARTCARD_Receive_IT(&hsmartcard2, &atr[i++], 1);
     if(ret != HAL_OK){
		printf("HAL_SMARTCARD_Receive error\n");
		return ret;
     }
     //printf("get Ts ok\n");
    /* get T0 */
     ret = HAL_SMARTCARD_Receive(&hsmartcard2, &atr[i], 1, SC_RECEIVE_TIMEOUT);
     //ret = HAL_SMARTCARD_Receive_IT(&hsmartcard2, &atr[i], 1);
     if(ret != HAL_OK){
		printf("HAL_SMARTCARD_Receive error\n");
		return ret;
     }

    temp = atr[i] >> 4;
    h_len = atr[i] & 0xf;
    index = i;
    i++;
    while (temp) {
        if (temp & 0x1) {
            /* TAi */
            ret = HAL_SMARTCARD_Receive(&hsmartcard2, &atr[i++], 1, SC_RECEIVE_TIMEOUT);
            //ret = HAL_SMARTCARD_Receive_IT(&hsmartcard2, &atr[i++], 1);
	     if(ret != HAL_OK){
			printf("HAL_SMARTCARD_Receive error\n");
			return ret;
	     }
        }
        if (temp & 0x2) {
            /* TBi */
            ret = HAL_SMARTCARD_Receive(&hsmartcard2, &atr[i++], 1, SC_RECEIVE_TIMEOUT);
             //ret = HAL_SMARTCARD_Receive_IT(&hsmartcard2, &atr[i++], 1);
	     if(ret != HAL_OK){
			printf("HAL_SMARTCARD_Receive error\n");
			return ret;
	     }
        }
        if (temp & 0x4) {
            /* TCi */
            ret = HAL_SMARTCARD_Receive(&hsmartcard2, &atr[i++], 1, SC_RECEIVE_TIMEOUT);
	     //ret = HAL_SMARTCARD_Receive_IT(&hsmartcard2, &atr[i++], 1);	
	     if(ret != HAL_OK){
			printf("HAL_SMARTCARD_Receive error\n");
			return ret;
	     }
        }
        if (temp & 0x8) {
            /* TDi */
            ret = HAL_SMARTCARD_Receive(&hsmartcard2, &atr[i], 1, SC_RECEIVE_TIMEOUT);
             //ret = HAL_SMARTCARD_Receive_IT(&hsmartcard2, &atr[i++], 1);
	     if(ret != HAL_OK){
			printf("HAL_SMARTCARD_Receive error\n");
			return ret;
	     }
            temp = atr[i++] >> 4;
            continue;
        }
        temp = 0;
    }
    /* get historical bytes */
    while (h_len) {
        h_len--;
         ret = HAL_SMARTCARD_Receive(&hsmartcard2, &atr[i++], 1, SC_RECEIVE_TIMEOUT);
           // ret = HAL_SMARTCARD_Receive_IT(&hsmartcard2, &atr[i++], 1);
	     if(ret != HAL_OK){
			printf("HAL_SMARTCARD_Receive error\n");
			return ret;
	     }
    }

#ifdef CONFIG_SE_GEMALTO_MTF008
    /* the last byte is checksum byte, it is existed in Gemalto's SE */
    ret = HAL_SMARTCARD_Receive(&hsmartcard2, &atr[i++], 1, SC_RECEIVE_TIMEOUT);
     //ret = HAL_SMARTCARD_Receive_IT(&hsmartcard2, &atr[i++], 1);
     if(ret != HAL_OK){
		printf("HAL_SMARTCARD_Receive error\n");
		return ret;
     }
#endif

    return i;
}

static uint8_t iso7816_decode_atr(uint8_t *string)
{
    uint32_t i = 0;
    uint32_t flag = 0;
    uint32_t buf = 0;
    uint32_t protocol = 0;

    g_card_atr.TS = string[0];  /* Initial character */
    g_card_atr.T0 = string[1];  /* Format character */

    g_card_atr.Hlength = g_card_atr.T0 & (uint8_t)0x0F;

    if ((g_card_atr.T0 & (uint8_t)0x80) == 0x80) {
        flag = 1;
    }

    for (i = 0; i < 4; i++) {
        g_card_atr.Tlength = g_card_atr.Tlength +
                    (((g_card_atr.T0 & (uint8_t)0xF0) >> (4 + i)) & (uint8_t)0x1);
    }

    for (i = 0; i < g_card_atr.Tlength; i++) {
        g_card_atr.T[i] = string[i + 2];
    }

    protocol = g_card_atr.T[g_card_atr.Tlength - 1] & (uint8_t)0x0F;

    while (flag) {
        if ((g_card_atr.T[g_card_atr.Tlength - 1] & (uint8_t)0x80) == 0x80) {
            flag = 1;
        }
        else {
            flag = 0;
        }

        buf = g_card_atr.Tlength;
        g_card_atr.Tlength = 0;

        for (i = 0; i < 4; i++) {
            g_card_atr.Tlength = g_card_atr.Tlength + (((g_card_atr.T[buf - 1] & (uint8_t)0xF0) >> (4 + i)) & (uint8_t)0x1);
        }

        for (i = 0; i < g_card_atr.Tlength; i++) {
            g_card_atr.T[buf + i] = string[i + 2 + buf];
        }
        g_card_atr.Tlength += (uint8_t)buf;
    }

    for (i = 0; i < g_card_atr.Hlength; i++) {
        g_card_atr.H[i] = string[i + 2 + g_card_atr.Tlength];
    }

    return (uint8_t)protocol;
}

int t0_send_command_recv_status(SC_ADPU_Commands *apdu,
                                SC_ADPU_Response *response)
{
	int i = 0;
	int ret = 0;
	uint8_t locData;

	memset(response->data, 0, LC_MAX);
	response->SW1 = 0;
	response->SW2 = 0;

	/* send apdu header */
	ret = HAL_SMARTCARD_Transmit(&hsmartcard2, &(apdu->header.CLA), 1, SC_RECEIVE_TIMEOUT);
	if(ret != HAL_OK){
		printf("HAL_SMARTCARD_Transmit CLA error\n");
		return ret;
	}
	ret = HAL_SMARTCARD_Transmit(&hsmartcard2, &(apdu->header.INS), 1, SC_RECEIVE_TIMEOUT);
	if(ret != HAL_OK){
		printf("HAL_SMARTCARD_Transmit INS error\n");
		return ret;
	}
	ret = HAL_SMARTCARD_Transmit(&hsmartcard2, &(apdu->header.P1), 1, SC_RECEIVE_TIMEOUT);
	if(ret != HAL_OK){
		printf("HAL_SMARTCARD_Transmit P1 error\n");
		return ret;
	}
	ret = HAL_SMARTCARD_Transmit(&hsmartcard2, &(apdu->header.P2), 1, SC_RECEIVE_TIMEOUT);
	if(ret != HAL_OK){
		printf("HAL_SMARTCARD_Transmit P2 error\n");
		return ret;
	}
	
	if(apdu->body.LC > 0){
		//printf("HAL_SMARTCARD_Transmit LC...\n");
		ret = HAL_SMARTCARD_Transmit(&hsmartcard2, &(apdu->body.LC), 1, SC_RECEIVE_TIMEOUT);
		if(ret != HAL_OK){
			printf("HAL_SMARTCARD_Transmit LC error\n");
			return ret;
		}
	}
	else if (apdu->body.LE > 0){
		//printf("HAL_SMARTCARD_Transmit LE...\n");
		ret = HAL_SMARTCARD_Transmit(&hsmartcard2, &(apdu->body.LE), 1, SC_RECEIVE_TIMEOUT);
		if(ret != HAL_OK){
			printf("HAL_SMARTCARD_Transmit LE error\n");
			return ret;
		}
	}

	if(HAL_SMARTCARD_Receive(&hsmartcard2, &locData, 1, SC_RECEIVE_TIMEOUT) == HAL_OK){
		//printf("SE:locData = 0x%x\n", locData);
		if(((locData & 0xF0) == 0x60) || ((locData & 0xF0) == 0x90))
		{
			/* SW1 received */
			response->SW1 = locData;

			if(HAL_SMARTCARD_Receive(&hsmartcard2, &locData, 1, SC_RECEIVE_TIMEOUT) == HAL_OK)
			{
				/* SW2 received */
				response->SW2 = locData;
			}
		}
		else if (((locData & 0xFE) == (((uint8_t)~(apdu->header.INS)) & 0xFE))
			||((locData & 0xFE) == (apdu->header.INS & 0xFE)))
		{
			response->data[0] = locData;/* ACK received */
		}
	}
	else
		return HAL_ERROR;

	/* if no status bytes received */
	if (response->SW1 == 0) {
		if (apdu->body.LC > 0) {
			//send data
			ret =  HAL_SMARTCARD_Transmit(&hsmartcard2,apdu->body.Data, apdu->body.LC, SC_RECEIVE_TIMEOUT);
			if(ret != HAL_OK){
				printf("HAL_SMARTCARD_Transmit LC Data error\n");
				return ret;
			}	
		} 
		else if (apdu->body.LE > 0) {
			//receive data
			ret = HAL_SMARTCARD_Receive(&hsmartcard2, response->data, apdu->body.LE + 2, SC_RECEIVE_TIMEOUT);
			if (HAL_OK == ret)
			{
				response->SW1 = response->data[apdu->body.LE];
				response->SW2 = response->data[apdu->body.LE + 1];

				//for (i = 0; i < apdu->body.LE + 2; i++)
				//	printf("response->data[%d] = 0x%x\n", i, response->data[i]);
			}else{
				printf("receive data error, ret = 0x%x\n", ret);
			}

			return ret;
		} 
		else {
			return HAL_ERROR;
		}
	}

	/* wait for SW1 SW2*/
	if (HAL_SMARTCARD_Receive(&hsmartcard2, &locData, 1, SC_RECEIVE_TIMEOUT) != 0) {
	return HAL_ERROR;
	}
	else{
	//printf("SW1 = 0x%x\n", locData);
	response->SW1 = locData;
	}

	if (HAL_SMARTCARD_Receive(&hsmartcard2, &locData, 1, SC_RECEIVE_TIMEOUT) != 0) {
	return HAL_ERROR;
	}
	else{
	//printf("SW2 = 0x%x\n", locData);
	response->SW2 = locData;
	}

	return HAL_OK;
}


int SC_AnswerReq(void)
{
	int len = 0;
	//int times = 3;
	//unsigned char string_atr[MAX_ATR_LENGTH];

	memset(SC_ATR_Table, 0, MAX_ATR_LENGTH);
	memset(&g_card_atr, 0, sizeof(g_card_atr));
	
	//#if 1 //need to reinit in each session.
	if(hal_smartcard_Init_r() != HAL_OK){
		printf("hal_smartcard_Init error\n");
		return HAL_ERROR;
	}
	//#endif 
	printf("hal_smartcard_Init OK\n");
	len = iso7816_get_atr(SC_ATR_Table);
	if (len <= 0x0 || (SC_ATR_Table[0] != ATR_BYTE0)) {
            /* get error ATR */
	     printf("iso7816_get_atr error\n");
            return HAL_ERROR;
        }
	printf("iso7816_get_atr OK\n");
	//for(len = 0; len < 40 && string_atr[len] != 0; len++)
	//	printf("0x%x\n", string_atr[len]);
	iso7816_decode_atr(SC_ATR_Table);
	 
	 return HAL_OK;
}

void SC_Stop(void)
{
  SC_Reset(GPIO_PIN_RESET);
  /* Deinitializes the SCHandle */
  hal_smartcard_DeInit();
}

static int string_to_apdu(SC_ADPU_Commands *apdu,
                          unsigned char *string,
                          int length)
{
    int i = 0;

    apdu->header.CLA = string[0];
    apdu->header.INS = string[1];
    apdu->header.P1 = string[2];
    apdu->header.P2 = string[3];
	  apdu->header.P3 = string[4];

    if (length > 5) {
        apdu->body.LC = string[4];
        for (i = 0; i < apdu->body.LC; i++) {
            apdu->body.Data[i] = string[5+i];
        }
    }
    else {
        apdu->body.LE = string[4];
    }

    return 0;
}

static int response_to_string(unsigned char *string,
                              SC_ADPU_Response *response,
                              int data_length)
{
    int i = 0;
    for (i = 0; i < data_length; i++) {
        string[i] = response->data[i];
    }
    string[i++] = response->SW1;
    string[i++] = response->SW2;

    return i;
}

int DeviceOpen(void **handle)
{
    uint8_t ret = HAL_OK;
	
    *handle = "device open";

    if (SC_AnswerReq() != HAL_OK) {
        /* get error ATR */
				printf("SC_Start error\n");
        return HAL_ERROR;
    }
    //exchange baud
    krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/10);
    #if 0
		if(SC_PTSConfig() != HAL_OK){
			printf("SC_PTSConfig error\n");
			return HAL_ERROR;
    }
		#endif
    return HAL_OK;
}

int DeviceTransmit(void *handle, unsigned char *input, int input_len,
                    unsigned char *output, int *output_len)
{
    SC_ADPU_Commands apdu;
    SC_ADPU_Response response;
    int ret = 0;

    memset(&apdu, 0, sizeof(SC_ADPU_Commands));
    string_to_apdu(&apdu, input, input_len);

    ret = t0_send_command_recv_status(&apdu, &response);
    if (ret != HAL_OK) {
	 printf("T0 cmd failed\n");
        return HAL_ERROR;
    }

    *output_len = response_to_string(output, &response, apdu.body.LE);
    if (response.SW1 == 0 && response.SW2 == 0) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

int DeviceClose(void *handle)
{
    SC_Stop();

    return HAL_OK;
}

