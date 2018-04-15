#include <iterator>
#include <optional>
#include <functional>
#include <iostream>
#include <memory>


namespace fun
{
    template <typename Container>
    class Iter;

    template <typename T>
    class Take;

    template <typename T>
    class Filter;

    template <typename Tin, typename Tout>
    class Map;
 
    template <typename Tfirst, typename Tsecond>
    class Zip;

    template <typename Container>
    auto iter(Container c)
    {
        return Iter<Container>(c);
    }

    template <typename T>
    class IIterator
    {
        public:
            virtual std::optional<T*> next() = 0;

            virtual ~IIterator(){};

            auto take(int count)
            {
                return Take<T>(*this, count);
            }

            auto filter(std::function<bool(const T&)> predicate)
            {
                return Filter<T>(*this, predicate);
            }

            template <typename Tout>
            auto map(std::function<Tout(const T&)> function)
            {
                return Map<T, Tout>(*this, function);
            }

            template <typename Tother>
            auto zip(IIterator<Tother>& other)
            {
                return Zip<T, Tother>(*this, other);
            }

            template <template <typename, typename...> class Container, typename... Args>
            auto collect()
            {
                Container<T, Args...> cont;
                
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
    class Iter : public IIterator<typename Container::value_type>
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

    template <typename T>
    class Take : public IIterator<T>
    {
        IIterator<T>& _iter;
        int _count;

      public:
        Take(IIterator<T>& iter, int count) 
            : _iter(iter)
            , _count(count)
        {
        }

        std::optional<T*> next() override
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

    template <typename T>
    class Filter : public IIterator<T>
    {
        IIterator<T>& _iter;
        std::function<bool(const T&)> _predicate;

      public:
        Filter(IIterator<T>& iter, std::function<bool(const T&)>  predicate)
            : _iter(iter)
            , _predicate(predicate)
        {
        }

        std::optional<T*> next() override
        {
            while(auto item = _iter.next())
            {
                std::cerr << "Test::next\n";
                if (_predicate(**item))
                {
                    std::cerr << "Emitting";
                    return *item;
                }
            }

            return {};
        }
    };

    template <typename Tin, typename Tout>
    class Map : public IIterator<Tout>
    {
        IIterator<Tin>& _iter;
        Tout _result;
        std::function<Tout(const Tin&)> _fun;

      public:
        Map(IIterator<Tin>& iter, std::function<Tout(const Tin&)> fun)
            : _iter(iter)
            , _fun(fun)
        {
        }

        std::optional<Tout*> next() override
        {
            if (auto item = _iter.next())
            {
                _result = _fun(**item);
                return &_result;
            }

            return {};
        }
    };

    template <typename Tfirst, typename Tsecond>
    class Zip : public IIterator<std::pair<Tfirst, Tsecond> >
    {
        IIterator<Tfirst>& _iter1;
        IIterator<Tsecond>& _iter2;

        using Pair = std::pair<Tfirst, Tsecond>;
        Pair _result;

      public:
        Zip(IIterator<Tfirst>& iter1, IIterator<Tsecond>& iter2)
            : _iter1(iter1)
            , _iter2(iter2)
        {
        }

        std::optional<std::pair<Tfirst, Tsecond>* > next() override
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
