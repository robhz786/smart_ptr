#ifndef BOOST_SMART_PTR_DETAIL_SHARED_INITIALIZER_HPP_INCLUDED
#define BOOST_SMART_PTR_DETAIL_SHARED_INITIALIZER_HPP_INCLUDED

//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <boost/smart_ptr/detail/sp_forward.hpp>
#include <boost/smart_ptr/shared_handle.hpp>

#if defined(__clang__)
#  if __clang_major__ < 4 && __clang_minor__ < 2
#     define BOOST_DETAIL_SHARED_INIT_NO_SFINAE_WITH_DECLTYPE
#     if  __cplusplus >= 201103L
#         define BOOST_DETAIL_SHARED_INIT_NO_SFINAE_WITH_OPERATOR_NEW
#     endif
#  endif

#elif defined(BOOST_GCC_VERSION)
#  if BOOST_GCC_VERSION < 40600
#    define BOOST_DETAIL_SHARED_INIT_NO_SFINAE_WITH_DECLTYPE
#  endif
#  if BOOST_GCC_VERSION < 40800
#    define BOOST_DETAIL_SHARED_INIT_NO_SFINAE_WITH_OPERATOR_NEW
#  endif


#elif defined(_MSC_VER)
#  if _MSC_VER < 1800
#    define BOOST_DETAIL_SHARED_INIT_NO_SFINAE_WITH_DECLTYPE
#  endif
#  if _MSC_VER < 1900
#    define BOOST_DETAIL_SHARED_INIT_NO_SFINAE_WITH_OPERATOR_NEW
#  endif

#elif defined(BOOST_NO_CXX11_TRAILING_RESULT_TYPES) || defined(BOOST_NO_CXX11_DECLTYPE)
#  define BOOST_DETAIL_SHARED_INIT_NO_SFINAE_WITH_DECLTYPE
#  define BOOST_DETAIL_SHARED_INIT_NO_SFINAE_WITH_OPERATOR_NEW
#endif


namespace boost {
namespace detail {

class shared_initializer
{
public:

    template <class T>
    shared_initializer(const boost::shared_ptr<T>& p)
        : m_hdl(p)
    {
    }

#if defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(BOOST_NO_CXX11_RVALUE_REFERENCES)

    template <class T>
    void construct(T* address) const
    {
        do_construct<T>(preference_0(), address);
    }

#else

    template<typename T, typename ... Args>
    void construct(T* address, Args && ... args) const
    {
        std::allocator<T> a;
        do_construct<T, std::allocator<T> >
            ( preference_0()
            , a
            , address
            , boost::detail::sp_forward<Args>( args )...
            );
    }

    template<typename T, typename A, typename ... Args>
    void construct(A& alloc, T* address, Args && ... args) const
    {

#if !defined( BOOST_NO_CXX11_ALLOCATOR )
        typedef typename std::allocator_traits<A>::template rebind_alloc<T> A2;
#else
        typedef A A2;
#endif
        A2 a2(alloc);
        do_construct<T, A2>
            ( preference_0()
            , a2
            , address
            , boost::detail::sp_forward<Args>( args )...
            );
    }

#endif //defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(BOOST_NO_CXX11_RVALUE_REFERENCES)

private:

    boost::shared_handle m_hdl;

    struct preference_6 {};

    struct preference_5: public preference_6 {};

    struct preference_4: public preference_5 {};

    struct preference_3: public preference_4 {};

    struct preference_2: public preference_3 {};

    struct preference_1: public preference_2 {};

    struct preference_0: public preference_1 {};

    template<typename> struct dummy_t
    {
        typedef void type;
    };

    template<typename T>
    typename dummy_t<typename T::using_boost_shared_handle>::type
    do_construct(const preference_0&, T* address) const
    {
        ::new(address) T(m_hdl);
    }
    template<typename T>
    void do_construct(const preference_6&, T* address) const
    {
        ::new(address) T();
    }

#if ! defined(BOOST_DETAIL_SHARED_INIT_NO_SFINAE_WITH_OPERATOR_NEW)

    static void* vptr();

    static const boost::shared_handle& hdl();

    template<int> struct dummy_i
    {
        typedef void type;
    };

    template<typename T>
    typename dummy_i<sizeof(::new(vptr()) T())>::type
    do_construct(const preference_1&, T* address) const
    {
        ::new((void*)address) T();
    }

    template<typename T>
    typename dummy_i<sizeof(::new(vptr()) T(hdl()))>::type
    do_construct(const preference_2&, T* address) const
    {
        ::new((void*)address) T(m_hdl);
    }

#endif

#if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES) && !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)

    template<typename T, typename A, typename ... Args>
    typename dummy_t<typename T::using_boost_shared_handle>::type
    do_construct(const preference_0&, A& a, T* address, Args && ... args) const
    {
#if !defined( BOOST_NO_CXX11_ALLOCATOR )
        std::allocator_traits<A>::construct
            ( a
            , address
            , m_hdl
            , boost::detail::sp_forward<Args>( args )...
            );
#else
        (void)a;
        ::new( address ) T(m_hdl, boost::detail::sp_forward<Args>( args )... );
#endif
    }

#if !defined(BOOST_DETAIL_SHARED_INIT_NO_SFINAE_WITH_DECLTYPE)
#if !defined(BOOST_DETAIL_SHARED_INIT_NO_SFINAE_WITH_OPERATOR_NEW)

    template<typename T, typename A, typename ... Args>
    auto do_construct(const preference_1&, A& a, T* address, Args && ... args) const
        ->decltype
            ( a.construct(address, m_hdl, boost::detail::sp_forward<Args>( args )...)
            , ::new(address) T(m_hdl, boost::detail::sp_forward<Args>( args )...)
            , void()
            )
    {
        a.construct(address, m_hdl, boost::detail::sp_forward<Args>( args )...);
    }

#endif // !defined(BOOST_DETAIL_SHARED_INIT_NO_SFINAE_WITH_OPERATOR_NEW)

    template<typename T, typename A, typename ... Args>
    auto do_construct(const preference_2&, A& a, T* address, Args && ... args) const
        ->decltype
            ( a.construct(address, boost::detail::sp_forward<Args>( args )...)
            , void()
            )
    {
        a.construct(address, boost::detail::sp_forward<Args>( args )...);
    }

    template<typename T, typename A, typename ... Args>
    auto do_construct(const preference_3&, A& a, T* address, Args && ... args) const
        ->decltype
            ( a.construct(address, m_hdl, boost::detail::sp_forward<Args>( args )...)
            , void()
            )
    {
        a.construct(address, m_hdl, boost::detail::sp_forward<Args>( args )...);
    }

#if ! defined(BOOST_DETAIL_SHARED_INIT_NO_SFINAE_WITH_OPERATOR_NEW)

    template<typename T, typename A, typename ... Args>
    auto do_construct(const preference_4&, A&, T* address, Args && ... args) const
        ->decltype
            ( ::new(address) T(boost::detail::sp_forward<Args>( args )... )
            , void()
            )
    {
        ::new(address) T(boost::detail::sp_forward<Args>( args )...);
    }

    template<typename T, typename A, typename ... Args>
    auto do_construct(const preference_5&, A&, T* address, Args && ... args) const
        ->decltype
            ( ::new(address) T(m_hdl, boost::detail::sp_forward<Args>( args )... )
            , void()
            )
    {
        ::new(address) T(m_hdl, boost::detail::sp_forward<Args>( args )...);
    }

#endif // ! defined(BOOST_DETAIL_SHARED_INIT_NO_SFINAE_WITH_OPERATOR_NEW)
#endif // ! defined(BOOST_DETAIL_SHARED_INIT_NO_SFINAE_WITH_DECLTYPE)

    template<typename T, typename A, typename ... Args>
    void do_construct(const preference_6&, A& a, T* address, Args && ... args) const
    {

#if !defined( BOOST_NO_CXX11_ALLOCATOR )

        std::allocator_traits<A>::construct
            ( a
            , address
            , boost::detail::sp_forward<Args>( args )...
            );
#else

        (void)a;
        ::new( address ) T(boost::detail::sp_forward<Args>( args )... );
#endif

    }


#endif // !defined( BOOST_NO_CXX11_VARIADIC_TEMPLATES ) && !defined( BOOST_NO_CXX11_RVALUE_REFERENCES )

}; // class shared_initializer
} // namespace detail
} // namespace boost

#endif // #ifndef BOOST_SMART_PTR_DETAIL_SHARED_INITIALIZER_HPP_INCLUDED

