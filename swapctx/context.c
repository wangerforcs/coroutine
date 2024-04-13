#include "context.h"

#define ALIGNDOWN(x, a) ((x) & ~((a) - 1))

int makeco(context_t newco, void* func, void* stack, size_t stack_size){
    // 返回地址
    newco->reg[9] = func;
    // rsp
    newco->reg[13] = (void*)ALIGNDOWN((size_t)stack + stack_size, 16) - 8;
    return 0;
}