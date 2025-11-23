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
#define TIMER1_REG(offset) (*(volatile uint32_t *)(TIMER1_BASE + offset))

#define TICK_CTRL_ENABLE (0x1)
#define TICK_CYCLE_MASK (0x1ff)

#define TIMER_INTE_OFFSET 0x40
#define TIMER_INTR_OFFSET 0x3c
#define TIMER_ALARM0_OFFSET 0x10
#define TIMER_TIMERAWL_OFFSET 0x28
#define TIMER_ALARM0_MASK (1 << 0)

#define SWTIMER_DELAY 1000000


#define RESET_TIMER (RESET_TIMER0 | RESET_TIMER1)

static uint32_t _tick;

#define MAX_TIMER 10
static struct timer timer_list[MAX_TIMER];


//tick主要用于产生1us的时间基准，在rp2350中TIMER的时钟源来自tick产生的1us,而tick的时钟源是clk_ref
//注意：在这里将TIMER0和TIMER1脱离了reset状态，所以在blink中才能直接使用
void timer_init()
{
    //三个时钟皆使用tick(1us)作为驱动，timer0用于delay,timer1用于软件时钟，riscv-timer用于基于时间片的调度算法。
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

    struct timer *t = &(timer_list[0]);
	for (int i = 0; i < MAX_TIMER; i++) {
		t->func = NULL; /* use .func to flag if the item is used */
		t->arg = NULL;
		t++;
	}
}

struct timer *timer_create(void (*handler)(void *arg), void *arg, uint32_t timeout)
{
	/* TBD: params should be checked more, but now we just simplify this */
	if (NULL == handler || 0 == timeout) {
		return NULL;
	}

	/* use lock to protect the shared timer_list between multiple tasks */
	spin_lock();

	struct timer *t = &(timer_list[0]);
	for (int i = 0; i < MAX_TIMER; i++) {
		if (NULL == t->func) {
			break;
		}
		t++;
	}
	if (NULL != t->func) {
		spin_unlock();
		return NULL;
	}

	t->func = handler;
	t->arg = arg;
	t->timeout_tick = _tick + timeout;

	spin_unlock();

	return t;
}

void timer_delete(struct timer *timer)
{
	spin_lock();

	struct timer *t = &(timer_list[0]);
	for (int i = 0; i < MAX_TIMER; i++) {
		if (t == timer) {
			t->func = NULL;
			t->arg = NULL;
			break;
		}
		t++;
	}

	spin_unlock();
}

static inline void timer_check()
{
	struct timer *t = &(timer_list[0]);
	for (int i = 0; i < MAX_TIMER; i++) {
		if (NULL != t->func) {
			if (_tick >= t->timeout_tick) {
				t->func(t->arg);
				/* once time, just delete it after timeout */
				t->func = NULL;
				t->arg = NULL;

				break;
			}
		}
		t++;
	}
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

static void w_timer_alarm(uint32_t delay)
{
    uint32_t x = TIMER1_REG(TIMER_TIMERAWL_OFFSET);
    x += delay;
    TIMER1_REG(TIMER_ALARM0_OFFSET) = x;
}

void timer_irq_init()
{
    _tick = 0;
    TIMER1_REG(TIMER_INTE_OFFSET) |= TIMER_ALARM0_MASK;
    w_timer_alarm(SWTIMER_DELAY);

    uint64_t x = r_mtime();
    x += 500000;
    w_mtimecmp(x);
}

void timer_irq_handler()
{
    schedule();
}
void timer_alarm0_irq_handler()
{
    _tick++;
    printf("tick: %d\n", _tick);

    timer_check();

    TIMER1_REG(TIMER_INTR_OFFSET) = TIMER_ALARM0_MASK;
    w_timer_alarm(SWTIMER_DELAY);
}