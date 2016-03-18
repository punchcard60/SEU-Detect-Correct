#include <reboot.h>
#include <uart.h>
#include <inttypes.h>
#include <stdio.h>
#include <seu.h>

void seu_start_check(void)
{
	uart_init();

debug("accessing backup SRAM\n");

	/* Get access to the 4K of SRAM in the backup domain */
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
	PWR->CR = PWR_CR_DBP;
    RCC->AHB1ENR |= RCC_AHB1ENR_BKPSRAMEN;
	reboot_block_t* backup_sram = (reboot_block_t*)BKPSRAM_BASE;
debug(" ok\n");

	if (backup_sram->signature != SEU_FIX_SIGNATURE) {
		/* This is a power on reset */
debug("   New Signature\n");
		backup_sram->signature = SEU_FIX_SIGNATURE;
		backup_sram->block_number = (uint16_t)0xFFFF;
		backup_sram->fixer = (uint16_t)0xFFFF;
	}
	else {
		if (backup_sram->block_number < BLOCK_COUNT) {
			debug("   Fix needed in block\n");
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
			debug("reboot\n");
			reboot();
		}
		if (backup_sram->block_number != (uint16_t)0xFFFF) {
			debug("   bad sram block number %d\n", backup_sram->block_number);
		}
	}

	debug("Turn on the crc background process\n");
	seu_init();
	debug("Startup check complete\n");
}

