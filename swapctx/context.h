#ifndef _CONTEXT_H_
#define _CONTEXT_H_
#include <stddef.h>

struct context{
    // 依序为
    /*
    0   1   2   3   4  5  6   7   8   9   10  11  12  13
    r15 r14 r13 r12 r9 r8 rbp rdi rsi rip rdx rcx rbx rax
    */
    void* reg[14];
};
typedef struct context* context_t;

int swapco(context_t curr, context_t pending);

int makeco(context_t newco, void* func, void* stack, size_t stack_size);

#endif