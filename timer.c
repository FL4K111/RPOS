#include "os.h"

extern void schedule(void);

#define TICK_TIMER0_CTRL_OFFSET 0x18
#define TICK_TIMER0_CYCLE_OFFSET 0x1c
#define TICK_TIMER1_CTRL_OFFSET 0x24
#define TICK_TIMER1_CYCLE_OFFSET 0x28
#define TICK_RISCV_CTRL_OFFSET 0x3c
#define TICK_RISCV_CYCLE_OFFSET 0x40

#define SIO_MTIME_CTRL_OFFSET 0x1a4
#define SIO_MTIME_OFFSET 0x1b0
#define SIO_MTIMEH_OFFSET 0x1b4
#define SIO_MTIMECMP_OFFSET 0x1b8
#define SIO_MTIMECMPH_OFFSET 0x1bc

#define TICK_REG(offset) (*(volatile uint32_t *)(TICKS_BASE + offset))
#define SIO_REG(offset) (*(volatile uint32_t *)(SIO_BASE + offset))

#define TICK_CTRL_ENABLE (0x1)
#define TICK_CYCLE_MASK (0x1ff)

#define RESET_TIMER (RESET_TIMER0 | RESET_TIMER1)


//tick主要用于产生1us的时间基准，在rp2350中TIMER的时钟源来自tick产生的1us,而tick的时钟源是clk_ref
//注意：在这里将TIMER0和TIMER1脱离了reset状态，所以在blink中才能直接使用
void timer_init()
{
    RESET_CLR_REG |= RESET_TIMER;
    while((RESET_DONE_REG & RESET_TIMER) != RESET_TIMER);

    TICK_REG(TICK_TIMER0_CYCLE_OFFSET) &= ~TICK_CYCLE_MASK;
    TICK_REG(TICK_TIMER0_CYCLE_OFFSET) |= 0xc;
    TICK_REG(TICK_TIMER0_CTRL_OFFSET) |= TICK_CTRL_ENABLE;

    TICK_REG(TICK_TIMER1_CYCLE_OFFSET) &= ~TICK_CYCLE_MASK;
    TICK_REG(TICK_TIMER1_CYCLE_OFFSET) |= 0xc;
    TICK_REG(TICK_TIMER1_CTRL_OFFSET) |= TICK_CTRL_ENABLE;

    TICK_REG(TICK_RISCV_CYCLE_OFFSET) &= ~TICK_CYCLE_MASK;
    TICK_REG(TICK_RISCV_CYCLE_OFFSET) |= 0xc;
    TICK_REG(TICK_RISCV_CTRL_OFFSET) |= TICK_CTRL_ENABLE;
}

static inline uint64_t r_mtime()
{
    uint32_t hi = SIO_REG(SIO_MTIMEH_OFFSET);
    uint32_t lo;

    do {
        lo = SIO_REG(SIO_MTIME_OFFSET);

        uint32_t next_hi = SIO_REG(SIO_MTIMEH_OFFSET);
        if (hi == next_hi)
            break;
        hi = next_hi;
    }while(true);

    return ((uint64_t) hi << 32u) | lo;
}
static inline void w_mtimecmp(uint64_t x)
{
    uint32_t lo = x & 0xffffffff;
    uint32_t hi = (x >> 32u) & 0xffffffff;

    SIO_REG(SIO_MTIMECMP_OFFSET) = lo;
    SIO_REG(SIO_MTIMECMPH_OFFSET) = hi;
}

void w_timer_sched(uint32_t timeslice)
{
    uint64_t x = r_mtime();
    x += timeslice;
    w_mtimecmp(x);
}

void timer_irq_init()
{
    w_timer_sched(1000000);
}

void timer_irq_handler()
{
    schedule();
}