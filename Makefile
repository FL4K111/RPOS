SRC_C = main.c \
		blink.c \
		uart.c \
		timer.c \
		system_init.c \
		page.c \
		printf.c \
		schedule.c 


SRC_S = start.S \
		mem.S \
		switch.S 

include ./common.mk
