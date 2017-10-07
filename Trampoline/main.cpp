#include "trampoline.hpp"
#include <iomanip>
#include <iostream>

#if defined(linux) || defined(__linux) || defined(__linux__) || defined(Macintosh) || defined(macintosh) || (defined(__APPLE__) && defined(__MACH__))
    #define COLOR_SUPPORT
#endif

void run_test(const std::string& header, bool (*test)()) {
#ifdef COLOR_SUPPORT
    static const std::string PASSED = "[\e[32m OK\e[0m ]";
    static const std::string FAILED = "[\e[31m FAILED\e[0m ]";
#else
    static const std::string PASSED = "[ OK ]";
    static const std::string FAILED = "[ FAILED ]";
#endif
    static int test_count = 0;
    std::cout << "Test " << ++test_count << ": " << header << " is running... ";
    std::cout.flush();

    bool result = false;
    try {
        result = test();
    } catch (...) {
        std::cerr << "Excepton caught!" << std::endl;
    }

    int full_header_len = 22 + (int)std::to_string(test_count).length() + (int)header.length() + ((result) ? 6:4);
    int shift = 75 - full_header_len; if(shift < 0) shift = 1;

    std::cout << std::setw(shift) << ((result) ? PASSED : FAILED) << std::endl;
}

bool test_base() {
    const int number = 42;
    Trampoline <int (int)> simple (
        [&] (int a) {
            return number + a;
        }
    );

    auto f = simple.get ();
    return (f(300) == (number + 300));
}

bool test_constructors() {
    bool result = true;

    Trampoline<int(int)> trump([](int a) { return a + 3;});
    Trampoline<int(int)> trump_1(std::move(trump));
    Trampoline<int(int)> trump_copy = std::move(trump);

    result = result & (trump.get()(3) == trump_1.get()(3));
    result = result & (trump.get()(3) == trump_copy.get()(3));
    return result;
}

bool test_less_six_args() {
    Trampoline<float (int, double, int, float, float)> t (
        [&] (int p0, double p1, int p2, float p3, float p4) {
            return (p1 + p2 + p3 + p4 + p0);
        }
    );
    return (5.4f == t.get()(1, 1.4, 1, 1.f, 1.f));
}

bool test_more_six_args() {
    bool result = true;

    {
        Trampoline <int (int, int, int, int, int, int, int, int)> eight (
            [&] (int a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7) {
                return (a0 + a1 + a2 + a3) + (a7 + a6 + a5 + a4);
            }
        );
        result = result & (8 == eight.get()(1, 1, 1, 1, 1, 1, 1, 1));
    }
    
    {
        Trampoline<long long (double&, int&, float&, int, int, double, double, float&)> t (
            [&] (double p0, int p1, float p2, int p3, int p4, double p5, double p6, float p7) {
                return p1 + p2 + p3 + p4 + p0 + p5 + p6 + p7;
            }
        );

        auto p = t.get();

        double a = 2;
        int    b = 1;
        float  c = 2.f, 
               d = 1.f;
        result = result & (10 == p(a, b, c, 1, 1, 1, 1, d));
    }    

    return result;
}

int main (/*int argc, char* args[]*/) {
    run_test("Base test", test_base);
    run_test("Constructors test", test_constructors);
    run_test("Less than six arguments", test_less_six_args);
    run_test("More than six arguments", test_more_six_args);
    return EXIT_SUCCESS;
}