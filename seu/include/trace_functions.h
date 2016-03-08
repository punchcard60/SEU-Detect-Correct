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

#include <string.h>
#include <reed_solomon.h>
#include <decode_rs.h>
#include <stm32f4xx_flash.h>
#include <reboot.h>

#ifndef NULL
#define NULL ((void*)0)
#endif

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

extern uint64_t crc_expire_times[BLOCK_COUNT];

#define CRC_EXPIRE_TIME  10 /* In units of (system tick * 100). CRC_EXPIRE_TIME = 10 means we check CRC once a second */

void __attribute__((no_instrument_function)) section1_profile_func_enter(uint32_t block_number, uint32_t depth);
void __attribute__((no_instrument_function)) section2_profile_func_enter(uint32_t block_number, uint32_t depth);
void __attribute__((no_instrument_function)) section3_profile_func_enter(uint32_t block_number, uint32_t depth);

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

/***** CRC function ***************************/

inline static uint32_t INLINE_ATTRIBUTE crc_check(uint32_t block_number, uint64_t tm_now) {
	register uint32_t* ptr;
	register uint32_t* crc_ptr;

	if (crc_expire_times[block_number] > tm_now) {
		return 0;
	}

	CRC->CR = CRC_CR_RESET;

	ptr = (uint32_t*)BLOCK_START(block_number);
	crc_ptr = (uint32_t*)(((uint32_t)ptr) + sizeof(block_t) - sizeof(uint32_t));

	while(ptr < crc_ptr)
	{
		CRC->DR = *ptr++;
	}

	if (CRC->DR ^ *crc_ptr) {
		return 1;
	}

	crc_expire_times[block_number] = tm_now + CRC_EXPIRE_TIME;

	return 0;
}


inline static void INLINE_ATTRIBUTE flash_copy_to_work(int flash_section, int count, error_marker_t** corrections){
	uint32_t* src = (uint32_t*)FlashSections[flash_section];
	uint32_t* src_limit = (uint32_t*)FlashSections[flash_section + 1];
	uint32_t dest = (uint32_t)FlashSections[WORK_FLASH_SECTION];
	int i = 0;

	FLASH_Unlock();
	FLASH_EraseSector((WORK_FLASH_SECTION << 3), VoltageRange_3);

	while (src < src_limit && i < count) {
		FLASH_ProgramWord(dest, (src == corrections[i]->pointer) ? corrections[i++]->corrected_dword : *src);
		src++;
		dest += sizeof(uint32_t);
	}

	while (src < src_limit) {
		FLASH_ProgramWord(dest, *src++);
		dest += sizeof(uint32_t);
	}

	FLASH_Lock();
}

inline static void INLINE_ATTRIBUTE flash_copy_from_work(int flash_section) {
	uint32_t* src = (uint32_t*)FlashSections[WORK_FLASH_SECTION];
	uint32_t dest = FlashSections[flash_section];
	uint32_t dest_limit = FlashSections[flash_section + 1];

	FLASH_Unlock();
	FLASH_EraseSector((uint32_t)flash_section << 3, VoltageRange_3);

	while (dest < dest_limit) {
		FLASH_ProgramWord(dest, *src);
		src++;
		dest += sizeof(uint32_t);
	}

	FLASH_Lock();
}

inline static void INLINE_ATTRIBUTE fix_block(uint32_t block_number) {
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
