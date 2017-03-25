#include "GeneticAlgorithm.h"
#include <cstdlib>
#include <iostream>

using namespace std;
using namespace GA;

int main(void)
{
    typedef GeneticAlgorithm<double, 64, 256> MyGeneticAlgorithm;

    auto Fitness = [](const MyGeneticAlgorithm::_BinaryType& _Domain)->double
    {
        auto x = get<0>(_Domain);
        auto y = get<1>(_Domain);

        return -0.0001 * pow(abs(sin(x) * sin(y) * exp(abs(100 - sqrt(x * x + y * y) / M_PI))) + 1, 0.1);
    };

    MyGeneticAlgorithm MyGA(-10, 10, Fitness);

    int Iterator;
    for (Iterator = 0; Iterator < 10000; Iterator++)
    {
        bool _Result = MyGA.Run();
        if (!_Result)
        {
            break;
        }
    }

    cout << endl << "Iterator: " << Iterator << endl;

    decltype(MyGA)::_BinaryType _Binary;
    cout << "Minimum: " << MyGA.GetMin(_Binary) << endl;
    cout << "x: " << get<0>(_Binary) << "  y: " << get<1>(_Binary) << endl;

    return EXIT_SUCCESS;
}