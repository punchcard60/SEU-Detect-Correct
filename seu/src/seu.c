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
#include <seu.h>
#include <stdio.h>
#include <inttypes.h>
#include <crcmodel.h>

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
printf("seu_init()\n");
	htimer = xTimerCreate("SEU", 300, pdTRUE, NULL, seu_timer);
printf("timer created\n");
	xTimerStart(htimer, 0);
printf("timer started\n");
printf("seu_init() exit\n");
}

void seu_timer(TimerHandle_t pxTimer) {
	uint32_t i = 0;
printf("Block Check");
	while (i <= FUNCT2_BLOCK) {
		section3_check_block(i++);
	}
	while (i < BLOCK_COUNT) {
		section1_check_block(i++);
	}
printf("  END\n");
}

/**************************************************************************
 *
 *     SECTION 1   SECTION 1   SECTION 1   SECTION 1   SECTION 1   SECTION 1
 *
 **************************************************************************/

void section1_check_block(uint32_t block_number) {

	/* Can't fix a block in the same physical flash section as this function */
	if (data_block_is_in_flash_section(block_number, FUNCT1_FLASH_SECTION)) {
		section2_check_block(block_number);
	}
	else {
		if (crc_check1(block_number) != 0)
		{
			reboot_and_fix_block(block_number, 1);
		}
	}
}

void section1_fix_block(uint32_t block_number) {
	if (data_block_is_in_flash_section(block_number, FUNCT1_FLASH_SECTION)) {
		section2_fix_block(block_number);
	}else
	{
		fix_block(block_number, crc_fix1);
	}
}

uint32_t crc_calc1(uint8_t* ptr, uint8_t* crc_ptr) {
	cm_t crc_model;

     crc_model.cm_width = 32;            // 32-bit CRC
     crc_model.cm_poly  = 0x04C11DB7;    // CRC-32 polynomial
     crc_model.cm_init  = 0xFFFFFFFF;    // CRC initialized to 1's
     crc_model.cm_refin = 0;         // CRC calculated MSB first
     crc_model.cm_refot = 0;         // Final result is not bit-reversed
     crc_model.cm_xorot = 0x00000000;    // Final result XOR'ed with this

     cm_ini(&crc_model);

	while(ptr < crc_ptr)
	{
		 cm_nxt(&crc_model,*ptr++);
	}

	return cm_crc(&crc_model);
}

uint32_t crc_check1(uint32_t block_number) {
	uint8_t* ptr = (uint8_t*)BLOCK_START(block_number);
	uint8_t* crc_ptr = &ptr[sizeof(block_t) - sizeof(uint32_t)];
	uint32_t crc, ref_crc;

	crc = crc_calc1(ptr, crc_ptr);
	ref_crc = *((uint32_t*)crc_ptr);

	return (crc ^ ref_crc);
}

void crc_fix1(uint32_t block_number, error_marker_t* marker) {
	uint8_t* ptr = (uint8_t*)BLOCK_START(block_number);
	uint8_t* crc_ptr = &ptr[sizeof(block_t) - sizeof(uint32_t)];

	marker->corrected_dword = crc_calc1(ptr, crc_ptr) ^ *((uint32_t*)crc_ptr);
	marker->pointer = (uint32_t*)crc_ptr;
}

/**************************************************************************
 *
 *     SECTION 2   SECTION 2   SECTION 2   SECTION 2   SECTION 2   SECTION 2
 *
 **************************************************************************/

void section2_check_block(uint32_t block_number) {

	/* Can't fix a block in the same physical flash section as this function */
	if (data_block_is_in_flash_section(block_number, FUNCT2_FLASH_SECTION)) {
		section3_check_block(block_number);
	}
	else {
		if (crc_check2(block_number) != 0)
		{
			reboot_and_fix_block(block_number, 2);
		}
	}
}

void section2_fix_block(uint32_t block_number) {
	if (data_block_is_in_flash_section(block_number, FUNCT2_FLASH_SECTION)) {
		section3_fix_block(block_number);
	}else
	{
		fix_block(block_number, crc_fix2);
	}
}

uint32_t crc_calc2(uint8_t* ptr, uint8_t* crc_ptr) {
	cm_t crc_model;

     crc_model.cm_width = 32;            // 32-bit CRC
     crc_model.cm_poly  = 0x04C11DB7;    // CRC-32 polynomial
     crc_model.cm_init  = 0xFFFFFFFF;    // CRC initialized to 1's
     crc_model.cm_refin = 0;         // CRC calculated MSB first
     crc_model.cm_refot = 0;         // Final result is not bit-reversed
     crc_model.cm_xorot = 0x00000000;    // Final result XOR'ed with this

     cm_ini(&crc_model);

	while(ptr < crc_ptr)
	{
		 cm_nxt(&crc_model,*ptr++);
	}

	return cm_crc(&crc_model);
}

uint32_t crc_check2(uint32_t block_number) {
	uint8_t* ptr = (uint8_t*)BLOCK_START(block_number);
	uint8_t* crc_ptr = &ptr[sizeof(block_t) - sizeof(uint32_t)];
	uint32_t crc, ref_crc;

	crc = crc_calc2(ptr, crc_ptr);
	ref_crc = *((uint32_t*)crc_ptr);

	return (crc ^ ref_crc);
}

void crc_fix2(uint32_t block_number, error_marker_t* marker) {
	uint8_t* ptr = (uint8_t*)BLOCK_START(block_number);
	uint8_t* crc_ptr = &ptr[sizeof(block_t) - sizeof(uint32_t)];

	marker->corrected_dword = crc_calc2(ptr, crc_ptr) ^ *((uint32_t*)crc_ptr);
	marker->pointer = (uint32_t*)crc_ptr;
}

/**************************************************************************
 *
 *     SECTION 3   SECTION 3   SECTION 3   SECTION 3   SECTION 3   SECTION 3
 *
 **************************************************************************/

void section3_check_block(uint32_t block_number) {

	/* Can't fix a block in the same physical flash section as this function */
	if (data_block_is_in_flash_section(block_number, FUNCT3_FLASH_SECTION)) {
		section1_check_block(block_number);
	}
	else {
		if (crc_check3(block_number) != 0)
		{
			reboot_and_fix_block(block_number, 3);
		}
	}
}

void section3_fix_block(uint32_t block_number) {
	if (data_block_is_in_flash_section(block_number, FUNCT3_FLASH_SECTION)) {
		section1_fix_block(block_number);
	}else
	{
		fix_block(block_number, crc_fix3);
	}
}

uint32_t crc_calc3(uint8_t* ptr, uint8_t* crc_ptr) {
	cm_t crc_model;

     crc_model.cm_width = 32;            // 32-bit CRC
     crc_model.cm_poly  = 0x04C11DB7;    // CRC-32 polynomial
     crc_model.cm_init  = 0xFFFFFFFF;    // CRC initialized to 1's
     crc_model.cm_refin = 0;         // CRC calculated MSB first
     crc_model.cm_refot = 0;         // Final result is not bit-reversed
     crc_model.cm_xorot = 0x00000000;    // Final result XOR'ed with this

     cm_ini(&crc_model);

	while(ptr < crc_ptr)
	{
		 cm_nxt(&crc_model,*ptr++);
	}

	return cm_crc(&crc_model);
}

uint32_t crc_check3(uint32_t block_number) {
	uint8_t* ptr = (uint8_t*)BLOCK_START(block_number);
	uint8_t* crc_ptr = &ptr[sizeof(block_t) - sizeof(uint32_t)];
	uint32_t crc, ref_crc;

	crc = crc_calc3(ptr, crc_ptr);
	ref_crc = *((uint32_t*)crc_ptr);

	return (crc ^ ref_crc);
}

void crc_fix3(uint32_t block_number, error_marker_t* marker) {
	uint8_t* ptr = (uint8_t*)BLOCK_START(block_number);
	uint8_t* crc_ptr = &ptr[sizeof(block_t) - sizeof(uint32_t)];

	marker->corrected_dword = crc_calc3(ptr, crc_ptr) ^ *((uint32_t*)crc_ptr);
	marker->pointer = (uint32_t*)crc_ptr;
}


