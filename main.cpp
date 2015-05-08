#include <iostream>
#include <string>

namespace match
{
    template <typename T>
    inline auto filter(T&& arg)
        -> typename std::enable_if<std::is_same<typename std::decay<T>::type, const char*>::value, T&&>::type
    {
        std::cout << "My filter... " << arg << std::endl;
        return std::forward<T>(arg);
    }
}

#include "match.hpp"
#include "capo/stopwatch.hpp"

template <typename... P>
inline auto match_case(P&&... args)
    -> decltype(std::forward_as_tuple(match::tester(std::forward<P>(args))...))
{
    return std::forward_as_tuple(match::tester(std::forward<P>(args))...);
}

#define Match2(...)                                        \
    {                                                      \
        auto target_ = std::forward_as_tuple(__VA_ARGS__); \
        if (false)

#define Case2(...) \
        } else if (target_ == match_case(__VA_ARGS__)) {

#define Otherwise2() \
        } else {

#define EndMatch2 \
    }

class Foo
{
public:
    virtual ~Foo(void) {}
};

template <int N>
class Bar : public Foo
{
};

int main(void)
{
    capo::stopwatch<> sw;

#define TEST_COUNT /*1*/1000000000u

    int a = 3, b = 321; long long c = 0;
    sw.start();
    for (long long i = 0; i < TEST_COUNT; ++i)
    if (a == 3 && b == 123)
        c += 1;
    else if (a == 1 && b == 2)
        c += 2;
    else if (a == 3 && b == 321)
        c += 3;
    else {}
    std::cout << "if-else: " << sw.elapsed<std::chrono::milliseconds>() << " ms, " << c << std::endl;

    c = 0;
    sw.restart();
    for (long long i = 0; i < TEST_COUNT; ++i)
    Match(a, b)
    {
        Case(3, 123) c += 1;
        Case(1, 2)   c += 2;
        Case(3, 321) c += 3;
        Otherwise()  ;
    }
    EndMatch
    std::cout << "Match: " << sw.elapsed<std::chrono::milliseconds>() << " ms, " << c << std::endl;

    c = 0;
    sw.restart();
    for (long long i = 0; i < TEST_COUNT; ++i)
    Match2(a, b)
    {
        Case2(3, 123) c += 1;
        Case2(1, 2)   c += 2;
        Case2(3, 321) c += 3;
        Otherwise2()  ;
    }
    EndMatch2
    std::cout << "Match2: " << sw.elapsed<std::chrono::milliseconds>() << " ms, " << c << std::endl;

    Match()
    {
        Case(a == 3, b == 123)    std::cout << 1 << std::endl;
        Case(c == TEST_COUNT * 3) std::cout << 2 << std::endl;
        Otherwise()               std::cout << "Otherwise..." << std::endl;
    }
    EndMatch

    Foo* foo = new Bar<2>;
    Match(foo, a)
    {
        Case(Type(Bar<1>), Type(short)) std::cout << 1 << std::endl;
        Case(Type(Bar<2>))              std::cout << 2 << std::endl;
        Case(Type(Bar<3>), b)           std::cout << 3 << std::endl;
        Otherwise()                     std::cout << "Otherwise..." << std::endl;
    }
    EndMatch

    std::string str = "\\w+(\\.\\w+)*@\\w+(\\.\\w+)+";
    Match("memleak@darkc.at")
    {
        Case("Hello World") std::cout << 1 << std::endl;
        Case("I Love You")  std::cout << 2 << std::endl;
        Case(Regex(str))    std::cout << 3 << std::endl;
        Otherwise()         std::cout << "Otherwise..." << std::endl;
    }
    EndMatch

    Match(5)
    {
        Case([](int x){ return (x & 1); })
            std::cout << "odd" << std::endl;
        Otherwise()
            std::cout << "even" << std::endl;
    }
    EndMatch

	return 0;
}
