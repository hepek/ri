#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <vector>
#include <optional>
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
//todo
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

    //Returning nullptr if there are less than n + 1 elements:
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

// TEST_CASE("for_each") TODO


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
}
