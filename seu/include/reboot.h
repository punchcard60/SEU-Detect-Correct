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

#ifndef _REBOOT_H
#define _REBOOT_H

#include <stm32f4xx.h>
#include <seu.h>
#include <stdio.h>

#ifndef INLINE_ATTRIBUTE
#define INLINE_ATTRIBUTE __attribute__((always_inline))
#endif

typedef struct {
	uint32_t signature;
	uint16_t fixer;
	uint16_t block_number;
} reboot_block_t;

#define SEU_FIX_SIGNATURE 0x87654321
#define BLOCK_COUNT 	((896 * 1024) / 13312) /* 68 */

extern void seu_start_check(void);

inline static reboot_block_t* INLINE_ATTRIBUTE access_backup_domain_sram() {
	/* Get access to the 4K of SRAM in the backup domain */
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
	PWR->CR = PWR_CR_DBP;
    RCC->AHB1ENR |= RCC_AHB1ENR_BKPSRAMEN;
	return (reboot_block_t*)BKPSRAM_BASE;
}

inline static void INLINE_ATTRIBUTE reboot() {
	/* Disable all interrupts */
	RCC->CIR = 0x00000000;

	/* Reboot */
    __ASM volatile ("dsb");                                    /* Ensure all outstanding memory accesses included
                                                                  buffered write are completed before reset */
	SCB->AIRCR  = ((0x5FA << SCB_AIRCR_VECTKEY_Pos)      |
				   (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) |
				   SCB_AIRCR_SYSRESETREQ_Msk);                 /* Keep priority group unchanged */
    __ASM volatile ("dsb");                                    /* Ensure completion of memory access */
	while(1);                                                  /* wait until reset */
}

inline static void INLINE_ATTRIBUTE reboot_and_fix_block(uint32_t block_number, uint32_t fixer) {
	/* Disable all interrupts */
	RCC->CIR = 0x00000000;

	reboot_block_t* backup_sram = access_backup_domain_sram();
	backup_sram->signature = SEU_FIX_SIGNATURE;
	backup_sram->block_number = (uint16_t)block_number;
	backup_sram->fixer = (uint16_t)fixer;

	reboot();
}

#endif /* _REBOOT_H */
