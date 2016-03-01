/*
 * Copyright (C) 2016 Lucien XU <sfietkonstantin@free.fr>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * "Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * The names of its contributors may not be used to endorse or promote
 *     products derived from this software without specific prior written
 *     permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 */

#ifndef MICROCORE_CORE_DATASOURCE_H
#define MICROCORE_CORE_DATASOURCE_H

#include "ijobfactory.h"
#include "globals.h"
#include <memory>
#include <map>
#include <utility>

namespace microcore { namespace core {

template<class Container, class Request, class ResultIr, class Result, class Error>
class DataSource
{
public:
    DataSource(std::unique_ptr<IJobFactory<Request, ResultIr, Error>> requestToIrFactory,
               std::unique_ptr<IJobFactory<ResultIr, Result, Error>> irToResultFactory)
        : m_requestToIrFactory(std::move(requestToIrFactory))
        , m_irToResultFactory(std::move(irToResultFactory))
    {
    }
    DISABLE_COPY_DISABLE_MOVE(DataSource);
    void applyRequest(Container &container, Request &&request)
    {
        cancelRequest(container);
        m_jobs.emplace(&container, JobExecutor(container, *this, std::move(request)));
    }
    void cancelRequest(Container &container)
    {
        m_jobs.erase(&container);
    }
private:
    class JobExecutor
    {
    public:
        explicit JobExecutor(Container &container,
                             DataSource &dataSource,
                             Request &&request)
            : m_container(container)
            , m_dataSource(dataSource)
            , m_requestToIrCallback(*this)
            , m_irToResultCallback(*this)
        {
            m_requestToIrJob = m_dataSource.m_requestToIrFactory->create(std::move(request));
            m_requestToIrJob->execute(m_requestToIrCallback);
        }
        DISABLE_COPY_DEFAULT_MOVE(JobExecutor);
    private:
        class RequestToIrCallback: public IJob<ResultIr, Error>::ICallback
        {
        public:
            RequestToIrCallback(JobExecutor &executor)
                : m_executor(executor)
            {
            }
            DISABLE_COPY_DEFAULT_MOVE(RequestToIrCallback);
            void onResult(ResultIr &&result) override
            {
                m_executor.m_irToResultJob = m_executor.m_dataSource.m_irToResultFactory->create(std::move(result));
                m_executor.m_irToResultJob->execute(m_executor.m_irToResultCallback);
            }
            void onError(Error &&error) override
            {
                m_executor.onError(std::move(error));
            }
        private:
            JobExecutor &m_executor;
        };
        class IrToResultCallback: public IJob<Result, Error>::ICallback
        {
        public:
            IrToResultCallback(JobExecutor &executor)
                : m_executor(executor)
            {
            }
            DISABLE_COPY_DEFAULT_MOVE(IrToResultCallback);
            void onResult(Result &&result) override
            {
                m_executor.onResult(std::move(result));
            }
            void onError(Error &&error) override
            {
                m_executor.onError(std::move(error));
            }
        private:
            JobExecutor &m_executor;
        };
        void onResult(Result result)
        {
            m_container.setResult(result);
            m_dataSource.cancelRequest(m_container);
        }
        void onError(Error error)
        {
            m_container.setError(error);
            m_dataSource.cancelRequest(m_container);
        }
        Container &m_container;
        DataSource &m_dataSource;
        std::unique_ptr<IJob<ResultIr, Error>> m_requestToIrJob {};
        RequestToIrCallback m_requestToIrCallback;
        std::unique_ptr<IJob<Result, Error>> m_irToResultJob {};
        IrToResultCallback m_irToResultCallback;
    };
    std::unique_ptr<IJobFactory<Request, ResultIr, Error>> m_requestToIrFactory {};
    std::unique_ptr<IJobFactory<ResultIr, Result, Error>> m_irToResultFactory {};
    std::map<Container *, JobExecutor> m_jobs {};
};

}}

#endif // MICROCORE_CORE_DATASOURCE_H
