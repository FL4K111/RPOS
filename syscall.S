#include "call_type.h"


.global uart_puts_u
uart_puts_u:
    li  a7, UART_PUT_CALL_TYPE
    ecall

    ret

.global wait_ms_u
wait_ms_u:
    li  a7, WAIT_MS_CALL_TYPE
    ecall

    ret

.global thisistest
thisistest:
    li a0, WAIT_MS_CALL_TYPE
    ret
