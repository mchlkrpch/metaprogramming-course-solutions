#include <array>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <lib/assert.hpp>
#include <span>
#include <type_traits>
#include <limits>
#include <vector>


// Like in stdlib according to cppreference
inline constexpr size_t    dynamic_extent = std::numeric_limits<size_t>::max();
// Same vibes
inline constexpr ptrdiff_t dynamic_stride = std::numeric_limits<size_t>::max();



// Base structure
//   To determine which stride has Slice class
template < ptrdiff_t _Sd >
class _stride_base
{ protected:
  _stride_base([[maybe_unused]] ptrdiff_t)
  {}
  
  // Main provided method
  static constexpr ptrdiff_t Stride()
  { return _Sd; }
};



// Specialization for dynamic
//   variation in the code.
// 
//   Default stride step is 1
template<>
class _stride_base< dynamic_stride >
{ protected:
  _stride_base([[maybe_unused]]ptrdiff_t step_size = 1)
    : _step(step_size)
  {}
  
  ptrdiff_t Stride() const
  { return _step; }

  private:
  ptrdiff_t _step{1};
};


// Extent base like in span class
//   provie either extension and stride
//   characteristics for Slice class
template< size_t _Ext, ptrdiff_t _Sd >
class _extent_base
: protected _stride_base< _Sd >
{ protected:
  using _stride_base< _Sd >::Stride;

  _extent_base(
      [[maybe_unused]]size_t    sz        = 0
    , [[maybe_unused]]ptrdiff_t step_size = 1)
    : _stride_base<_Sd>(step_size)
  { if (sz > _Ext) {
      throw ::mpc::detail::AssertException();
    }
  }

  static constexpr size_t Size()
  { return _Ext; }
};



// Extent base like in span class
//   provie either extension and stride
//   characteristics for Slice class
// 
//   Default extension is 0
template< ptrdiff_t _Sd >
class _extent_base< dynamic_extent, _Sd >
  : protected _stride_base<_Sd>
{ protected:
  using _stride_base<_Sd>::Stride;

  _extent_base(
      [[maybe_unused]]size_t    sz   = 0
    , [[maybe_unused]]ptrdiff_t step = 1)
    : _stride_base<_Sd>(step)
    , _ext(sz)
  {}

  constexpr size_t Size() const
  { return _ext; }

  private:
  size_t _ext;
};


// Iterator class for Slice class
//   Provide logic connected with stride and
//   boudary check (in rt)
template
  < typename _Tp
  , bool _Cnst
  , size_t _It_Ext >
struct _It_slice
{ public:
  // Standard menu for pointer class
  using value_type        = std::conditional< _Cnst, const _Tp , _Tp  >::type;
  using pointer           = std::conditional< _Cnst, const _Tp*, _Tp* >::type;
  using reference         = std::conditional< _Cnst, const _Tp&, _Tp& >::type;
  using difference_type   = ptrdiff_t;
  // Using to address to current iterator type
  using _It_slice_this = _It_slice< _Tp, _Cnst, _It_Ext >;



  // Constructors from the most privitive
  //   To the most complicated

  // Constructor for custom step and
  //   And particular pointer
  // param p(): 
  _It_slice(
      [[maybe_unused]]pointer   p = nullptr
    , [[maybe_unused]]ptrdiff_t s = 1)
    : _ptr(p)
    , _stp(s)
  {}

  _It_slice(      _It_slice&  I) = default;
  _It_slice(const _It_slice&  I) = default;
  _It_slice(      _It_slice&& I) = default;
  _It_slice(const _It_slice&& I) = default;



  // Based functions.
  size_t Step()
  { return _stp; }


  // Operators
  reference operator*() const
  { return *_ptr; }

  ptrdiff_t operator-(const _It_slice& I) const
  { return (_ptr - I._ptr) / _stp; }

  _It_slice& operator=(const _It_slice&  I) = default;
  _It_slice& operator=(      _It_slice&& I) = default;
  
  pointer operator->() const
  { return _ptr; }

  _It_slice
    < _Tp
    , _Cnst
    , _It_Ext >& operator++()
  { _ptr += Step();
    if (_It_Ext * Step() < size_t(_ptr - _start)) {
      throw ::mpc::detail::AssertException();
    }
    return *this;
  }

  // Custom Operator ++
  //   param cnt(int): number of steps to make
  _It_slice
    < _Tp
    , _Cnst
    , _It_Ext > operator++(int cnt)
  { _It_slice_this i(_ptr, Step());
    _ptr += Step() * cnt;
    return i;
  }

  _It_slice_this& operator--()
  { _ptr -= Step();
    if (_It_Ext * Step() < size_t(_start - _ptr)) {
      throw ::mpc::detail::AssertException();
    }
    return *this;
  }

  // Custom Operator ++
  //   param cnt(int): number of steps to make
  _It_slice_this operator--(int cnt)
  { _It_slice_this i(_ptr, Step());
    _ptr -= Step() * cnt;

    return i;
  }

  bool operator==(const _It_slice_this& I) const
  { return _ptr == I._ptr; }

  _It_slice_this operator+(ptrdiff_t b) const
  { _It_slice_this temp(_ptr);
    temp += b;
    return temp;
  }

  friend _It_slice_this operator+(ptrdiff_t idx, const _It_slice& I)
  { return I + idx; }

  _It_slice_this operator-(ptrdiff_t idx) const
  { _It_slice_this temp(_ptr);
    temp -= idx;
    return temp;
  }

  std::strong_ordering operator<=> (const _It_slice_this& I) const
  { return _ptr <=> I._ptr; }

  // += operator for iterator
  //   param cnt(ptrdiff_t): number of steps to make.
  _It_slice
    < _Tp
    , _Cnst
    , _It_Ext >& operator+=(const ptrdiff_t cnt)
  { _ptr += Step() * cnt;
    return *this;
  }

  reference operator [](size_t cnt) const
  { return *(_ptr + cnt * _stp); }


  // -= operator for iterator
  //   param cnt(ptrdiff_t): number of steps to make.
  _It_slice_this& operator-=(const ptrdiff_t cnt)
  { _ptr -= Step() * cnt;
    return *this;
  }

  operator _It_slice<_Tp, true, _It_Ext> () const 
  { return _It_slice<_Tp, true, _It_Ext>(_ptr, _stp); }

  private:
  // Current pointer of iterator
  pointer _ptr{nullptr};

  // Initial state of ptr to
  //   calculate difference between _ptr and
  //   start pointers and realize if
  //   current pointer looks at not available
  //   memory in conteiner / range
  pointer _start{_ptr};

  // Step of iterator
  //   to pass some cells during indexing
  ptrdiff_t _stp{1};
};


// Main slice class.
template
  < class      _Tp
  , size_t     _Ext = dynamic_extent
  , ptrdiff_t  _Sd  = 1 >
class Slice
: protected _extent_base< _Ext, _Sd >
{ public:
  // Default stdlib menu
  using element_type     =       _Tp;
  using value_type       =       std::decay_t< _Tp >;
  using size_type        =       size_t;
  using difference_type  =       ptrdiff_t;
  using pointer          =       std::decay_t< _Tp >*;
  using const_pointer    = const std::decay_t< _Tp >*;
  using reference        =       std::decay_t< _Tp >&;
  using const_reference  = const std::decay_t< _Tp >&;

  // Iterators
  using iterator               = _It_slice< _Tp, false, _Ext >;
  using const_iterator         = _It_slice< _Tp, true,  _Ext >;
  using reverse_iterator       = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  // Based functions.
  using _extent_base<_Ext, _Sd>::Size;
  using _extent_base<_Ext, _Sd>::Stride;

  // From other slice
  template
    < class      _Tp_other
    , size_t    _Ext_other
    , ptrdiff_t _Stp_other >
  Slice(const Slice<_Tp_other, _Ext_other, _Stp_other>& other)
    : _extent_base< _Ext, _Sd >(other.Size(), other.Stride())
    , _ptr(other.Data())
  {}


  template< class Container >
  Slice(Container& c)
  requires(requires(Container c) {{ c.begin() } -> std::contiguous_iterator; c.size();})
    : _extent_base< _Ext, _Sd >(c.size() / (_Sd == dynamic_stride ? 1 : _Sd), 1)
    , _ptr(std::to_address(c.begin()))
  { if (c.size() < Size()) {
      throw ::mpc::detail::AssertException();
    }
  }

  Slice()
    : _extent_base< _Ext, _Sd >(0, 1)
    , _ptr(nullptr)
  {}
  
  // prarm p   : pointer to begin of the data.
  // param ext : extension size of data
  // param stp : step for step
  Slice(_Tp* p, size_t ext = 0, ptrdiff_t stp = 1)
    : _extent_base< _Ext, _Sd >(ext, stp)
    , _ptr(p)
  {}

  template< std::contiguous_iterator It >
  Slice(It I, size_t cnt = 0, ptrdiff_t stp = 1)
    : _extent_base< _Ext, _Sd >(cnt, stp)
    , _ptr(std::to_address(I))
  {}

  template< class Container >
  Slice(Container& c, size_t sd)
    : _extent_base< _Ext, _Sd >(c.size() / (_Sd == dynamic_stride ? 1 : _Sd), 1)
    , _ptr(std::to_address(c.begin()))
  { if (sd != _Sd) {
      throw ::mpc::detail::AssertException();
    }
  }

  template< typename Range >
  Slice(const Range&& r)
    : _ptr(std::ranges::data(r))
    , _extent_base< _Ext, _Sd >(std::ranges::size(r)
    , r.Stride())
  {}




  // Simple func
  _Tp* Data() const
  { return _ptr; }

  Slice< _Tp, dynamic_extent, _Sd > First(size_t count) const
  { return Slice
      < _Tp
      , dynamic_extent
      , _Sd >(_ptr, count, Stride());
  }



  // Simple functions
  iterator begin() const
  { return iterator(_ptr, Stride()); }

  iterator end() const
  { if (Size() <= 0) {
      throw ::mpc::detail::AssertException();
    }
    return iterator(_ptr + (Size()) * Stride(), Stride());
  }


  template< size_t _Cnt >
  Slice< _Tp, _Cnt, _Sd > First() const
  { return Slice< _Tp, _Cnt, _Sd > (_ptr, _Cnt, Stride()); }

  Slice< _Tp, dynamic_extent, _Sd > Last(size_t count) const
  { return Slice
      < _Tp
      , dynamic_extent
      , _Sd >(_ptr + Stride() * (Size() - count), count, Stride());
  }

  template <size_t _Cnt>
  Slice< _Tp, _Cnt, _Sd > Last() const
  { return Slice
      < _Tp
      , _Cnt
      , _Sd >(_ptr + Stride() * (Size() - _Cnt), _Cnt, Stride());
  }

  reverse_iterator rend() const
  { return reverse_iterator(_ptr, Stride()); }

  reverse_iterator rbegin() const
  { return reverse_iterator(_ptr + (Size()-1)*Stride(), Stride()); }

  Slice< _Tp, dynamic_extent, _Sd >
  DropFirst(size_t cnt) const
  { if (cnt > Size()) {
      throw ::mpc::detail::AssertException();
    }
    return Last(Size() - cnt);
  }

  template< size_t _Cnt >
  Slice
    < _Tp, (_Ext == dynamic_extent ? dynamic_extent : _Ext - _Cnt)
    , _Sd
    >
  DropFirst() const
  { if (_Cnt > Size()) {
      throw ::mpc::detail::AssertException();
    }
    if constexpr (_Ext == dynamic_extent) {
      return Last(Size() - _Cnt);
    } else {
      return Last<_Ext - _Cnt>();
    }
  }

  Slice< _Tp, dynamic_extent, _Sd >
  DropLast(size_t count) const
  { if (count > Size()) {
      throw ::mpc::detail::AssertException();
    }
    return First(Size() - count);
  }

  template<size_t count>
  Slice
    < _Tp, (_Ext == dynamic_extent ? dynamic_extent : _Ext - count)
    , _Sd
    >
  DropLast() const
  { if (count > Size()) {
      throw ::mpc::detail::AssertException();
    }
    if constexpr (_Ext == dynamic_extent){
      return First(Size() - count);
    } else{
      return First< _Ext - count >();
    }
  }

  Slice<_Tp, dynamic_extent, dynamic_stride>
  Skip(ptrdiff_t skip) const
  { return Slice
      < _Tp
      , dynamic_extent
      , dynamic_stride
      >(_ptr, (Size()-1) / skip + 1, Stride() * skip);
  }

  template <ptrdiff_t _Stp>
  Slice
    < _Tp
    , _Sd != dynamic_stride && _Ext != dynamic_extent? (_Ext - 1) / _Stp + 1 : dynamic_extent
    , _Sd != dynamic_stride ? _Sd * _Stp : _Sd
    >
  Skip() const
  { return Slice
      < _Tp
      , (_Sd != dynamic_stride && _Ext != dynamic_extent? (_Ext - 1) / _Stp + 1 : dynamic_extent)
      , (_Sd != dynamic_stride? _Sd * _Stp : _Sd)
      >(_ptr, (Size()-1) / _Stp + 1, Stride() * _Stp);
  }


  template
    < class      _Tp_other
    , size_t    _Ext_other
    , ptrdiff_t _Stp_other >
  bool operator==(const Slice<_Tp_other, _Ext_other, _Stp_other>& other) const
  { return (
        _ptr == other.Data() &&
      Size() == other.Size() &&
    Stride() == other.Stride());
  }

  _Tp& operator[](const size_t idx)
  { if (idx >= Size()) {
      throw ::mpc::detail::AssertException();
    }
    return *(_ptr + idx * Stride());
  }

  _Tp& operator[](const size_t idx) const
  { if (idx >= Size()) {
      throw ::mpc::detail::AssertException();
    }
    return *(_ptr + idx * Stride());
  }

private:
  _Tp* _ptr;
};


// Deduction guide for slice
template
  < std::contiguous_iterator Iterator
  , typename extent
  , typename stride >
Slice(Iterator, extent, stride)
-> Slice
  < std::remove_reference_t< typename Iterator::value_type >
  , dynamic_extent
  , dynamic_stride >;
template
  < typename T >


Slice(T)
-> Slice
  < std::remove_reference_t<typename T::value_type> >;

template< typename T >
Slice(std::vector< T >&)
-> Slice
  < std::remove_reference_t<typename T::value_type>
  , dynamic_extent >;


template
  < typename T
  , size_t extent >
Slice(std::array<T, extent>&)
-> Slice
  < std::remove_reference_t<T>
  , extent
  , 1 >;
