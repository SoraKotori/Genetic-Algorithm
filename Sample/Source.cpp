#define _USE_MATH_DEFINES
#include "GeneticAlgorithm.h"
#include <cstdlib>
#include <iostream>

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

using namespace std;
using namespace GA;

int main(void)
{
    GeneticAlgorithm<double> MyGA(32, 1024, -10, 10, [](const auto& _Domain)
    {
        auto x = get<0>(_Domain);
        auto y = get<1>(_Domain);

        return -0.0001 * pow(abs(sin(x) * sin(y) * exp(abs(100 - sqrt(x * x + y * y) / M_PI))) + 1, 0.1);
    }, random_device()());

    auto _Begin = __rdtsc();

    auto Iterator = size_t(0);
    for (; Iterator < 10000; Iterator++)
    {
        bool _Result = MyGA.Run();
        if (!_Result)
        {
            break;
        }

        if (0 == Iterator % 100)
        {
            cout << Iterator << endl;
        }
    }

    cout << "Time: " << static_cast<double>(__rdtsc() - _Begin) / 2.5e9 << endl;

    decltype(MyGA)::_DomainType _Domain;
    auto _BestSolution = MyGA.GetBestSolution(_Domain);

    cout << "Iterator: " << Iterator << endl;
    cout << "Minimum: " << _BestSolution << endl;
    cout << "x: " << get<0>(_Domain) << "  y: " << get<1>(_Domain) << endl;

    return EXIT_SUCCESS;
}