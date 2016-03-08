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

uint64_t crc_expire_times[BLOCK_COUNT];
uint64_t clock = 0;

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
void seu_timer(TimerHandle_t pxTimer);

void seu_init(void) {
	htimer = xTimerCreate("SEU", 100, pdTRUE, 0, seu_timer);
	xTimerStart(htimer, 0);
}

void seu_timer(TimerHandle_t pxTimer) {
	clock++;
}

void __cyg_profile_func_enter (void* this_func, void* caller) {
	section1_profile_func_enter((uint32_t)caller - (uint32_t)BLOCK_BASE / sizeof(block_t), 0);
}

/**************************************************************************/

void __cyg_profile_func_exit(void *func, void *caller) {
    return;
}

/**************************************************************************/

void __attribute__((no_instrument_function)) section1_profile_func_enter(uint32_t block_number, uint32_t depth) {

	depth++;

	if (depth < 3 && crc_check(FUNCT1_BLOCK, clock) != 0)
	{
		/* If this section doesn't pass crc_check(), use the next section to fix this one. */
		section2_profile_func_enter(FUNCT1_BLOCK, depth);
	}

	if (crc_check(block_number, clock) != 0)
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
		if (data_block_is_in_flash_section(block_number, FUNCT1_FLASH_SECTION)) {
			section2_fix_block(block_number);
		}
		else {
			fix_block(block_number);
		}
}

/**************************************************************************/

void __attribute__((no_instrument_function)) section2_profile_func_enter(uint32_t block_number, uint32_t depth) {

	depth++;

	if (depth < 3 && crc_check(FUNCT2_BLOCK, clock) != 0)
	{
		/* If this section doesn't pass crc_check(), use the next section to fix this one. */
		section3_profile_func_enter(FUNCT2_BLOCK, ++depth);
	}

	if (crc_check(block_number, clock) != 0)
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
		if (data_block_is_in_flash_section(block_number, FUNCT2_FLASH_SECTION)) {
			section3_fix_block(block_number);
		}
		else {
			fix_block(block_number);
		}
}

/**************************************************************************/

void __attribute__((no_instrument_function)) section3_profile_func_enter(uint32_t block_number, uint32_t depth) {

	depth++;

	if (depth < 3 && crc_check(FUNCT3_BLOCK, clock) != 0)
	{
		/* If this section doesn't pass crc_check(), use the next section to fix this one. */
		section1_profile_func_enter(FUNCT3_BLOCK, depth);
	}

	if (crc_check(block_number, clock) != 0)
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
		if (data_block_is_in_flash_section(block_number, FUNCT3_FLASH_SECTION)) {
			section1_fix_block(block_number);
		}
		else {
			fix_block(block_number);
		}
}



