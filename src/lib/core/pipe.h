#ifndef MICROCORE_CORE_PIPE_H
#define MICROCORE_CORE_PIPE_H

#include "ijobfactory.h"
#include <map>
#include <QtGlobal>

namespace microcore { namespace core {

template<class Request, class Result, class Error>
class Pipe
{
public:
    Pipe(IJobFactory<Request, Result, Error> &factory, typename IJob<Result, Error>::ICallback &callback)
        : m_factory(factory), m_internalCallback(*this), m_callback(*this, callback)
    {
    }
    template<class T>
    std::unique_ptr<Pipe<T, Request, Error>> prepend(IJobFactory<T, Request, Error> &factory)
    {
        return std::unique_ptr<Pipe<T, Request, Error>>(new Pipe<T, Request, Error>(factory, m_internalCallback));
    }
    void send(Request &&request)
    {
        Q_ASSERT(!m_job);
        m_job = m_factory.create(std::move(request));
        m_job->execute(m_callback);
    }
    void sendError(Error &&error)
    {
        m_callback.onError(std::move(error));
    }
private:
    class InternalCallback: public IJob<Request, Error>::ICallback
    {
    public:
        explicit InternalCallback(Pipe<Request, Result, Error> &parent)
            : m_parent(parent)
        {
        }
        void onResult(Request &&result) override
        {
            m_parent.send(std::move(result));
        }
        void onError(Error &&error) override
        {
            m_parent.sendError(std::move(error));
        }
    private:
        Pipe<Request, Result, Error> &m_parent;
    };
    class CallbackAdaptor: public IJob<Result, Error>::ICallback
    {
    public:
        explicit CallbackAdaptor(Pipe<Request, Result, Error> &parent,
                                 typename IJob<Result, Error>::ICallback &callback)
            : m_parent(parent)
            , m_callback(callback)
        {
        }
        void onResult(Result &&result) override
        {
            m_parent.m_job.reset();
            m_callback.onResult(std::move(result));
        }
        void onError(Error &&error) override
        {
            m_parent.m_job.reset();
            m_callback.onError(std::move(error));
        }
    private:
        Pipe<Request, Result, Error> &m_parent;
        typename IJob<Result, Error>::ICallback &m_callback;
    };
    IJobFactory<Request, Result, Error> &m_factory;
    std::unique_ptr<IJob<Result, Error>> m_job {};
    InternalCallback m_internalCallback;
    CallbackAdaptor m_callback;
};

}}

#endif // MICROCORE_CORE_PIPE_H
