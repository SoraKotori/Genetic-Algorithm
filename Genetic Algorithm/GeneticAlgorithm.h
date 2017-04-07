#pragma once
#include <algorithm>
#include <functional>
#include <numeric>
#include <random>

namespace GA
{
    using namespace std;

    template<typename _Type, size_t _Length, size_t _Count, typename _EngineType = default_random_engine>
    class GeneticAlgorithm
    {
    public:
        typedef vector<bool> _ChromosomeType;
        typedef tuple<_Type, _Type> _DomainType;
        typedef _Type(*_FitnessFunction)(const _DomainType&);

        GeneticAlgorithm() = default;
        ~GeneticAlgorithm() = default;

        template<typename... _Args>
        GeneticAlgorithm(_Type __min, _Type __max, _FitnessFunction __func, _Args&&... __args) :
            _Parent(_Count, _ChromosomeType(_Length)),
            _Child(_Count, _ChromosomeType(_Length)),
            _Fitness(_Count),
            _Shift(min(__min, __max)),
            _FitnessFunc(__func),
            _Engine(forward<_Args>(__args)...),
            _ChromosomeDistribution(0, _Length)
        {
            auto _BinaryMax = pow(_Type(2), _Length / size_t(2)) - _Type(1);
            _Interval = abs(__max - __min) / _BinaryMax;

            Reset();
        }

        void Reset()
        {
            bernoulli_distribution _Distribution;

            for (auto& _Chromosome : _Parent)
            {
                generate(_Chromosome.begin(), _Chromosome.end(), [this, &_Distribution]
                { return _Distribution(_Engine); });
            }
        }

        bool Run()
        {
            bool _Converged = Converged();
            if (!_Converged)
            {
                RouletteWheelSelection();
                SinglePointCrossover();

                swap(_Parent, _Child);
            }

            return !_Converged;
        }

        _Type GetBestSolution(_DomainType& _Domain)
        {
            auto _Max = max_element(_Fitness.begin(), _Fitness.end());
            auto _Index = distance(_Fitness.begin(), _Max);

            _Domain = Decoding(_Parent[_Index]);
            return _FitnessFunc(_Domain);
        }

        _Type GetBestSolution()
        {
            return GetBestSolution(_DomainType());
        }

    private:
        vector<_ChromosomeType> _Parent;
        vector<_ChromosomeType> _Child;
        vector<_Type> _Fitness;

        _Type _Shift = _Type(0);
        _Type _Interval = _Type(0);
        _FitnessFunction _FitnessFunc;

        _EngineType _Engine;
        uniform_int_distribution<_ChromosomeType::size_type> _ChromosomeDistribution;

        bool Converged()
        {
            bool _Converged = true;

            transform(_Parent.begin(), _Parent.end(), _Fitness.begin(), [&](const auto& _Chromosome)
            {
                auto _Domain = this->Decoding(_Chromosome);
                auto _Range = _FitnessFunc(_Domain);
                auto _Value_Fitness = _Range > _Type(0) ? _Type(1) / _Range : -_Range;

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
            discrete_distribution<typename decltype(_Parent)::size_type>
                _ParentDistribution(_Fitness.begin(), _Fitness.end());

            for (auto& _Chromosome : _Child)
            {
                auto _ParentIndex = _ParentDistribution(_Engine);
                _Chromosome = _Parent[_ParentIndex];
            }
        }

        void SinglePointCrossover()
        {
            for (auto _First = _Child.begin(), _Last = _Child.end(); _First != _Last; ++_First)
            {
                auto _Prev = _First->begin();
                if (++_First == _Last)
                {
                    break;
                }

                auto GetPoint = [this] { return _ChromosomeDistribution(_Engine); };
                swap_ranges(_Prev, next(_Prev, GetPoint()), _First->begin());
            }
        }

        _DomainType Decoding(const _ChromosomeType& _Chromosome)
        {
            auto _First = _Chromosome.begin();
            auto _Middle = next(_First, _Chromosome.size() / 2);
            auto _Last = _Chromosome.end();

            auto _BinaryFunc = [](const _Type _Init, const auto _Flag)
            {
                return _Flag ? _Init * _Type(2) + _Type(1) : _Init * _Type(2);
            };

            auto x = accumulate(_First, _Middle, _Type(0), _BinaryFunc);
            auto y = accumulate(_Middle, _Last, _Type(0), _BinaryFunc);

            x = x * _Interval + _Shift;
            y = y * _Interval + _Shift;
            return { x, y };
        }
    };
}
