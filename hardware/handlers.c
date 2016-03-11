


void __attribute__((no_instrument_function)) NMI_Handler(void) { for(;;); }


void __attribute__((no_instrument_function)) HardFault_Handler(void) {
	for(;;);
}
/*
#define HFSR ((uint32_t*)0xE000ED2C)
SCB_HFSR_DEBUGEVT_Msk
SCB_HFSR_FORCED_Msk
SCB_HFSR_VECTTBL_Msk
*/
void __attribute__((no_instrument_function)) MemManage_Handler(void) { for(;;); }


void __attribute__((no_instrument_function)) BusFault_Handler(void) { for(;;); }


void __attribute__((no_instrument_function)) UsageFault_Handler(void) { for(;;); }


//void __attribute__((no_instrument_function)) SVC_Handler(void) { for(;;); }


void __attribute__((no_instrument_function)) DebugMon_Handler(void) { for(;;); }


//void __attribute__((no_instrument_function)) PendSV_Handler(void) { for(;;); }


//void __attribute__((no_instrument_function)) SysTick_Handler(void) { for(;;); }


void __attribute__((no_instrument_function)) WWDG_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) PVD_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) TAMP_STAMP_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) RTC_WKUP_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) FLASH_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) RCC_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) EXTI0_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) EXTI1_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) EXTI2_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) EXTI3_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) EXTI4_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) DMA1_Stream0_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) DMA1_Stream1_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) DMA1_Stream2_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) DMA1_Stream3_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) DMA1_Stream4_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) DMA1_Stream5_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) DMA1_Stream6_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) ADC_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) CAN1_TX_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) CAN1_RX0_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) CAN1_RX1_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) CAN1_SCE_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) EXTI9_5_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) TIM1_BRK_TIM9_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) TIM1_UP_TIM10_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) TIM1_TRG_COM_TIM11_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) TIM1_CC_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) TIM2_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) TIM3_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) TIM4_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) I2C1_EV_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) I2C1_ER_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) I2C2_EV_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) I2C2_ER_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) SPI1_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) SPI2_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) USART1_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) USART2_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) USART3_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) EXTI15_10_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) RTC_Alarm_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) OTG_FS_WKUP_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) TIM8_BRK_TIM12_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) TIM8_UP_TIM13_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) TIM8_TRG_COM_TIM14_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) TIM8_CC_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) DMA1_Stream7_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) FSMC_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) SDIO_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) TIM5_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) SPI3_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) UART4_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) UART5_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) TIM6_DAC_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) TIM7_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) DMA2_Stream0_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) DMA2_Stream1_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) DMA2_Stream2_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) DMA2_Stream3_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) DMA2_Stream4_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) ETH_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) ETH_WKUP_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) CAN2_TX_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) CAN2_RX0_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) CAN2_RX1_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) CAN2_SCE_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) OTG_FS_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) DMA2_Stream5_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) DMA2_Stream6_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) DMA2_Stream7_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) USART6_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) I2C3_EV_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) I2C3_ER_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) OTG_HS_EP1_OUT_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) OTG_HS_EP1_IN_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) OTG_HS_WKUP_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) OTG_HS_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) DCMI_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) CRYP_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) HASH_RNG_IRQHandler(void) { for(;;); }


void __attribute__((no_instrument_function)) FPU_IRQHandler(void) { for(;;); }

