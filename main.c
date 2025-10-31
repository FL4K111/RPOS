#include "os.h"

extern void uart_init(void);
extern void uart_loop(void);

extern void page_init(void);
extern void _byte_init(void);
extern void page_test(void);
extern void byte_test(void);

extern void schedule_init(void);



int main()
{
    uart_init();
    printf("Hello RPOS!\n");

    page_init();
    _byte_init();
    
    schedule_init();
    schedule();

    while (1);
    
    return 0;
}