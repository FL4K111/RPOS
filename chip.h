#ifndef __CHIP_H
#define __CHIP_H


#define ATOMIC_XOR 0x1000
#define ATOMIC_SET 0x2000
#define ATOMIC_CLR 0x3000 

//base address
#define RESETS_BASE 0x40020000
#define CLOCKS_BASE 0x40010000
#define TICKS_BASE 0x40108000
#define XOSC_BASE 0x40048000

#define UART0_BASE 0x40070000

#define IO_BANK0_BASE 0x40028000
#define PADS_BANK0_BASE   0x40038000
#define SIO_BASE 0xd0000000

#define TIMER0_BASE 0x400b0000
#define TIMER1_BASE 0x400b8000

//每个外设，在使用前的基本操作是reset，故init的头两句基本相同
//但是，如果程序简单也可以直接设置，因为最开始就是reset状态
//reset register 

#define RESET_ADDR (RESETS_BASE + 0x00)
#define RESET_DONE_ADDR (RESETS_BASE + 0x08)

#define RESET_SET_REG (*(volatile uint32_t *)(RESET_ADDR + ATOMIC_SET))
#define RESET_XOR_REG (*(volatile uint32_t *)(RESET_ADDR + ATOMIC_XOR))
#define RESET_CLR_REG (*(volatile uint32_t *)(RESET_ADDR + ATOMIC_CLR))

#define RESET_DONE_REG (*(volatile uint32_t *) RESET_DONE_ADDR)

//bits 0f reset
#define RESET_IOBANK0 (1 << 6)
#define RESET_PADSBANK0 (1 << 9)
#define RESET_TIMER0 (1 << 23)
#define RESET_TIMER1 (1 << 24)
#define RESET_UART0 (1 << 26)
#define RESET_PLL_SYS (1 << 14)




#endif

