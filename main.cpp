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

    template<typename Container>
    class Cloned;

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

            Cloned<Container> cloned() 
            {
                return Cloned<Container>(*this);
            }

            Container collect()
            {
                Container cont;
                
                while (auto item = next())
                {
                    cont.push_back(std::move(*item));
                }

                return cont;
            }
    };

    template <typename Container>
    class IntoIter : public IIterator<Container>
    {
        protected:
            Container _container;
            typename Container::iterator _begin;
            typename Container::iterator _end;

        public:
            IntoIter(IntoIter& other) = default;

            IntoIter(Container&& cont)
                : _container(std::move(cont))
                , _begin(std::begin(_container))
                , _end(std::end(_container))
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
                    return std::move(res);
                }
            }
    };

    template <typename Container>
    class Iter : public IIterator<Container>
    {
        protected:
            Container& _container;
            typename Container::iterator _begin;
            typename Container::iterator _end;

        public:
            Iter(Iter& other) = default;

            Iter(Container& cont)
                : _container(cont)
                , _begin(std::begin(_container))
                , _end(std::end(_container))
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
                        return std::move(*item);
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
                return std::move(*item);
            }

            return {};
        }
    };

    template <typename Container>
    class Cloned : public IIterator<Container>
    {
        IIterator<Container>& _iter;

      public:
        Cloned(IIterator<Container>& iter)
            : _iter(iter)
        {
        }

        std::optional<typename Container::value_type> next() override
        {
            if (auto item = _iter.next())
            {
                return *item;
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
    std::vector<Test> a { Test(1), Test(2) };

    fun::Iter it(a);

    //auto it2 = it.map<std::vector<int>>(square).take(2).collect();
    auto vec2 = it.take(2).take(1).collect();

    for (auto& x : vec2)
      std::cerr << x.member << " ";

    std::cerr << std::endl;

    for (auto& x : a)
        std::cerr << x.member << " ";

    std::cerr << std::endl;

    return 0;
}
