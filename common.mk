CROSS_COMPILE = riscv64-unknown-elf-
CFLAGS += -nostdlib -fno-builtin -g -Wall -march=rv32imac -mabi=ilp32
LFLAGS += -T mem.ld

CC = ${CROSS_COMPILE}gcc
OBJCOPY = ${CROSS_COMPILE}objcopy
OBJDUMP = ${CROSS_COMPILE}objdump
OPENOCD = openocd
MKDIR = mkdir -p
RM = rm -rf
COM = minicom

OUTPUT_PATH = out

OBJS_S := $(addprefix ${OUTPUT_PATH}/, $(patsubst %.S, %.o, ${SRC_S}))
OBJS_C := $(addprefix ${OUTPUT_PATH}/, $(patsubst %.c, %.o, ${SRC_C}))
OBJS = ${OBJS_S} ${OBJS_C}

ELF = ${OUTPUT_PATH}/os.elf
BIN = ${OUTPUT_PATH}/os.bin

.DEFAULT_GOAL := all
all : ${OUTPUT_PATH} ${ELF}

${OUTPUT_PATH} : 
	@${MKDIR} $@


${ELF} : ${OBJS}
	${CC} ${CFLAGS} ${LFLAGS} -nostartfiles -o ${ELF} $^
	${OBJCOPY} -O binary ${ELF} ${BIN}

${OUTPUT_PATH}/%.o : %.c
	${CC} ${CFLAGS} -c -o $@ $<

${OUTPUT_PATH}/%.o : %.S
	${CC} ${CFLAGS} -c -o $@ $<

.PHONY : clean
clean : 
	@${RM} ${OUTPUT_PATH}

.PHONY : run
run : all
	${OPENOCD} -f interface/cmsis-dap.cfg -f target/rp2350-riscv.cfg \
			   -c "adapter speed 4000" \
			   -c "program ${ELF} reset exit"	

.PHONY : analyze
analyze : 
	${OBJDUMP} -d ${ELF} | less

.PHONY : show
show : 
	${COM}