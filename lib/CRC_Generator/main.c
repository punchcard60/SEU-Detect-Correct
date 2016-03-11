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

#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "crcmodel.h"
/* #include "crc_generator.h" */
#include <reed_solomon.h>
#include <encode_rs.h>

typedef struct block {
	uint16_t	reed_solomon_data[SYMBOL_TABLE_WORDS]; /*same as 3328 * sizeof(uint32_t) so the alignment works. */
	uint32_t	crc;
} block_t;

int main(int argc, char** argv) {
    // arguments:
    // argv[1] input file name
    // argv[2] offset to start of .text section
    // argv[3] output file name

    if (argc != 4) {
        fprintf(stderr, "Usage: <input.elf> <.text-offset> <output.elf>\n");
        exit(1);
    }

    FILE *inputFile, *outputFile;

    if ((inputFile = fopen(argv[1], "rb")) == NULL) {
        printf("Error opening %s\n", argv[1]);
        return 1;
    }

    if ((outputFile = fopen(argv[3], "wb")) == NULL) {
        printf("Error opening %s\n", argv[3]);
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

    uint32_t text_start_offs = strtol(argv[2], NULL, 16);
	uint32_t block_start_offset = text_start_offs - *((uint32_t*)(inputData + text_start_offs));
    block_t* blocks = (block_t*)(inputData + block_start_offset);
    uint32_t blockCount = *((uint32_t*)(inputData + text_start_offs + 4)); //pointer to symbol defined in Linker Script

    int idx, numBytes;
	numBytes = (sizeof(block_t) - sizeof(uint32_t));
    for (idx = 0; idx < blockCount; idx++) {
        encode_rs((word_t*) &blocks[idx]);
        blocks[idx].crc = CRC_CalcBlockCRC((uint8_t*)&blocks[idx], numBytes);
		printf("%d   %"PRIu32"\n", idx, blocks[idx].crc);
	}

    //Write modified binary to new file
    fwrite(inputData, sizeof(char), inputFileLen, outputFile);

    free(inputData);
    return 0;
}
