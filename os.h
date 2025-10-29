#include "chip.h"
#include "types.h"

#include <stddef.h>
#include <stdarg.h>

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


