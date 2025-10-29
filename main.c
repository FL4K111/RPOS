#include "os.h"

extern void uart_init(void);
extern void uart_loop(void);

int main()
{

    uart_init();
    uart_puts("hello rpos!\n");
    uart_loop();
    
    while (1);
    
    return 0;
}
