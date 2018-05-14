#define CATCH_CONFIG_MAIN
#include <map>
#include <optional>
#include <vector>
#include <chrono>
#include "catch.hpp"
#include "ri.h"


// Tests generated from Rust documentation
// https://doc.rust-lang.org/std/iter/trait.Iterator.html

TEST_CASE("basic usage")
{
    std::vector<int> a{1, 2, 3};

    auto iter = ri::iter(a);

    // A call to next() returns the next value...
    REQUIRE(&a[0] == iter->next());
    REQUIRE(&a[1] == iter->next());
    REQUIRE(&a[2] == iter->next());

    // ... and then None once it's over.
    REQUIRE(nullptr ==  iter->next());

    // More calls may or may not return None. Here, they always will.
    REQUIRE(nullptr == iter->next());
    REQUIRE(nullptr == iter->next());
}

TEST_CASE("cycle")
{
    std::vector<int> a{1, 2, 3};
    auto iter = ri::iter(a)->cycle();

    REQUIRE(*iter->next() == 1);
    REQUIRE(*iter->next() == 2);
    REQUIRE(*iter->next() == 3);
    REQUIRE(*iter->next() == 1);
    REQUIRE(*iter->next() == 2);
    REQUIRE(*iter->next() == 3);
    REQUIRE(*iter->next() == 1);
    REQUIRE(*iter->next() == 2);
    REQUIRE(*iter->next() == 3);
}

TEST_CASE("sum")
{
    std::vector<int> a{1, 2, 3};
    auto s = ri::iter(a)->sum();

    REQUIRE(s == 6);
}

TEST_CASE("product")
{
    auto factorial = [](int n)
    {
        auto firstN = ri::gen(1)->take_while([=](auto i) { return i <= n; });
        return firstN->product();
    };

    REQUIRE(factorial(0) == 1);
    REQUIRE(factorial(1) == 1);
    REQUIRE(factorial(5) == 120);
}


TEST_CASE("count")
{
    std::vector<int> a {1, 2, 3};
    REQUIRE(ri::iter(a)->count() == 3);

    a = {1, 2, 3, 4, 5};
    REQUIRE(ri::iter(a)->count() == 5);
}

TEST_CASE("last")
{
    std::vector<int> a {1, 2, 3};
    REQUIRE(ri::iter(a)->last() == &a[2]);
}

TEST_CASE("nth")
{
    std::vector<int> a {1, 2, 3};
    auto it = ri::iter(a);
    REQUIRE(it->nth(1) == &a[1]);

    // Calling nth() multiple times doesn't rewind the iterator:
    REQUIRE(!it->nth(1));
}

TEST_CASE("chain")
{
    std::vector<int> a1 = {1, 2, 3};
    std::vector<int> a2 = {4, 5, 6};

    auto iter = ri::iter(a1)->chain(ri::iter(a2));

    REQUIRE(iter->next() == &a1[0]);
    REQUIRE(iter->next() == &a1[1]);
    REQUIRE(iter->next() == &a1[2]);
    REQUIRE(iter->next() == &a2[0]);
    REQUIRE(iter->next() == &a2[1]);
    REQUIRE(iter->next() == &a2[2]);
    REQUIRE(!iter->next());
}

TEST_CASE("zip")
{   
    std::vector<int> a1 = {1, 2, 3, 4};
    std::vector<int> a2 = {4, 5, 6};

    auto iter = ri::iter(a1)->zip<int>(ri::iter(a2));

    REQUIRE(*iter->next() == std::make_pair(1,4));
    REQUIRE(*iter->next() == std::make_pair(2,5));
    REQUIRE(*iter->next() == std::make_pair(3,6));
    REQUIRE(!iter->next());
}

TEST_CASE("map")
{   
    std::vector<int> a = {1, 2, 3};

    auto iter = ri::iter(a)->map<int>([](auto x) { return 2*x; });

    REQUIRE(*iter->next() == 2);
    REQUIRE(*iter->next() == 4);
    REQUIRE(*iter->next() == 6);
    REQUIRE(!iter->next());
}

TEST_CASE("scan")
{
    std::vector<int> a = {1, 2, 3};
    auto iter = ri::iter(a)->scan<float>(1.f, [](float state, int x)
            {
                return state*x;
            });

    REQUIRE(*iter->next() == 1.f);
    REQUIRE(*iter->next() == 2.f);
    REQUIRE(*iter->next() == 6.f);
    REQUIRE(!iter->next());
}

TEST_CASE("for_each")
{
    int i = 0;
    auto iter = ri::gen(0,10);

    iter->for_each([&](auto x) { i++; });

    REQUIRE(i == 10);
}


TEST_CASE("filter")
{
    std::vector<int> a = {0, 1, 2};

    auto iter = ri::iter(a)->filter([](auto x) { return x > 0; });

    REQUIRE(*iter->next() == 1);
    REQUIRE(*iter->next() == 2);
    REQUIRE(!iter->next());
}

TEST_CASE("filter_map")
{
    std::vector<std::string> a = {"1", "lol", "3", "NaN", "5"};

    auto iter = ri::iter(a)->filter_map<int>(
            [](const std::string& s) -> std::optional<int>
            { 
                try
                {
                    int res = std::stoi(s);
                    return res;
                }
                catch (std::exception&)
                {
                    return {};
                }
            });

    REQUIRE(*iter->next() == 1);
    REQUIRE(*iter->next() == 3);
    REQUIRE(*iter->next() == 5);
    REQUIRE(!iter->next());
}

TEST_CASE("enumerate")
{
    std::vector<char> a = {'a', 'b', 'c'};

    auto iter = ri::iter(a)->enumerate();

    REQUIRE(*iter->next() == std::make_pair(size_t(0), 'a'));
    REQUIRE(*iter->next() == std::make_pair(size_t(1), 'b'));
    REQUIRE(*iter->next() == std::make_pair(size_t(2), 'c'));
    REQUIRE(!iter->next());
}

//TODO Peekable

TEST_CASE("skip_while")
{
    std::vector<int> a = {-1, -2, 0, 1, 2, -3};
    auto iter = ri::iter(a)->skip_while([](auto x) { return x < 0; });

    REQUIRE(*iter->next() == 0);
    REQUIRE(*iter->next() == 1);
    REQUIRE(*iter->next() == 2);
    REQUIRE(*iter->next() == -3);
    REQUIRE(!iter->next());
}

TEST_CASE("take_while")
{
    std::vector<int> a = {-1, -2, 0, 1, 2, -3};
    auto iter = ri::iter(a)->take_while([](auto x) { return x < 0; });

    REQUIRE(*iter->next() == -1);
    REQUIRE(*iter->next() == -2);
    REQUIRE(!iter->next());
}

TEST_CASE("skip")
{
    std::vector<char> a = {'a', 'b', 'c'};

    auto iter = ri::iter(a)->skip(2);

    REQUIRE(*iter->next() == 'c');
    REQUIRE(!iter->next());
}

TEST_CASE("take")
{
    std::vector<char> a = {'a', 'b', 'c'};

    auto iter = ri::iter(a)->take(2);

    REQUIRE(*iter->next() == 'a');
    REQUIRE(*iter->next() == 'b');
    REQUIRE(!iter->next());
}

TEST_CASE("generator")
{
    auto iter = ri::gen(0,4);

    REQUIRE(*iter->next() == 0);
    REQUIRE(*iter->next() == 1);
    REQUIRE(*iter->next() == 2);
    REQUIRE(*iter->next() == 3);
    REQUIRE(!iter->next());
}

TEST_CASE("flat_map")
{
    std::vector<int> a { 1, 2, 3 };
    auto iter = ri::iter(a);

    auto m = iter->flat_map<int>([](auto x) { return ri::gen<int>(0,x); });

    REQUIRE(*m->next() == 0);
    REQUIRE(*m->next() == 0);
    REQUIRE(*m->next() == 1);
    REQUIRE(*m->next() == 0);
    REQUIRE(*m->next() == 1);
    REQUIRE(*m->next() == 2);
    REQUIRE(!m->next());
}

TEST_CASE("collect")
{
    std::vector<std::string> b { "test", "hello", "world", "aloha" };
    auto numbers = ri::gen<int>(1,5,50);
    auto strings = ri::iter(b);

    auto sq = [](auto a) { return a*a; };

    auto transformed = numbers->map<int>(sq);
    auto v = strings->zip<int>(transformed)->collect<std::map<std::string,int>>();

    REQUIRE(v.find("hello")->second == 6*6);
}


TEST_CASE("partition")
{
    std::vector<int> a = {-1, -2, 0, 1, 2, -3};
    auto iter = ri::iter(a);

    auto[neg, pos] = iter->partition<std::vector>([](auto a) { return a < 0; });

    REQUIRE(neg.size() == 3);
    REQUIRE(pos.size() == 3);

    REQUIRE(neg[0] == -1);
    REQUIRE(pos[2] == 2);
}

TEST_CASE("fold")
{
    std::vector<int> a = {1, 2, 3};

    auto sum = ri::iter(a)->fold<int>(0, [](auto acc, auto x) { return acc + x; });

    REQUIRE(sum == 6);
}


TEST_CASE("all")
{
    std::vector<int> a = {0, 1, 2};
    auto iter = ri::iter(a);

    REQUIRE(iter->all([](auto x) { return x >= 0; }));
}

TEST_CASE("any")
{
    std::vector<int> a = {3, 0, 1, 2};
    auto iter = ri::iter(a);

    REQUIRE(iter->any([](auto x) { return x == 0; }));
}

TEST_CASE("find")
{
    std::vector<int> a = {3, 0, 1, 2};
    auto iter = ri::iter(a);

    REQUIRE(*iter->find([](auto x) { return x == 0; }) == 0);
    REQUIRE(!iter->find([](auto x) { return x == 1389; }));
}

TEST_CASE("position")
{
    std::vector<int> a = {3, 0, 1, 2};
    auto iter = ri::iter(a);

    REQUIRE(*iter->position([](auto x) { return x == 0; }) == 1);
    REQUIRE(!iter->position([](auto x) { return x == 1389; }));
}

TEST_CASE("min and max")
{
    std::vector<int> a = {3, 0, 1, 2};

    REQUIRE(*ri::iter(a)->max() == 3);
    REQUIRE(*ri::iter(a)->min() == 0);

    std::vector<int> b;

    REQUIRE(!ri::iter(b)->max());
    REQUIRE(!ri::iter(b)->min());
}

/*
TEST_CASE("lines")
{
    auto l = ri::lines(ri::fs::path("/etc/passwd"));

    auto it = l->take(5);

    while(auto line = it->next())
        std::cerr << *line << "\n";
}
*/

TEST_CASE("eq")
{
    REQUIRE(ri::gen(1,10)->eq(ri::gen(1,10)));
    REQUIRE(!ri::gen(1,10)->eq(ri::gen(1,8)));
    REQUIRE(!ri::gen(1,10)->eq(ri::gen(11,20)));
}

/* WIP
TEST_CASE("ne")
{
    REQUIRE(!ri::gen(1,10)->ne(ri::gen(1,10)));
    REQUIRE(ri::gen(1,8)->ne(ri::gen(1,10)));
    REQUIRE(ri::gen(1,10)->ne(ri::gen(11,20)));
}
*/


class Perf
{    
    std::chrono::high_resolution_clock::time_point _start;
    std::string _name;

    public:
        Perf(const std::string& name)
            : _start(std::chrono::high_resolution_clock::now())
            , _name(name)
        {
        }

        ~Perf()
        {
            auto now = std::chrono::high_resolution_clock::now();
            std::cerr << _name << ": " <<  (std::chrono::duration_cast<std::chrono::microseconds>(now - _start).count()/1000.0) << " ms\n";
        }
};

TEST_CASE("perf")
{
    std::vector<int> a = ri::gen<int>(1,1000000)->collect<std::vector>();
    std::vector<int> b;
    std::vector<int> c;

    {
        Perf test("for loop");
        for (auto& x : a)
            b.push_back(x*x);
    }

    {
        Perf test("ri::it+map");

        b = ri::iter(a)->map<int>([](auto x){ return x*x;})->collect<std::vector>();
    }


    {
        Perf test("ri::it+map+take");

        b = ri::iter(a)->map<int>([](auto x){ return x*x;})->take(1000000)->collect<std::vector>();
    }
}
