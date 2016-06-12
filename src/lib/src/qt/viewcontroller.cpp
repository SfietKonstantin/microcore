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

#include <microcore/qt/viewcontroller.h>

namespace microcore { namespace qt {

ViewController::ViewController(QObject *parent)
    : QObject(parent), QQmlParserStatus()
{
}

ViewController::~ViewController()
{
    using namespace std::placeholders;
    using ExecutorPair = std::pair<Executor_t * const, std::unique_ptr<Executor_t>>;
    std::for_each(std::begin(m_executors), std::end(m_executors), [this](ExecutorPair &pair) {
        pair.second->removeListener(*this);
    });
}

void ViewController::classBegin()
{
}

void ViewController::componentComplete()
{
}

ViewController::Status ViewController::status() const
{
    return m_status;
}

QString ViewController::errorMessage() const
{
    return m_errorMessage;
}

ViewController::Executor_t & ViewController::addExecutor(std::unique_ptr<Executor_t> executor)
{
    auto result = m_executors.emplace(executor.get(), std::move(executor));
    Q_ASSERT(result.second); // Should be added
    ViewController::Executor_t &returned {*(result.first->second)};
    returned.addListener(*this);
    return returned;
}

void ViewController::setStatus(ViewController::Status status)
{
    if (m_status != status) {
        m_status = status;
        Q_EMIT statusChanged();
    }
}

ViewController::ExecutorListener::ExecutorListener(ViewController &parent)
    : m_parent(parent)
{
}

void ViewController::ExecutorListener::onStart()
{
    Q_ASSERT(m_parent.m_status != Busy);
    m_parent.setStatus(Busy);
}

void ViewController::ExecutorListener::onFinish()
{
    Q_ASSERT(m_parent.m_status == Busy);
    m_parent.setStatus(Idle);
    Q_EMIT m_parent.finished();
}

void ViewController::ExecutorListener::onError(const ViewController::Error_t &errorValue)
{
    Q_ASSERT(m_parent.m_status == Busy);
    m_parent.setStatus(Error);
    if (m_parent.m_errorMessage != errorValue.message()) {
        m_parent.m_errorMessage = errorValue.message();
        Q_EMIT m_parent.errorMessageChanged();
    }
    Q_EMIT m_parent.error();
}

void ViewController::ExecutorListener::onInvalidation(ViewController::Executor_t &source)
{
    m_parent.m_executors.erase(&source);
}

}}
