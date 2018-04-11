#include <iostream>
#include <vector>
#include <list>
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
    auto square = std::function([](int a) -> int { return a*a; });
    auto gt0 = std::function([](const int& a) -> bool { return a > 0; });

    //std::vector<int> a{ 0, 1, 2, 3 };
    std::list<Test> a { Test(2), Test(3), Test(4) };
    std::list<int> b { 42, 43, 44 };

    std::cerr << "Mkiter" << std::endl;

    fun::ref::Iter it(a);

    //auto it2 = it.map<std::vector<int>>(square).take(2).collect();
    auto vec2 = it.take(3).map<std::list<Test>>([](const Test& a)
            { return Test(a.member*a.member); }).collect();
    std::cerr << "\n\n";

    for (auto& x : vec2)
      std::cerr << x.member << " ";

    std::cerr << std::endl;

    return 0;
}
