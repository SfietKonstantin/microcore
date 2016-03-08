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

#ifndef MICROCORE_DATA_MODELOPERATOR_H
#define MICROCORE_DATA_MODELOPERATOR_H

#include "core/pipe.h"
#include <functional>
#include <set>

namespace microcore { namespace data {

template<class Model, class Request, class Error, class Operator>
class IModelOperatorListener;

template<class Model, class Request, class Result, class Error, class Operator>
class ModelOperator
{
public:
    using Factory_t = ::microcore::core::IJobFactory<Request, Result, Error>;
    using Listener_t = IModelOperatorListener<Model, Request, Error, Operator>;
    using OperatorFunction_t = std::function<void (Model &, Result &&)>;
    using StartFunction_t = std::function<void (Listener_t &)>;
    using FinishFunction_t = std::function<void (Listener_t &)>;
    using ErrorFunction_t = std::function<void (Listener_t &, const Error &)>;
    using InvalidationFunction_t = std::function<void (Listener_t &)>;
    ModelOperator(Model &model, std::unique_ptr<Factory_t> factory, OperatorFunction_t &&operatorFunction,
                  StartFunction_t &&startFunction, FinishFunction_t &&finishFunction,
                  ErrorFunction_t &&errorFunction, InvalidationFunction_t &&invalidationFunction)
        : m_model(model), m_factory(std::move(factory)), m_operatorFunction(std::move(operatorFunction))
        , m_startFunction(std::move(startFunction)), m_finishFunction(std::move(finishFunction))
        , m_errorFunction(std::move(errorFunction)), m_invalidationFunction(std::move(invalidationFunction))
    {
    }
    virtual ~ModelOperator()
    {
        using namespace std::placeholders;
        std::for_each(std::begin(m_listeners), std::end(m_listeners), [this](Listener_t *listener) {
            m_invalidationFunction(*listener);
        });
    }
    bool busy() const
    {
        return m_pipe;
    }
    bool start(Request &&request)
    {
        using namespace std::placeholders;
        if (m_pipe) {
            return false;
        }

        m_error = Error();

        OnResult_t onResult {[this](Result &&result) {
            m_operatorFunction(m_model, std::move(result));
            finish();
        }};
        OnError_t onError {std::bind(&This_t::error, this, _1)};

        m_pipe.reset(new Pipe_t(*m_factory, std::move(onResult), std::move(onError)));
        m_pipe->send(std::move(request));
        std::for_each(std::begin(m_listeners), std::end(m_listeners), [this](Listener_t *listener) {
            m_startFunction(*listener);
        });
        return true;
    }
    void error(Error &&error)
    {
        using namespace std::placeholders;
        m_pipe.reset();
        m_error = std::move(error);
        std::for_each(std::begin(m_listeners), std::end(m_listeners), [this](Listener_t *listener) {
            m_errorFunction(*listener, m_error);
        });
    }
    void finish()
    {
        using namespace std::placeholders;
        m_pipe.reset();
        m_error = Error();
        std::for_each(std::begin(m_listeners), std::end(m_listeners), [this](Listener_t *listener) {
            m_finishFunction(*listener);
        });
    }
    void addListener(Listener_t &listener)
    {
        m_listeners.insert(&listener);

        if (m_pipe) {
            m_startFunction(listener);
        } else if (!m_error.empty()){
            m_errorFunction(listener, m_error);
        }
    }
    void removeListener(Listener_t &listener)
    {
        m_listeners.erase(&listener);
    }
private:
    using This_t = ModelOperator<Model, Request, Result, Error, Operator>;
    using Pipe_t = ::microcore::core::Pipe<Request, Result, Error>;
    using OnResult_t = typename ::microcore::core::IJob<Result, Error>::OnResult_t;
    using OnError_t = typename ::microcore::core::IJob<Result, Error>::OnError_t;

    Model &m_model;
    std::unique_ptr<Factory_t> m_factory;
    OperatorFunction_t m_operatorFunction {};
    StartFunction_t m_startFunction {};
    FinishFunction_t m_finishFunction {};
    ErrorFunction_t m_errorFunction {};
    InvalidationFunction_t m_invalidationFunction {};
    std::set<Listener_t *> m_listeners {};
    std::unique_ptr<Pipe_t>  m_pipe {};
    Error m_error {};
};

template<class Model, class Request, class Error>
class ModelAppender;

template<class Model, class Request, class Error>
class IModelOperatorListener<Model, Request, Error, ModelAppender<Model, Request, Error>>
{
public:
    virtual ~IModelOperatorListener() {}
    virtual void onAppendStart() = 0;
    virtual void onAppendFinish() = 0;
    virtual void onAppendError(const Error &error) = 0;
    virtual void onAppendInvalidation() = 0;
};

template<class Model, class Request, class Error>
class ModelAppender: public ModelOperator<Model, Request, typename Model::SourceItems_t, Error, ModelAppender<Model, Request, Error>>
{
public:
    using Result_t = typename Model::SourceItems_t;
    using Factory_t = ::microcore::core::IJobFactory<Request, Result_t, Error>;
    using Listener_t = IModelOperatorListener<Model, Request, Error, ModelAppender<Model, Request, Error>>;
    ModelAppender(Model &model, std::unique_ptr<Factory_t> factory)
        : Parent_t(model, std::move(factory), OperatorFunction_t(&Model::append),
                   StartFunction_t(&Listener_t::onAppendStart), FinishFunction_t(&Listener_t::onAppendFinish),
                   ErrorFunction_t(&Listener_t::onAppendError), InvalidationFunction_t(&Listener_t::onAppendInvalidation))
    {
    }
private:
    using Parent_t = ModelOperator<Model, Request, Result_t, Error, ModelAppender<Model, Request, Error>>;
    using OperatorFunction_t = std::function<void (Model &, Result_t &&)>;
    using StartFunction_t = std::function<void (Listener_t &)>;
    using FinishFunction_t = std::function<void (Listener_t &)>;
    using ErrorFunction_t = std::function<void (Listener_t &, const Error &)>;
    using InvalidationFunction_t = std::function<void (Listener_t &)>;
};

}}

#endif // MICROCORE_DATA_MODELOPERATOR_H
