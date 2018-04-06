#include <iostream>
#include <iterator>
#include <optional>
#include <vector>
#include <functional>

namespace fun
{
    template<typename It, typename Fun>
    class Map;

    template <typename It>
    class Iterator
    {
      protected:
        It& _begin;
        It& _end;
      public:
        Iterator(It& begin, It& end)
            : _begin(begin)
            , _end(end)
        {
        }

        Iterator(Iterator& other) = default;

        virtual std::optional<typename It::value_type> next()
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

        template <typename Fun>
        Map<It, Fun> map(Fun fun)
        {
            return Map<It, Fun>(*this, fun);
        }
    };

    template <typename It, typename Fun>
    class Map : public Iterator<It>
    {
        using super = Iterator<It>;
        Fun _f;

      public:
          Map(It& begin, It& end, Fun f)
              : Iterator<It>(begin, end)
              , _f(f)
          {
          }

          Map(Iterator<It>& it, Fun f)
              : Iterator<It>(it)
              , _f(f)
          {
          }

          std::optional<typename Fun::result_type> next() override
          {
              if (auto n = super::next())
              {
                  return _f(*n);
              }
              else
              {
                  return {};
              }
          }
    };
}


int main(int argc, char** argv)
{
    std::vector<int> a{ 2, 1, 2, 3 };
    auto b = a.begin();
    auto e = a.end();

    fun::Iterator it(b, e);

    auto map = it.map(std::function([](int a) -> int { return a*a; }));

    while (auto x = map.next())
        std::cout << *x << " ";

    std::cout << std::endl;

    return 0;
}
