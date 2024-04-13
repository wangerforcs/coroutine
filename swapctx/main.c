#include "context.h"
#include <stdio.h>

struct context mn;
struct context fc;

char stack[4096];

volatile int i = 0;

void func(){
    i++;
    // 为什么printf会导致段错误？
    // System V AMD64 ABI 调用约定要求栈帧至少对齐至 16 字节边界
    // 而进入func函数时，会push rbp，此后栈帧必须要满足16字节对齐，因此需要在makeco函数中对栈指针进行对齐，必须对齐8字节而不对齐16字节
    // 不按16字节对齐的后果？
    // 后续在进入printf函数后，嵌套调用中存在movaps %xmm1,0x10(%rsp)指令，该指令要求内存地址是16字节对齐的
    // 因此如果未能按约定的那样对齐栈，在这句命令就会出现段错误
    // 可以试下将makeco中改为newco->reg[13] = stack + stack_size，使用gdb调试就能观察到这个现象
    printf("hello, world!\n");
    swapco(&fc, &mn);
}

int main(){
    // 初始化func的上下文，由14个寄存器组成
    makeco(&fc, func, stack, sizeof(stack));
    swapco(&mn, &fc);
    printf("i = %d\n", i);
}