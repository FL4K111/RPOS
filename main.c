#include "os.h"

extern void uart_init(void);
extern void uart_loop(void);

extern void page_init(void);
extern void _byte_init(void);
extern void page_test(void);
extern void byte_test(void);

extern void schedule_init(void);
extern void task_test(void);

extern void trap_init(void);
extern void trap_test(void);

extern void wait_ms(uint32_t delay);
#define MAIN_LOCK

int main()
{
#ifdef MAIN_LOCK
    spin_lock();
#endif
    uart_init();
    printf("Hello RPOS!\n");

    page_init();
    _byte_init();
    schedule_init();

    //task_test();
    trap_init();

#ifdef MAIN_LOCK
    spin_unlock();
#endif
    task_test();

    while (1)
    {
    uint32_t p;
    asm volatile("csrr %0, mip" : "=r" (p));
    printf("MIP:%p\n", p);
    wait_ms(200);
    };
    
    return 0;
}