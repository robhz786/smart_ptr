#ifndef BOOST_SMART_PTR_SHARED_HANDLE_HPP_INCLUDED
#define BOOST_SMART_PTR_SHARED_HANDLE_HPP_INCLUDED

//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt

#include <boost/weak_ptr.hpp>

namespace boost {

namespace detail {
class shared_initializer;
}

class shared_handle
{
public:
    shared_handle( const shared_handle& other ) BOOST_NOEXCEPT : pn( other.pn )
    {
    }

    template< typename T >
    boost::weak_ptr< T > forcibly_assemble_weak_ptr( T* ptr )  const BOOST_NOEXCEPT
    {
        boost::weak_ptr< T > wptr;
        if(ptr != 0) 
        {
            wptr.pn = pn;
            wptr.px = ptr;
        }
        return wptr;
    }

    template< typename T >
    boost::shared_ptr< T > forcibly_assemble_shared_ptr( T* ptr ) const BOOST_NOEXCEPT
    {
        boost::shared_ptr< T > sptr;
        if(ptr != 0) 
        {
            boost::detail::shared_count sc(pn, boost::detail::sp_nothrow_tag());
            if( ! sc.empty())
            {
                sptr.pn.swap( sc );
                sptr.px = ptr;
            }
        }
        return sptr;
    }

private:

    friend class boost::detail::shared_initializer;

    template< class Y >
    shared_handle( const boost::shared_ptr< Y >& sp ) BOOST_NOEXCEPT : pn(sp.pn)
    {
    }

    shared_handle()
    {
    }

    shared_handle& operator=( const shared_handle& other ) BOOST_NOEXCEPT
    {
        pn = other.pn;
        return * this;
    }

    boost::detail::weak_count pn;
};

} // namespace boost

#endif // #ifndef BOOST_SMART_PTR_SHARED_HANDLE_HPP_INCLUDED

