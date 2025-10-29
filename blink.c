#include "chip.h"
#include "types.h"

#define IO_BANK0_GPIO25_CTRL (IO_BANK0_BASE + 0xcc)
#define GPIO25_CTRL_REG (*(volatile uint32_t *) (IO_BANK0_GPIO25_CTRL))

#define PADS_BANK0_GPIO25 (PADS_BANK0_BASE + 0x68)
#define PADS_GPIO25_REG (*(volatile uint32_t *) PADS_BANK0_GPIO25)

#define SIO_GPIO_OE (SIO_BASE + 0x38)
#define SIO_GPIO_OUT_SET (SIO_BASE + 0x18)
#define SIO_GPIO_OUT_CLR (SIO_BASE + 0x20)

#define SIO_OE_REG (*(volatile uint32_t *) SIO_GPIO_OE)
#define SIO_OUT_SET_REG (*(volatile uint32_t *) SIO_GPIO_OUT_SET)
#define SIO_OUT_CLR_REG (*(volatile uint32_t *) SIO_GPIO_OUT_CLR)

#define TIMER0_LR_REG (*(volatile uint32_t *)(TIMER0_BASE + 0x0c))
#define TIMER0_HR_REG (*(volatile uint32_t *)(TIMER0_BASE + 0x08))


#define SIO_GPIO25 (1 << 25)

void TIMER0_Init(void)
{
    RESET_CLR_REG = RESET_TIMER0;
    while((RESET_DONE_REG & RESET_TIMER0) != RESET_TIMER0);
}
void wait_ms(uint32_t delay)
{
    uint64_t start = ((uint64_t)TIMER0_HR_REG << 32) | TIMER0_LR_REG;
    uint64_t end = delay * 1000 + start;
    while(end >= start)
    {
        start = ((uint64_t)TIMER0_HR_REG << 32 | TIMER0_LR_REG);
    }
}

void LED_Init()
{
    RESET_CLR_REG |= (RESET_IOBANK0 | RESET_PADSBANK0);
    while((RESET_DONE_REG & (RESET_IOBANK0 | RESET_PADSBANK0)) != (RESET_IOBANK0 | RESET_PADSBANK0));
    TIMER0_Init();

    GPIO25_CTRL_REG &= ~0x1f;
    GPIO25_CTRL_REG |= 0x05;

    SIO_OE_REG = SIO_GPIO25;
    PADS_GPIO25_REG &= ~((1<<7)|(1<<8));

}
void LED_Blink(uint32_t delay)
{
    while(1)
    {
        SIO_OUT_SET_REG |= SIO_GPIO25;
        wait_ms(delay);
        SIO_OUT_CLR_REG |= SIO_GPIO25;
        wait_ms(delay);
    }
}
