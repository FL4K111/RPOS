## 介绍
本节对应[05-traps](https://github.com/plctlab/riscv-operating-system-mooc/tree/main/code/os/05-traps),在本节中主要实现


## 解决思路
trap可以看作任务调度的更高级的状态，任务调用中需要先保存上下文再恢复下个任务的上下文来执行；而trap的思路则是，当trap到来的时候先保存csr中之前的数据，再切换根据trap类型改变相应的值，后保存上下文，进入trap处理函数，之后恢复上下文，设置某些csr后通过mret退出