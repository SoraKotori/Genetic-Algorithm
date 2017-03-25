#pragma once
#define _USE_MATH_DEFINES
#include <algorithm>
#include <functional>
#include <numeric>
#include <random>

namespace GA
{
    using namespace std;

    template<typename _Type, size_t _Length, size_t _Count>
    class GeneticAlgorithm
    {
    public:
        typedef vector<bool> _ChromosomeType;
        typedef tuple<_Type, _Type> _BinaryType;
        typedef _Type(*_BinaryFunc)(const _BinaryType&);

        GeneticAlgorithm() = default;
        ~GeneticAlgorithm() = default;

        template<typename... _Args>
        GeneticAlgorithm(_Type __min, _Type __max, _BinaryFunc __func, _Args&&... __args) :
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
                for (auto _First = _Chromosome.begin(), _Last = _Chromosome.end(); _First != _Last; ++_First)
                {
                    *_First = _Distribution(_Engine);
                }
            }
        }

        bool Run()
        {
            bool _bResult = Converged();
            if (!_bResult)
            {
                RouletteWheelSelection();
                SinglePointCrossover();

                swap(_Parent, _Child);
            }

            return !_bResult;
        }

        _Type GetMin(_BinaryType& _Binary)
        {
            auto _Max = max_element(_Fitness.begin(), _Fitness.end());
            auto _Index = distance(_Fitness.begin(), _Max);

            _Binary = Decoding(_Parent[_Index]);
            return _FitnessFunc(_Binary);
        }

        _Type GetMin()
        {
            return GetMin(_BinaryType());
        }

    private:
        vector<_ChromosomeType> _Parent;
        vector<_ChromosomeType> _Child;
        vector<_Type> _Fitness;

        _Type _Shift = _Type(0);
        _Type _Interval = _Type(0);
        _BinaryFunc _FitnessFunc;

        default_random_engine _Engine;
        uniform_int_distribution<_ChromosomeType::size_type> _ChromosomeDistribution;

        bool Converged()
        {
            auto _First_Parent = _Parent.begin();
            auto _Last_Parent = _Parent.end();
            auto _First_Fitness = _Fitness.begin();

            if (_First_Parent == _Last_Parent)
            {
                return false;
            }

            bool Converged = true;

            for (; _First_Parent != _Last_Parent; ++_First_Parent, ++_First_Fitness)
            {
                auto _Domain = Decoding(*_First_Parent);
                auto _Range = _FitnessFunc(_Domain);
                auto _Value_Fitness = _Range > _Type(0) ? _Type(1) / _Range : -_Range;

                *_First_Fitness = _Value_Fitness;
                if (Converged && *_Fitness.begin() != _Value_Fitness)
                {
                    Converged = false;
                }
            }

            return Converged;
        }

        void RouletteWheelSelection()
        {
            discrete_distribution<vector<_ChromosomeType>::size_type>
                _Distribution(_Fitness.begin(), _Fitness.end());

            for (auto& _Chromosome : _Child)
            {
                auto _ParentIndex = _Distribution(_Engine);
                _Chromosome = _Parent[_ParentIndex];
            }
        }

        void SinglePointCrossover()
        {
            auto _FirstChild = _Child.begin();
            auto _LastChild = _Child.end();

            for (; _FirstChild != _LastChild; ++_FirstChild)
            {
                auto _PrevChild = _FirstChild++;
                if (_FirstChild == _LastChild)
                {
                    break;
                }

                auto _ChromosomeFirst = _PrevChild->begin();
                auto _ChromosomeLast = next(_ChromosomeFirst, _ChromosomeDistribution(_Engine));
                swap_ranges(_ChromosomeFirst, _ChromosomeLast, _FirstChild->begin());
            }
        }

        _Type Binary(_Type _Init, _ChromosomeType::value_type b)
        {
            _Init *= _Type(2);
            if (b)
            {
                _Init += _Type(1);
            }

            return _Init;
        }

        _BinaryType Decoding(const _ChromosomeType& _Chromosome)
        {
            auto _First = _Chromosome.begin();
            auto _Middle = next(_First, _Chromosome.size() / 2);
            auto _Last = _Chromosome.end();

            auto x = accumulate(_First, _Middle, _Type(0),
                bind(&GeneticAlgorithm::Binary, this, placeholders::_1, placeholders::_2));

            auto y = accumulate(_Middle, _Last, _Type(0),
                bind(&GeneticAlgorithm::Binary, this, placeholders::_1, placeholders::_2));

            x = x * _Interval + _Shift;;
            y = y * _Interval + _Shift;;
            return { x, y };
        }
    };
}
