# os-ex2a

- ```document```:	相关资料
- ```linux```:		修改后的内核源码
- ```test```:		测试程序源码

测试程序编译命令：

```bash
gcc xxx.c -o xxx -nostdlib -nostdinc -static -Ttext 0x1000 -e main
```
