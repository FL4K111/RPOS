#include "os.h"

extern void switch_to(struct context *next);
extern void wait_ms(uint32_t delay);
extern void w_timer_sched(uint32_t timeslice);

#define tasks_max 10
static struct TCB tasks[tasks_max];

static int _top;
static int _current;

struct userdata {
	int counter;
	char *str;
};

/* Jack must be global */
struct userdata person;

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
    if (_current == -1) {
        _current = 0;
    }
    else {
        _current = (_current + 1) % _top;
    }

    i = (_current + 1) % _top;
    temp = _current;
    while (i != _current) {
        if (tasks[i].priority < tasks[temp].priority) {
            temp = i;
        }
        i = (i + 1) % _top;
    }
    _current = temp;
    //这里只是切换，主要的目的是找到一个合适的_current
    struct context *next = &(tasks[_current].env);
    //这里需要一个设置时间片的函数
    w_timer_sched(tasks[_current].timeslice);

    switch_to(next);
}

void timer_func(void *arg)
{
	if (NULL == arg)
		return;

	struct userdata *param = (struct userdata *)arg;
	param->counter++;
	printf("======> TIMEOUT: %s: %d\n", param->str, param->counter);

    //uart_puts("hello\n");
}

int task_create(void (*task)(void* param), void *param, uint8_t priority, uint32_t timeslice)
{
    if(_top < tasks_max)
    {
        uint8_t *p = (uint8_t *)malloc(STACK_SIZE);
        tasks[_top].stack_p = p;

        tasks[_top].env.sp = (reg_t) &p[STACK_SIZE];
        tasks[_top].env.ra = (reg_t) task;
        tasks[_top].env.a0 = (reg_t) param;
        tasks[_top].env.mepc = (reg_t)task;

        tasks[_top].priority = priority;
        tasks[_top].timeslice = timeslice;
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
    int i = _current;
    free(tasks[_current].stack_p);
    for( ; i < _top - 1; i++)
    {
        tasks[i] = tasks[i + 1];
    }
    _top--;
    _current = -1;
    w_mscratch(0);
    /*注意：一定要在删除一个任务后执行w_mscratch(0)，为的是在下一次switch_to时直接恢复上下文而不用保存，
           因为就算删除了任务，此时程序的上下文（尤其是sp）仍然是被删除任务的，直接执行switch_to,会导致
           上下文被存入位置区域导致错误*/
    
    w_timer_sched(100);
}


void user_task0()
{
    person.counter = 0;
    person.str = "jack";
    struct timer *t1 = timer_create(timer_func, &person, 3);
	if (NULL == t1) {
		printf("timer_create() failed!\n");
	}
	struct timer *t2 = timer_create(timer_func, &person, 5);
	if (NULL == t2) {
		printf("timer_create() failed!\n");
	}
	struct timer *t3 = timer_create(timer_func, &person, 7);
	if (NULL == t3) {
		printf("timer_create() failed!\n");
	}
    printf("Task0: Created!\n");
    while(1)
    {
        printf("Task0: Running\n");
        wait_ms(200);
        //schedule();
    }
}

//#define USE_LOCK
void user_task1()
{
    uart_puts("Task1: Created!\n");

    while(1) {
        #ifdef USE_LOCK
        spin_lock();
        #endif
        for (int i = 0; i < 10; i++) {
            uart_puts("Task1: Running!\n");
            wait_ms(200);
        }
        #ifdef USE_LOCK
        spin_unlock();
        #endif
    }
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
        wait_ms(200);
    }
}
void task_test()
{
    task_create(user_task0, NULL, 0, 1050000);
    task_create(user_task1, NULL, 0, 550000);
    void *args = malloc(8);
    ((int *)args)[0] = 10;
    ((int *)args)[1] = 20;
    task_create(user_task2, args, 1, 1050000);
}

