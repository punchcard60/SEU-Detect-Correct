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

#include <FreeRTOS.h>
#include <timers.h>
#include <stm32f4xx.h>
#include <trace_functions.h>
#include <stdio.h>
#include <inttypes.h>

#define RCC_AHB1Periph_CRC               ((uint32_t)0x00001000)

int64_t crc_expire_times[BLOCK_COUNT];
int64_t clock = -1;
uint32_t crc_enable = CRC_SIGNATURE;

const uint32_t FlashSections[FLASH_SECTIONS + 1] = {
    FLASH_BASE,
    FLASH_BASE + K(16),
    FLASH_BASE + K(32),
    FLASH_BASE + K(48),
    FLASH_BASE + K(64),
    FLASH_BASE + K(128),
    FLASH_BASE + K(256),
    FLASH_BASE + K(384),
    FLASH_BASE + K(512),
    FLASH_BASE + K(640),
    FLASH_BASE + K(768),
    FLASH_BASE + K(896) /* This is the work area */
};

TimerHandle_t  htimer;

void __attribute__((no_instrument_function)) seu_init(void) {
printf("seu_init()\n");
	htimer = xTimerCreate("SEU", 10, pdTRUE, NULL, seu_timer);
printf("timer created\n");
	xTimerStart(htimer, 0);
printf("timer started\n");
	crc_enable = ~CRC_SIGNATURE;
	clock = 1;
printf("seu_init() exit\n");
}

void __attribute__((no_instrument_function)) seu_timer(TimerHandle_t pxTimer) {
printf("tick\n");
	clock++;
}

/**************************************************************************/

void __attribute__((no_instrument_function)) __cyg_profile_func_enter(void* this_func, void* caller) {
printf("__cyg_profile_func_enter(%08lX, %08lX)\n", (uint32_t)this_func, (uint32_t)caller);
	if (crc_enable ^ CRC_SIGNATURE) {
		uint32_t blk_num = ((uint32_t)caller - (uint32_t)BLOCK_BASE) / sizeof(block_t);
		if (blk_num >= BLOCK_COUNT) {
			for(;;);
		}
		section1_profile_func_enter(blk_num, 0);
	}
}

void __attribute__((no_instrument_function)) __cyg_profile_func_exit(void *func, void *caller) {
    return;
}

/**************************************************************************
 *
 *     SECTION 1   SECTION 1   SECTION 1   SECTION 1   SECTION 1   SECTION 1
 *
 **************************************************************************/

void __attribute__((no_instrument_function)) section1_profile_func_enter(uint32_t block_number, uint32_t depth) {
printf("section1_profile_func_enter(%"PRIu32", %"PRIu32")\n", block_number, depth);
	depth++;

	if (depth < 3 && crc_check1(FUNCT1_BLOCK, clock) != 0)
	{
		/* If this section doesn't pass crc_check(), use the next section to fix this one. */
		section2_profile_func_enter(FUNCT1_BLOCK, depth);
	}

	if (crc_check1(block_number, clock) != 0)
	{
		/* Can't fix a block in the same physical flash section as this function */
		if (data_block_is_in_flash_section(block_number, FUNCT1_FLASH_SECTION)) {
			section2_profile_func_enter(block_number, depth);
		}
		else {
			reboot_and_fix_block(block_number, 1);
		}
	}
}

void __attribute__((no_instrument_function)) section1_fix_block(uint32_t block_number) {
printf("section1_fix_block(%"PRIu32")\n", block_number);
		if (data_block_is_in_flash_section(block_number, FUNCT1_FLASH_SECTION)) {
			section2_fix_block(block_number);
		}
		else {
			fix_block(block_number);
			crc_check1(block_number, clock);
		}
}

uint32_t __attribute__((no_instrument_function)) crc_check1(uint32_t block_number, uint64_t tm_now) {
printf("crc_check1(%"PRIu32", %"PRIu64")", block_number, tm_now);
	register uint32_t* ptr;
	register uint32_t* crc_ptr;
	uint32_t rc;

	if (crc_expire_times[block_number] > tm_now) {
printf(" Not time yet.\n");
		return 0;
	}

printf(" In progress");
	RCC->AHB1ENR |= RCC_AHB1Periph_CRC; /* Enable CRC */

	CRC->CR = CRC_CR_RESET; /* Reset CRC unit */

	ptr = (uint32_t*)BLOCK_START(block_number);
	crc_ptr = (uint32_t*)BLOCK_START(block_number + 1);
	crc_ptr--;

	while(ptr < crc_ptr)
	{
		CRC->DR = *ptr++;
	}

printf(" results %"PRIu32" ^ %"PRIu32"\n", CRC->DR, *crc_ptr);
	if (CRC->DR ^ *crc_ptr) {
		rc = 1;
	}
	else {
		rc = 0;
		crc_expire_times[block_number] = tm_now + CRC_EXPIRE_TIME;
	}

	RCC->AHB1ENR &= ~RCC_AHB1Periph_CRC; /* Disable CRC */

	return rc;
}

/**************************************************************************
 *
 *     SECTION 2   SECTION 2   SECTION 2   SECTION 2   SECTION 2   SECTION 2
 *
 **************************************************************************/

void __attribute__((no_instrument_function)) section2_profile_func_enter(uint32_t block_number, uint32_t depth) {
printf("section2_profile_func_enter(%"PRIu32", %"PRIu32")\n", block_number, depth);

	depth++;

	if (depth < 3 && crc_check2(FUNCT2_BLOCK, clock) != 0)
	{
		/* If this section doesn't pass crc_check(), use the next section to fix this one. */
		section3_profile_func_enter(FUNCT2_BLOCK, ++depth);
	}

	if (crc_check2(block_number, clock) != 0)
	{
		/* Can't fix a block in the same physical flash section as this function */
		if (data_block_is_in_flash_section(block_number, FUNCT2_FLASH_SECTION)) {
			section3_profile_func_enter(block_number, depth);
		}
		else {
			reboot_and_fix_block(block_number, 2);
		}
	}
}

void __attribute__((no_instrument_function)) section2_fix_block(uint32_t block_number) {
printf("section2_fix_block(%"PRIu32")\n", block_number);
		if (data_block_is_in_flash_section(block_number, FUNCT2_FLASH_SECTION)) {
			section3_fix_block(block_number);
		}
		else {
			fix_block(block_number);
			crc_check2(block_number, clock);
		}
}

uint32_t __attribute__((no_instrument_function)) crc_check2(uint32_t block_number, uint64_t tm_now) {
printf("crc_check2(%"PRIu32", %"PRIu64")", block_number, tm_now);
	register uint32_t* ptr;
	register uint32_t* crc_ptr;
	uint32_t rc;

	if (crc_expire_times[block_number] > tm_now) {
printf(" Not time yet.\n");
		return 0;
	}

	RCC->AHB1ENR |= RCC_AHB1Periph_CRC; /* Enable CRC */

	CRC->CR = CRC_CR_RESET; /* Reset CRC unit */

	ptr = (uint32_t*)BLOCK_START(block_number);
	crc_ptr = (uint32_t*)BLOCK_START(block_number + 1);
	crc_ptr--;

	while(ptr < crc_ptr)
	{
		CRC->DR = *ptr++;
	}

printf(" crc results %"PRIu32" ^ %"PRIu32"\n", CRC->DR, *crc_ptr);
	if (CRC->DR ^ *crc_ptr) {
		rc = 1;
	}
	else {
		rc = 0;
		crc_expire_times[block_number] = tm_now + CRC_EXPIRE_TIME;
	}

	RCC->AHB1ENR &= ~RCC_AHB1Periph_CRC; /* Disable CRC */

	return rc;
}

/**************************************************************************
 *
 *     SECTION 3   SECTION 3   SECTION 3   SECTION 3   SECTION 3   SECTION 3
 *
 **************************************************************************/


void __attribute__((no_instrument_function)) section3_profile_func_enter(uint32_t block_number, uint32_t depth) {
printf("section3_profile_func_enter(%"PRIu32", %"PRIu32")\n", block_number, depth);

	depth++;

	if (depth < 3 && crc_check3(FUNCT3_BLOCK, clock) != 0)
	{
		/* If this section doesn't pass crc_check(), use the next section to fix this one. */
		section1_profile_func_enter(FUNCT3_BLOCK, depth);
	}

	if (crc_check3(block_number, clock) != 0)
	{
		/* Can't fix a block in the same physical flash section as this function */
		if (data_block_is_in_flash_section(block_number, FUNCT3_FLASH_SECTION)) {
			section1_profile_func_enter(block_number, depth);
		}
		else {
			reboot_and_fix_block(block_number, 3);
		}
	}
}

void __attribute__((no_instrument_function)) section3_fix_block(uint32_t block_number) {
printf("section3_fix_block(%"PRIu32")\n", block_number);
		if (data_block_is_in_flash_section(block_number, FUNCT3_FLASH_SECTION)) {
			section1_fix_block(block_number);
		}
		else {
			crc_check3(block_number, clock);
			fix_block(block_number);
		}
}

uint32_t __attribute__((no_instrument_function)) crc_check3(uint32_t block_number, uint64_t tm_now) {
printf("crc_check3(%"PRIu32", %"PRIu64")", block_number, tm_now);
	register uint32_t* ptr;
	register uint32_t* crc_ptr;
	uint32_t rc;

	if (crc_expire_times[block_number] > tm_now) {
printf(" Not time yet.\n");
		return 0;
	}

	RCC->AHB1ENR |= RCC_AHB1Periph_CRC; /* Enable CRC */

	CRC->CR = CRC_CR_RESET; /* Reset CRC unit */

	ptr = (uint32_t*)BLOCK_START(block_number);
	crc_ptr = (uint32_t*)BLOCK_START(block_number + 1);
	crc_ptr--;

	while(ptr < crc_ptr)
	{
		CRC->DR = *ptr++;
	}

printf(" crc results %"PRIu32" ^ %"PRIu32"\n", CRC->DR, *crc_ptr);
	if (CRC->DR ^ *crc_ptr) {
		rc = 1;
	}
	else {
		rc = 0;
		crc_expire_times[block_number] = tm_now + CRC_EXPIRE_TIME;
	}

	RCC->AHB1ENR &= ~RCC_AHB1Periph_CRC; /* Disable CRC */

	return rc;
}



