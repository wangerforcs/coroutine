#include <ucontext.h>
#include <iostream>
#include <vector>
#include <functional>
#include <memory>
#include <random>

using namespace std;

void wrapfunc();
typedef std::function<void()> Task;
enum State{
    FREE,
    RUNNABLE,
    RUNNING,
};
enum{
    STACK_SIZE = 4096,
    MAX_CO = 24
};
struct coroutine{
    char stack[STACK_SIZE];
    State state;
    ucontext_t ctx;
    Task task;
    bool isDead(){
        return state == FREE;
    }
};
coroutine coroutines[MAX_CO];
coroutine* current = nullptr;
size_t cnt = 0;

void init(){
    current = coroutines;
    current->state = RUNNING;
}

void schedule(){
    coroutine *t, *next = nullptr;
    t = current + 1;
    for(int i=0; i< MAX_CO ;i++){
        if( t >= coroutines + MAX_CO){
            t = coroutines;
        }
        if(t->state == RUNNABLE){
            next = t;
            break;
        }
        t++;
    }
    if(!next){
        cout<<"no coroutine to run"<<endl;
        exit(1);
    }
    if(current != next){
        next->state = RUNNING;
        t = current;
        current = next;
        swapcontext(&t->ctx, &next->ctx);
    }
}

coroutine* create(Task task){
    coroutine *t = nullptr;
    for(int i=0; i< MAX_CO ;i++){
        if(coroutines[i].state == FREE){
            t = coroutines + i;
            break;
        }
    }
    if(!t){
        cout<<"no free coroutine"<<endl;
        exit(1);
    }
    t->state = RUNNABLE;
    t->task = task;
    getcontext(&t->ctx);
    t->ctx.uc_link = NULL;
    t->ctx.uc_stack.ss_sp = t->stack;
    t->ctx.uc_stack.ss_size = STACK_SIZE;
    makecontext(&t->ctx, wrapfunc, 0);
    cnt++;
    return t;
}

void yield(){
    current->state = RUNNABLE;
    schedule();
}

void join(){
    while(cnt != 0){
        yield();
    }
}

void wrapfunc(){
    current->task();
    current->state = FREE;
    cnt--;
    schedule();
}

void fun(int id){
    for(int i=0; i<5; i++){
        cout<<"coroutine "<<id<<": "<<i<<endl;
        yield();
    }
}

string s;
const int cap = 20;

void producer(char c){
    static default_random_engine e;
    e.seed(time(0));
    for(int i = 0; i < 10; i++){
        int n = e() % cap + 1;
        s = s + string(n, c);
        yield();
    }
}

void consumer(){
    int maxempty = 10;
    while(1){
        if(s.empty()){
            maxempty--;
            if(maxempty == 0){
                break;
            }
            yield();
            continue;
        }
        cout<<s<<endl;
        s.clear();
        yield();
    }
}

int main(){
    cout << "main start" << endl;
    init();
    create(std::bind(fun, 1));
    create(std::bind(fun, 2));

    // 全部协程结束后在main函数中退出
    join();

    // init();
    // create(std::bind(producer, 'A'));
    // create(std::bind(producer, 'B'));
    // create(consumer);
    // join();
    cout << "main end" << endl;
    return 0;
}