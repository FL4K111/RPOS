#include "os.h"

extern void uart_init(void);
extern void uart_loop(void);
extern void page_init(void);
extern void _byte_init(void);
extern void page_test(void);
extern void byte_test(void);


int main()
{
    uart_init();
    printf("hello rpos!\n");

    page_init();
    _byte_init();
    
    page_test();
    byte_test();

    while (1);
    
    return 0;
}