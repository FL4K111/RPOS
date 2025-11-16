#include "types.h"
#include "chip.h"

#define DR 0x000
#define RSR 0x004
#define FR 0x18
#define ILPR 0x020
#define IBRD 0x024
#define FBRD 0x028
#define LCR 0x02c
#define CR 0x30
#define IFLS 0x34
#define IMSC 0x38

#define GPIOS_GPIO0_CTRL_OFFSET 0x04
#define GPIOS_GPIO1_CTRL_OFFSET 0x0c

#define PADS_GPIO0_OFFSET 0x04
#define PADS_GPIO1_OFFSET 0x08

#define GPIOS_REG(offset) (*(volatile uint32_t *)(IO_BANK0_BASE + offset))
#define PADS_REG(offset) (*(volatile uint32_t *)(PADS_BANK0_BASE + offset))

#define UART0_REG(offset) (*(volatile uint32_t *)(UART0_BASE + offset))

#define PADS_IE (1 << 6)
#define PADS_OD (1 << 7)
#define PADS_ISO (1 << 8)

#define GPIO_FUNSEL_UART (0x2)
#define GPIO_FUNSEL_MASK (0x1f)

#define UART_INIT_BITS (RESET_IOBANK0 | RESET_PADSBANK0 | RESET_UART0)
#define UART_CR_UARTEN (1 << 0)
#define UART_CR_TXE (1 << 8)
#define UART_CR_RXE (1 << 9)
#define UART_LCR_WLEN (3 << 5)
#define UART_LCR_FEN (1 << 4)
#define UART_LCR_STP2 (1 << 3)
#define UART_LCR_PEN (1 << 1)
#define UART_FR_TXFF (1 << 5)
#define UART_FR_RXFE (1 << 4)
#define UART_DR_DATA_MASK (0xff << 0)

#define UART_IRQ_TXFIFO_LEN (0b000 << 0)
#define UART_IRQ_RXFIFO_LEN (0b000 << 3)
#define UART_IRQ_MASK_TX (0b1 << 5)
#define UART_IRQ_MASK_RX (0b1 << 4)



#define BAUD_RATE 115200
#define BAUD_IBRD 6
#define BAUD_FBRD 33





void uart_init()
{
    RESET_CLR_REG |= UART_INIT_BITS;
    while((RESET_DONE_REG & UART_INIT_BITS) != UART_INIT_BITS);

    //GPIO Init
    PADS_REG(PADS_GPIO0_OFFSET) &= ~PADS_OD;
    PADS_REG(PADS_GPIO0_OFFSET) |= PADS_IE;
    PADS_REG(PADS_GPIO1_OFFSET) &= ~PADS_OD;
    PADS_REG(PADS_GPIO1_OFFSET) |= PADS_IE;

    GPIOS_REG(GPIOS_GPIO0_CTRL_OFFSET) &= ~GPIO_FUNSEL_MASK;
    GPIOS_REG(GPIOS_GPIO0_CTRL_OFFSET) |= GPIO_FUNSEL_UART;
    GPIOS_REG(GPIOS_GPIO1_CTRL_OFFSET) &= ~GPIO_FUNSEL_MASK;
    GPIOS_REG(GPIOS_GPIO1_CTRL_OFFSET) |= GPIO_FUNSEL_UART;

    PADS_REG(PADS_GPIO0_OFFSET) &= ~PADS_ISO;
    PADS_REG(PADS_GPIO1_OFFSET) &= ~PADS_ISO;

    //UART Init
    UART0_REG(CR) &= ~UART_CR_UARTEN;

    UART0_REG(IBRD) = BAUD_IBRD;
    UART0_REG(FBRD) = BAUD_FBRD;

    UART0_REG(LCR) &= ~UART_LCR_PEN;
    UART0_REG(LCR) &= ~UART_LCR_STP2;
    UART0_REG(LCR) &= ~UART_LCR_FEN;
    UART0_REG(LCR) |= UART_LCR_WLEN;

    UART0_REG(CR) |= UART_CR_TXE | UART_CR_RXE | UART_CR_UARTEN;

}

int uart_putc(char ch)
{
    while((UART0_REG(FR) & UART_FR_TXFF) == UART_FR_TXFF);
    return UART0_REG(DR) = ch;
}

void uart_puts(char* s)
{
    while (*s){
        if(*s == '\n')
        {
            uart_putc('\r');
        }
        uart_putc(*s++);
    }
}

static uint8_t uart_getc()
{
    while(UART0_REG(FR) & UART_FR_RXFE);
    return (uint8_t)(UART0_REG(DR) & UART_DR_DATA_MASK);
}

void uart_loop()
{
    uint8_t ch;
    while(1){
        ch = uart_getc();
        uart_putc(ch);
    }
}
void uart_irq_init()
{
    UART0_REG(IMSC) |= UART_IRQ_MASK_RX;
}

void uart_irq_handler()
{
    uint8_t ch;
    ch = (uint8_t)UART0_REG(DR) & UART_DR_DATA_MASK;
    uart_putc(ch);
}
