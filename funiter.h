#include <iterator>
#include <optional>
#include <functional>
#include <iostream>


namespace fun
{
    template <typename Type, template <typename, typename...> class Container, typename... Args>
    class Take;

    template <typename Type, template <typename, typename...> class Container, typename... Args>
    class Filter;

    template <typename Type, template <typename, typename...> class Container, typename... Args>
    class Cloned;

    template <typename InType, typename OutType, template <typename, typename...> class Container, typename... Args>
    class Map;
 
    /*
    template <typename Type1, typename Type2, template <typename, typename...> class Container, typename... Args>
    class Zip;
    */

    template <typename Type, template <typename, typename...> class Container, typename... Args>
    class IIterator
    {
        public:
            virtual std::optional<Type*> next() = 0;

            auto take(int count)
            {
                return Take<Type, Container, Args...>(*this, count);
            }

            auto filter(std::function<bool(const Type&)> predicate)
            {
                return Filter<Type, Container, Args...>(*this, predicate);
            }

            auto cloned()
            {
                return Cloned<Type, Container, Args...>(*this);
            }

            template <typename OutType>
            auto map(std::function<OutType(const Type&)> function)
            {
                return Map<Type, OutType, Container, Args...>(*this, function);
            }

            /*
            template <typename OtherType>
            IIterator<std::pair<Type, OtherType>, Container, Args...> zip(IIterator<OtherType, Container, Args...>& other)
            {
                return Zip<Type, OtherType, Container, Args...>(*this, other);
            }
            */

            auto collect()
            {
                Container<Type, Args...> cont;
                
                while (auto item = next())
                    cont.push_back(**item);

                return cont;
            }

            template <template <typename, typename...> class OutContainer, typename... OutArgs>
            auto convert()
            {
                OutContainer<Type, Args...> cont;

                while (auto item = next())
                    cont.push_back(**item);

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

    template <typename Type, template <typename, typename...> class Container, typename... Args>
    class Iter : public IIterator<Type, Container, Args...>
    {
        protected:
            typename Container<Type, Args...>::iterator _begin;
            typename Container<Type, Args...>::iterator _end;

        public:
            Iter(Iter& other) = default;

            Iter(Container<Type, Args...>& cont)
                : _begin(std::begin(cont))
                , _end(std::end(cont))
            {
            }

            std::optional<Type*> next() override
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

    template <typename Type, template <typename, typename...> class Container, typename... Args>
    class Take : public IIterator<Type, Container, Args...>
    {
        IIterator<Type, Container, Args...>& _iter;
        int _count;

      public:
        Take(IIterator<Type, Container, Args...>& iter, int count) 
            : _iter(iter)
            , _count(count)
        {
        }

        std::optional<Type*> next() override
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

    template <typename Type, template <typename, typename...> class Container, typename... Args>
    class Filter : public IIterator<Type, Container, Args...>
    {
        IIterator<Type, Container, Args...>& _iter;
        std::function<bool(const Type&)> _predicate;

      public:
        Filter(IIterator<Type, Container, Args...>& iter, std::function<bool(const Type&)>  predicate)
            : _iter(iter)
            , _predicate(predicate)
        {
        }

        std::optional<Type*> next() override
        {
            while(auto item = _iter.next())
                if (_predicate(**item))
                    return *item; 

            return {};
        }
    };

    template <typename Type, template <typename, typename...> class Container, typename... Args>
    class Cloned : public IIterator<Type, Container, Args...>
    {
        IIterator<Type, Container, Args...>& _iter;
        Type _current;

      public:
        Cloned(IIterator<Type, Container, Args...>& iter)
            : _iter(iter)
        {
        }

        std::optional<Type*> next() override
        {
            if (auto item = _iter.next())
            {
                _current = **item;
                return &_current;
            }

            return {};
        }
    };

    template <typename InType, typename OutType, template <typename, typename...> class Container, typename... Args>
    class Map : public IIterator<OutType, Container, Args...>
    {
        IIterator<InType, Container, Args...>& _iter;
        OutType _result;
        std::function<OutType(const InType&)> _fun;

      public:
        Map(IIterator<InType, Container, Args...>& iter, std::function<OutType(const InType&)> fun)
            : _iter(iter)
            , _fun(fun)
        {
        }

        std::optional<OutType*> next() override
        {
            if (auto item = _iter.next())
            {
                _result = _fun(**item);
                return &_result;
            }

            return {};
        }
    };

    /*
    template <typename Type, typename OtherType, template <typename, typename...> class Container, ...Args>
    class Zip : public IIterator<std::pair<Type, OtherType>, Container, Args...>
    {
        IIterator<Type, Container, Args...>& _iter1;
        IIterator<OtherType, Container, OtherArgs...>& _iter2;

        std::pair<Type, OtherType> _result;

      public:
        Zip(IIterator<Type, Container, Args...>& iter1, IIterator<OtherType, Container, Args...>& iter2)
            : _iter1(iter1)
            , _iter2(iter2)
        {
        }

        std::optional<std::pair<Type, OtherType>*> next() override
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
    */
} // fun
