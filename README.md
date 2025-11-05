## 介绍
本节对应[04-multitask](https://github.com/plctlab/riscv-operating-system-mooc/tree/main/code/os/04-multitask),在本节中已经实现练习9


## 练习思路
放弃教程中的tcb和栈分开存储的思想，转而只用一个tcb其中包含了栈，优先级和上下文。先实现tcb中包含栈和上下文再加入优先级，之后再修改调度算法。