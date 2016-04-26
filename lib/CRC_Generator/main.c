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

#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "crcmodel.h"
#include <reed_solomon.h>
#include <encode_rs.h>


uint32_t* 	RS_DATA;
uint32_t* 	RS_PARITY;
uint32_t 	BLOCK_COUNT;
uint32_t*   CRCs;

int main(int argc, char** argv) {
    // arguments:
    // argv[1] input file name
    // argv[2] offset to start of .text section
    // argv[3] offset to start of .ecc_data section
    // argv[3] output file name

    if (argc != 5) {
        fprintf(stderr, "Usage: <input.elf> <.text-offset> <.ecc-offset> <output.elf>\n");
        exit(1);
    }

    FILE *inputFile, *outputFile;

    if ((inputFile = fopen(argv[1], "rb")) == NULL) {
        printf("Error opening %s\n", argv[1]);
        return 1;
    }

    if ((outputFile = fopen(argv[4], "wb")) == NULL) {
        printf("Error opening %s\n", argv[4]);
        return 1;
    }

    int inputFileLen;
    char* inputData;

    if (fseek(inputFile, 0L, SEEK_END) == 0) { //Gets length of file and reads input file in to memory
        inputFileLen = ftell(inputFile);
        inputData = malloc(inputFileLen);
        fseek(inputFile, 0L, SEEK_SET);
        fread(inputData, sizeof(char), inputFileLen, inputFile);
    }

    uint32_t text_start_offs = strtol(argv[2], NULL, 16); /* The offset of the .text section in the executable
												* is passed into the CRCGenerator as argv[2]
												*/

    uint8_t* data_ptr = (uint8_t*)&inputData[text_start_offs]; //pointer to symbol defined in Linker Script


    uint32_t ecc_start_offs = strtol(argv[3], NULL, 16); /* The offset of the .ecc_data section in the executable
														   * is passed into the CRCGenerator as argv[3]
														   */

    uint32_t* ecc_ptr = (uint32_t*)&inputData[ecc_start_offs]; //pointer to symbol defined in Linker Script

    RS_DATA = (uint32_t*)data_ptr;
    RS_PARITY = (uint32_t*)(&data_ptr[ecc_ptr[1] - ecc_ptr[0]]);
    CRCs = (uint32_t*)(&data_ptr[ecc_ptr[2] - ecc_ptr[0]]);
    BLOCK_COUNT = ecc_ptr[3];

    int idx;

    for (idx = 0; idx < BLOCK_COUNT; idx++) {
        encode_rs(idx);
	}

    for (idx = 0; idx < BLOCK_COUNT; idx++) {
        CRCs[idx] = CRC_CalcBlockCRC((uint8_t*)&data_ptr[idx * SYMBOL_TABLE_WORDS], SYMBOL_TABLE_WORDS * sizeof(uint32_t));
		printf("%d   %"PRIu32"\n", idx, CRCs[idx]);
	}



    //Write modified binary to new file
    fwrite(inputData, sizeof(char), inputFileLen, outputFile);

    free(inputData);
    return 0;
}
