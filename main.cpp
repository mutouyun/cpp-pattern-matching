#include "match.hpp"

#include <iostream>
#include <string>
#include <functional>

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

struct tester
{
    template <typename T>
    constexpr bool operator()(T) const { return std::is_integral<T>::value; }
};

void test_predicate(void)
{
    TEST_CASE_();

    Match(5)
    {
        Case([](auto x) { return (x & 1); })
            std::cout << "odd" << std::endl;
        Otherwise()
            std::cout << "even" << std::endl;
    }
    EndMatch

    Match(5)
    {
        Case(tester{})
            std::cout << "is integral" << std::endl;
        Otherwise()
            std::cout << "not integral" << std::endl;
    }
    EndMatch
}

void test_regex(void)
{
    TEST_CASE_();

    std::string str = "\\w+(\\.\\w+)*@\\w+(\\.\\w+)+";
    Match("memleak@darkc.at")
    {
        Case("Hello World") std::cout << 1 << std::endl;
        Case("I Love You")  std::cout << 2 << std::endl;
        Case(Regex(str))    std::cout << 3 << std::endl;
        Otherwise()         std::cout << "Otherwise..." << std::endl;
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

    Foo& foo = *(new Bar<2>);
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

struct node
{
    std::string v_;
    node* l_;
    node* r_;
    void destroy(void) { if (l_) l_->destroy(); if (r_) r_->destroy(); delete this; }
};
MATCH_REGIST_TYPE(node, std::string, node*, node*)

struct tree
{
    node* root_;
    void destroy(void) { if (root_) root_->destroy(); }
};
MATCH_REGIST_TYPE(tree, node*)

void test_constructor(void)
{
    TEST_CASE_();

    xx_t xx; int a;
    Match(xx)
    {
        Case(C<xx_t>(_, 2.0, nullptr, _)) std::cout << "(_, 2.0, nullptr, _)" << std::endl;
        Case(C<xx_t>(a, _  , nullptr, _)) std::cout << "(a, _, nullptr, _): a = " << a << std::endl;
    }
    EndMatch

    tree tr = { new node{ "root", new node{ "left", nullptr, nullptr }, new node{ "right", nullptr, nullptr } } };
    Match(tr)
    {
        Case( C(C<node*>(_, C<node*>("left", _, _), C<node*>("right", nullptr, nullptr))) )
            std::cout << "bingo" << std::endl;
    }
    EndMatch
    tr.destroy();
}

#include <list>
void test_sequence(void)
{
    TEST_CASE_();

    std::list<int> ll = { 1, 2, 3 };
    Match(ll)
    {
        Case(S(1, _, _)) std::cout << "{ 1, 2, 3 } matchs: " << "(1, _, _)" << std::endl;
        Case(S(_, 1, _)) std::cout << "{ 1, 2, 3 } matchs: " << "(_, 1, _)" << std::endl;
        Case(S(_, _, 1)) std::cout << "{ 1, 2, 3 } matchs: " << "(_, _, 1)" << std::endl;
    }
    EndMatch

    auto il = { 3, 2, 1 };
    Match(il)
    {
        Case(S(1, _, _)) std::cout << "{ 3, 2, 1 } matchs: " << "(1, _, _)" << std::endl;
        Case(S(_, 1, _)) std::cout << "{ 3, 2, 1 } matchs: " << "(_, 1, _)" << std::endl;
        Case(S(_, _, 1)) std::cout << "{ 3, 2, 1 } matchs: " << "(_, _, 1)" << std::endl;
    }
    EndMatch
}

void test_or_and_guard(void)
{
    TEST_CASE_();

    auto detect_zero_or = [](auto x, auto y)
    {
        std::cout << "(" << x << ", " << y << ") ->: ";
        Match(x, y)
        {
            With( P(0, 0) || P(0, _) || P(_, 0) )
                std::cout << "Zero found." << std::endl;
            Otherwise()
                std::cout << "Both nonzero." << std::endl;
        }
        EndMatch
    };
    detect_zero_or(0, 0);
    detect_zero_or(1, 0);
    detect_zero_or(0, 10);
    detect_zero_or(10, 15);

    auto detect_zero_and = [](auto x, auto y)
    {
        std::cout << "(" << x << ", " << y << ") ->: ";
        decltype(x) a; decltype(y) b;
        Match(x, y)
        {
            Case(0, 0)
                std::cout << "Both values zero." << std::endl;
            With( P(a, b) && P(0, _) )
                std::cout << "First value is 0 in (" << a << ", " << b << ")" << std::endl;
            With( P(a, b) && P(_, 0) )
                std::cout << "Second value is 0 in (" << a << ", " << b << ")" << std::endl;
            Otherwise()
                std::cout << "Both nonzero." << std::endl;
        }
        EndMatch
    };
    detect_zero_and(0, 0);
    detect_zero_and(1, 0);
    detect_zero_and(0, 10);
    detect_zero_and(10, 15);

    int a = 3, b = 321, c;
    Match(a, b)
    {
        With(P(3, c) && 3 < c) std::cout << 1 << " -- " << c << std::endl;
        Case  (1, 2)           std::cout << 2 << std::endl;
        With(P(3, c) && 3 > c) std::cout << 3 << " -- " << c << std::endl;
        Otherwise()            std::cout << "Otherwise..." << std::endl;
    }
    EndMatch
}

int main(void)
{
    test_constant_variable();
    test_predicate();
    test_regex();
    test_type();
    test_constructor();
    test_sequence();
    test_or_and_guard();
    std::cout << std::endl;
    return 0;
}
