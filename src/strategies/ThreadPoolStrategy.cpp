/*************************************************************************
* Copyright (c) 2013 eProsima. All rights reserved.
*
* This copy of FASTRPC is licensed to you under the terms described in the
* FASTRPC_LICENSE file included in this distribution.
*
*************************************************************************/

#include <strategies/ThreadPoolStrategy.h>
#include "ServerStrategyImpl.h"
#include <transports/ServerTransport.h>

#include <boost/config/user.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#include <boost/bind/bind.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>

static const char* const CLASS_NAME = "ThreadPoolStrategy";

namespace eprosima {
namespace rpc {
namespace strategy {

class ThreadPoolStrategyJob
{
public:

    ThreadPoolStrategyJob(
            boost::function<void()> callback)
        : m_callback(callback)
    {
    }

    void run()
    {
        m_callback();
    }

private:

    boost::function<void()> m_callback;
};

class ThreadPoolStrategyImpl : public ServerStrategyImpl
{
public:

    ThreadPoolStrategyImpl(
            unsigned int threadCount)
    {
        m_pool = new boost::asio::thread_pool(threadCount);
    }

    virtual ~ThreadPoolStrategyImpl()
    {
        delete m_pool;
    }

    boost::asio::thread_pool* getPool()
    {
        return m_pool;
    }

    void schedule(
            boost::function<void()> callback)
    {
        boost::shared_ptr<ThreadPoolStrategyJob> job(new ThreadPoolStrategyJob(callback));
        boost::asio::post(*m_pool, boost::bind(&ThreadPoolStrategyJob::run, job));
    }

private:

    boost::asio::thread_pool * m_pool;
};

} // namespace strategy
} // namespace rpc
} // namespace eprosima

using namespace eprosima::rpc;
using namespace ::strategy;
using namespace ::transport;

ThreadPoolStrategy::ThreadPoolStrategy(
        unsigned int threadCount)
    : m_impl(NULL)
{
    m_impl = new ThreadPoolStrategyImpl(threadCount);
}

ThreadPoolStrategy::~ThreadPoolStrategy()
{
    if (m_impl != NULL)
    {
        delete m_impl;
    }
}

ServerStrategyImpl* ThreadPoolStrategy::getImpl()
{
    return m_impl;
}

