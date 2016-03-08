
typedef struct {
	uint32_t signature;
	uint16_t fixer;
	uint16_t block_number;
} reboot_block_t;

#define SEU_FIX_SIGNATURE 0x87654321
#define BLOCK_COUNT 	((896 * 1024) / 13312) /* 68 */

void __attribute__((no_instrument_function)) section1_fix_block(uint32_t block_number);
void __attribute__((no_instrument_function)) section2_fix_block(uint32_t block_number);
void __attribute__((no_instrument_function)) section3_fix_block(uint32_t block_number);

inline static reboot_block_t* INLINE_ATTRIBUTE access_backup_domain_sram() {
	/* Get access to the 4K of SRAM in the backup domain */
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
	PWR_BackupAccessCmd(ENABLE)
    RCC->RCC_AHB1ENR |= RCC_AHB1ENR_BKPSRAMEN;
	return (reboot_block_t*)BKPSRAM_BASE;
}

inline static void INLINE_ATTRIBUTE reboot_and_fix_block(uint32_t block_number, uint32_t fixer) {
	/* Disable all interrupts */
	RCC->CIR = 0x00000000;

	reboot_block_t* backup_sram = access_backup_domain_sram();
	backup_sram->signature = SEU_FIX_SIGNATURE;
	backup_sram->block_number = (uint16_t)block_number;
	backup_sram->fixer = (uint16_t)fixer;
	NVIC_SystemReset();
}

inline static void INLINE_ATTRIBUTE seu_start_check()
{
	reboot_block_t* backup_sram = access_backup_domain_sram();

	if (backup_sram->signature != SEU_FIX_SIGNATURE) {
		/* This is a power on reset */
		backup_sram->signature = SEU_FIX_SIGNATURE;
		backup_sram->block_number = (uint16_t)0xFFFF;
		backup_sram->fixer = (uint16_t)0xFFFF;
	}
	else {
		if (backup_sram->block_number < BLOCK_COUNT) {
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
			NVIC_SystemReset();
		}
	}
}