#include <iterator>
#include <optional>
#include <functional>
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <experimental/filesystem>

namespace ri
{
    namespace fs = std::experimental::filesystem;

    /// Iterators

    // Creates Iterator over stl Container
    template <typename Container>
    class Iter;

    // Creates Iterator over input file
    class LinesInFile;

    // Creates a generator
    template <typename T>
    class Generator;

    // Empty Iterator
    template <typename T>
    class Empty;

    // Produces value once
    template <typename T>
    class Once;

    // Repeats value forever
    template <typename T>
    class Repeat;
    /// Adapters

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
    class Scan;

    template <typename Tin, typename Tout>
    class FlatMap;

    template <typename T>
    class Inspect;

    template <typename Tin, typename Tout>
    class FilterMap;
 
    template <typename Tfirst, typename Tsecond>
    class Zip;

    template <typename T>
    class Chain;

    template <typename T>
    class Cycle;

    template <typename T>
    class Fuse;

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
    auto empty()
    {
        return std::make_shared<Empty<T>>();
    }

    template <typename T>
    auto once(const T& value)
    {
        return std::make_shared<Once<T>>(value);
    }

    template <typename T>
    auto repeat(const T& value)
    {
        return std::make_shared<Repeat<T>>(value);
    }

    auto lines(const fs::path& path)
    {
        return std::make_shared<LinesInFile>(path);
    }

    template <typename T>
    class IIterator : public std::enable_shared_from_this<IIterator<T>>
    {
        public:
            using Ptr = std::shared_ptr<IIterator>;

            virtual T* next() = 0;
            virtual IIterator<T>::Ptr clone() = 0;
            virtual ~IIterator(){};

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

            auto inspect(std::function<void(const T&)> function)
            {
                return std::make_shared<Inspect<T>>(this->shared_from_this(), function);
            }

            template <typename Tout>
            auto filter_map(std::function<std::optional<Tout>(const T&)> function)
            {
                return std::make_shared<FilterMap<T, Tout>>(this->shared_from_this(), function);
            }
            
            template <typename Tout>
            auto flat_map(std::function<typename IIterator<Tout>::Ptr(const T&)> function)
            {
                return std::make_shared<FlatMap<T, Tout>>(this->shared_from_this(), function);
            }

            template <typename Tout>
            auto scan(const Tout& init,
                   std::function<Tout(const Tout&, const T&)> function)
            {
                return std::make_shared<Scan<T,Tout>>(this->shared_from_this(), init, function);
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

            auto cycle()
            {
                return std::make_shared<Cycle<T>>(this->shared_from_this());
            }
            

            //TODO: fold
            //TODO: scan
            //TODO: rev
            //TODO: unzip

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

            auto fuse()
            {
                return std::make_shared<Fuse<T>>(this->shared_from_this());
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

            template <template <typename, typename...> class Container, typename... Args>
            auto partition(std::function<bool (const T&)> pred)
            {
                Container<T, Args...> contTrue;
                Container<T, Args...> contFalse;
                
                while (auto item = next())
                {
                    if (pred(*item))
                        contTrue.insert(std::end(contTrue), *item);
                    else
                        contFalse.insert(std::end(contFalse), *item);
                }

                return std::make_pair(contTrue, contFalse);
            }

            template <typename OutContainer>
            auto partition(std::function<bool (const T&)> pred)
            {
                OutContainer contTrue;
                OutContainer contFalse;
                
                while (auto item = next())
                {
                    if (pred(*item))
                        contTrue.insert(std::end(contTrue), *item);
                    else
                        contFalse.insert(std::end(contFalse), *item);
                }

                return std::make_pair(contTrue, contFalse);
            }

            bool all(std::function<bool(const T&)> predicate)
            {
                while (auto item = next())
                    if (!predicate(*item))
                        return false;

                return true;
            }

            bool any(std::function<bool(const T&)> predicate)
            {
                while (auto item = next())
                    if (predicate(*item))
                        return true;

                return false;
            }

            T* find(std::function<bool(const T&)> predicate)
            {
                while (auto item = next())
                    if (predicate(*item))
                        return item;

                return nullptr;
            }

            std::optional<size_t> position(std::function<bool(const T&)> predicate)
            {
                size_t count = 0;

                while (auto item = next())
                    if (predicate(*item))
                        return count;
                    else
                        ++count;

                return {};
            }

            std::optional<T> max()
            {
                return max_by([](auto& a, auto& b) { return a < b; });
            }
            
            std::optional<T> max_by(std::function<bool(const T& a, const T& b)> cmp)
            {
                std::optional<T> max;

                while (auto item = next())
                {
                    if (!max)
                        max = *item;
                    else if (!cmp(*item, *max))
                        max = *item;
                }

                return max;
            }

            std::optional<T> min()
            {
                return min_by([](auto& a, auto& b) { return a < b; });
            }

            std::optional<T> min_by(std::function<bool(const T& a, const T& b)> cmp)
            {
                std::optional<T> min;

                while (auto item = next())
                {
                    if (!min)
                        min = *item;
                    else if (cmp(*item, *min))
                        min = *item;
                }

                return min;
            }

            void for_each(std::function<void(const T&)> fun)
            {
                while (auto item = next())
                    fun(*item);
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

            template <typename Tout>
            auto fold(const Tout& init,
                    std::function<Tout(const T& init, const T& current)> function)
            {
                Tout res = init;

                while (auto item = next())
                    res = function(res, *item);

                return res;
            }

            bool eq(IIterator<T>::Ptr other)
            {
                T* fst = nullptr;
                T* snd = nullptr;

                while (true)
                {
                    fst = next();
                    snd = other->next();

                    if (!fst && snd)
                        return false;
                    else if (fst && !snd)
                        return false;
                    else if (!fst && !snd)
                        return true;
                    else if (*fst != *snd)
                        return false;
                    else
                        continue;
                }
            }
            
            bool ne(IIterator<T>::Ptr other)
            {
                T* fst = nullptr;
                T* snd = nullptr;

                while (true)
                {
                    fst = next();
                    snd = other->next();

                    if (!fst && snd)
                        return true;
                    else if (fst && !snd)
                        return true;
                    else if (!fst && !snd)
                        return true;
                    else if (*fst == *snd)
                        return false;
                    else
                        continue;
                }
            }
    };

    template <typename Container>
    class Iter : public IIterator<typename Container::value_type>
    {
        protected:
            typename Container::iterator _begin;
            typename Container::iterator _end;

        public:
            Iter(const Iter& other) = default;
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

            typename IIterator<typename Container::value_type>::Ptr clone() override
            {
                return std::make_shared<Iter<Container>>(*this);
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
            Generator(const Generator& other) = default;
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
            
            typename IIterator<T>::Ptr clone() override
            {
                return std::make_shared<Generator<T>>(*this);
            }

    };

    template <typename T>
    class Empty : public IIterator<T>
    {
        public:
            Empty(const Empty& other) = default;
            Empty()
            {
            }

            T* next() override
            {
                return nullptr;
            }

            typename IIterator<T>::Ptr clone() override
            {
                return std::make_shared<Empty<T>>(*this);
            }
    };

    template <typename T>
    class Once : public IIterator<T>
    {
        T _value;
        bool _emitted;

        public:
            Once(const Once& other) = default;
            Once(const T& value)
                : _value(value)
                , _emitted(false)
            {
            }

            T* next() override
            {
                if (!_emitted)
                {
                    _emitted = true;
                    return &_value;
                }
                else
                {
                    return nullptr;
                }
            }

            typename IIterator<T>::Ptr clone() override
            {
                return std::make_shared<Once<T>>(*this);
            }
    };

    template <typename T>
    class Repeat : public IIterator<T>
    {
        T _value;

        public:
            Repeat(const Repeat& other) = default;
            Repeat(const T& value)
                : _value(value)
            {
            }

            T* next() override
            {
                return &_value;
            }

            typename IIterator<T>::Ptr clone() override
            {
                return std::make_shared<Repeat<T>>(*this);
            }
    };

    class LinesInFile : public IIterator<std::string>
    {
        fs::path _path;
        std::ifstream _file;
        std::string _currentLine;

      public:
        LinesInFile(const LinesInFile& other)
            : _path(other._path)
            , _file(_path)
        {
        }

        LinesInFile(const fs::path& path)
            : _path(path)
            , _file(_path)
        {
        }

        std::string* next() override
        {
            if (!_file.is_open())
                return nullptr;

            if (std::getline(_file, _currentLine))
                return &_currentLine;
            else
                return nullptr;
        }

        typename IIterator<std::string>::Ptr clone() override
        {
            return std::make_shared<LinesInFile>(*this);
        }
    };

    template <typename T>
    class Take : public IIterator<T>
    {
        typename IIterator<T>::Ptr _iter;
        int _count;

      public:
        Take(const Take& other) = default;
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

        typename IIterator<T>::Ptr clone() override
        {
            return std::make_shared<Take<T>>(*this);
        }
    };

    template <typename T>
    class TakeWhile : public IIterator<T>
    {
        typename IIterator<T>::Ptr _iter;
        std::function<bool(const T&)> _pred;
        bool _done;

      public:
        TakeWhile(const TakeWhile& other) = default;
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

        typename IIterator<T>::Ptr clone() override
        {
            return std::make_shared<TakeWhile<T>>(*this);
        }
    };

    template <typename T>
    class Skip : public IIterator<T>
    {
        typename IIterator<T>::Ptr _iter;
        int _current;
        int _count;

      public:
        Skip(const Skip& other) = default;
        Skip(typename IIterator<T>::Ptr iter, int count) 
            : _iter(iter)
            , _current(0)
            , _count(count)
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

        typename IIterator<T>::Ptr clone() override
        {
            return std::make_shared<Skip<T>>(*this);
        }
    };

    template <typename T>
    class SkipWhile : public IIterator<T>
    {
        typename IIterator<T>::Ptr _iter;
        std::function<bool(const T&)> _pred;
        bool _done;

      public:
        SkipWhile(const SkipWhile& other) = default;
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

        typename IIterator<T>::Ptr clone() override
        {
            return std::make_shared<SkipWhile<T>>(*this);
        }
    };

    template <typename T>
    class Filter : public IIterator<T>
    {
        typename IIterator<T>::Ptr _iter;
        std::function<bool(const T&)> _predicate;

      public:
        Filter(const Filter& other) = default;
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

        typename IIterator<T>::Ptr clone() override
        {
            return std::make_shared<Filter<T>>(*this);
        }
    };

    template <typename Tin, typename Tout>
    class Map : public IIterator<Tout>
    {
        typename IIterator<Tin>::Ptr _iter;
        Tout _result;
        std::function<Tout(const Tin&)> _fun;

      public:
        Map(const Map& other) = default;
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

        typename IIterator<Tout>::Ptr clone() override
        {
            return std::make_shared<Map<Tin,Tout>>(*this);
        }
    };

    template <typename T>
    class Inspect : public IIterator<T>
    {
        typename IIterator<T>::Ptr _iter;
        std::function<void(const T&)> _fun;

      public:
        Inspect(const Inspect& other) = default;
        Inspect(typename IIterator<T>::Ptr iter, std::function<void(const T&)> fun)
            : _iter(iter)
            , _fun(fun)
        {
        }

        T* next() override
        {
            if (auto item = _iter->next())
            {
                _fun(*item);
                return item;
            }

            return nullptr;
        }

        typename IIterator<T>::Ptr clone() override
        {
            return std::make_shared<Inspect<T>>(*this);
        }
    };

    template <typename Tin, typename Tout>
    class FilterMap : public IIterator<Tout>
    {
        typename IIterator<Tin>::Ptr _iter;
        Tout _result;
        std::function<std::optional<Tout>(const Tin&)> _fun;

      public:
        FilterMap(const FilterMap& other) = default;
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

        typename IIterator<Tout>::Ptr clone() override
        {
            return std::make_shared<FilterMap<Tin,Tout>>(*this);
        }
    };

    template <typename Tin, typename Tout>
    class FlatMap : public IIterator<Tout>
    {
        typename IIterator<Tin>::Ptr _iter;
        Tout _result;
        std::function<typename IIterator<Tout>::Ptr (const Tin&)> _fun;
        typename IIterator<Tout>::Ptr _iterOut;

      public:
        FlatMap(const FlatMap& other) = default;
        FlatMap(typename IIterator<Tin>::Ptr iter,
                std::function<typename IIterator<Tout>::Ptr(const Tin&)> fun)
            : _iter(iter)
            , _fun(fun)
            , _iterOut(empty<Tout>())
        {
        }

        Tout* next() override
        {
            if (auto item = _iterOut->next())
            {
                return item;
            }
            else
            {
                if (auto in = _iter->next())
                {
                    _iterOut = _fun(*in);
                    return next();
                }
                else
                {
                    return nullptr;
                }
            }
        }

        typename IIterator<Tout>::Ptr clone() override
        {
            return std::make_shared<FlatMap<Tin,Tout>>(*this);
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
        Zip(const Zip& other) = default;
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

        typename IIterator<std::pair<Tfirst, Tsecond>>::Ptr clone() override
        {
            return std::make_shared<Zip<Tfirst, Tsecond>>(*this);
        }
    };

    template <typename T>
    class Chain : public IIterator<T>
    {
        typename IIterator<T>::Ptr _iter1;
        typename IIterator<T>::Ptr _iter2;
        bool _consumedFirst;

      public:
        Chain(const Chain& other) = default;
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

        typename IIterator<T>::Ptr clone() override
        {
            return std::make_shared<Chain<T>>(*this);
        }
    };

    template <typename T>
    class Cycle : public IIterator<T>
    {
        typename IIterator<T>::Ptr _iterOrig;
        typename IIterator<T>::Ptr _iter;

      public:
        Cycle(const Cycle& other) = default;
        Cycle(typename IIterator<T>::Ptr iter)
            : _iterOrig(iter)
            , _iter(iter->clone())
        {
        }

        T* next() override
        { 
            if (auto item = _iter->next())
            {
                return item;
            }
            else
            {
                _iter = _iterOrig->clone();
                return next();
            }
        }

        typename IIterator<T>::Ptr clone() override
        {
            return std::make_shared<Cycle<T>>(*this);
        }
    };

    template <typename T>
    class Fuse : public IIterator<T>
    {
        typename IIterator<T>::Ptr _iter;
        bool _done;

      public:
        Fuse(const Fuse& other) = default;
        Fuse(typename IIterator<T>::Ptr iter)
            : _iter(iter)
            , _done(false)
        {
        }

        T* next() override
        { 
            if (!_done)
            {
                if (auto item = _iter->next())
                {
                    return item;
                }
                else
                {
                    _done = true;
                    return nullptr;
                }
            }
            else
            {
                return nullptr;
            }
        }

        typename IIterator<T>::Ptr clone() override
        {
            return std::make_shared<Fuse<T>>(*this);
        }
    };

    template <typename Tin, typename Tout>
    class Scan : public IIterator<Tout>
    {
        typename IIterator<Tin>::Ptr _iter;
        Tout _result;
        std::function<Tout(const Tout&, const Tin&)> _fun;

      public:
        Scan(const Scan& other) = default;
        Scan(typename IIterator<Tin>::Ptr iter,
                const Tout& init,
                std::function<Tout(const Tout&, const Tin&)> fun)
            : _iter(iter)
            , _result(init)
            , _fun(fun)
        {
        }

        Tout* next() override
        {
            if (auto item = _iter->next())
            {
                _result = _fun(_result, *item);
                return &_result;
            }

            return nullptr;
        }

        typename IIterator<Tout>::Ptr clone() override
        {
            return std::make_shared<Scan<Tin,Tout>>(*this);
        }
    };

} // ri namespace
