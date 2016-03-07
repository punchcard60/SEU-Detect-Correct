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

#include <stm32f4xx.h>
#include <trace_functions.h>

 uint32_t crc_expire_times[BLOCK_COUNT];

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

void __cyg_profile_func_enter (void* this_func, void* caller) {
	section1_profile_func_enter((uint32_t)caller - (uint32_t)BLOCK_BASE / sizeof(block_t));
}

/**************************************************************************/

void __cyg_profile_func_exit(void *func, void *caller) {
    return;
}

/**************************************************************************/

static void __attribute__((no_instrument_function)) section1_profile_func_enter(uint32_t block_number) {

	if (crc_check(FUNCT1_BLOCK, get_the_time()) != 0)
	{
		/* If this section doesn't pass crc_check(), use the next section to fix this one. */
		section2_profile_func_enter(FUNCT1_BLOCK);
	}

	if (crc_check(block_number, get_the_time()) != 0)
	{
		/* Can't fix a block in the same physical flash section as this function */
		if (data_block_is_in_flash_section(block_number, FUNCT1_FLASH_SECTION)) {
			section2_profile_func_enter(block_number);
		}
		else {
			fix_block(block_number);
		}
	}
}

/**************************************************************************/

static void __attribute__((no_instrument_function)) section2_profile_func_enter(uint32_t block_number) {

	if (crc_check(FUNCT2_BLOCK, get_the_time()) != 0)
	{
		/* If this section doesn't pass crc_check(), use the next section to fix this one. */
		section3_profile_func_enter(FUNCT2_BLOCK);
	}

	if (crc_check(block_number, get_the_time()) != 0)
	{
		/* Can't fix a block in the same physical flash section as this function */
		if (data_block_is_in_flash_section(block_number, FUNCT2_FLASH_SECTION)) {
			section3_profile_func_enter(block_number);
		}
		else {
			fix_block(block_number);
		}
	}
}

/**************************************************************************/

static void __attribute__((no_instrument_function)) section3_profile_func_enter(uint32_t block_number) {

	if (crc_check(FUNCT3_BLOCK, get_the_time()) != 0)
	{
		/* If this section doesn't pass crc_check(), use the next section to fix this one. */
		section3_profile_func_enter(FUNCT3_BLOCK);
	}

	if (crc_check(block_number, get_the_time()) != 0)
	{
		/* Can't fix a block in the same physical flash section as this function */
		if (data_block_is_in_flash_section(block_number, FUNCT3_FLASH_SECTION)) {
			section1_profile_func_enter(block_number);
		}
		else {
			fix_block(block_number);
		}
	}
}



