#include "os.h"

#define UART0_IRQ 33
#define TIMER1_IRQ_0 4
#define IRQ_MASK (0x2fc)

extern void trap_vector(void);
extern void uart_irq_init(void);
extern void uart_irq_handler(void);
extern void timer_irq_handler(void);
extern void timer_irq_init(void);
extern void timer_alarm0_irq_handler(void);
extern void wait_ms(uint32_t);


void enable_irq(uint32_t irq)
{
    uint32_t index = irq / 16;
    uint32_t mask = 1u << (irq % 16);
    asm volatile("csrs 0xbe0, %0\n" : : "r" (index | (mask << 16)));
}

void interrupt_init()
{
    //Hazard3
    enable_irq(UART0_IRQ);
    enable_irq(TIMER1_IRQ_0);
    //uart
    uart_irq_init();
    //timer
    //timer_irq_init有个问题是，实际的初始化应该只是s_mie(MIE_MTIE)，而这里只是设置了一个1秒的中断，真正要用定时的时候再设置就行
    timer_irq_init();
    //RISCV
    //s_mstatus(MSTATUS_MIE);
    s_mie(MIE_MEIE);
    s_mie(MIE_MTIE); 
}

void trap_init()
{
    w_mtvec((reg_t)trap_vector);
    interrupt_init();
}

void interrupt_handler()
{
    uint32_t ch;
    ch = r_meinext();
    ch = ch & IRQ_MASK;
    ch = ch >> 2;
    printf("extern interrupt:%d\n", ch);
    switch(ch)
    {
        case 33:
            uart_irq_handler();
            break;
        case 4:
            timer_alarm0_irq_handler();
            break;
        default:
            break;
    }
}

reg_t trap_handler(reg_t mepc, reg_t mcause)
{
    reg_t return_epc = mepc;
    reg_t cause_code = mcause & MCAUSE_MASK_ECODE;
    if (mcause & MCAUSE_MASK_INTERRUPT) {
        switch (cause_code) {
            case 3:
                uart_puts("software interruption!\n");
                break;
            case 7:
                uart_puts("timer interruption!\n");
                timer_irq_handler();
                break;
            case 11:
                uart_puts("external interruption!\n");
                interrupt_handler();
                break;
            default:
                printf("Unknown async exception! Code = %ld\n", cause_code);
                break;
            
        }
    }
    else {
        printf("Sync exceptions! Code = %ld\n", cause_code);
        printf("Mepc: %p\n", mepc);
        //panic("OOPS! What can I do!");
        wait_ms(200);
        return_epc += 4;
    }

    return return_epc;
}

void trap_test()
{
    asm volatile("ecall");
    uart_puts("Yeah! I'm return back from trap!\n");
}



/*RISCV的中断系统很简单：
    MSTATUS, MIE, MIP
    其中MSTATUS中保存中断的总开关，MIE控制E，S，T三种类型的中断，而MIP则是用来表示中断的出现
    
    而hazard3的中的系统是对MIP显示中断出现加了一些条件，一是优先级高于现在的中断，二是MEIEA中相应IRQ被置1*/
