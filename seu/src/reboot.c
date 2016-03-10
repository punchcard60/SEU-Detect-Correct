#include <reboot.h>
#include <uart.h>
#include <inttypes.h>
#include <stdio.h>
#include <trace_functions.h>

void __attribute__((no_instrument_function)) seu_start_check(void)
{
	crc_enable = CRC_SIGNATURE;

	uart_init();

	reboot_block_t* backup_sram = access_backup_domain_sram();

	if (backup_sram->signature != SEU_FIX_SIGNATURE) {
		/* This is a power on reset */
printf("New Signature\n");
		backup_sram->signature = SEU_FIX_SIGNATURE;
		backup_sram->block_number = (uint16_t)0xFFFF;
		backup_sram->fixer = (uint16_t)0xFFFF;
	}
	else {
		if (backup_sram->block_number < BLOCK_COUNT) {
printf("Fix needed in block %d\n", backup_sram->block_number);
			/* we're in the middle of a fixup */
			switch(backup_sram->fixer) {
				case 1:
					backup_sram->fixer = 2u;
					section1_fix_block(backup_sram->block_number);
					break;

				case 2:
					backup_sram->fixer = 3u;
					section2_fix_block(backup_sram->block_number);
					break;

				default:
					backup_sram->fixer = 1u;
					section3_fix_block(backup_sram->block_number);
			}
			/* Must have been successful fixing the block */
			backup_sram->block_number = (uint16_t)0xFFFF;
			backup_sram->fixer = (uint16_t)0xFFFF;
			reboot();
		}
	}
	seu_init(); /* Kicks off a timer to update the expiration clock */
}

