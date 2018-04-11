#include <iterator>
#include <optional>
#include <functional>
#include <iostream>


namespace fun
{
namespace into
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

            virtual Container collect()
            {
                Container cont;
                
                while (auto item = next())
                    cont.push_back(*item);

                return cont;
            }

            size_t count()
            {
                size_t cnt = 0;

                while (next())
                    cnt++;

                return cnt;
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

            Container collect() override
            {
                Container cont;
                
                while (auto item = next())
                    cont.push_back(std::move(*item));

                return cont;
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
} // into

namespace ref
{
    template<typename ContainerIn, typename ContainerOut>
    class Map;

    template<typename Container>
    class Filter;

    template<typename Container>
    class Take;

    template<typename Container>
    class Cloned;

    template<typename Container, typename Container2>
    class Zip;

    template <typename Container>
    class IIterator
    {
        public:
            virtual std::optional<typename Container::value_type*> next() = 0;

            template<typename OutCont>
            Map<OutCont, Container> map(std::function<typename OutCont::value_type(const typename Container::value_type&)> fun)
            {
                return Map<OutCont, Container>(*this, fun);
            }

            Filter<Container> filter(std::function<bool(const typename Container::value_type&)> pred)
            {
                return Filter<Container>(*this, pred);
            }

            Take<Container> take(int count)
            {
                return Take<Container>(*this, count);
            }

            virtual Container collect()
            {
                Container cont;
                
                while (auto item = next())
                {
                    std::cerr << "collect\n";
                    cont.push_back(**item);
                }

                return cont;
            }

            size_t count()
            {
                size_t cnt = 0;

                while (next())
                    cnt++;

                return cnt;
            }
    };

    template <typename Container>
    class Iter : public IIterator<Container>
    {
        protected:
            typename Container::iterator _begin;
            typename Container::iterator _end;

        public:
            Iter(Iter& other) = default;

            Iter(Container& cont)
                : _begin(std::begin(cont))
                , _end(std::end(cont))
            {
            }

            std::optional<typename Container::value_type*> next() override
            {
                if (_begin == _end)
                {
                    return {};
                }
                else
                {
                    std::cerr << "iter next\n";
                    auto& res = *_begin;
                    _begin++;
                    return &res;
                }
            }
    };

    template <typename Container>
    class Filter : public IIterator<Container>
    {
        IIterator<Container>& _iter;
        std::function<bool(typename Container::value_type)> _predicate;

        public:
            Filter(IIterator<Container>& iter, std::function<bool(const typename Container::value_type&)> pred)
                : _iter(iter)
                , _predicate(pred)
            {
            }

            std::optional<typename Container::value_type*> next() override
            {
                while(auto item = _iter.next())
                {
                    if (_predicate(**item))
                        return *item;
                }

                return {};
            }
    };
    
    template <typename Container, typename OutContainer>
    class Map : public IIterator<OutContainer>
    {
        IIterator<Container>& _iter;
        typename OutContainer::value_type _result;
        std::function<typename OutContainer::value_type(const typename Container::value_type&)> _fun;

      public:
        Map(IIterator<Container>& iter, std::function<typename OutContainer::value_type(typename Container::value_type)> fun)
            : _iter(iter)
            , _fun(fun)
        {
        }

        std::optional<typename OutContainer::value_type*> next() override
        {
            if (auto item = _iter.next())
            {
                std::cerr << "map next\n";
                _result = _fun(**item);
                return &_result;
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

        std::optional<typename Container::value_type*> next() override
        {
            if (_count > 0)
            if (auto item = _iter.next())
            {
                std::cerr << "take next\n";
                _count--;
                return *item;
            }

            return {};
        }
    };

    template <typename Container>
    class Cloned : public IIterator<Container>
    {
        IIterator<Container>& _iter;
        typename Container::value_type _current;

      public:
        Cloned(IIterator<Container>& iter)
            : _iter(iter)
        {
        }

        std::optional<typename Container::value_type*> next() override
        {
            if (auto item = _iter.next())
            {
                _current = **item;
                return &_current;
            }

            return {};
        }
    };

/*
    template<typename Container, typename Container2>
    class Zip : public IIterator<std::pair<Container,Container2>>
    {
        IIterator<Container>& _iter1;
        IIterator<Container2>& _iter2;

      public:
        Zip(IIterator<Container>& iter1, IIterator<Container2>& iter2)
            : _iter1(iter1)
            , _iter2(iter2)
        {
        }

        std::optional<std::pair<typename Container::value_type*,
                                typename Container2::value_type*>> next() override
        {
            if (auto item1 = _iter1.next())
            if (auto item2 = _iter2.next())
                return std::make_pair(*item1, *item2);

            return {};
        }
    };
    */
} // ref
} // fun
