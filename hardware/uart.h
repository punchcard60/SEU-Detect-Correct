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

#ifndef _UART_H
#define _UART_H

#include <stm32f4xx_usart.h>

#ifndef NULL
#define NULL ((void*)0)
#endif

extern void uart_init(void);

inline static void __attribute__((always_inline)) uart_putc(char c) {
	if ( c > 0) {
		if (USART6->CR1 & USART_CR1_UE) {
			while ((USART6->SR & USART_FLAG_TXE) == 0) {
			}
			USART6->DR = (uint16_t)(c);
		}
	}
}

inline static void __attribute__((always_inline)) uart_puts(char* str) {
	if (str != NULL) {
		while (*str) {
			uart_putc(*str++);
		}
	}
}

#endif /* _UART_H */
