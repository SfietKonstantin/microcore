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

#ifndef MICROCORE_DATA_DATAOPERATOR_H
#define MICROCORE_DATA_DATAOPERATOR_H

#include "core/pipe.h"
#include "core/executor.h"
#include <set>

namespace microcore { namespace data {

template<class Data, class Request, class Result, class Error>
class DataOperator: public ::microcore::core::Executor<Error>
{
public:
    using Factory_t = ::microcore::core::IJobFactory<Request, Result, Error>;
    using OperatorFunction_t = std::function<void (Data &, Result &&)>;
    using Listener_t = typename ::microcore::core::Executor<Error>::IListener;
    using Executor_t = ::microcore::core::Executor<Error>;
    DataOperator(Data &data, std::unique_ptr<Factory_t> factory, OperatorFunction_t &&operatorFunction)
        : m_data(data), m_factory(std::move(factory)), m_operatorFunction(std::move(operatorFunction))
    {
    }
    bool start(Request &&request)
    {
        using namespace std::placeholders;
        if (!Executor_t::canStart()) {
            return false;
        }

        OnResult_t onResult {[this](Result &&result) {
            m_operatorFunction(m_data, std::move(result));
            finish();
        }};
        OnError_t onError {std::bind(&This_t::error, this, _1)};

        m_pipe.reset(new Pipe_t(*m_factory, std::move(onResult), std::move(onError)));
        m_pipe->send(std::move(request));

        Executor_t::doStart();
        return true;
    }
private:
    using This_t = DataOperator<Data, Request, Result, Error>;
    using Pipe_t = ::microcore::core::Pipe<Request, Result, Error>;
    using OnResult_t = typename ::microcore::core::IJob<Result, Error>::OnResult_t;
    using OnError_t = typename ::microcore::core::IJob<Result, Error>::OnError_t;
    void error(Error &&error)
    {
        m_pipe.reset();
        Executor_t::doError(std::move(error));
    }
    void finish()
    {
        m_pipe.reset();
        Executor_t::doFinish();
    }
    Data &m_data;
    std::unique_ptr<Factory_t> m_factory;
    OperatorFunction_t m_operatorFunction {};
    std::unique_ptr<Pipe_t>  m_pipe {};
};

}}

#endif // MICROCORE_DATA_DATAOPERATOR_H
