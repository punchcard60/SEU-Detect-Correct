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

#ifndef _TRACE_FUNCTIONS_H
#define _TRACE_FUNCTIONS_H

#include <FreeRTOS.h>
#include <timers.h>
#include <stm32f4xx.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <reed_solomon.h>
#include <decode_rs.h>
#include <reboot.h>

#ifndef NULL
#define NULL ((void*)0)
#endif

extern void __attribute__((no_instrument_function)) seu_timer(TimerHandle_t pxTimer);

/***** Flash physical description ***************************/

#define K(x) 				(1024 * (x))
#define FLASH_SECTIONS  	(11)
#define WORK_FLASH_SECTION	(11)
extern const uint32_t FlashSections[FLASH_SECTIONS + 1];

typedef struct block {
	uint16_t	reed_solomon_data[SYMBOL_TABLE_WORDS]; /*same as 3328 * sizeof(uint32_t) so the alignment works. */
	uint32_t	crc;
} block_t;

#define FUNCT1_BLOCK	0
#define FUNCT2_BLOCK	4
#define FUNCT3_BLOCK	9

#define FUNCT1_FLASH_SECTION 0
#define FUNCT2_FLASH_SECTION 3
#define FUNCT3_FLASH_SECTION 4

#define BLOCK_BASE		((block_t*)FLASH_BASE)
#define BLOCK_START(b)	(&BLOCK_BASE[(b)])

extern int64_t crc_expire_times[BLOCK_COUNT];
extern uint32_t crc_enable;
#define CRC_SIGNATURE 0xABCD
#define CRC_EXPIRE_TIME  10 /* In units of (system tick * 100). CRC_EXPIRE_TIME = 10 means we check CRC once a second */

extern void __attribute__((no_instrument_function)) __cyg_profile_func_enter (void* this_func, void* caller);
extern void __attribute__((no_instrument_function)) __cyg_profile_func_exit (void* this_func, void* caller);

extern void __attribute__((no_instrument_function)) section1_profile_func_enter(uint32_t block_number, uint32_t depth);
extern void __attribute__((no_instrument_function)) section2_profile_func_enter(uint32_t block_number, uint32_t depth);
extern void __attribute__((no_instrument_function)) section3_profile_func_enter(uint32_t block_number, uint32_t depth);

extern uint32_t __attribute__((no_instrument_function)) crc_check1(uint32_t block_number, uint64_t tm_now);
extern uint32_t __attribute__((no_instrument_function)) crc_check2(uint32_t block_number, uint64_t tm_now);
extern uint32_t __attribute__((no_instrument_function)) crc_check3(uint32_t block_number, uint64_t tm_now);

/***** Test if block[block_number] overlaps flash_section[section_number] */

inline static int INLINE_ATTRIBUTE data_block_is_in_flash_section(int block_number, int section_number) {

	if (((uint32_t)BLOCK_START(block_number)) >= FlashSections[section_number + 1]) {
		return 0;
	}

	if (((uint32_t)BLOCK_START(block_number + 1)) <= FlashSections[section_number]) {
		return 0;
	}

	return 1;
}

/***** find which flash section that a pointer points into */

inline static uint32_t INLINE_ATTRIBUTE ptr_to_flash_section(uint32_t* ptr) {
	uint32_t i;

	for (i = 0; i < FLASH_SECTIONS; i++) {
		if ((uint32_t)ptr >= FlashSections[i] && (uint32_t)ptr < FlashSections[i + 1]) {
			return i;
		}
	}

	return 0xFFFFFFFF;
}

/***** Flash Copy ***************************/

#define FLASH_WAIT_FOR_READY while((FLASH->SR & FLASH_FLAG_BSY) == FLASH_FLAG_BSY);
#define FLASH_KEY1               ((uint32_t)0x45670123)
#define FLASH_KEY2               ((uint32_t)0xCDEF89AB)
#define FLASH_FLAG_BSY                 ((uint32_t)0x00010000)  /*!< FLASH Busy flag                           */
#define FLASH_PSIZE_WORD           ((uint32_t)0x00000200)
#define CR_PSIZE_MASK              ((uint32_t)0xFFFFFCFF)
#define SECTOR_MASK               ((uint32_t)0xFFFFFF07)


inline static void INLINE_ATTRIBUTE flash_copy_to_work(int flash_section, int count, error_marker_t** corrections){
	uint32_t* src = (uint32_t*)FlashSections[flash_section];
	uint32_t* src_limit = (uint32_t*)FlashSections[flash_section + 1];
	uint32_t* dest = (uint32_t*)FlashSections[WORK_FLASH_SECTION];
	int i = 0;
printf("flash_copy_to_work(%d)\n", flash_section);
	if((FLASH->CR & FLASH_CR_LOCK) != RESET)
	{
	/* Authorize the FLASH Registers access */
	FLASH->KEYR = FLASH_KEY1;
	FLASH->KEYR = FLASH_KEY2;
	}

	FLASH_WAIT_FOR_READY;

    /* if the previous operation is completed, proceed to erase the sector */
    FLASH->CR &= CR_PSIZE_MASK;
    FLASH->CR |= FLASH_PSIZE_WORD;
    FLASH->CR &= SECTOR_MASK;
    FLASH->CR |= FLASH_CR_SER | (flash_section << 3);
    FLASH->CR |= FLASH_CR_STRT;

	FLASH_WAIT_FOR_READY;

    FLASH->CR &= CR_PSIZE_MASK;
    FLASH->CR |= FLASH_PSIZE_WORD;
    FLASH->CR |= FLASH_CR_PG;

	FLASH_WAIT_FOR_READY;

	while (src < src_limit && i < count) {
    	*dest = (src == corrections[i]->pointer) ? corrections[i++]->corrected_dword : *src;
		src++;
		dest++;
		FLASH_WAIT_FOR_READY;
	}

	while (src < src_limit) {
    	*(__IO uint32_t*)dest = *src;
		src++;
		dest += sizeof(uint32_t);
		FLASH_WAIT_FOR_READY;
	}

    FLASH->CR &= (~FLASH_CR_PG);
	FLASH->CR |= FLASH_CR_LOCK;
}


inline static void INLINE_ATTRIBUTE flash_copy_from_work(int flash_section) {
	uint32_t* src = (uint32_t*)FlashSections[WORK_FLASH_SECTION];
	uint32_t* dest = (uint32_t*)FlashSections[flash_section];
	uint32_t* dest_limit = (uint32_t*)FlashSections[flash_section + 1];
printf("flash_copy_from_work(%d)\n", flash_section);

	if((FLASH->CR & FLASH_CR_LOCK) != RESET)
	{
	/* Authorize the FLASH Registers access */
	FLASH->KEYR = FLASH_KEY1;
	FLASH->KEYR = FLASH_KEY2;
	}

	FLASH_WAIT_FOR_READY;

    /* if the previous operation is completed, proceed to erase the sector */
    FLASH->CR &= CR_PSIZE_MASK;
    FLASH->CR |= FLASH_PSIZE_WORD;
    FLASH->CR &= SECTOR_MASK;
    FLASH->CR |= FLASH_CR_SER | (flash_section << 3);
    FLASH->CR |= FLASH_CR_STRT;

    /* Wait for last operation to be completed */
	FLASH_WAIT_FOR_READY;

    FLASH->CR &= CR_PSIZE_MASK;
    FLASH->CR |= FLASH_PSIZE_WORD;
    FLASH->CR |= FLASH_CR_PG;

	FLASH_WAIT_FOR_READY;

	while (dest < dest_limit) {
    	*dest = *src;
		src++;
		dest++;
		FLASH_WAIT_FOR_READY;
	}

    FLASH->CR &= (~FLASH_CR_PG);
	FLASH->CR |= FLASH_CR_LOCK;
}

inline static void INLINE_ATTRIBUTE fix_block(uint32_t block_number) {
printf("fix_block(%"PRIu32")\n", block_number);
	error_marker_t corrections[RS_MAX_CORRECTIONS];
	error_marker_t* sorted_corrections[RS_MAX_CORRECTIONS];
	error_marker_t* ptr;

	int	correction_count;
	int i, swaps;
	uint32_t flash_section;

	if (decode_rs((symbol_t*)BLOCK_START(block_number), &correction_count, corrections) > 0)
	{ /* decode failed */

		/* Sort the returned pointers */
		for(i=0; i<correction_count; i++) {
			sorted_corrections[i] = &corrections[i];
		}

		do {
			swaps = 0;

			for(i = 0; i < correction_count - 1; i++) {
				if (sorted_corrections[i]->pointer > sorted_corrections[i + 1]->pointer) {
					swaps ++;
					ptr = sorted_corrections[i];
					sorted_corrections[i] = sorted_corrections[i + 1];
					sorted_corrections[i + 1] = ptr;
				}
			}
		} while (swaps > 0);

		/* Blocks can straddle flash sections so process each flash section */
		flash_section = ptr_to_flash_section(sorted_corrections[0]->pointer);
		i = 1;
		while ((i < correction_count) && (flash_section == ptr_to_flash_section(sorted_corrections[i]->pointer))) {
			i++;
		}
		flash_copy_to_work(flash_section, i, sorted_corrections);
		flash_copy_from_work(flash_section);

		if (i < correction_count) {
			flash_section++;
			flash_copy_to_work(flash_section, correction_count - i, &sorted_corrections[i]);
			flash_copy_from_work(flash_section);
		}
	}
}

#endif /* #define _TRACE_FUNCTIONS_H */
