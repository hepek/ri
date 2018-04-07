#include <iostream>
#include <iterator>
#include <optional>
#include <vector>
#include <functional>

namespace fun
{
    template<typename ContainerIn, typename ContainerOut>
    class Map;

    template<typename Container>
    class Filter;

    template<typename Container>
    class Take;

    template <typename Container>
    class IIterator
    {
        public:
            virtual std::optional<typename Container::value_type> next() = 0;

            template<typename OutCont>
            Map<OutCont, Container> map(std::function<typename OutCont::value_type(typename Container::value_type)> fun)
            {
                return Map<OutCont, Container>(*this, fun);
            }

            Filter<Container> filter(std::function<bool(typename Container::value_type)> pred)
            {
                return Filter<Container>(*this, pred);
            }

            Take<Container> take(int count)
            {
                return Take<Container>(*this, count);
            }

            Container collect()
            {
                Container cont;
                
                while (auto item = next())
                {
                    cont.push_back(*item);
                }

                return cont;
            }
    };

    template <typename Container>
    class Iterator : public IIterator<Container>
    {
        protected:
            typename Container::iterator _begin;
            typename Container::iterator _end;

        public:
            Iterator(Iterator& other) = default;

            Iterator(Container& cont)
                : _begin(std::begin(cont))
                , _end(std::end(cont))
            {
            }

            std::optional<typename Container::value_type> next() override
            {
                if (_begin == _end)
                {
                    return {};
                }
                else
                {
                    auto& res = *_begin;
                    _begin++;
                    return res;
                }
            }

    };

    template <typename Container>
    class Filter : public IIterator<Container>
    {
        IIterator<Container>& _iter;
        std::function<bool(typename Container::value_type)> _predicate;

        public:
            Filter(IIterator<Container>& iter, std::function<bool(typename Container::value_type)> pred)
                : _iter(iter)
                , _predicate(pred)
            {
            }

            std::optional<typename Container::value_type> next() override
            {
                while(auto item = _iter.next())
                {
                    if (_predicate(*item))
                        return item;
                }

                return {};
            }
    };
    
    template <typename Container, typename OutContainer>
    class Map : public IIterator<OutContainer>
    {
        IIterator<Container>& _iter;
        std::function<typename OutContainer::value_type(typename Container::value_type)> _fun;

      public:
        Map(IIterator<Container>& iter, std::function<typename OutContainer::value_type(typename Container::value_type)> fun)
            : _iter(iter)
            , _fun(fun)
        {
        }

        std::optional<typename OutContainer::value_type> next() override
        {
            if (auto item = _iter.next())
            {
                return _fun(*item);
            }

            return {};
        }
    };

    template <typename Container>
    class Take : public IIterator<Container>
    {
        IIterator<Container>& _iter;
        int _count;

      public:
        Take(IIterator<Container>& iter, int count) 
            : _iter(iter)
            , _count(count)
        {
        }

        std::optional<typename Container::value_type> next() override
        {
            if (_count > 0)
            if (auto item = _iter.next())
            {
                _count--;
                return item;
            }

            return {};
        }
    };
}

struct Test
{
    int member;

    Test()
    {
    }

    Test(const Test& other)
    {
        std::cerr << "copy ";
    }

    Test(Test&& other)
    {
        std::cerr << "mov ";
    }

    Test& operator=(const Test& oth)
    {
        std::cerr << "= ";
    }
};


int main(int argc, char** argv)
{
    auto square = std::function([](int a) -> int { return a*a; });
    auto gt0 = std::function([](const int& a) -> bool { return a > 0; });

    //std::vector<int> a{ 0, 1, 2, 3 };
    std::vector<Test> a { Test(), Test() };

    fun::Iterator it(a);

    //auto it2 = it.map<std::vector<int>>(square).take(2).collect();
    auto it2 = it.take(2).take(2).collect();

    //for (auto& x : it2)
    //  std::cout << x << " ";

    std::cout << std::endl;

    return 0;
}
