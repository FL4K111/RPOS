## 介绍
此仓库为[RVOS](https://github.com/plctlab/riscv-operating-system-mooc)在树莓派pico2上的移植。每节课代码在对应的分支中。
## 准备
### Debian/Ubuntu
```shell
sudo apt install gcc-riscv64-unknown-elf binutils-riscv64-unknown-elf gdb-multiarch openocd
```
### Arch
```shell
sudo pacman -S riscv64-elf-gcc riscv64-elf-binutils riscv64-elf-gdb openocd
```
## 运行
编译和烧录：
```
git checkout ch0
make
make run
```
默认采用的是minicom进行通信，minicom的安装如下：
```
sudo pacman -S minicom
```
串口通信使用115200波特率，数据格式为8N1。安装好minicom后可以通过如下命令和开发板进行串口通信：
```
make show
```
查看反汇编：
```
make analyze
```
