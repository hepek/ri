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
    
    friend std::ostream& operator<<(std::ostream& os, const Test& t);  
};

std::ostream& operator<<(std::ostream& out, const Test& t)
{
    out << "Test(" << t.member << ")";
    return out;
}

int main(int argc, char** argv)
{
    std::list<int> a { (0), (1), (2), (3), (42), (43), (44) };
    std::list<std::string> b { "test", "hello", "world", "aloha" };
    auto it1 = fun::iter(a);
    auto strings = fun::iter(b);

    auto gt2 = [](auto a) { return a > 2; };
    auto sq = [](auto a) { return a*a; };

    auto transformed = it1.take(5).filter(gt2).map<int>(sq);

    auto v = strings.zip<int>(transformed).collect<std::map<std::string,int>>();

    std::cerr << "\nelems of " << typeid(v).name() << "\n";

    for (auto& a : v)
        std::cerr << a.first << " = " << a.second << "\n";

    return 0;
}
