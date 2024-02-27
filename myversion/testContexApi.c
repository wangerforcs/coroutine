#include<stdio.h>
#include<ucontext.h>

void funtion(){
    printf("run this\n");
}

int main()
{
    printf("in main\n");
    char stack[1024];
    ucontext_t main,other;
    getcontext(&main);   //获取当前上下文
    
    main.uc_stack.ss_sp = stack;          //指定栈空间
    main.uc_stack.ss_size = sizeof(stack);//指定栈空间大小
    main.uc_link = &other;                //将后继上下文指向other

    makecontext(&main,funtion,0);         //为main指定要执行的函数
    
    swapcontext(&other,&main);            //激活main，并将当前上下文保存到other
    printf("in main\n");
    return 0;
}
