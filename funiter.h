#include <iterator>
#include <optional>
#include <functional>
#include <iostream>


namespace fun
{
    template <typename Container>
    class Take;

    template <typename Container>
    class Filter;

    template <typename InContainer, typename OutContainer>
    class Map;
 
    template <typename FirstContainer, typename SecondContainer>
    class Zip;

    template <typename Container>
    class IIterator
    {
        public:
            virtual std::optional<typename Container::value_type*> next() = 0;

            auto take(int count)
            {
                return Take<Container>(*this, count);
            }

            auto filter(std::function<bool(const typename Container::value_type&)> predicate)
            {
                return Filter<Container>(*this, predicate);
            }


            template <typename OutContainer>
            auto map(std::function<typename OutContainer::value_type(const typename Container::value_type&)> function)
            {
                return Map<Container, OutContainer>(*this, function);
            }

            template <typename OtherContainer>
            IIterator<std::pair<typename Container::value_type, typename OtherContainer::value_type>>
            zip(IIterator<OtherContainer>& other)
            {
                return Zip<Container, OtherContainer>(*this, other);
            }

            auto collect()
            {
                Container cont;
                
                while (auto item = next())
                    cont.insert(std::end(cont), **item);

                return cont;
            }

            template <typename OutContainer>
            auto collect()
            {
                OutContainer cont;

                while (auto item = next())
                    cont.insert(std::end(cont), **item);

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
                    auto& res = *_begin;
                    _begin++;
                    return &res;
                }
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
                _count--;
                return *item;
            }

            return {};
        }
    };

    template <typename Container>
    class Filter : public IIterator<Container>
    {
        IIterator<Container>& _iter;
        std::function<bool(const typename Container::value_type&)> _predicate;

      public:
        Filter(IIterator<Container>& iter, std::function<bool(const typename Container::value_type&)>  predicate)
            : _iter(iter)
            , _predicate(predicate)
        {
        }

        std::optional<typename Container::value_type*> next() override
        {
            while(auto item = _iter.next())
                if (_predicate(**item))
                    return *item; 

            return {};
        }
    };

    template <typename InContainer, typename OutContainer>
    class Map : public IIterator<OutContainer>
    {
        IIterator<InContainer>& _iter;
        typename OutContainer::value_type _result;
        std::function<typename OutContainer::value_type(const typename InContainer::value_type&)> _fun;

      public:
        Map(IIterator<InContainer>& iter, std::function<typename OutContainer::value_type(const typename InContainer::value_type&)> fun)
            : _iter(iter)
            , _fun(fun)
        {
        }

        std::optional<typename OutContainer::value_type*> next() override
        {
            if (auto item = _iter.next())
            {
                _result = _fun(**item);
                return &_result;
            }

            return {};
        }
    };

    template <typename FirstContainer, typename SecondContainer>
    class Zip : public IIterator<std::pair<typename FirstContainer::value_type, typename SecondContainer::value_type>>
    {
        IIterator<FirstContainer>& _iter1;
        IIterator<SecondContainer>& _iter2;

        using Pair = std::pair<typename FirstContainer::value_type, typename SecondContainer::value_type>;
        Pair _result;

      public:
        Zip(IIterator<FirstContainer>& iter1, IIterator<SecondContainer>& iter2)
            : _iter1(iter1)
            , _iter2(iter2)
        {
        }

        std::optional<Pair*> next() override
        {
            if (auto item1 = _iter1.next())
            if (auto item2 = _iter2.next())
            {
                _result = std::make_pair(**item1, **item2);
                return &_result;
            }

            return {};
        }
    };
} // fun
