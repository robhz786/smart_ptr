//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt

#include <boost/detail/lightweight_test.hpp>
#include <boost/make_shared.hpp>

class x
{
public:

    x(boost::shared_handle hdl) : m_hdl(hdl)
    {
    }

    ~x()
    {
        boost::shared_ptr<x> sp = m_hdl.forcibly_assemble_shared_ptr(this);
        BOOST_ASSERT( ! sp);
        
        boost::weak_ptr<x> wp  = m_hdl.forcibly_assemble_weak_ptr(this);
        BOOST_ASSERT( ! wp.lock());
    }
    
private:

    boost::shared_handle m_hdl;
};


int main()
{
    boost::make_shared<x>();
    return boost::report_errors();
}
