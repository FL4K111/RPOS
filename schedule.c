#include "os.h"

extern void switch_to(struct context *next);
extern void wait_ms(uint32_t delay);

#define STACK_SIZE 1024
uint8_t __attribute((aligned(16))) task_stack[STACK_SIZE];
struct context ctx_task;

static void w_mscratch(reg_t x)
{
    asm volatile(
        "csrw mscratch, %0"
        : 
        : "r" (x)
    );
}

void user_task0(void);
void schedule_init()
{
    w_mscratch(0);
    ctx_task.sp = (reg_t) &task_stack[STACK_SIZE];
    ctx_task.ra = (reg_t) user_task0;
}
void schedule()
{
    struct context *next = &ctx_task;
    switch_to(next);
}
void user_task0()
{
    printf("Task0: Created!");
    while(1)
    {
        printf("Task0: Running\n");
        wait_ms(1000);
    }
}