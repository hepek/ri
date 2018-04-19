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
    auto iter(Container& c)
    {
        return std::make_shared<Iter<Container>>(c);
    }

    template <typename T>
    class IIterator : public std::enable_shared_from_this<IIterator<T>>
    {
        public:
            virtual T* next() = 0;
            virtual ~IIterator(){};

            using Ptr = std::shared_ptr<IIterator>;

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

            template <typename Tother>
            auto zip(typename IIterator<Tother>::Ptr other)
            {
                return std::make_shared<Zip<T, Tother>>(this->shared_from_this(), other);
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
            {
                std::cerr << "Test::next\n";
                if (_predicate(*item))
                {
                    std::cerr << "Emitting";
                    return item;
                }
            }

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
} // fun
