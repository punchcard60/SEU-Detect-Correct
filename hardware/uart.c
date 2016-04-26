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

 /******************************************************************************
  * @attention
  *
  *  COPYRIGHT 2015 STMicroelectronics
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

#include <stm32f4xx.h>
#include <math.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>
#include "uart.h"
#include <dprint.h>

#define UART_BAUD 115200
#define CR1_CLEAR_MASK            ((uint16_t)(USART_CR1_M | USART_CR1_PCE | \
                                              USART_CR1_PS | USART_CR1_TE | \
                                              USART_CR1_RE | USART_CR1_PEIE | \
											  USART_CR1_TXEIE | USART_CR1_TCIE | \
											  USART_CR1_RXNEIE | USART_CR1_IDLEIE))

#define CR3_CLEAR_MASK            ((uint16_t)(USART_CR3_RTSE | USART_CR3_CTSE))

static const uint8_t APBAHBPrescTable[16] = {0, 0, 0, 0, 1, 2, 3, 4, 1, 2, 3, 4, 6, 7, 8, 9};

/************************************************************
 * Configure USART6(PC6, PC7)
 */
void __attribute__((no_instrument_function)) uart_init(void) {
    uint32_t tmpreg = 0x00, apbclock = 0x00;
    uint32_t tmp, presc, pllvco, pllp;
    uint32_t SYSCLK_Frequency, HCLK_Frequency;
    uint32_t integerdivider;
    uint32_t fractionaldivider;
    float numerator;
    float denominator;
    float midfreq;


    /* Enable GPIO clock */
    RCC->AHB1ENR |= RCC_AHB1Periph_GPIOC;

    /* Enable UART clock */
    RCC->APB2ENR |= RCC_APB2Periph_USART6;

    /* Connect PXx to USART6_Tx*/
    tmp = ((uint32_t)((uint32_t)GPIO_PinSource6 & (uint32_t)0x07) * 4);
    GPIOC->AFR[GPIO_PinSource6 >> 0x03] &= ~((uint32_t)0xF << tmp);
    GPIOC->AFR[GPIO_PinSource6 >> 0x03] |= ((uint32_t)(GPIO_AF_USART6) <<  tmp);

    /* Connect PXx to USART6_Rx*/
    tmp = ((uint32_t)((uint32_t)GPIO_PinSource7 & (uint32_t)0x07) * 4);
    GPIOC->AFR[GPIO_PinSource7 >> 0x03] &= ~((uint32_t)0xF << tmp);
    GPIOC->AFR[GPIO_PinSource7 >> 0x03] |= ((uint32_t)(GPIO_AF_USART6) <<  tmp);

    /* Configure USART Tx as alternate function  */

    /* Pin mode configuration */
    GPIOC->MODER  &= ~(GPIO_MODER_MODER0 << 12);
    GPIOC->MODER |= (((uint32_t)GPIO_Mode_AF) << 12);
    /* Speed mode configuration */
    GPIOC->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR0 << 12);
    GPIOC->OSPEEDR |= ((uint32_t)(GPIO_Speed_50MHz) << 12);

    /* Output mode configuration*/
    GPIOC->OTYPER  &= ~((GPIO_OTYPER_OT_0) << (uint16_t)6) ;
    GPIOC->OTYPER |= (uint16_t)(((uint16_t)GPIO_OType_PP) << (uint16_t)6);

    /* Configure USART Rx as alternate function  */

    /* Pin mode configuration */
    GPIOC->MODER  &= ~(GPIO_MODER_MODER0 << 14);
    GPIOC->MODER |= (((uint32_t)GPIO_Mode_AF) << 14);

    /* Pull-up Pull down resistor configuration*/
    GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPDR0 << (uint16_t)14);
    GPIOC->PUPDR |= (((uint32_t)GPIO_PuPd_UP) << 14);

    /* USART configuration */

    /* usart enable */
    USART6->CR1 |= USART_CR1_UE;

    /*---------------------------- USART CR2 Configuration -----------------------*/
    tmpreg = USART6->CR2;

    /* Clear STOP[13:12] bits */
    tmpreg &= (uint32_t)~((uint32_t)USART_CR2_STOP);

    /* Configure the USART Stop Bits, Clock, CPOL, CPHA and LastBit :
        Set STOP[13:12] bits according to USART_StopBits value */
    tmpreg |= (uint32_t)USART_StopBits_1;

    /* Write to USART CR2 */
    USART6->CR2 = (uint16_t)tmpreg;

    /*---------------------------- USART CR1 Configuration -----------------------*/
    tmpreg = USART6->CR1;

    /* Clear M, PCE, PS, TE and RE bits */
    tmpreg &= (uint32_t)~((uint32_t)CR1_CLEAR_MASK);

    /* Configure the USART Word Length, Parity and mode:
        Set the M bits according to USART_WordLength value
        Set PCE and PS bits according to USART_Parity value
        Set TE and RE bits according to USART_Mode value */
    tmpreg |= (uint32_t)(USART_WordLength_8b | USART_Parity_No | USART_Mode_Tx | USART_Mode_Rx);

    /* Write to USART CR1 */
    USART6->CR1 = (uint16_t)tmpreg;

    /*---------------------------- USART CR3 Configuration -----------------------*/
    tmpreg = USART6->CR3;

    /* Clear CTSE and RTSE bits */
    tmpreg &= (uint32_t)~((uint32_t)CR3_CLEAR_MASK);

    /* Configure the USART HFC :
        Set CTSE and RTSE bits according to USART_HardwareFlowControl value */
    tmpreg |= USART_HardwareFlowControl_None;

    /* Write to USART CR3 */
    USART6->CR3 = (uint16_t)tmpreg;

    /*---------------------------- USART BRR Configuration -----------------------*/
    /* Configure the USART Baud Rate */

    /* Get SYSCLK source -------------------------------------------------------*/
    tmp = RCC->CFGR & RCC_CFGR_SWS;
    switch (tmp)
    {
        case 0x00:  /* HSI used as system clock source */
            SYSCLK_Frequency = HSI_VALUE;
            break;

        case 0x04:  /* HSE used as system clock  source */
            SYSCLK_Frequency = HSE_VALUE;
            break;

        case 0x08:  /* PLL P used as system clock  source */

            /* PLL_VCO = (HSE_VALUE or HSI_VALUE / PLLM) * PLLN
            SYSCLK = PLL_VCO / PLLP
            */
            if (((RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC) >> 22) != 0)
            {
                /* HSE used as PLL clock source */
                pllvco = (HSE_VALUE / (RCC->PLLCFGR & RCC_PLLCFGR_PLLM)) * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> 6);
            }
            else
            {
                /* HSI used as PLL clock source */
                pllvco = (HSI_VALUE / (RCC->PLLCFGR & RCC_PLLCFGR_PLLM)) * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> 6);
            }

            pllp = (((RCC->PLLCFGR & RCC_PLLCFGR_PLLP) >>16) + 1 ) * 2;
            SYSCLK_Frequency = pllvco/pllp;
            break;

        default:
            SYSCLK_Frequency = HSI_VALUE;
            break;
    }

    /* Get HCLK prescaler */
    uint32_t idx = (RCC->CFGR & RCC_CFGR_HPRE) >> 4;
    presc = APBAHBPrescTable[idx];

    /* HCLK clock frequency */
    HCLK_Frequency = SYSCLK_Frequency >> presc;

    /* Get PCLK2 prescaler */
    idx = (RCC->CFGR & RCC_CFGR_PPRE2) >> 13;
    presc = APBAHBPrescTable[idx];

    /* PCLK2 clock frequency */
    apbclock = HCLK_Frequency >> presc;

    denominator = ((USART6->CR1 & USART_CR1_OVER8) != 0) ? 8.0f : 16.0f;
    midfreq = (float)apbclock / (float)UART_BAUD / denominator;

    /* Determine the integer part */
    integerdivider = (uint32_t)midfreq;
    midfreq -= (float)integerdivider;

    numerator = floor(midfreq * denominator + 0.5f);
    if (numerator >= denominator) {
        integerdivider++;
        numerator = 0.0f;
    }
    fractionaldivider = (uint32_t)numerator;

    /* Write to USART BRR register */
    USART6->BRR = (uint16_t)((integerdivider << 4) | fractionaldivider);

    USART6->CR1 |= USART_CR1_TE; /* Send an idle frame */

    dprint("start");
    dprint("\n\n\n****************************************\nStartup\n");
    dprint("UART init complete\n");
}

