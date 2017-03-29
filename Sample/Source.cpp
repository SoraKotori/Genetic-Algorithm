#define _USE_MATH_DEFINES
#include "GeneticAlgorithm.h"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <intrin.h>

using namespace std;
using namespace GA;

int main(void)
{
    GeneticAlgorithm<double, 64, 4096> MyGA(-10, 10, [](const auto& _Domain)
    {
        auto x = get<0>(_Domain);
        auto y = get<1>(_Domain);

        return -0.0001 * pow(abs(sin(x) * sin(y) * exp(abs(100 - sqrt(x * x + y * y) / M_PI))) + 1, 0.1);
    }, default_random_engine::result_type(chrono::system_clock::now().time_since_epoch().count()));

    auto _Begin = __rdtsc();

    auto Iterator = 0;
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

    cout << "Time: " << (__rdtsc() - _Begin) / 2.5e9 << endl;

    decltype(MyGA)::_DomainType _Domain;

    cout << "Iterator: " << Iterator << endl;
    cout << "Minimum: " << MyGA.GetBestSolution(_Domain) << endl;
    cout << "x: " << get<0>(_Domain) << "  y: " << get<1>(_Domain) << endl;

    return EXIT_SUCCESS;
}