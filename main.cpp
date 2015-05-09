#include "match.hpp"

#include <iostream>
#include <string>
#include <functional>
#include <type_traits>

#define TEST_CASE_()                                           \
    std::cout << std::endl << __func__ << " ->:" << std::endl; \
    using namespace match

void test_constant_variable(void)
{
    TEST_CASE_();

    int x = 100;
    Match(x)
    {
        Case(10)    std::cout << 1 << std::endl;
        Case(100)   std::cout << 2 << std::endl;
        Case(1000)  std::cout << 3 << std::endl;
    }
    EndMatch

    int a = 3, b = 321;
    Match(a, b)
    {
        Case(3, 123) std::cout << 1 << std::endl;
        Case(1, 2)   std::cout << 2 << std::endl;
        Case(3, 321) std::cout << 3 << std::endl;
        Otherwise()  std::cout << "Otherwise..." << std::endl;
    }
    EndMatch

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
}

void test_predicate(void)
{
    TEST_CASE_();

    Match(5)
    {
        Case([](int x) { return (x & 1); })
            std::cout << "odd" << std::endl;
        Otherwise()
            std::cout << "even" << std::endl;
    }
    EndMatch
}

class Foo
{
public:
    Foo(void) = default;
    Foo(const Foo&)
    {
        static int counter = 0;
        std::cout << "Foo -- Copy... " << ++counter << std::endl;
    }
    virtual ~Foo(void) {}
};

template <int N>
class Bar : public Foo
{
};

void test_type(void)
{
    TEST_CASE_();

    Foo* foo = new Bar<2>;
    Match(foo)
    {
        Case(Type(Bar<1>)) std::cout << "Bar<1>" << std::endl;
        Case(Type(Bar<2>)) std::cout << "Bar<2>" << std::endl;
        Case(Type(Bar<3>)) std::cout << "Bar<3>" << std::endl;
        Otherwise()        std::cout << "Otherwise..." << std::endl;
    }
    EndMatch
}

struct xx_t
{
    int    a_ = 2;
    double b_ = 1.0;
    xx_t*  c_ = nullptr;
    Foo    foo_;
};
MATCH_REGIST_TYPE(xx_t, int, double, xx_t*, Foo)

void test_constructor(void)
{
    TEST_CASE_();

    xx_t xx;

    auto  x0 = bindings<xx_t>::layout_t::get<0>(xx);
    auto  x1 = bindings<xx_t>::layout_t::get<1>(xx);
    auto  x2 = bindings<xx_t>::layout_t::get<2>(xx);
    auto& x3 = bindings<xx_t>::layout_t::get<3>(xx);

    int a;
    Match(xx)
    {
        Case(Cons<xx_t>(_, 2.0, nullptr, _)) std::cout << "(_, 2.0, nullptr, _)" << std::endl;
        Case(Cons<xx_t>(a, _  , nullptr, _)) std::cout << "(a, _, nullptr, _): a = " << a << std::endl;
    }
    EndMatch
}

int main(void)
{
    test_constant_variable();
    test_predicate();
    test_type();
    test_constructor();
    std::cout << std::endl;
	return 0;
}
