## 介绍
本节对应[10-swtimer](https://github.com/plctlab/riscv-operating-system-mooc/tree/main/code/os/10-swtimer),本节主要实现了软件时钟。练习一，二主要是数据结构方面的问题，对理解OS用处不大故没有去实现，而练习三基于硬件的延迟在之前已经通过delay_ms实现了。
## 注意事项
1.本章中使用了TIMER0,TIMER1和RISCV Platform TIMER三个定时器，其中TIMER0用于实现基于硬件的延时（不使用中断），TIMER1用于实现swtimer，而RISCV Platform TIMER则用于实现时间片调度。
2.在写程序时，如果需要用到全局变量，则该变量的初始化最好放到函数中，不然容易产生异常。
3.可能是因为启用了TIMER1的中断的原因，现在在烧录完成后必须手动reset不然程序很容易运行在错误的环境上。