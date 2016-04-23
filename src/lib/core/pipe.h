#ifndef MICROCORE_CORE_PIPE_H
#define MICROCORE_CORE_PIPE_H

#include "ijobfactory.h"
#include <map>
#include <QtGlobal>

namespace microcore { namespace core {

/**
 * @brief A pipe for IJob
 *
 * This class is a component of a pipeline that will chain
 * execution of IJob, by passing results of a IJob as a request
 * to another IJob.
 *
 * Pipelines can be build with IJobFactory to create the
 * IJob to execute. A pipeline is build backward, by using
 * Pipe() to create the last pipe, and prepending pipes
 * via prepend().
 *
 * The last section of the pipeline uses callbacks functions
 * to indicate execution success or failure, just like IJob.
 * A pipeline will be successfully executed if all pipes
 * executed successfully. If one pipe failed, the whole pipeline
 * will fail, and the reported error will be the one of the failing
 * pipe.
 *
 * Pipe handle the lifecycle of the IJob it creates using
 * IJobFactory, but do not handle the lifecycle of the
 * IJobFactory. You will also need to handle the lifecycle
 * of Pipe.
 */
template<class Request, class Result, class Error>
class Pipe
{
public:
    /**
     * @brief Constructor
     *
     * Construct a pipe.
     *
     * This constructor takes success and error callbacks
     * as well as an IJobFactory. Usually it should be used
     * to create the last section of a pipeline.
     *
     * @param factory factory used to create IJob for the pipe.
     * @param onResult callback used to indicate if the pipe is successful.
     * @param onError callback used to indicate if the pipe has failed.
     */
    Pipe(const IJobFactory<Request, Result, Error> &factory,
         typename IJob<Result, Error>::OnResult_t onResult,
         typename IJob<Result, Error>::OnError_t onError)
        : m_factory {factory}
        , m_onResult {onResult}
        , m_onError {onError}
    {
    }
    /**
     * @brief Prepend a pipe
     *
     * Prepend a pipe to this pipe.
     *
     * The returned pipe will execute this pipe if it's execution
     * is successful. It will send an error to this pipe if it's
     * execution has failed.
     *
     * @param factory factory used to create IJob for the prepended pipe.
     * @return the prepended pipe.
     */
    template<class T>
    std::unique_ptr<Pipe<T, Request, Error>> prepend(const IJobFactory<T, Request, Error> &factory)
    {
        using namespace std::placeholders;
        OnResult_t<Request> onResult {std::bind(&Pipe<Request, Result, Error>::send, this, _1)};
        OnError_t onError {std::bind(&Pipe<Request, Result, Error>::sendError, this, _1)};
        return std::unique_ptr<Pipe<T, Request, Error>>(new Pipe<T, Request, Error>(factory, std::move(onResult), std::move(onError)));
    }
    /**
     * @brief Execute this pipe
     *
     * Execute this pipe using the provided request.
     *
     * If this pipe is part of a pipeline, the whole pipeline will be
     * executed.
     *
     * @param request request used to execute this pipe.
     */
    void send(Request &&request)
    {
        Q_ASSERT(!m_job);
        using namespace std::placeholders;
        OnResult_t<Result> onResult {std::bind(&Pipe<Request, Result, Error>::onResult, this, _1)};
        OnError_t onError {std::bind(&Pipe<Request, Result, Error>::sendError, this, _1)};
        m_job = m_factory.create(std::move(request));
        m_job->execute(std::move(onResult), std::move(onError));
    }
    /**
     * @brief Send an error through this pipe
     *
     * Send the provided error through this pipe.
     *
     * @param error error to be sent.
     */
    void sendError(Error &&error)
    {
        m_onError(std::move(error));
    }
private:
    template<class T>
    using OnResult_t = typename IJob<T, Error>::OnResult_t;
    using OnError_t = typename IJob<Result, Error>::OnError_t;
    void onResult(Result &&result)
    {
        m_onResult(std::move(result));
    }
    const IJobFactory<Request, Result, Error> &m_factory;
    std::unique_ptr<IJob<Result, Error>> m_job {};
    OnResult_t<Result> m_onResult {};
    OnError_t m_onError {};
};

}}

#endif // MICROCORE_CORE_PIPE_H
