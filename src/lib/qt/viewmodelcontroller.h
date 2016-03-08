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

#ifndef MICROCORE_QT_VIEWMODELCONTROLLER_HPP
#define MICROCORE_QT_VIEWMODELCONTROLLER_HPP

#include "iviewmodelcontroller.h"
#include "core/executor.h"
#include "error/error.h"
#include "data/modeloperator.h"

namespace microcore { namespace qt {

template<class Model>
class ViewModelController
        : public IViewModelController
        , private ::microcore::core::Executor< ::microcore::error::Error>::IListener
{
public:
    explicit ViewModelController(QObject *parent = nullptr)
        : IViewModelController(parent)
    {
    }
    virtual Model & model() = 0;
    void classBegin() override {}
    void componentComplete() override {}
    Status status() const override final
    {
        return m_status;
    }
    QString errorMessage() const override final
    {
        return m_errorMessage;
    }
protected:
    using Error_t = ::microcore::error::Error;
    using Executor_t = ::microcore::core::Executor<Error_t>;
    void addExecutor(std::unique_ptr<Executor_t> executor)
    {
        m_executors.emplace(executor.get(), executor);
    }
    template<class Executor, class Request>
    bool start(Executor &executor, Request &&request)
    {
        if (m_executors.find(&executor) == std::end(m_executors)) {
            return false;
        }

        if (m_status == Busy) {
            return false;
        }

        executor.start(std::move(request));
        return true;
    }
private:
    void onStart() override final
    {
        Q_ASSERT(m_status != Busy);
        setStatus(Busy);
    }
    void onFinish() override final
    {
        Q_ASSERT(m_status == Busy);
        setStatus(Idle);
    }
    void onError(const Error_t &error) override final
    {
        Q_ASSERT(m_status == Busy);
        setStatus(Error);
        if (m_errorMessage != error.message()) {
            m_errorMessage = error.message();
            Q_EMIT errorMessageChanged();
        }
    }
    void onInvalidation(Executor_t &source) override final
    {
        m_executors.erase(&source);
    }
    void setStatus(Status status)
    {
        if (m_status != status) {
            m_status = status;
            Q_EMIT statusChanged();
        }
    }
    Status m_status {Idle};
    QString m_errorMessage {};
    std::map<Executor_t *, std::unique_ptr<Executor_t>> m_executors {};
};

}}

#endif // MICROCORE_QT_VIEWMODELCONTROLLER_HPP
