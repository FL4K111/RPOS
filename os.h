#ifndef __OS_H__
#define __OS_H__

#include "chip.h"
#include "types.h"
#include "riscv.h"

#include <stddef.h>
#include <stdarg.h>

#define STACK_SIZE 1024
#define true 1

extern void LED_Init(void);
extern void LED_Blink(uint32_t);

extern void uart_puts(char *s);
extern int uart_putc(char ch);

extern int printf(const char* s, ...);
extern void panic(char *s);


extern void *page_alloc(int npages);
extern void page_free(void *p);

extern void *malloc(uint32_t size);
extern void free(void *p);

struct context {
	/* ignore x0 */
	reg_t ra;
	reg_t sp;
	reg_t gp;
	reg_t tp;
	reg_t t0;
	reg_t t1;
	reg_t t2;
	reg_t s0;
	reg_t s1;
	reg_t a0;
	reg_t a1;
	reg_t a2;
	reg_t a3;
	reg_t a4;
	reg_t a5;
	reg_t a6;
	reg_t a7;
	reg_t s2;
	reg_t s3;
	reg_t s4;
	reg_t s5;
	reg_t s6;
	reg_t s7;
	reg_t s8;
	reg_t s9;
	reg_t s10;
	reg_t s11;
	reg_t t3;
	reg_t t4;
	reg_t t5;
	reg_t t6;


	reg_t mepc;
};

struct TCB {
	struct context env;
	uint8_t priority;
	uint32_t timeslice;
	void *stack_p;
};

extern void schedule(void);

extern int spin_lock();
extern int spin_unlock();


#endif
