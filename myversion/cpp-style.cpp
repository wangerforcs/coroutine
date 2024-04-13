#include <assert.h>
#include <functional>
#include <iostream>
#include <memory>
#include <random>
#include <string.h>
#include <ucontext.h>
#include <vector>

namespace coroutine
{
class CoScheduler;
class Coroutine
{
private:
    enum State { FREE, RUNNABLE, RUNNING, SUSPEND };
    typedef std::function<void()> Task;
    friend class CoScheduler;
    char *stack;
    State state;
    ucontext_t context;
    Task task;
    size_t sz;
    size_t cap;
    void saveStack(char *sk, size_t sksz)
    {
        char pos;
        char *top = sk + sksz;
        assert(sk < &pos);
        assert(&pos < top);
        if (cap < top - &pos) {
            cap = top - &pos;
            if (stack) {
                delete[] stack;
            }
            stack = new char[cap];
        }
        sz = top - &pos;
        memcpy(stack, &pos, sz);
    }

public:
    Coroutine(Task tk)
    {
        task = tk;
        stack = nullptr;
        sz = 0;
        cap = 0;
    }
    void release()
    {
        if (stack) {
            delete[] stack;
        }
        stack = nullptr;
        sz = 0;
        cap = 0;
        state = FREE;
    }
    ~Coroutine()
    {
        if (stack) {
            delete[] stack;
        }
    }
    bool dead() { return state == FREE; }
};

class CoScheduler
{
private:
    enum { STACK_SIZE = 4096, MAX_CO = 2 };
    char stack[STACK_SIZE];
    std::vector<Coroutine> coroutines;
    int curId;
    int cnt;
    ucontext_t main;
    CoScheduler()
    {
        cnt = 0;
        curId = -1;
        getcontext(&main);
    }
    ~CoScheduler()
    {
        for (size_t i = 0; i < coroutines.size(); i++) {
            if (coroutines[i].stack) {
                delete[] coroutines[i].stack;
            }
        }
    }

public:
    CoScheduler(const CoScheduler &) = delete;
    CoScheduler &operator=(const CoScheduler &) = delete;
    static CoScheduler &getInstance()
    {
        static CoScheduler sch;
        return sch;
    }
    int createCoroutine(Coroutine::Task tk)
    {
        if (cnt >= MAX_CO) {
            return -1;
        }
        int id = -1;
        for (size_t i = 0; i < coroutines.size(); i++) {
            if (coroutines[i].state == Coroutine::FREE) {
                coroutines[i].task = tk;
                id = i;
                break;
            }
        }
        if (id == -1) {
            coroutines.emplace_back(tk);
            id = coroutines.size() - 1;
        }
        cnt++;
        Coroutine &co = coroutines[id];
        co.state = Coroutine::RUNNABLE;
        getcontext(&co.context);
        co.context.uc_stack.ss_sp = stack;
        co.context.uc_stack.ss_size = STACK_SIZE;
        co.context.uc_link = &main;
        // 最初wrapFunc定义的是非静态函数，然后尝试用以下方法segfault，原因是target只能得到bind对象，此处返回nullptr
        // Coroutine::Task t(std::bind(&CoScheduler::wrapFunc, this));
        // makecontext(&co.context, t.target<void()>(), 0);
        makecontext(&co.context, (void (*)(void)) & CoScheduler::wrapFunc, 2, this, id);
        return id;
    }
    void yield()
    {
        Coroutine &cur = coroutines[curId];
        cur.saveStack(stack, STACK_SIZE);
        cur.state = Coroutine::SUSPEND;
        swapcontext(&cur.context, &main);
    }
    void resume(size_t id)
    {
        if (id >= coroutines.size() || coroutines[id].state == Coroutine::FREE) {
            return;
        }
        if (id == curId) {
            return;
        }
        Coroutine &next = coroutines[id];
        Coroutine &cur = coroutines[curId];
        curId = id;
        if (next.state == Coroutine::SUSPEND) {
            memcpy(stack + STACK_SIZE - next.cap, next.stack, next.cap);
        }
        next.state = Coroutine::RUNNING;
        swapcontext(&main, &next.context);
    }
    void destroy(size_t id)
    {
        if (id >= coroutines.size() || coroutines[id].state == Coroutine::FREE) {
            return;
        }
        coroutines[id].release();
        cnt--;
    }
    static void wrapFunc(CoScheduler *sch, size_t id)
    {
        Coroutine &co = sch->coroutines[id];
        co.task();
        co.release();
        sch->cnt--;
        sch->curId = -1;
    }
    bool dead(size_t id) { return coroutines[id].dead(); }
};
CoScheduler &sch = CoScheduler::getInstance();
int create(std::function<void()> tk)
{
    return sch.createCoroutine(tk);
}
void resume(size_t id)
{
    sch.resume(id);
}
void yield()
{
    sch.yield();
}
bool dead(size_t id)
{
    return sch.dead(id);
}
void destroy(size_t id)
{
    sch.destroy(id);
}
} // namespace coroutine

void func(int id)
{
    for (int i = 0; i < 10; i++) {
        std::cout << "coroutine " << id << " : " << i << std::endl;
        coroutine::yield();
    }
}

void test()
{
    int id1 = coroutine::create(std::bind(func, 1));
    int id2 = coroutine::create(std::bind(func, 2));
    assert(id1 != -1);
    assert(id2 != -1);
    std::cout << "main start" << std::endl;
    while (!coroutine::dead(id1) && !coroutine::dead(id2)) {
        coroutine::resume(id1);
        coroutine::resume(id2);
    }

    int id3 = coroutine::create(std::bind(func, 3));
    assert(id3 != -1);
    int id4 = coroutine::create(std::bind(func, 4));
    assert(id4 != -1);
    coroutine::resume(id3);
    coroutine::destroy(id3);
    int id5 = coroutine::create(std::bind(func, 5));
    assert(id5 != -1);

    while (!coroutine::dead(id4) && !coroutine::dead(id5)) {
        coroutine::resume(id4);
        coroutine::resume(id5);
    }

    std::cout << "main end" << std::endl;
}
int main()
{
    test();
}