# match-case
ML-style pattern matching in C++.  
The code has been compiled in MSVC-2005-CTP & g++-4.9.1(-std=c++1y).
 
For using it, you only need to include match.hpp.  
Some examples:
```cpp
/*
 * For matching more than one condition at the same time.
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

Match()
{
    Case(a == 3, b == 123)    std::cout << 1 << std::endl;
    Case(c == TEST_COUNT * 3) std::cout << 2 << std::endl;
    Otherwise()               std::cout << "Otherwise..." << std::endl;
}
EndMatch

/*
 * For matching a special type. If the target of matching is a polymorphic type, 
 * the matching-case will try to cast the target, and match the right subclass.
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
Match(foo, a)
{
    Case(Type(Bar<1>), Type(short)) std::cout << 1 << std::endl;
    Case(Type(Bar<2>))              std::cout << 2 << std::endl;
    Case(Type(Bar<3>), b)           std::cout << 3 << std::endl;
    Otherwise()                     std::cout << "Otherwise..." << std::endl;
}
EndMatch

/*
 * Regular expression
*/
std::string str = "\\w+(\\.\\w+)*@\\w+(\\.\\w+)+";
Match("memleak@darkc.at")
{
    Case("Hello World") std::cout << 1 << std::endl;
    Case("I Love You")  std::cout << 2 << std::endl;
    Case(Regex(str))    std::cout << 3 << std::endl;
    Otherwise()         std::cout << "Otherwise..." << std::endl;
}
EndMatch

/*
 * Automatically identify the closures, and using it as the matching pattern.
*/
Match(5)
{
    Case([](int x){ return (x & 1); })
        std::cout << "odd" << std::endl;
    Otherwise()
        std::cout << "even" << std::endl;
}
EndMatch

/*
 * You can define your own filter for some special case.
 * The custom filter will work when the case pattern accords with it.
 * If the pattern matches more than one filter, you would get an ambiguity compile error.
*/
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
```
Yuriy Solodkyy, Gabriel Dos Reis, Bjarne Stroustrup had written a paper:  
http://www.stroustrup.com/OpenPatternMatching.pdf  
And we can find their codes here:  
http://parasol.tamu.edu/mach7/  
 
Their library is very powerful and big, maybe a little complicated.  
Compared with their library, mine is very small and simple.  
I wrote only 170 lines of code for mine library.  
 
If you just want to use a pattern matching in C++ quickly, and don't like to read doc & study usage, maybe you could try my library.