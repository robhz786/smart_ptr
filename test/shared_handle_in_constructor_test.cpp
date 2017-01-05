//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt

#include <boost/detail/lightweight_test.hpp>
#include <boost/make_shared.hpp>

class x
{
public:

    x(boost::shared_handle hdl, boost::shared_ptr<x>& sp, bool& destroyed)
        : m_destroyed(destroyed)
    {
        m_destroyed = false;
        sp = hdl.forcibly_assemble_shared_ptr(this);
        BOOST_TEST(sp.use_count() > 1);
    }

    x(boost::shared_handle hdl, boost::weak_ptr<x>& wp, bool& destroyed)
        : m_destroyed(destroyed)
    {
        m_destroyed = false;
        wp = hdl.forcibly_assemble_weak_ptr(this);
        BOOST_TEST(wp.lock());
    }

    ~x()
    {
        m_destroyed = true;
    }
    
private:

    bool& m_destroyed;
};


int main()
{
    {
        bool destroyed = false;
        {
            boost::shared_ptr<x> spx1;
            boost::shared_ptr<x> spx2 = boost::make_shared<x>(spx1, destroyed);

            BOOST_TEST(spx2.use_count() == 2);
            BOOST_TEST(spx1.get() == spx2.get());
            BOOST_TEST(!(spx1 < spx2) && !(spx2 < spx1));
        }
        BOOST_TEST(destroyed);
    }

    {
        bool destroyed = false;
        {

            boost::weak_ptr<x> wpx1;
            boost::shared_ptr<x> spx1;
            boost::shared_ptr<x> spx2 = boost::make_shared<x>(wpx1, destroyed);
        
            BOOST_TEST(spx2.use_count() == 1);
            spx1 = wpx1.lock();

            BOOST_TEST(spx2.use_count() == 2);
            BOOST_TEST(spx1.get() == spx2.get());
            BOOST_TEST(!(spx1 < spx2) && !(spx2 < spx1));
        }
        BOOST_TEST(destroyed);
    }

    return boost::report_errors();
}
