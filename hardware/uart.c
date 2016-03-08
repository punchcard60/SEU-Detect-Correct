/*
 * Copyright (C) 2016 Nano Avionics
 *
 * Licensed under the GNU GENERAL PUBLIC LICENSE, Version 3 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License from the Free Software Foundation, Inc.
 * at
 *
 *    http://fsf.org/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <uart.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>

/************************************************************
 * Configure USART6(PC6, PC7)
 */
void uart_init(void) {
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStruct;

  /* Enable GPIO clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    /* Enable UART clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);

  /* Connect PXx to USARTx_Tx*/
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);

  /* Connect PXx to USARTx_Rx*/
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6);

  /* Configure USART Tx as alternate function  */
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* Configure USART Rx as alternate function  */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* USART configuration */
  USART_StructInit(&USART_InitStruct);
  USART_InitStruct.USART_BaudRate = 115200;
  USART_InitStruct.USART_WordLength = USART_WordLength_8b;
  USART_InitStruct.USART_StopBits = USART_StopBits_1;
  USART_InitStruct.USART_Parity = USART_Parity_No;
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  USART_Init(USART6, &USART_InitStruct);

  /* Enable USART */
  USART_Cmd(USART6, ENABLE);
}

