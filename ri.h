#include <iterator>
#include <optional>
#include <functional>
#include <iostream>
#include <memory>

namespace ri
{
    template <typename Container>
    class Iter;

    template <typename T>
    class Take;

    template <typename T>
    class TakeWhile;

    template <typename T>
    class Skip;

    template <typename T>
    class SkipWhile;

    template <typename T>
    class Filter;

    template <typename Tin, typename Tout>
    class Map;

    template <typename Tin, typename Tout>
    class FilterMap;
 
    template <typename Tfirst, typename Tsecond>
    class Zip;

    template <typename T>
    class Chain;
    
    template <typename T>
    class Generator;

    template <typename Container>
    auto iter(Container& c)
    {
        return std::make_shared<Iter<Container>>(c);
    }

    template <typename T>
    auto gen(const T& start)
    {
        return std::make_shared<Generator<T>>(start, [](auto& n) { n++; });
    }

    template <typename T>
    auto gen(const T& start, const T& end)
    {
        auto ltEnd = [=](auto& n) { return n < end; };
        return gen(start)->take_while(ltEnd);;
    }

    template <typename T>
    auto gen(const T& start, const T& step, const T& end)
    {
        auto ltEnd = [=](auto& n) { return n < end; };
        return std::make_shared<Generator<T>>(start, [=](auto& n) { n = n + step; })->take_while(ltEnd);
    }

    template <typename T>
    class IIterator : public std::enable_shared_from_this<IIterator<T>>
    {
        public:
            virtual T* next() = 0;
            virtual ~IIterator(){};

            using Ptr = std::shared_ptr<IIterator>;

            auto last()
            {
                T* last = nullptr;

                while (auto item = next())
                    last = item;

                return last;
            }

            auto nth(int n)
            {
                int c = 0;

                while (c < n)
                {
                    next();
                    c++;
                }

                return next();
            }

            auto take(int count)
            {
                return std::make_shared<Take<T>>(this->shared_from_this(), count);
            }

            auto filter(std::function<bool(const T&)> predicate)
            {
                return std::make_shared<Filter<T>>(this->shared_from_this(), predicate);
            }

            template <typename Tout>
            auto map(std::function<Tout(const T&)> function)
            {
                return std::make_shared<Map<T, Tout>>(this->shared_from_this(), function);
            }

            template <typename Tout>
            auto filter_map(std::function<std::optional<Tout>(const T&)> function)
            {
                return std::make_shared<FilterMap<T, Tout>>(this->shared_from_this(), function);
            }

            template <typename Tother>
            auto zip(typename IIterator<Tother>::Ptr other)
            {
                return std::make_shared<Zip<T, Tother>>(this->shared_from_this(), other);
            }

            auto chain(IIterator<T>::Ptr other)
            {
                return std::make_shared<Chain<T>>(this->shared_from_this(), other);
            }

            //TODO: for_each
            //TODO: peekable
            //TODO: scan
            //TODO: flat_map
            //TODO: fuse
            //TODO: inspect
            //TODO: partition
            //TODO: try_fold
            //TODO: fold
            //TODO: any, all
            //TODO: find
            //TODO: position, rposition
            //TODO: rev
            //TODO: min, max, max_by_key, max_by, min_by_key, min_by
            //TODO: unzip
            //TODO: ordering stuff
            //TODO: cycle

            auto enumerate()
            {
                auto nums = gen<size_t>(0);
                return std::make_shared<Zip<size_t, T>>(nums, this->shared_from_this());
            }

            auto take_while(std::function<bool(const T&)> pred)
            {
                return std::make_shared<TakeWhile<T>>(this->shared_from_this(), pred);
            }

            auto skip(int count)
            {
                return std::make_shared<Skip<T>>(this->shared_from_this(), count);
            }
            
            auto skip_while(std::function<bool(const T&)> pred)
            {
                return std::make_shared<SkipWhile<T>>(this->shared_from_this(), pred);
            }

            template <template <typename, typename...> class Container, typename... Args>
            auto collect()
            {
                Container<T, Args...> cont;
                
                while (auto item = next())
                    cont.insert(std::end(cont), *item);

                return cont;
            }

            template <typename OutContainer>
            auto collect()
            {
                OutContainer cont;

                while (auto item = next())
                    cont.insert(std::end(cont), *item);

                return cont;
            }

            size_t count()
            {
                size_t cnt = 0;

                while (next())
                    cnt++;

                return cnt;
            }

            T sum()
            {
                T sum = T(0);

                while(auto item = next())
                    sum = sum + *item;

                return sum;
            }
            
            T product()
            {
                T prod = T(1);

                while(auto item = next())
                    prod = prod * *item;

                return prod;
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

            typename Container::value_type* next() override
            {
                if (_begin == _end)
                {
                    return nullptr;
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
    class Generator : public IIterator<T>
    {
        protected:
            T _current;
            std::function<void(T&)> _increment;
            bool _first;

        public:
            Generator(Generator& other) = default;

            Generator(const T& start, std::function<void(T&)> increment)
                : _current(start)
                , _increment(increment)
                , _first(true)
            {
            }

            T* next() override
            {
                if (_first)
                {
                    _first = false;
                    return &_current;
                }
                else
                {
                    _increment(_current);
                    return &_current;
                }
            }
    };

    template <typename T>
    class Take : public IIterator<T>
    {
        typename IIterator<T>::Ptr _iter;
        int _count;

      public:
        Take(typename IIterator<T>::Ptr iter, int count) 
            : _iter(iter)
            , _count(count)
        {
        }

        T* next() override
        {
            if (_count > 0)
            if (auto item = _iter->next())
            {
                _count--;
                return item;
            }

            return nullptr;
        }
    };

    template <typename T>
    class TakeWhile : public IIterator<T>
    {
        typename IIterator<T>::Ptr _iter;
        std::function<bool(const T&)> _pred;
        bool _done;

      public:
        TakeWhile(typename IIterator<T>::Ptr iter, std::function<bool(const T&)> pred) 
            : _iter(iter)
            , _pred(pred)
            , _done(false)
        {
        }

        T* next() override
        {
            while (auto item = _iter->next())
            {
                if (!_done && _pred(*item))
                {
                    return item;
                }
                else
                {
                    _done = false;
                    return nullptr;
                }
            }

            return nullptr;
        }
    };

    template <typename T>
    class Skip : public IIterator<T>
    {
        typename IIterator<T>::Ptr _iter;
        int _current;
        int _count;
        bool _done;

      public:
        Skip(typename IIterator<T>::Ptr iter, int count) 
            : _iter(iter)
            , _count(count)
            , _current(0)
            , _done(false)
        {
        }

        T* next() override
        {
            while (auto item = _iter->next())
            {
                if (_current < _count)
                    _current++;
                else
                    return item;
            }

            return nullptr;
        }
    };

    template <typename T>
    class SkipWhile : public IIterator<T>
    {
        typename IIterator<T>::Ptr _iter;
        std::function<bool(const T&)> _pred;
        bool _done;

      public:
        SkipWhile(typename IIterator<T>::Ptr iter, std::function<bool(const T&)> pred) 
            : _iter(iter)
            , _pred(pred)
            , _done(false)
        {
        }

        T* next() override
        {
            while (auto item = _iter->next())
            {
                if (!_done)
                {
                    if (_pred(*item))
                    {
                        continue;
                    }
                    else
                    {
                        _done = true;
                        return item;
                    }
                }
                else
                {
                    return item;
                }
            }

            return nullptr;
        }
    };


    template <typename T>
    class Filter : public IIterator<T>
    {
        typename IIterator<T>::Ptr _iter;
        std::function<bool(const T&)> _predicate;

      public:
        Filter(typename IIterator<T>::Ptr iter, std::function<bool(const T&)>  predicate)
            : _iter(iter)
            , _predicate(predicate)
        {
        }

        T* next() override
        {
            while(auto item = _iter->next())
                if (_predicate(*item))
                    return item;

            return nullptr;
        }
    };

    template <typename Tin, typename Tout>
    class Map : public IIterator<Tout>
    {
        typename IIterator<Tin>::Ptr _iter;
        Tout _result;
        std::function<Tout(const Tin&)> _fun;

      public:
        Map(typename IIterator<Tin>::Ptr iter, std::function<Tout(const Tin&)> fun)
            : _iter(iter)
            , _fun(fun)
        {
        }

        Tout* next() override
        {
            if (auto item = _iter->next())
            {
                _result = _fun(*item);
                return &_result;
            }

            return nullptr;
        }
    };

    template <typename Tin, typename Tout>
    class FilterMap : public IIterator<Tout>
    {
        typename IIterator<Tin>::Ptr _iter;
        Tout _result;
        std::function<std::optional<Tout>(const Tin&)> _fun;

      public:
        FilterMap(typename IIterator<Tin>::Ptr iter, std::function<std::optional<Tout>(const Tin&)> fun)
            : _iter(iter)
            , _fun(fun)
        {
        }

        Tout* next() override
        {
            while (auto item = _iter->next())
            {
                if (auto res = _fun(*item))
                {
                    _result = *res;
                    return &_result;
                }
            }

            return nullptr;
        }
    };
    template <typename Tfirst, typename Tsecond>
    class Zip : public IIterator<std::pair<Tfirst, Tsecond> >
    {
        typename IIterator<Tfirst>::Ptr _iter1;
        typename IIterator<Tsecond>::Ptr _iter2;

        using Pair = std::pair<Tfirst, Tsecond>;
        Pair _result;

      public:
        Zip(typename IIterator<Tfirst>::Ptr iter1, typename IIterator<Tsecond>::Ptr iter2)
            : _iter1(iter1)
            , _iter2(iter2)
        {
        }

        std::pair<Tfirst, Tsecond>* next() override
        { 
            if (auto item1 = _iter1->next())
            if (auto item2 = _iter2->next())
            {
                _result = std::make_pair(*item1, *item2);
                return &_result;
            }

            return nullptr;
        }
    };

    template <typename T>
    class Chain : public IIterator<T>
    {
        typename IIterator<T>::Ptr _iter1;
        typename IIterator<T>::Ptr _iter2;
        bool _consumedFirst;

      public:
        Chain(typename IIterator<T>::Ptr iter1, typename IIterator<T>::Ptr iter2)
            : _iter1(iter1)
            , _iter2(iter2)
            , _consumedFirst(false)
        {
        }

        T* next() override
        { 
            if (!_consumedFirst)
            {
                if (auto item1 = _iter1->next())
                    return item1;
                else
                    _consumedFirst = true;
            }

            if (_consumedFirst)
            {
                if (auto item2 = _iter2->next())
                    return item2;
            }

            return nullptr;
        }
    };
} // fun
