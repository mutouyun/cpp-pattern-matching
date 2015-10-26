# A C++ Pattern Matching Library

[![Build Status](https://travis-ci.org/mutouyun/cpp-pattern-matching.svg?branch=master)](https://travis-ci.org/mutouyun/cpp-pattern-matching)
[![Build status](https://ci.appveyor.com/api/projects/status/3la9m4ddps5uj3ls/branch/master?svg=true)](https://ci.appveyor.com/project/mutouyun/cpp-pattern-matching)

ML-style pattern matching in C++.
# Compiler Support
 - MSVC-2015  
 - g++-4.9.2(-std=c++1y)  
 - clang-3.4(-std=c++1y)
 
Yuriy Solodkyy, Gabriel Dos Reis, Bjarne Stroustrup had written a paper:  
http://www.stroustrup.com/OpenPatternMatching.pdf  
And we can find their codes here:  
http://parasol.tamu.edu/mach7/  
 
Their library is very powerful and big, maybe a little complicated.  
Compared with their library, mine is very small and simple.  
 
If you just want to use a pattern matching in C++ quickly,   
and don't like to read doc & study usage, maybe you could try my library.
# License
Codes covered by the MIT License.
# Tutorial
For using it, you only need to include match.hpp.  
Some examples:
```cpp
/*
 * Constant pattern
*/
int a = 100;
Match(a)
{
    Case(10)    std::cout << 1 << std::endl;
    Case(100)   std::cout << 2 << std::endl;
    Case(1000)  std::cout << 3 << std::endl;
}
EndMatch

/*
 * Variable & wildcard pattern
*/
std::function<int(int)> fac = [&fac](int n)
{
    unsigned short m;
    Match(n)
    {
        Case(0) return 1;               // Constant pattern
        Case(m) return m * fac(m - 1);  // Variable pattern
        Case(_) return -1;              // Wildcard pattern
    }
    EndMatch
};
std::cout << fac(10) << " " << fac(-10) << std::endl;

/*
 * Predicate pattern, for lambda-expressions or other callable objects.
*/
Match(5)
{
    Case([](int x) { return (x & 1); })
        std::cout << "odd" << std::endl;
    Otherwise()
        std::cout << "even" << std::endl;
}
EndMatch

/*
 * Type pattern
*/
class Foo
{
public:
    virtual ~Foo(void) {}
};

template <int N>
class Bar : public Foo
{
};

Foo* foo = new Bar<2>;
Match(foo)
{
    Case(Type(Bar<1>)) std::cout << "Bar<1>" << std::endl;
    Case(Type(Bar<2>)) std::cout << "Bar<2>" << std::endl;
    Case(Type(Bar<3>)) std::cout << "Bar<3>" << std::endl;
    Otherwise()        std::cout << "Otherwise..." << std::endl;
}
EndMatch

/*
 * Constructor pattern
*/
struct xx_t
{
    int    a_ = 2;
    double b_ = 1.0;
    xx_t*  c_ = nullptr;
    Foo    foo_;
};
MATCH_REGIST_TYPE(xx_t, int, double, xx_t*, Foo)

int a;
Match(xx)
{
    Case(C<xx_t>(_, 2.0, nullptr, _)) std::cout << "(_, 2.0, nullptr, _)" << std::endl;
    Case(C<xx_t>(a, _  , nullptr, _)) std::cout << "(a, _, nullptr, _): a = " << a << std::endl;
}
EndMatch

/*
 * Sequence pattern
*/
#include <list>
std::list<int> ll = { 1, 2, 3 };
Match(ll)
{
    Case(S(1, _, _)) std::cout << "{ 1, 2, 3 } matchs: " << "(1, _, _)" << std::endl;
    Case(S(_, 1, _)) std::cout << "{ 1, 2, 3 } matchs: " << "(_, 1, _)" << std::endl;
    Case(S(_, _, 1)) std::cout << "{ 1, 2, 3 } matchs: " << "(_, _, 1)" << std::endl;
}
EndMatch

/*
 * You can match more than one pattern at the same time.
*/
int a = 3, b = 321;
Match(a, b)
{
    Case(3, 123) std::cout << 1 << std::endl;
    Case(1, 2)   std::cout << 2 << std::endl;
    Case(3, 321) std::cout << 3 << std::endl;
    Otherwise()  std::cout << "Otherwise..." << std::endl;
}
EndMatch

/*
 * You could define your own converter for some special case to cooperate with a custom pattern.
 * The converter will work when the case argument accords with it.
 * If the argument accords more than one converter, you would get an ambiguity compile error.
*/
namespace match
{
    template <typename T>
    inline auto converter(T&& arg)
        -> typename std::enable_if<std::is_same<typename std::decay<T>::type, const char*>::value, T&&>::type
    {
        std::cout << "My converter... " << arg << std::endl;
        return std::forward<T>(arg);
    }
}
#include "match.hpp"
```
