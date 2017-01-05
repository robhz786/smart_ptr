//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt

#include <boost/detail/lightweight_test.hpp>
#include <boost/make_shared.hpp>
#include <string>
#include <map>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/is_array.hpp>
#include <boost/type_traits/remove_extent.hpp>
#include <boost/utility/enable_if.hpp>

struct allocator_log 
{
    allocator_log()
        : construct_calls(0)
        , destroy_calls(0)
    {
    }
    int construct_calls;
    int destroy_calls;
};


template <class T> struct allocator_typedefs
{
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef void* void_pointer;
    typedef T value_type;
    typedef T& reference;
    typedef const T& const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
};


template <> struct allocator_typedefs<void>
{
    typedef void* pointer;
    typedef const void* const_pointer;
    typedef void* void_pointer;
    typedef void value_type;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
};

template <typename T> struct allocator_typedefs<T[]>: public allocator_typedefs<T>
{
};


template<class T, class U> class alloc_with_construct: public allocator_typedefs<T>
{
public:
                                    
    alloc_with_construct(allocator_log& l): log(l)
    {
    }
    template< class Y > alloc_with_construct(alloc_with_construct<Y, U> const & r)
        : log(r.log)
    {
    }
    T * allocate( std::size_t n )
    {
        return static_cast< T* >( ::operator new( n * sizeof( T ) ) );
    }
    T * allocate( std::size_t n, const void* )
    {
        return allocate(n);
    }

    void deallocate( T * p, std::size_t n )
    {
        ::operator delete( p );
    }

#if !defined( BOOST_NO_CXX11_VARIADIC_TEMPLATES ) && !defined( BOOST_NO_CXX11_RVALUE_REFERENCES )
    template<class... Args> void construct( T * p, Args&&... args )
    {
        if (boost::is_same<T, U>::value)
        {
            log.construct_calls++;
        }
        ::new( static_cast< void* >( p ) ) T( std::forward<Args>( args )... );
    }
#else
    void construct( T * p, const T& t )
    {
        if (boost::is_same<T, U>::value)
        {
            log.construct_calls++;
        }
        ::new( static_cast< void* >( p ) ) T( t );
    }
#endif
    void destroy( T * p )
    {
        if (boost::is_same<T, U>::value)
        {
            log.destroy_calls++;
        }
        p->~T();
    }
    template <class X> struct rebind
    {
        typedef alloc_with_construct<X, U> other;
    };
    allocator_log& log;
};


template< class T > class alloc_without_construct: public allocator_typedefs<T>
{
public:
    
    template <class X> struct rebind
    {
        typedef alloc_without_construct<X> other;
    };
    alloc_without_construct()
    {
    }
    template< class Y > alloc_without_construct( alloc_without_construct<Y> const & )
    {
    }
    T * allocate( std::size_t n )
    {
        return static_cast< T* >( ::operator new( n * sizeof( T ) ) );
    }
    T * allocate( std::size_t n, const void* )
    {
        return allocate(n);
    }
    void deallocate( T * p, std::size_t )
    {
        ::operator delete( p );
    }
    void destroy( T * p )
    {
        p->~T();
    }
};

struct with_public_ctor
{
public:

#if defined(BOOST_DETAIL_SHARED_INIT_NO_SFINAE_WITH_OPERATOR_NEW)
    struct using_boost_shared_handle{};
#endif
    
    with_public_ctor(const boost::shared_handle& hdl, int a1=0, int a2=0)
        : m_value(a1 + a2)
        , m_hdl(hdl) 
    {
        ++instances;
    }

    ~with_public_ctor()
    {
        --instances;
    }

    boost::shared_ptr<with_public_ctor> shared_from_this()
    {
        return m_hdl.forcibly_assemble_shared_ptr(this);
    }

    int value()
    {
        return m_value;
    }

    static int instances;

private:

    int m_value;
    boost::shared_handle m_hdl;
};

int with_public_ctor::instances = 0;

class with_private_ctor
{
public:
    struct using_boost_shared_handle{};

    ~with_private_ctor()
    {
        --instances;
    }

    boost::shared_ptr<with_private_ctor> shared_from_this()
    {
        return m_hdl.forcibly_assemble_shared_ptr(this);
    }

    boost::shared_ptr<const with_private_ctor> shared_from_this() const
    {
        return m_hdl.forcibly_assemble_shared_ptr(this);
    }

    int value()
    {
        return m_value;
    }

    static int instances;

private:
    friend class alloc_with_construct<with_private_ctor, with_private_ctor>;

    with_private_ctor(const boost::shared_handle& hdl, int a1 =0, int a2 = 0)
        : m_value(a1 + a2)
        , m_hdl(hdl)
    {
        ++instances;
    }

    int m_value;
    boost::shared_handle m_hdl;
};

int with_private_ctor::instances = 0;


template <class T1, class T2>
bool share_ownership(const boost::shared_ptr<T1>& p1, const boost::shared_ptr<T2>& p2)
{
    return !(p1 < p2) && !(p2 < p1);
}

struct no_args_tag {};
struct with_args_tag {};
struct make_shared_tag {};
struct allocate_shared_tag {};

template <class T, class A>
boost::shared_ptr<T> make_shared_ptr(A&, make_shared_tag, with_args_tag)
{
    return boost::make_shared<T>(1, 2);
}
template <class T, class A>
boost::shared_ptr<T> make_shared_ptr(A&, make_shared_tag, no_args_tag)
{
    return boost::make_shared<T>();
}
template <class T, class A>
boost::shared_ptr<T> make_shared_ptr(A& a, allocate_shared_tag, no_args_tag)
{
    return boost::allocate_shared<T>(a);
}
template <class T, class A>
boost::shared_ptr<T> make_shared_ptr(A& a, allocate_shared_tag, with_args_tag)
{
    return boost::allocate_shared<T>(a, 1, 2);
}
template <class T, class A>
boost::shared_ptr<T> make_shared_ptr_array(A&, make_shared_tag, no_args_tag)
{
    return boost::make_shared<T>(2);
}
template <class T, class A>
boost::shared_ptr<T> make_shared_ptr_array(A& a, allocate_shared_tag, no_args_tag)
{
    return boost::allocate_shared<T>(a, 2);
}
template <class T, class A>
boost::shared_ptr<T> make_shared_ptr_size_array(A&, make_shared_tag, no_args_tag)
{
    return boost::make_shared<T>();
}
template <class T, class A>
boost::shared_ptr<T> make_shared_ptr_size_array(A& a, allocate_shared_tag, no_args_tag)
{
    return boost::allocate_shared<T>(a);
}

template <class T> struct pointers_fixture
{
    boost::shared_ptr<T> p1, p2;

    template <class Alloc, class tag_maker, class tag_args>
    pointers_fixture(Alloc& alloc, tag_maker tag1, tag_args tag2)
        : p1(make_shared_ptr<T>(alloc, tag1, tag2))
        , p2(p1->shared_from_this())
    {
    }
    template <int TEST_CASE_ID>  void test_value(no_args_tag)
    {
        BOOST_TEST(p1->value() == 0);
    }
    template <int TEST_CASE_ID>  void test_value(with_args_tag)
    {
        BOOST_TEST(p1->value() == 3);
    }
    template <int TEST_CASE_ID>  void test_pointers()
    {
        BOOST_TEST(T::instances == 1);
        BOOST_TEST(p1 == p2);
        BOOST_TEST(share_ownership(p1, p2));
        BOOST_TEST(p1.use_count() == 2);
    }
    void reset_pointers()
    {
        p1.reset();
        p2.reset();
    }
};

template <class T, class TArr> struct pointers_array_test
{
    boost::shared_ptr<TArr> p_arr;
    boost::shared_ptr<T> p0, p1;
    typedef T x;
    pointers_array_test(const boost::shared_ptr<TArr>& p)
        : p_arr(p)
        , p0(p_arr[0].shared_from_this())
        , p1(p_arr[1].shared_from_this())
    {
    }
    template <int TEST_CASE_ID>  void test_value(no_args_tag)
    {
        BOOST_TEST(p0->value() == 0);
        BOOST_TEST(p1->value() == 0);
    }
    template <int TEST_CASE_ID>  void test_pointers()
    {
        BOOST_TEST(x::instances == 2);
        BOOST_TEST(p0 != p1);
        BOOST_TEST(share_ownership(p_arr, p0));
        BOOST_TEST(share_ownership(p_arr, p1));
        BOOST_TEST(share_ownership(p0, p1));
        BOOST_TEST(p1.use_count() == 3);
    }
    void reset_pointers()
    {
        p_arr.reset();
        p0.reset();
        p1.reset();
    }
};

template <class T> struct pointers_fixture<T[]>: public pointers_array_test<T, T[]>
{
    template <class Alloc, class Tag_maker>
    pointers_fixture(Alloc& alloc, Tag_maker tag_maker, no_args_tag tag_args)
        : pointers_array_test<T, T[]>(make_shared_ptr_array<T[]>(alloc, tag_maker, tag_args))
    {
    }
};

template <class T> struct pointers_fixture<T[2]>: public pointers_array_test<T, T[2]>
{
    template <class Alloc, class Tag_maker>
    pointers_fixture(Alloc& alloc, Tag_maker tag_maker, no_args_tag tag_args)
        : pointers_array_test<T, T[2]>(make_shared_ptr_size_array<T[2]>(alloc, tag_maker, tag_args))
    {
    }
};

template <class, class A> struct allocator_fixture
{
    A alloc;

    template <int TEST_CASE_ID>  void test_allocator()
    {
    }
};

template <class T> struct allocator_fixture
    < T
    , alloc_with_construct<void, typename boost::remove_extent<T>::type>
    >
{
    allocator_fixture()
        : alloc(alloc_log)
    {
    }
    template <int TEST_CASE_ID, class U>
    typename boost::enable_if<boost::is_array<U>, void>::type test_allocator_helper()
    {
         BOOST_TEST(alloc_log.construct_calls == 2);
    }
    template <int TEST_CASE_ID, class U>
    typename boost::disable_if<boost::is_array<U>, void>::type test_allocator_helper()
    {
         BOOST_TEST(alloc_log.construct_calls == 1);
    }
    template <int TEST_CASE_ID>
    void test_allocator()
    {
        test_allocator_helper<TEST_CASE_ID, T>();
    }
    allocator_log alloc_log;
    alloc_with_construct<void, typename boost::remove_extent<T>::type> alloc;
};

template <class T, class A, class tag_maker, class tag_args> struct fixture
    : allocator_fixture<T, A>
    , pointers_fixture<T>
{
    fixture()
        : pointers_fixture<T>(allocator_fixture<T, A>::alloc, tag_maker(), tag_args())
    {
    }
    template <int TEST_CASE_ID> void run_test()
    {
        pointers_fixture<T>::template test_value<TEST_CASE_ID>(tag_args());
        pointers_fixture<T>::template test_pointers<TEST_CASE_ID>();
        allocator_fixture<T, A>::template test_allocator<TEST_CASE_ID>();
        pointers_fixture<T>::reset_pointers();
        BOOST_TEST(boost::remove_extent<T>::type::instances == 0);
    }
};

template <class T> struct allocator_value_type_traits
{
#if !defined( BOOST_NO_CXX11_ALLOCATOR )
    typedef void type;
#else
    typedef T type;
#endif    
};

template 
    < int TEST_CASE_ID
    , class T
    , template <class, class> class A
    , class tag_maker
    , class tag_args
    >
void test()
{
    typedef typename boost::remove_extent<T>::type T_no_ext;
    typedef typename allocator_value_type_traits<T_no_ext>::type A_value_type;
    fixture<T, A<A_value_type, T_no_ext>, tag_maker, tag_args> f;
    f.template run_test<TEST_CASE_ID>();
}

template
    < int TEST_CASE_ID
    , class T
    , template <class> class A
    , class tag_maker
    , class tag_args
    >
void test()
{
    fixture<T, A<typename allocator_value_type_traits<T>::type>, tag_maker, tag_args> f;
    f.template run_test<TEST_CASE_ID>();
}

template <class, class> struct dummy_allocator {};

int main()
{
    test<1, with_public_ctor,    dummy_allocator, make_shared_tag, no_args_tag>();
    test<2, with_public_ctor[],  dummy_allocator, make_shared_tag, no_args_tag>();
    test<3, with_public_ctor[2], dummy_allocator, make_shared_tag, no_args_tag>();
    test<4, with_public_ctor,    alloc_without_construct, allocate_shared_tag, no_args_tag>();
    test<5, with_public_ctor[],  alloc_without_construct, allocate_shared_tag, no_args_tag>();
    test<6, with_public_ctor[2], alloc_without_construct, allocate_shared_tag, no_args_tag>();

#if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES) && !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)

    test<7, with_public_ctor, dummy_allocator,         make_shared_tag,     with_args_tag>();
    test<8, with_public_ctor, alloc_without_construct, allocate_shared_tag, with_args_tag>();

#if !defined( BOOST_NO_CXX11_ALLOCATOR )

    test<9,  with_public_ctor,     alloc_with_construct, allocate_shared_tag, with_args_tag>();
    test<10, with_private_ctor,    alloc_with_construct, allocate_shared_tag, with_args_tag>();
    test<11, with_private_ctor,    alloc_with_construct, allocate_shared_tag, no_args_tag>();
    test<12, with_private_ctor[],  alloc_with_construct, allocate_shared_tag, no_args_tag>();
    test<13, with_private_ctor[2], alloc_with_construct, allocate_shared_tag, no_args_tag>();
    test<14, with_public_ctor,     alloc_with_construct, allocate_shared_tag, no_args_tag>();
    test<15, with_public_ctor[],   alloc_with_construct, allocate_shared_tag, no_args_tag>();
    test<16, with_public_ctor[2],  alloc_with_construct, allocate_shared_tag, no_args_tag>();

#endif
#endif

    return boost::report_errors();
}

