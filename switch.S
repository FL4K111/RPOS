#define LOAD        lw
#define STORE       sw
#define SIZE_REG    4

# 用t6存储上下文的存储地址（因为t6是最后一个寄存器，方便代码编写）

.macro reg_save base
	STORE ra,   0*SIZE_REG(\base)
	STORE sp,   1*SIZE_REG(\base)
	STORE t0,   4*SIZE_REG(\base)
	STORE t1,   5*SIZE_REG(\base)
	STORE t2,   6*SIZE_REG(\base)
	STORE s0,   7*SIZE_REG(\base)
	STORE s1,   8*SIZE_REG(\base)
	STORE a0,   9*SIZE_REG(\base)
	STORE a1,  10*SIZE_REG(\base)
	STORE a2,  11*SIZE_REG(\base)
	STORE a3,  12*SIZE_REG(\base)
	STORE a4,  13*SIZE_REG(\base)
	STORE a5,  14*SIZE_REG(\base)
	STORE a6,  15*SIZE_REG(\base)
	STORE a7,  16*SIZE_REG(\base)
	STORE s2,  17*SIZE_REG(\base)
	STORE s3,  18*SIZE_REG(\base)
	STORE s4,  19*SIZE_REG(\base)
	STORE s5,  20*SIZE_REG(\base)
	STORE s6,  21*SIZE_REG(\base)
	STORE s7,  22*SIZE_REG(\base)
	STORE s8,  23*SIZE_REG(\base)
	STORE s9,  24*SIZE_REG(\base)
	STORE s10, 25*SIZE_REG(\base)
	STORE s11, 26*SIZE_REG(\base)
	STORE t3,  27*SIZE_REG(\base)
	STORE t4,  28*SIZE_REG(\base)
	STORE t5,  29*SIZE_REG(\base)
.endm

.macro reg_restore base
	LOAD ra,   0*SIZE_REG(\base)
	LOAD sp,   1*SIZE_REG(\base)
	LOAD t0,   4*SIZE_REG(\base)
	LOAD t1,   5*SIZE_REG(\base)
	LOAD t2,   6*SIZE_REG(\base)
	LOAD s0,   7*SIZE_REG(\base)
	LOAD s1,   8*SIZE_REG(\base)
	LOAD a0,   9*SIZE_REG(\base)
	LOAD a1,  10*SIZE_REG(\base)
	LOAD a2,  11*SIZE_REG(\base)
	LOAD a3,  12*SIZE_REG(\base)
	LOAD a4,  13*SIZE_REG(\base)
	LOAD a5,  14*SIZE_REG(\base)
	LOAD a6,  15*SIZE_REG(\base)
	LOAD a7,  16*SIZE_REG(\base)
	LOAD s2,  17*SIZE_REG(\base)
	LOAD s3,  18*SIZE_REG(\base)
	LOAD s4,  19*SIZE_REG(\base)
	LOAD s5,  20*SIZE_REG(\base)
	LOAD s6,  21*SIZE_REG(\base)
	LOAD s7,  22*SIZE_REG(\base)
	LOAD s8,  23*SIZE_REG(\base)
	LOAD s9,  24*SIZE_REG(\base)
	LOAD s10, 25*SIZE_REG(\base)
	LOAD s11, 26*SIZE_REG(\base)
	LOAD t3,  27*SIZE_REG(\base)
	LOAD t4,  28*SIZE_REG(\base)
	LOAD t5,  29*SIZE_REG(\base)
	LOAD t6,  30*SIZE_REG(\base)
.endm



#在trap_vector中必须增加对于mscratch为0的处理，因为在schedule_init后mscratch为0,会在0x00000000处写入和读取上下文从而导致系统卡死
.text 
.global trap_vector
trap_vector:
	csrrw		t6, mscratch, t6
	beqz		t6, 1f

	reg_save	t6

	mv			t5, t6
	csrr		t6, mscratch
	STORE		t6, 30*SIZE_REG(t5)

	csrr		a0, mepc
	STORE		a0, 31*SIZE_REG(t5)
	csrw		mscratch, t5
	j			2f
1:
	csrw		mscratch, t6
2:
	csrr		a0, mepc
	csrr		a1, mcause
	csrr		a2, mscratch
	call		trap_handler

	csrw		mepc, a0
	
	csrr		t6, mscratch
	beqz		t6, 3f
	reg_restore t6
3:
	mret


#当采用基于中断的调度时，不能在switch_to里面保存上下文，而是在trap_vector中保存
#可以这样理解，在trap_vector中保存的是任务运行的上下文，是有效的；而在switch_to中保存的只是中断执行的上下文，是无效的。
#有效无效也很好理解，主要就是mret是直接跳回任务的中断点而非trap_vector的某一位置，这就导致记录中断的上下文无意义甚至出错。
.global switch_to
.balign 4
switch_to:

	csrw		mscratch, a0
	LOAD		a1, 31*SIZE_REG(a0)
	csrw		mepc, a1

	mv          t6, a0
    reg_restore t6
    mret

.end
