#include "os.h"

extern void switch_to(struct context *next);
extern void wait_ms(uint32_t delay);

#define tasks_max 10
static struct TCB tasks[tasks_max];

static int _top;
static int _current;

static void w_mscratch(reg_t x)
{
    asm volatile(
        "csrw mscratch, %0"
        : 
        : "r" (x)
    );
}

void user_task0();
void user_task1();

void schedule_init()
{
    w_mscratch(0);
    _current = -1;
    _top = 0;
}
void schedule()
{
    uint32_t i;
    uint32_t temp;

    if(!_top)
    {
        printf("Error! no task here\n");
        return;
    }
    //寻找下标的过程才是优先级算法的主要步骤
    if(_current == -1)
    {
        _current = 0;
        i = (_current + 1) % _top;
        temp = _current;
        while(i != _current)
        {
            if(tasks[i].priority < tasks[temp].priority)
                temp = i;
            i = (i + 1) % _top;
        }
        if(tasks[temp].priority < tasks[_current].priority)
            _current = temp;
    }
    else
    {
        i = (_current + 1) % _top;
        temp = _current;
        while(i != _current)
        {
            if(tasks[i].priority <= tasks[temp].priority)
                temp = i;
            i = (i + 1) % _top;
        }
        _current = temp;
    }


    //这里只是切换，主要的目的是找到一个合适的_current
    struct context *next = &(tasks[_current].env);
    switch_to(next);
}

int task_create(void (*task)(void* param), void *param, uint8_t priority)
{
    if(_top < tasks_max)
    {
        uint8_t *p = (uint8_t *)malloc(STACK_SIZE);
        tasks[_top].stack_p = p;
        tasks[_top].env.sp = (reg_t) &p[STACK_SIZE];
        tasks[_top].env.ra = (reg_t) task;
        tasks[_top].env.a0 = (reg_t) param;
        tasks[_top].priority = priority;
        _top++;
        return 0;
    }
    else{
        return -1;
    }
}
static void *memcpy(void *dest, const void *src, unsigned long n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

void task_exit()
{
    printf("task[1].sp = %p\ttask[2].sp = %p\n", tasks[1].env.sp, tasks[2].env.sp);
    int i = _current;
    free(tasks[_current].stack_p);
    for( ; i < _top - 1; i++)
    {
        tasks[i] = tasks[i + 1];
    }
    printf("new task[1].sp = %p \n", tasks[1].env.sp);
    _top--;
    _current = -1;
    schedule();
    //问题主要还是出在栈上，现在一个解决方案就是把栈单独提出来用指针指向该区域
    //为了整体的方便，采用将malloc的最小分配粒度改为16字节
}

void user_task0()
{
    printf("Task0: Created!\n");
    while(1)
    {
        printf("Task0: Running\n");
        wait_ms(1000);
        schedule();
    }
}
void user_task1()
{
    int i = 0;
    printf("Task1: Created!\n");
    while(i < 3)
    {
        printf("Task1: Running\n");
        i++;
        wait_ms(1000);
        schedule();
    }
    task_exit();
}
void user_task2(void *param)
{
    struct {
        int a;
        int b;
    } *args = param;
    printf("Task2: Created!\n");
    while(1)
    {
        printf("Task2: Running\n");
        //printf("%d\t%d\n", args->a, args->b);
        wait_ms(1000);
        schedule();
    }
}
void task_test()
{
    task_create(user_task0, NULL, 1);
    task_create(user_task1, NULL, 0);
    void *args = malloc(8);
    ((int *)args)[0] = 10;
    ((int *)args)[1] = 20;
    task_create(user_task2, args, 0);

    printf("_top = %d _current = %d\n", _top, _current);
    schedule();
}

