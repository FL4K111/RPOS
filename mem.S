#define SIZE_PTR .word

.global DATA_START
DATA_START: SIZE_PTR __data_start

.global DATA_END
DATA_END: SIZE_PTR __data_end

.global BSS_START
BSS_START: SIZE_PTR __bss_start

.global BSS_END
BSS_END: SIZE_PTR __bss_end

.global SCRATCH_X_START
SCRATCH_X_START: SIZE_PTR __scratch_x_start

.global SCRATCH_X_END
SCRATCH_X_END: SIZE_PTR __scratch_x_end

.global SCRATCH_Y_START
SCRATCH_Y_START: SIZE_PTR __scratch_y_start

.global SCRATCH_Y_END
SCRATCH_Y_END: SIZE_PTR __scratch_y_end

.global MEM_START
MEM_START: SIZE_PTR __memory_start

.global MEM_END
MEM_END: SIZE_PTR __memory_end

.global HEAP_START
HEAP_START: SIZE_PTR __heap_start

.global HEAP_SIZE
HEAP_SIZE: SIZE_PTR __heap_size