#pragma once

#include <value_types.hpp>
#include <type_lists.hpp>

// Natural
//   Just generate sequence of x, x + 1
template <int _Num>
struct Natural
{ using Head = value_types::ValueTag<_Num>;
	using Tail = Natural<_Num + 1>;
};

using Nats = Natural<0>;


// Fibonnacci (almost same vibe)
template
	< int A_cur
	, int A_nect>
struct FibHelper {
	using Head = value_types::ValueTag<A_cur>;
	using Tail = FibHelper<A_nect, A_cur + A_nect>;
};

using Fib = FibHelper<0, 1>;


// Primes

// This helper check which number
//   is the greatest and it's squre doens't exceed N
template
	< std::size_t _Fr
	, std::size_t _To
	, int _Num
	>
struct SqrtWrapper;

template
	< std::size_t _Fr
	, std::size_t _To
	, int _Num
	>requires(
		_To - _Fr > 1 &&
		((_To + _Fr) / 2) * ((_To + _Fr) / 2) <= _Num
	)
struct SqrtWrapper<_Fr, _To, _Num>
{ static constexpr std::size_t sqrt = SqrtWrapper<(_To + _Fr) / 2, _To, _Num>::sqrt; };

template
	< std::size_t _Fr
	, std::size_t _To
	, int _Num
	>requires(
		_To - _Fr > 1 &&
		((_To + _Fr) / 2) * ((_To + _Fr) / 2) > _Num
	)
struct SqrtWrapper<_Fr, _To, _Num>
{ static constexpr std::size_t sqrt = SqrtWrapper<_Fr, (_To + _Fr) / 2, _Num>::sqrt; };

template
	< std::size_t _Fr
	, std::size_t _To
	, int _Num>requires(
		_To - _Fr <= 1
	)
struct SqrtWrapper<_Fr, _To, _Num>
{ static constexpr std::size_t sqrt = _To; };


// Initial sequens of sqrt
template
	< int _Num >
constexpr std::size_t SQRT2 = SqrtWrapper
	< 1
	, std::size_t(_Num)
	, _Num >::sqrt;

template<>
constexpr std::size_t SQRT2<0> = 0;

template<>
constexpr std::size_t SQRT2<1> = 1;


// Primes do not have delimiters
//   this helper will check if the number divisable by numb
template
	< typename _Num
	, typename D
	>
struct HasDelimiter;


template
	< int _Num
	, int D >
struct HasDelimiter
	< value_types::ValueTag<_Num>
	, value_types::ValueTag<D>> {
		static constexpr bool Value = (_Num % D == 0);
	};

template
	< typename T >
struct _IsPrime;

template
	< int _Num >
struct _IsPrime<value_types::ValueTag<_Num>>
{ static constexpr bool Value = (
	(_Num != 0) && (_Num != 1) && ( (_Num == 2 || _Num == 3) ||
	(
		type_lists::ListSize
			< type_lists::Filter2P
				< HasDelimiter
				, type_lists::Drop
					< 2
					, type_lists::Take<SQRT2<_Num>, Nats>
					>
				, value_types::ValueTag<_Num>
				>
			> == 0
	))
	);
};

using Primes = type_lists::Filter<_IsPrime, Nats>;
