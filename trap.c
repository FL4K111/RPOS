#include "os.h"

extern void trap_vector(void);
void trap_init()
{
    w_mtvec((reg_t)trap_vector);
}

reg_t trap_handler(reg_t mepc, reg_t mcause)
{
    reg_t return_epc = mepc;
    reg_t cause_code = mcause & MCAUSE_MASK_ECODE;
    if (mcause & MCAUSE_MASK_INTERRUPT)
    {
        switch(cause_code)
        {
            case 3:
                uart_puts("software interruption!\n");
                break;
            case 7:
                uart_puts("timer interruption!\n");
                break;
            case 11:
                uart_puts("external interruption!\n"); 
                break;
            default:
                printf("Unknown async exception! Code = %ld\n", cause_code);
                break;
            
        }
    }
    else
    {
        printf("Sync exceptions! Code = %ld\n", cause_code);
        //panic("OOPS! What can I do!");
        return_epc += 4;
    }

    return return_epc;
}

void trap_test()
{
    asm volatile("ecall");
    uart_puts("Yeah! I'm return back from trap!\n");
}

void trap_ttt()
{
    printf("get rid of trap\n");
    return;
}

