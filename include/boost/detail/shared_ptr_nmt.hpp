#ifndef BOOST_DETAIL_SHARED_PTR_NMT_HPP_INCLUDED
#define BOOST_DETAIL_SHARED_PTR_NMT_HPP_INCLUDED

//
//  detail/shared_ptr_nmt.hpp - shared_ptr.hpp without member templates
//
//  (C) Copyright Greg Colvin and Beman Dawes 1998, 1999.
//  Copyright (c) 2001, 2002 Peter Dimov
//
//  Permission to copy, use, modify, sell and distribute this software
//  is granted provided this copyright notice appears in all copies.
//  This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.
//
//  See http://www.boost.org for most recent version including documentation.
//

#include <boost/assert.hpp>
#include <boost/checked_delete.hpp>
#include <boost/detail/atomic_count.hpp>

#ifndef BOOST_NO_AUTO_PTR
#include <memory>             // for std::auto_ptr
#endif

#include <algorithm>          // for std::swap
#include <functional>         // for std::less

namespace boost
{

template<class T> class shared_ptr
{
private:

    typedef detail::atomic_count count_type;

public:

    typedef T element_type;

    explicit shared_ptr(T * p = 0): px(p)
    {
        try  // prevent leak if new throws
        {
            pn = new count_type(1);
        }
        catch(...)
        {
            checked_delete(p);
            throw;
        } 
    }

    ~shared_ptr()
    {
        if(--*pn == 0)
        {
            checked_delete(px);
            delete pn;
        }
    }

    shared_ptr(shared_ptr const & r): px(r.px)  // never throws
    {
        pn = r.pn;
        ++*pn;
    }

    shared_ptr & operator=(shared_ptr const & r)
    {
        shared_ptr(r).swap(*this);
        return *this;
    }

#ifndef BOOST_NO_AUTO_PTR

    explicit shared_ptr(std::auto_ptr<T> & r)
    { 
        pn = new count_type(1); // may throw
        px = r.release(); // fix: moved here to stop leak if new throws
    } 

    shared_ptr & operator=(std::auto_ptr<T> & r)
    {
        shared_ptr(r).swap(*this);
        return *this;
    }

#endif

    void reset(T * p = 0)
    {
        shared_ptr(p).swap(*this);
    }

    T & operator*() const  // never throws
    {
        BOOST_ASSERT(px != 0);
        return *px;
    }

    T * operator->() const  // never throws
    {
        BOOST_ASSERT(px != 0);
        return px;
    }

    T * get() const  // never throws
    {
        return px;
    }

    long use_count() const  // never throws
    {
        return *pn;
    }

    bool unique() const  // never throws
    {
        return *pn == 1;
    }
    
    void swap(shared_ptr<T> & other)  // never throws
    {
        std::swap(px, other.px);
        std::swap(pn, other.pn);
    }

private:

    T * px;            // contained pointer
    count_type * pn;   // ptr to reference counter
};

template<class T, class U> inline bool operator==(shared_ptr<T> const & a, shared_ptr<U> const & b)
{
    return a.get() == b.get();
}

template<class T, class U> inline bool operator!=(shared_ptr<T> const & a, shared_ptr<U> const & b)
{
    return a.get() != b.get();
}

template<class T> inline bool operator<(shared_ptr<T> const & a, shared_ptr<T> const & b)
{
    return std::less<T*>()(a.get(), b.get());
}

template<class T> void swap(shared_ptr<T> & a, shared_ptr<T> & b)
{
    a.swap(b);
}

// get_pointer() enables boost::mem_fn to recognize shared_ptr

template<class T> inline T * get_pointer(shared_ptr<T> const & p)
{
    return p.get();
}

} // namespace boost

#endif  // #ifndef BOOST_DETAIL_SHARED_PTR_NMT_HPP_INCLUDED
