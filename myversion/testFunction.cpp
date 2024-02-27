#include <iostream>
#include <functional>
using namespace std;
class A
{
public:
    void output(int x) { cout << x << endl; }
};

void testFunc(void (*f)())
{
    cout << f << endl;
}

int main()
{
    A a;
    function<void()> func(bind(&A::output, &a, 1));
    func();
    testFunc(func.target<void()>());
    auto f = func.target<_Bind<void (A::*(A*, int))(int x)>>();
    (*f)();
}