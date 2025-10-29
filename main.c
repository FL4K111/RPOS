#include "os.h"

extern void LED_Init(void);

int main()
{
    LED_Init();
    LED_Blink(1000);
    while (1);
    
    return 0;
}
