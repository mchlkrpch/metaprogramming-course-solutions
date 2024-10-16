#pragma once

#include <concepts>

#include <type_tuples.hpp>


namespace type_lists
{

template<typename TL>
concept TypeSequence =
	requires {
		typename TL::Head;
		typename TL::Tail;
	};

struct Nil {};

template
	< typename TL >
concept Empty = std::derived_from<TL, Nil>;

template
	< typename TL >
concept TypeList = Empty<TL> || TypeSequence<TL>;

// Your fun, fun metaalgorithms :)

// https://www.codingwiththomas.com/blog/getting-started-with-typelists
template
	< typename H
	, TypeList T >
struct Cons
{ using Head = H;
	using Tail = T;
};




// Basic functions
// 



// Scan function
template
	< template
		< class, class > typename _Cmnd
	, class H
	, TypeList T>
struct Scanl
{ using Head = H;
	using Tail = Scanl<_Cmnd, _Cmnd<H, typename T::Head>, typename T::Tail>;
};

template
	< template
		< class, class > typename _Cmnd
	, class H
	, Empty T >
struct Scanl<_Cmnd, H, T>
{ using Head = H;
	using Tail = Nil;
};

// In tests
template
	< std::size_t _Cnt
	, class H >
struct Replicate
{ using Head = H;
	using Tail = Replicate<_Cnt - 1, H>;
};

template
	< class H >
struct Replicate<0, H> : Nil {};


// Applies funciton to
//   all array's elements
template
	< template
		< typename > typename Func
	, TypeList T >
struct Map
{ using Head = Func<typename T::Head>;
	using Tail = Map<Func, typename T::Tail>;
};

template
	< template
		< typename > typename Func
	, Empty T>
struct Map<Func, T> : Nil {};

// Repeat in tests
template<typename T>
struct Repeat
{ using Head = T;
	using Tail = Repeat<T>;
}; 


// Enumeration class to determine which size has 
//   array of types
template
	< TypeList T
	, std::size_t _Cnt >
struct Sizer
{ static constexpr
	std::size_t size = Sizer<typename T::Tail, _Cnt + 1>::size;
};

template
	< TypeList T >
constexpr std::size_t ListSize = Sizer<T, 0>::size;

// End of the sequence
template
	< Empty T
	, std::size_t _Cnt >
struct Sizer<T, _Cnt>
{ static constexpr std::size_t size = _Cnt; };

// Filter
template
	< template
		< typename > typename P
	, TypeList T>
struct Filter;

template
	< template
		< typename > typename P
	, TypeList T> requires(!P<typename T::Head>::Value)
struct Filter<P, T> : Filter<P, typename T::Tail>{};

template
	< template
		< typename > typename P
	, TypeList T > requires(P<typename T::Head>::Value)
struct Filter<P, T>
{ using Head = typename T::Head;
	using Tail = Filter<P, typename T::Tail>;
};

template
	< template
		< typename > typename P
	, Empty T >
struct Filter<P, T> : Nil{};


template
	< template
		< typename > typename F
	, class T >
struct Iterate
{ using Head = T;
	using Tail = Iterate<F, F<T>>;
};


// End
// Convertion from the tuple class
template
	< type_tuples::TypeTuple >
struct FromTuple;

// Pass first argument to head
//  and remain part will be tail
template
	< typename H
	, typename... AS >
struct FromTuple<type_tuples::TTuple<H, AS...>>
{ using Head = H;
	using Tail = FromTuple<type_tuples::TTuple<AS...>>;
};


// Okay for take, drop
//   we will use same signature as
//   get: https://en.cppreference.com/w/cpp/utility/tuple/get

// Take action
// Pass head ot typelists to (Head)
//   remaining part - pass to future
template
	< std::size_t _Cnt
	, TypeList T >
struct Take
{ using Head = typename T::Head;
	using Tail = Take<_Cnt - 1, typename T::Tail>;
};

// Ending with _Cnt
template
	< TypeSequence T >
struct Take<0, T> : Nil{};

// Overflow size
template
	< std::size_t _Cnt
	, Empty T >
struct Take<_Cnt, T> : Nil{};

// And this will be end of the tuple
template<>
struct FromTuple<type_tuples::TTuple<>> : Nil{};

// Drop action: encounter
//   step and get tail
template
	< std::size_t _Cnt
	, TypeList T >
struct Drop : Drop<_Cnt - 1, typename T::Tail> {};

// End with size
template<TypeSequence T>
struct Drop<0, T> {
	using Head = typename T::Head;
	using Tail = typename T::Tail;
};

// End overflow
template
	< std::size_t _Cnt
	, Empty T >
struct Drop<_Cnt, T> : Nil{};


// Ctor from _Cnt-th element
template
	< std::size_t _Cnt
	, TypeList T
	, TypeList AS >
struct CtorSeq
{ using Head = Take<_Cnt, AS>;
	using Tail = CtorSeq<_Cnt + 1, typename T::Tail, AS>;
};

// End (reashed size ending)
template
	< std::size_t _Cnt
	, Empty T
	, TypeList AS >
struct CtorSeq<_Cnt, T, AS>
{ using Head = AS;
	using Tail = Nil;
};




// Drop first initializer
template
	< std::size_t _Cnt
	,TypeList T
	, TypeList AS >
struct CtorFromTail
{ using Head = Drop<_Cnt, AS>;
	using Tail = CtorFromTail<_Cnt + 1, typename T::Tail, AS>;
};

template
	< std::size_t _Cnt
	, Empty T
	, TypeList AS>
struct CtorFromTail<_Cnt, T, AS>
{ using Head = Nil;
	using Tail = Nil;
};



// Wrapper to init
template
	< TypeList T >
using Inits = CtorSeq<0, T, T>;

// Init tail
template
	< TypeList T >
using Tails = CtorFromTail<0, T, T>;

// Get head action
template
	< TypeList T >
using getHead = typename T::Head;

// Get head action
template
	< TypeList T >
using getTail = typename T::Tail;










// Strane
// 
// 

template
	< TypeList T
	, TypeList AS>
struct Suslo;

template
	< TypeList T
	, Empty AS>
struct Suslo<T, AS> : Nil{};

template
	< TypeSequence T
	, TypeSequence AS>
struct Suslo<T, AS>
{ using Head = typename T::Head;
	using Tail = Suslo<typename T::Tail, AS>;
};

template
	< Empty T
	, TypeSequence AS>
struct Suslo<T, AS>
{ using Head = typename AS::Head;
	using Tail = Suslo<typename AS::Tail, AS>;
};

template
	< TypeList T >
using Cycle = Suslo<T, T>;

template
	< TypeList T
	, typename ... AS >
struct Conv2Tuple;

// Fill tup with T::Tail and args
template
	< TypeSequence T
	, typename... AS >
struct Conv2Tuple<T, AS...>
{ using H = typename Conv2Tuple
		< typename T::Tail
		, AS..., typename T::Head >::H;
};


template<Empty T, typename ... AS>
struct Conv2Tuple<T, AS ...>
{ using H = typename type_tuples::TTuple<AS ...>; };

// test interface
template<TypeList T>
using ToTuple = typename Conv2Tuple<T>::H;


template
	< template
		< class, class > typename _Cmnd
	, class H
	, TypeList AS >
struct PileObj
{ using InnerVal = typename PileObj
		< _Cmnd
		, _Cmnd
			< H
			, typename AS::Head>
		, typename AS::Tail>::InnerVal;
};

template
	< template
		< class, class > typename _Cmnd
	, class H
	, Empty empty >
struct PileObj<_Cmnd, H, empty>
{ using InnerVal = H; };

// test interface
template
	< template
		< class, class > typename _Cmnd
	, class H
	, TypeList AS >
using Foldl = typename PileObj<_Cmnd, H, AS>::InnerVal;



template
	< template
		< class, class > typename _Cmnd
	, typename H
	, TypeList AS
	>
struct PileRObj {
  using Value = _Cmnd
		< typename AS::Head
		, typename PileRObj
			< _Cmnd
			, H
			, typename AS::Tail>::Value>;
};

template
	< template
		< class, class > typename _Cmnd
	, typename H
	, TypeList AS>
  requires Empty<AS>
struct PileRObj<_Cmnd, H, AS>
{ using Value = H; };

template
	< template
		< class, class > typename _Cmnd
	, typename H
	, TypeList AS
	>
using Foldr = PileRObj<_Cmnd, H, AS>::Value;























// Hard ations

// Grouper base
template
	< template
		< class, class > typename _Equal
	, std::size_t _Cnt
	, TypeList T
	, TypeList AS
	>
struct Groupper;

template
	< template
		< class, class > typename _Equal
	, std::size_t _Cnt
	, TypeList T
	, Empty AS
	>
struct Groupper<_Equal, _Cnt, T, AS> : Nil{};

template
	< template
		< class, class > typename _Equal
	, std::size_t _Cnt
	, Empty T
	, TypeSequence AS
	>
struct Groupper<_Equal, _Cnt, T, AS> : Nil{};

template
	< template
		< class, class > typename _Equal
	, std::size_t _Cnt
	, TypeSequence T
	, TypeSequence AS
	>
	requires (
		Empty
		< typename T::Tail > || (
			TypeSequence
			< typename T::Tail > &&
			!_Equal
			< typename T::Head
			, typename T::Tail::Head >::Value
		)
	)
struct Groupper<_Equal, _Cnt, T, AS>
{ using Head = Take<_Cnt, AS>;
	using Tail = Groupper<_Equal, 1, Drop<_Cnt, AS>, Drop<_Cnt, AS>>;
};

template
	< template
		< class, class > typename _Equal
	, std::size_t _Cnt
	, TypeSequence T
	, TypeSequence AS
	>
	requires(
		TypeSequence
			< typename T::Tail > &&
			_Equal
			< typename T::Head
			, typename T::Tail::Head >::Value
		)
struct Groupper<_Equal, _Cnt, T, AS> : Groupper<_Equal, _Cnt + 1, typename T::Tail, AS> {};

// Test interface
template
	< template
		< class, class > typename _Equal
	, TypeList T>
using GroupBy = Groupper<_Equal, 1, T, T>;



// Zip two
template<TypeList L, TypeList R>
struct Zip2
{ using Head = type_tuples::TTuple<typename L::Head, typename R::Head>;
	using Tail = Zip2<typename L::Tail, typename R::Tail>;
};

template
	< TypeList L
	, TypeList R >requires(Empty<L> || Empty<R>)
struct Zip2<L, R> : Nil {};

template
	< TypeList T >
struct Zipper
{ using Head = ToTuple<Map<getHead, T>>;
	using Tail = Zipper<Map<getTail, T>>;
};

template
	< Empty T >
struct Zipper<T> : Nil {};

// Filter 2p
template
	<template
		< class, class > typename _Cond
	, TypeList T
	, typename First
	>
struct Filter2P;

template
	< template
		< class, class > typename _Cond
	, TypeList T
	, typename First > requires(!_Cond<First, typename T::Head>::Value)
struct Filter2P<_Cond, T, First> : Filter2P<_Cond, typename T::Tail, First>{};

template
	< template
		< class, class > typename _Cond
	, TypeList T
	, typename First > requires(_Cond<First, typename T::Head>::Value)
struct Filter2P<_Cond, T, First>
{ using Head = typename T::Head;
	using Tail = Filter2P<_Cond, typename T::Tail, First>;
};

template
	< template
		< class, class > typename _Cond
	, Empty T
	, typename First>
struct Filter2P<_Cond, T, First> : Nil{};


// Zip interface
template
	< TypeList... T >
using Zip = Zipper<FromTuple<type_tuples::TTuple<T...>>>;


} // namespace type_lists
