#include <iostream>
#include <vector>
#include <list>
#include <map>
#include "funiter.h"

struct Test
{
    int member;

    Test()
    {
    }
    
    Test(int n)
    {
        member = n;
    }

    Test(const Test& other)
    {
        member = other.member;
        std::cerr << "copy" << member << " ";
    }

    Test(Test&& other)
    { 
        member = other.member;
        other.member+=10;
        std::cerr << "mov" << member << " ";
    }

    Test& operator=(const Test& oth)
    {
        member = oth.member;
        std::cerr << "=" << member << " ";

        return *this;
    }
};

int main(int argc, char** argv)
{
    std::list<int> a { 0, 1, 2, 42, 43, 44 };
    std::list<std::string> b { "test", "hello", "world", "aloha" };
    fun::Iter it1(a);
    fun::Iter it2(b);

    auto map = [](int a) { return a*a; };

    //auto it2 = it.map<std::vector<int>>(square).take(2).collect();
    auto v = it1.take(5).filter([](auto a) { return a > 2;}).map<std::vector<float>>([](auto i) { return 1.0*i; })
        .zip(it2).collect();

    std::cerr << "elems of " << typeid(v).name() << "\n";
    for (auto& a : v)
        std::cerr << a << "\n";

    return 0;
}
