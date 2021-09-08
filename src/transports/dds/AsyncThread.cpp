/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FASTRPC is licensed to you under the terms described in the
 * FASTRPC_LICENSE file included in this distribution.
 *
 *************************************************************************/
#include <config.h>

#if RPC_WITH_RTIDDS

#include "AsyncThread.h"
#include <transports/dds/DDSAsyncTask.h>
#include <transports/dds/components/ProxyProcedureEndpoint.h>
#include <utils/Typedefs.h>
#include <exceptions/ServerTimeoutException.h>
#include <algorithm>
#include <stdio.h>

static const char* const CLASS_NAME = "eprosima::rpc::transport::dds::AsyncThread";

using namespace eprosima::rpc;
using namespace ::transport::dds;
using namespace ::exception;

bool asyncListSort(AsyncListPair i, AsyncListPair j)
{
    return (i.first < j.first);
}

AsyncThread::AsyncThread() : m_guardWaitSet(NULL), m_exit(0)
{
    m_timeout.sec = DDS_DURATION_INFINITE_SEC;
    m_timeout.nanosec = DDS_DURATION_INFINITE_NSEC;
}

int AsyncThread::init()
{
    const char* const METHOD_NAME = "init";

    m_guardWaitSet = new DDS::GuardCondition();

    if(m_guardWaitSet != NULL)
    {
        m_waitSet = new DDS::WaitSet();

        if(m_waitSet != NULL)
        {
            m_mutex =  new std::mutex();

            if(m_mutex != NULL)
            {
                try
                {
                    m_thread = std::thread(&AsyncThread::run, this);
                    return 0;
                }
                catch(std::exception ex)
                {
                    printf("ERROR<%s::%s>: Cannot create thread\n", CLASS_NAME, METHOD_NAME);
                }

                delete m_mutex;
            }
            else
            {
                printf("ERROR<%s::%s>: Cannot create mutex\n", CLASS_NAME, METHOD_NAME);
            }

            delete m_waitSet;
        }
        else
        {
            printf("ERROR<%s::%s>: Cannot create waitset\n", CLASS_NAME, METHOD_NAME);
        }

        delete m_guardWaitSet;
    }
    else
    {
        printf("ERROR<%s::%s>: Cannot create guard condition\n", CLASS_NAME, METHOD_NAME);
    }

    return -1;
}

void AsyncThread::exit()
{
    m_exit = 1;
    m_guardWaitSet->set_trigger_value(1);
    m_thread.join();
    delete m_mutex;
    delete m_waitSet;
    delete m_guardWaitSet;
}

void AsyncThread::run()
{
    const char* const METHOD_NAME = "run";
    DDS::Duration_t timeout = {DDS_DURATION_INFINITE_SEC, DDS_DURATION_INFINITE_NSEC};

    if(m_waitSet->attach_condition(m_guardWaitSet) == DDS_RETCODE_OK)
    {
        while(m_exit == 0)
        {
            DDS::ReturnCode_t retCode;
            DDSConditionSeq conds;
            AsyncVector::iterator it;

            auto init_wait_moment = std::chrono::system_clock::now();

            retCode = m_waitSet->wait(conds, timeout);

            auto final_wait_moment = std::chrono::system_clock::now();

            std::lock_guard<std::mutex> lock_guard(*m_mutex);

            // Update timeouts.
            auto td = final_wait_moment - init_wait_moment;

            for(it = m_vector.begin(); it != m_vector.end(); ++it)
            {
                if(it->first >= td) {
                    it->first = it->first - td;
                }
                else {
                    it->first = std::chrono::milliseconds(0);
                }
            }

            if(retCode == DDS_RETCODE_OK)
            {
                for(int i = 0; i < conds.length(); ++i)
                {
                    if(conds[i] == m_guardWaitSet)
                    {
                        m_guardWaitSet->set_trigger_value(BOOLEAN_FALSE);
                        // Signal the add function.
                        m_cond_wake_up.notify_one();
                    }
                    else
                    {
                        it = m_vector.begin();

                        for(; it != m_vector.end(); ++it)
                        {
                            if(conds[i] == it->second.first)
                            {
                                break;
                            }
                        }

                        if(it != m_vector.end())
                        {
                            it->second.second->execute(it->second.first);
                            m_waitSet->detach_condition(it->second.first);
                            it->second.second->getProcedureEndpoint()->freeQuery(it->second.first);
                            delete it->second.second;
                            m_vector.erase(it);
                        }
                    }
                }
            }
            else if(retCode == DDS_RETCODE_TIMEOUT)
            {
                printf("WARNING <%s::%s>: Wait timeout. Retrying.\n", CLASS_NAME, METHOD_NAME);

                // Find the out of life tasks.
                it = m_vector.begin();
                while(it != m_vector.end())
                {
                    if(it->first == std::chrono::milliseconds(0))
                    {
                        it->second.second->on_exception(ServerTimeoutException("Asynchronous task exceed the time to wait the server's reply"));
                        m_waitSet->detach_condition(it->second.first);
                        it->second.second->getProcedureEndpoint()->freeQuery(it->second.first);
                        delete it->second.second;
                        it = m_vector.erase(it);
                    }
                    else
                    {
                        ++it;
                    }
                }
            }

            if(m_vector.empty())
            {
                timeout = m_timeout;
                m_timeout.sec = DDS_DURATION_INFINITE_SEC;
                m_timeout.nanosec = DDS_DURATION_INFINITE_NSEC;
            }
            else
            {
                DDS_TIMEOUT(auxTimeout, m_vector.begin()->first);

                if((auxTimeout.sec < m_timeout.sec) ||
                        (auxTimeout.sec == m_timeout.sec && auxTimeout.nanosec < m_timeout.nanosec))
                {
                    timeout = auxTimeout;
                }
                else
                    timeout = m_timeout;
            }
        }

        std::lock_guard<std::mutex> lock_guard(*m_mutex);

        m_waitSet->detach_condition(m_guardWaitSet);

        if(m_vector.size() > 0)
            printf("ERROR<%s::%s>: There must not be any task in the thread's vector\n", CLASS_NAME, METHOD_NAME);
    }
    else
    {
        printf("ERROR<%s::%s>: Cannot attach the guard condition to waitset\n", CLASS_NAME, METHOD_NAME);
    }

    return;
}

int AsyncThread::addTask(DDS::QueryCondition *query, DDSAsyncTask *task, long timeout)
{
    const char* const METHOD_NAME = "addTask";
    int returnedValue = -1;

    if(query != NULL && task != NULL)
    {
        AsyncTaskPair tpair(query, task);
        AsyncListPair lpair(std::chrono::milliseconds(timeout), tpair);

        std::unique_lock<std::mutex> lock(*m_mutex);

        // Set timeout like new for next calculation.
        DDS_TIMEOUT_SET(m_timeout, lpair.first);

        // Wake up the waitSet.
        m_guardWaitSet->set_trigger_value(BOOLEAN_TRUE);
        m_cond_wake_up.wait(lock);

        // Add new task and sort vector.
        m_vector.push_back(lpair);
        std::sort(m_vector.begin(), m_vector.end(), asyncListSort);

        // Attach query.
        if(m_waitSet->attach_condition(query) == DDS_RETCODE_OK)
        {
            returnedValue = 0;
        }
        else
        {
            printf("ERROR<%s::%s>: Cannot attach query condition\n", CLASS_NAME, METHOD_NAME);
        }
    }
    else
    {
        printf("ERROR<%s::%s>: Bad parameters\n", CLASS_NAME, METHOD_NAME);
    }

    return returnedValue;
}

void AsyncThread::deleteAssociatedAsyncTasks(ProxyProcedureEndpoint *pe)
{
    const char* const METHOD_NAME = "deleteAssociatedAsyncTasks";

    if(pe != NULL)
    {
        std::unique_lock<std::mutex> lock(*m_mutex);

        // Wake up the waitSet.
        m_guardWaitSet->set_trigger_value(BOOLEAN_TRUE);
        m_cond_wake_up.wait(lock);

        // Remove tasks.
        AsyncVector::iterator it = m_vector.begin();

        while(it != m_vector.end())
        {
            if(it->second.second->getProcedureEndpoint() == pe)
            {
                m_waitSet->detach_condition(it->second.first);
                it->second.second->getProcedureEndpoint()->freeQuery(it->second.first);
                delete it->second.second;
                it = m_vector.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
    else
    {
        printf("ERROR<%s::%s>: Bad parameters\n", CLASS_NAME, METHOD_NAME);
    }
}

#endif // RPC_WITH_RTIDDS
