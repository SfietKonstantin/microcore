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
    Pipe(IJobFactory<Request, Result, Error> &factory,
         typename IJob<Result, Error>::OnResult_t onResult,
         typename IJob<Result, Error>::OnError_t onError)
        : m_factory {factory}
        , m_onResult {onResult}
        , m_onError {onError}
    {
    }
    template<class T>
    std::unique_ptr<Pipe<T, Request, Error>> prepend(IJobFactory<T, Request, Error> &factory)
    {
        using namespace std::placeholders;
        auto onResult {std::bind(&Pipe<Request, Result, Error>::send, this, _1)};
        auto onError {std::bind(&Pipe<Request, Result, Error>::sendError, this, _1)};
        return std::unique_ptr<Pipe<T, Request, Error>>(new Pipe<T, Request, Error>(factory, std::move(onResult), std::move(onError)));
    }
    void send(Request &&request)
    {
        Q_ASSERT(!m_job);
        using namespace std::placeholders;
        auto onResult {std::bind(&Pipe<Request, Result, Error>::onResult, this, _1)};
        auto onError {std::bind(&Pipe<Request, Result, Error>::sendError, this, _1)};
        m_job = m_factory.create(std::move(request));
        m_job->execute(std::move(onResult), std::move(onError));
    }
    void sendError(Error &&error)
    {
        m_job.reset();
        m_onError(std::move(error));
    }
private:
    void onResult(Result &&result)
    {
        m_job.reset();
        m_onResult(std::move(result));
    }

    IJobFactory<Request, Result, Error> &m_factory;
    std::unique_ptr<IJob<Result, Error>> m_job {};
    typename IJob<Result, Error>::OnResult_t m_onResult {};
    typename IJob<Result, Error>::OnError_t m_onError {};
};

}}

#endif // MICROCORE_CORE_PIPE_H
