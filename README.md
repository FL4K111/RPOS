## 介绍
本节对应[11-systemcall](https://github.com/plctlab/riscv-operating-system-mooc/tree/main/code/os/11-syscall),在RP2350中，U模式下是不具备访问外设的权力的，因此本章节直接提供方外UART和TIMER0的系统调用，而非课程中的特权指令。