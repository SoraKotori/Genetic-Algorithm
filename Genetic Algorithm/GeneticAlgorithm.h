#pragma once
#include <algorithm>
#include <functional>
#include <numeric>
#include <random>

namespace GA
{
    using namespace std;

    template<typename _Type, typename _EngineType = default_random_engine>
    class GeneticAlgorithm
    {
    public:
        using _DomainType = tuple<_Type, _Type>;
        using _FitnessFunction = _Type(*)(const _DomainType&);
        using _ChromosomeType = vector<bool>;
        using size_type1 = vector<_ChromosomeType>::size_type;
        using size_type2 = _ChromosomeType::size_type;

        GeneticAlgorithm() = default;
        ~GeneticAlgorithm() = default;

        template<typename... _Args>
        GeneticAlgorithm(size_type1 _PopulationSize, size_type2 _ChromosomeLength, double _CrossoverRate,
            _Type _Min, _Type _Max, _FitnessFunction _Func, _Args&&... __args
        ) :
            _Parent(_PopulationSize, _ChromosomeType(_ChromosomeLength)),
            _Child(_PopulationSize, _ChromosomeType(_ChromosomeLength)),
            _Fitness(_PopulationSize),
            _Shift(min(_Min, _Max)),
            _FitnessFunc(_Func),
            _Engine(forward<_Args>(__args)...),
            _CrossoverDistribution(_CrossoverRate),
            _ChromosomeDistribution(0, _ChromosomeLength)
        {
            auto&& _BinaryMax = pow(_Type(2), _ChromosomeLength / size_type2(2)) - _Type(1);
            _Interval = abs(_Max - _Min) / _BinaryMax;

            Reset();
        }

        void Reset()
        {
            bernoulli_distribution _Distribution;

            for (auto&& _Chromosome : _Parent)
            {
                generate(_Chromosome.begin(), _Chromosome.end(), [this, &_Distribution]
                { return _Distribution(_Engine); });
            }
        }

        decltype(auto) Run()
        {
            auto&& _Converged = Converged();
            if (!_Converged)
            {
                RouletteWheelSelection();
                SinglePointCrossover();

                swap(_Parent, _Child);
            }

            return (!_Converged);
        }


        decltype(auto) GetBestSolution(_DomainType&& _Domain)
        {
            auto&& _Max = max_element(_Fitness.begin(), _Fitness.end());
            auto&& _Index = distance(_Fitness.begin(), _Max);

            _Domain = Decoding(_Parent[_Index]);
            return _FitnessFunc(forward<_DomainType>(_Domain));
        }

        decltype(auto) GetBestSolution()
        {
            return GetBestSolution(_DomainType());
        }

    private:
        vector<_ChromosomeType> _Parent;
        vector<_ChromosomeType> _Child;
        vector<_Type> _Fitness;

        _Type _Shift{ 0 };
        _Type _Interval{ 0 };
        _FitnessFunction _FitnessFunc;

        _EngineType _Engine;
        bernoulli_distribution _CrossoverDistribution;
        uniform_int_distribution<size_type1> _ChromosomeDistribution;

        decltype(auto) Converged()
        {
            auto _Converged{ true };

            transform(_Parent.begin(), _Parent.end(), _Fitness.begin(), [&](auto&& _Chromosome)
            {
#ifdef _MSC_VER
                auto&& _Domain = Decoding(_Chromosome);
#else
                auto&& _Domain = this->Decoding(_Chromosome);
#endif
                auto&& _Range = _FitnessFunc(_Domain);
                auto&& _Value_Fitness = _Range > _Type(0) ? _Type(1) / _Range : -_Range;

                if (_Converged && _Value_Fitness != *_Fitness.begin())
                {
                    _Converged = false;
                }

                return _Value_Fitness;
            });

            return _Converged;
        }

        void RouletteWheelSelection()
        {
            discrete_distribution<size_type1> _ParentDistribution(_Fitness.begin(), _Fitness.end());

            for (auto&& _Chromosome : _Child)
            {
                auto&& _ParentIndex = _ParentDistribution(_Engine);
                _Chromosome = _Parent[_ParentIndex];
            }
        }

        void SinglePointCrossover()
        {
            for (auto&& _First = _Child.begin(), &&_Last = _Child.end(); _First != _Last; ++_First)
            {
                auto&& _Prev = _First->begin();
                if (++_First == _Last)
                {
                    break;
                }

                auto&& _Crossover = _CrossoverDistribution(_Engine);
                if (_Crossover)
                {
                    auto GetPoint{ [this] { return _ChromosomeDistribution(_Engine); } };
                    swap_ranges(_Prev, next(_Prev, GetPoint()), _First->begin());
                }
            }
        }

        template<typename _ChromosomeType>
        decltype(auto) Decoding(_ChromosomeType&& _Chromosome)
        {
            auto&& _First = _Chromosome.begin();
            auto&& _Middle = next(_First, _Chromosome.size() / 2);
            auto&& _Last = _Chromosome.end();

            auto BinaryFunc{ [](auto&& _Init, auto&& _Flag)
            {
                return _Flag ? _Init * _Type{ 2 } +_Type{ 1 } : _Init * _Type{ 2 };
            } };

            auto&& x = accumulate(_First, _Middle, _Type{ 0 }, BinaryFunc);
            auto&& y = accumulate(_Middle, _Last, _Type{ 0 }, BinaryFunc);

            x = x * _Interval + _Shift;
            y = y * _Interval + _Shift;
            return make_tuple(x, y);
        }
    };
}
