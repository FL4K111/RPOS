#include "chip.h"
#include "types.h"
#include "riscv.h"

extern void timer_init(void);

#define CLK_REG(offset) (*(volatile uint32_t *)(CLOCKS_BASE + offset))
#define CLK_REF_CTRL_OFFSET 0x30
#define CLK_REF_SELECTED_OFFSET 0x38
#define CLK_SYS_CTRL_OFFSET 0x3c
#define CLK_SYS_SELECTED_OFFSET 0x44
#define CLK_PERI_CTRL_OFFSET 0x48
#define CLK_PERI_SELECTED_OFFSET 0x50 

#define XOSC_REG(offset) (*(volatile uint32_t *)(XOSC_BASE + offset))
#define XOSC_CTRL_OFFSET 0x00
#define XOSC_STATUS_OFFSET 0x04
#define XOSC_STARTUP_OFFSET 0x0c

#define XOSC_CTRL_FREQ_RANGE_MASK (0xfff)
#define XOSC_CTRL_FREQ_RANGE_1_15MHz (0xaa0)
#define XOSC_CTRL_ENABLE_MASK (0xfff << 12)
#define XOSC_CTRL_ENABLE (0xfab << 12)
#define XOSC_STARTUP_DELAY_MASK (0x3fff)
#define XOSC_STARTUP_DELAY (469)
#define XOSC_STATUS_STABLE (1 << 31)

#define CLK_REF_MAIN_SRC (0x0)
#define CLK_REF_XOSC_SRC (0x2)
#define CLK_REF_XOSC_SELECTED (1 << 2)

#define CLK_SYS_MAIN_SRC (0x0)
#define CLK_SYS_AUX_SRC (0x1)
#define CLK_SYS_MAIN_SELECTED (0x1)
#define CLK_SYS_AUX_SELECTED (0x1 << 1)
#define CLK_SYS_AUX_XOSC_SRC (0x3 << 5)
#define CLK_PERI_XOSC_SRC (0x4 << 5)
#define CLK_AUX_MASK (0x7 << 5)

#define CLK_CTRL_ENABLE (1 << 11)

static inline void delay_cycle(uint32_t cycles)
{
    while(cycles--)
        __asm volatile("nop");
}

void clk_init(void)
{
    /*
     * 时钟初始化有三种模式
     *第一种：在无毛刺源之间的切换，直接切换就行
     *第二种：有无毛刺源又有有毛刺源，无毛刺源先切换到主源（0x0）上，再切换有毛刺源，再将无毛刺源切换到有毛刺源上
     *第三种：有毛刺源间的切换，先关闭再等待一定延迟，切换源，再等待延迟，再打开。
     *
     *
     */
    //XOSC start
    XOSC_REG(XOSC_CTRL_OFFSET) &= ~XOSC_CTRL_FREQ_RANGE_MASK;
    XOSC_REG(XOSC_CTRL_OFFSET) |= XOSC_CTRL_FREQ_RANGE_1_15MHz;

    XOSC_REG(XOSC_STARTUP_OFFSET) &= ~XOSC_STARTUP_DELAY_MASK;
    XOSC_REG(XOSC_STARTUP_OFFSET) |= XOSC_STARTUP_DELAY;

    XOSC_REG(XOSC_CTRL_OFFSET) &= ~XOSC_CTRL_ENABLE_MASK;
    XOSC_REG(XOSC_CTRL_OFFSET) |= XOSC_CTRL_ENABLE;

    while((XOSC_REG(XOSC_STATUS_OFFSET) & XOSC_STATUS_STABLE) != XOSC_STATUS_STABLE);


    //clk_ref init
    CLK_REG(CLK_REF_CTRL_OFFSET) &= ~0x3;
    CLK_REG(CLK_REF_CTRL_OFFSET) |= CLK_REF_XOSC_SRC;
    while((CLK_REG(CLK_REF_SELECTED_OFFSET) & CLK_REF_XOSC_SELECTED) != CLK_REF_XOSC_SELECTED);

    //clk_sys init
    CLK_REG(CLK_SYS_CTRL_OFFSET) &= ~0x1;
    CLK_REG(CLK_SYS_CTRL_OFFSET) |= CLK_SYS_MAIN_SRC;
    while((CLK_REG(CLK_SYS_SELECTED_OFFSET) & CLK_SYS_MAIN_SELECTED) != CLK_SYS_MAIN_SELECTED);

    CLK_REG(CLK_SYS_CTRL_OFFSET) &= ~(0x7 << 5);
    CLK_REG(CLK_SYS_CTRL_OFFSET) |= CLK_SYS_AUX_XOSC_SRC;

    CLK_REG(CLK_SYS_CTRL_OFFSET) &= ~0x1;
    CLK_REG(CLK_SYS_CTRL_OFFSET) |= CLK_SYS_AUX_SRC;
    while((CLK_REG(CLK_SYS_SELECTED_OFFSET) & CLK_SYS_AUX_SELECTED) != CLK_SYS_AUX_SELECTED);

    //clk_peri init
    CLK_REG(CLK_PERI_CTRL_OFFSET) &= ~CLK_CTRL_ENABLE;
    delay_cycle(5);
    CLK_REG(CLK_PERI_CTRL_OFFSET) &= ~CLK_AUX_MASK;
    CLK_REG(CLK_PERI_CTRL_OFFSET) |= CLK_PERI_XOSC_SRC;
    
    CLK_REG(CLK_PERI_CTRL_OFFSET) |= CLK_CTRL_ENABLE;
    delay_cycle(5);
}


void system_init(void)
{
    clk_init();
    timer_init();
    s_mstatus(MSTATUS_MPP | MSTATUS_MPIE);
}
