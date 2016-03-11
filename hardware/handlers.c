
#include <stdint.h>

void __attribute__((naked)) NMI_Handler(void) { for(;;); }

void prvGetRegistersFromStack( uint32_t *pulFaultStackAddress )
{
	/* These are volatile to try and prevent the compiler/linker optimising them
	away as the variables never actually get used.  If the debugger won't show the
	values of the variables, make them global my moving their declaration outside
	of this function. */
	volatile uint32_t __attribute__((unused)) r0;
	volatile uint32_t __attribute__((unused)) r1;
	volatile uint32_t __attribute__((unused)) r2;
	volatile uint32_t __attribute__((unused)) r3;
	volatile uint32_t __attribute__((unused)) r12;
	volatile uint32_t __attribute__((unused)) lr; /* Link register. */
	volatile uint32_t __attribute__((unused)) pc; /* Program counter. */
	volatile uint32_t __attribute__((unused)) psr;/* Program status register. */


    r0 = pulFaultStackAddress[ 0 ];
    r1 = pulFaultStackAddress[ 1 ];
    r2 = pulFaultStackAddress[ 2 ];
    r3 = pulFaultStackAddress[ 3 ];

    r12 = pulFaultStackAddress[ 4 ];
    lr = pulFaultStackAddress[ 5 ];
    pc = pulFaultStackAddress[ 6 ];
    psr = pulFaultStackAddress[ 7 ];

    /* When the following line is hit, the variables contain the register values. */
    for( ;; );
}


void __attribute__((naked)) HardFault_Handler(void) {
    __asm volatile
    (
        " tst lr, #4                                                \n"
        " ite eq                                                    \n"
        " mrseq r0, msp                                             \n"
        " mrsne r0, psp                                             \n"
        " ldr r1, [r0, #24]                                         \n"
        " ldr r2, handler2_address_const                            \n"
        " bx r2                                                     \n"
        " handler2_address_const: .word prvGetRegistersFromStack    \n"
    );
}

/*
#define HFSR ((uint32_t*)0xE000ED2C)
SCB_HFSR_DEBUGEVT_Msk
SCB_HFSR_FORCED_Msk
SCB_HFSR_VECTTBL_Msk
*/
void __attribute__((naked)) MemManage_Handler(void) { for(;;); }


void __attribute__((naked)) BusFault_Handler(void) { for(;;); }


void __attribute__((naked)) UsageFault_Handler(void) { for(;;); }


//void __attribute__((naked)) SVC_Handler(void) { for(;;); }


void __attribute__((naked)) DebugMon_Handler(void) { for(;;); }


//void __attribute__((naked)) PendSV_Handler(void) { for(;;); }


//void __attribute__((naked)) SysTick_Handler(void) { for(;;); }


void __attribute__((naked)) WWDG_IRQHandler(void) { for(;;); }


void __attribute__((naked)) PVD_IRQHandler(void) { for(;;); }


void __attribute__((naked)) TAMP_STAMP_IRQHandler(void) { for(;;); }


void __attribute__((naked)) RTC_WKUP_IRQHandler(void) { for(;;); }


void __attribute__((naked)) FLASH_IRQHandler(void) { for(;;); }


void __attribute__((naked)) RCC_IRQHandler(void) { for(;;); }


void __attribute__((naked)) EXTI0_IRQHandler(void) { for(;;); }


void __attribute__((naked)) EXTI1_IRQHandler(void) { for(;;); }


void __attribute__((naked)) EXTI2_IRQHandler(void) { for(;;); }


void __attribute__((naked)) EXTI3_IRQHandler(void) { for(;;); }


void __attribute__((naked)) EXTI4_IRQHandler(void) { for(;;); }


void __attribute__((naked)) DMA1_Stream0_IRQHandler(void) { for(;;); }


void __attribute__((naked)) DMA1_Stream1_IRQHandler(void) { for(;;); }


void __attribute__((naked)) DMA1_Stream2_IRQHandler(void) { for(;;); }


void __attribute__((naked)) DMA1_Stream3_IRQHandler(void) { for(;;); }


void __attribute__((naked)) DMA1_Stream4_IRQHandler(void) { for(;;); }


void __attribute__((naked)) DMA1_Stream5_IRQHandler(void) { for(;;); }


void __attribute__((naked)) DMA1_Stream6_IRQHandler(void) { for(;;); }


void __attribute__((naked)) ADC_IRQHandler(void) { for(;;); }


void __attribute__((naked)) CAN1_TX_IRQHandler(void) { for(;;); }


void __attribute__((naked)) CAN1_RX0_IRQHandler(void) { for(;;); }


void __attribute__((naked)) CAN1_RX1_IRQHandler(void) { for(;;); }


void __attribute__((naked)) CAN1_SCE_IRQHandler(void) { for(;;); }


void __attribute__((naked)) EXTI9_5_IRQHandler(void) { for(;;); }


void __attribute__((naked)) TIM1_BRK_TIM9_IRQHandler(void) { for(;;); }


void __attribute__((naked)) TIM1_UP_TIM10_IRQHandler(void) { for(;;); }


void __attribute__((naked)) TIM1_TRG_COM_TIM11_IRQHandler(void) { for(;;); }


void __attribute__((naked)) TIM1_CC_IRQHandler(void) { for(;;); }


void __attribute__((naked)) TIM2_IRQHandler(void) { for(;;); }


void __attribute__((naked)) TIM3_IRQHandler(void) { for(;;); }


void __attribute__((naked)) TIM4_IRQHandler(void) { for(;;); }


void __attribute__((naked)) I2C1_EV_IRQHandler(void) { for(;;); }


void __attribute__((naked)) I2C1_ER_IRQHandler(void) { for(;;); }


void __attribute__((naked)) I2C2_EV_IRQHandler(void) { for(;;); }


void __attribute__((naked)) I2C2_ER_IRQHandler(void) { for(;;); }


void __attribute__((naked)) SPI1_IRQHandler(void) { for(;;); }


void __attribute__((naked)) SPI2_IRQHandler(void) { for(;;); }


void __attribute__((naked)) USART1_IRQHandler(void) { for(;;); }


void __attribute__((naked)) USART2_IRQHandler(void) { for(;;); }


void __attribute__((naked)) USART3_IRQHandler(void) { for(;;); }


void __attribute__((naked)) EXTI15_10_IRQHandler(void) { for(;;); }


void __attribute__((naked)) RTC_Alarm_IRQHandler(void) { for(;;); }


void __attribute__((naked)) OTG_FS_WKUP_IRQHandler(void) { for(;;); }


void __attribute__((naked)) TIM8_BRK_TIM12_IRQHandler(void) { for(;;); }


void __attribute__((naked)) TIM8_UP_TIM13_IRQHandler(void) { for(;;); }


void __attribute__((naked)) TIM8_TRG_COM_TIM14_IRQHandler(void) { for(;;); }


void __attribute__((naked)) TIM8_CC_IRQHandler(void) { for(;;); }


void __attribute__((naked)) DMA1_Stream7_IRQHandler(void) { for(;;); }


void __attribute__((naked)) FSMC_IRQHandler(void) { for(;;); }


void __attribute__((naked)) SDIO_IRQHandler(void) { for(;;); }


void __attribute__((naked)) TIM5_IRQHandler(void) { for(;;); }


void __attribute__((naked)) SPI3_IRQHandler(void) { for(;;); }


void __attribute__((naked)) UART4_IRQHandler(void) { for(;;); }


void __attribute__((naked)) UART5_IRQHandler(void) { for(;;); }


void __attribute__((naked)) TIM6_DAC_IRQHandler(void) { for(;;); }


void __attribute__((naked)) TIM7_IRQHandler(void) { for(;;); }


void __attribute__((naked)) DMA2_Stream0_IRQHandler(void) { for(;;); }


void __attribute__((naked)) DMA2_Stream1_IRQHandler(void) { for(;;); }


void __attribute__((naked)) DMA2_Stream2_IRQHandler(void) { for(;;); }


void __attribute__((naked)) DMA2_Stream3_IRQHandler(void) { for(;;); }


void __attribute__((naked)) DMA2_Stream4_IRQHandler(void) { for(;;); }


void __attribute__((naked)) ETH_IRQHandler(void) { for(;;); }


void __attribute__((naked)) ETH_WKUP_IRQHandler(void) { for(;;); }


void __attribute__((naked)) CAN2_TX_IRQHandler(void) { for(;;); }


void __attribute__((naked)) CAN2_RX0_IRQHandler(void) { for(;;); }


void __attribute__((naked)) CAN2_RX1_IRQHandler(void) { for(;;); }


void __attribute__((naked)) CAN2_SCE_IRQHandler(void) { for(;;); }


void __attribute__((naked)) OTG_FS_IRQHandler(void) { for(;;); }


void __attribute__((naked)) DMA2_Stream5_IRQHandler(void) { for(;;); }


void __attribute__((naked)) DMA2_Stream6_IRQHandler(void) { for(;;); }


void __attribute__((naked)) DMA2_Stream7_IRQHandler(void) { for(;;); }


void __attribute__((naked)) USART6_IRQHandler(void) { for(;;); }


void __attribute__((naked)) I2C3_EV_IRQHandler(void) { for(;;); }


void __attribute__((naked)) I2C3_ER_IRQHandler(void) { for(;;); }


void __attribute__((naked)) OTG_HS_EP1_OUT_IRQHandler(void) { for(;;); }


void __attribute__((naked)) OTG_HS_EP1_IN_IRQHandler(void) { for(;;); }


void __attribute__((naked)) OTG_HS_WKUP_IRQHandler(void) { for(;;); }


void __attribute__((naked)) OTG_HS_IRQHandler(void) { for(;;); }


void __attribute__((naked)) DCMI_IRQHandler(void) { for(;;); }


void __attribute__((naked)) CRYP_IRQHandler(void) { for(;;); }


void __attribute__((naked)) HASH_RNG_IRQHandler(void) { for(;;); }


void __attribute__((naked)) FPU_IRQHandler(void) { for(;;); }

