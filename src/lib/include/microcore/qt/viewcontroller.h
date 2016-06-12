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

#ifndef MICROCORE_QT_VIEWCONTROLLER_H
#define MICROCORE_QT_VIEWCONTROLLER_H

#include <microcore/core/globals.h>
#include <QtCore/QObject>
#include <QtQml/QQmlParserStatus>
#include <memory>
#include <microcore/core/executor.h>
#include <microcore/error/error.h>

namespace microcore { namespace qt {

class ViewController
        : public QObject, public QQmlParserStatus
        , private ::microcore::core::Executor< ::microcore::error::Error>::IListener
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    Q_ENUMS(Status)
public:
    enum Status
    {
        Idle,
        Busy,
        Error
    };
    DISABLE_COPY_DISABLE_MOVE(ViewController);
    ~ViewController();
    void classBegin() override;
    void componentComplete() override;
    Status status() const;
    QString errorMessage() const;
Q_SIGNALS:
    void statusChanged();
    void errorMessageChanged();
    void finished();
    void error();
protected:
    using Error_t = ::microcore::error::Error;
    using Executor_t = ::microcore::core::Executor< ::microcore::error::Error>;
    explicit ViewController(QObject *parent = nullptr);
    Executor_t & addExecutor(std::unique_ptr<Executor_t> executor);
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
    class ExecutorListener: public ::microcore::core::Executor< ::microcore::error::Error>::IListener
    {
    public:
        explicit ExecutorListener(ViewController &parent);
        void onStart() override final;
        void onFinish() override final;
        void onError(const Error_t &errorValue) override final;
        void onInvalidation(Executor_t &source) override final;
    private:
        ViewController &m_parent;
    };
    void setStatus(Status status);
    Status m_status {Idle};
    QString m_errorMessage {};
    std::map<Executor_t *, std::unique_ptr<Executor_t>> m_executors {};
};

}}

#endif // MICROCORE_QT_VIEWCONTROLLER_H
