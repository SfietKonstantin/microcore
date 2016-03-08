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

#ifndef MICROCORE_CORE_EXECUTOR_H
#define MICROCORE_CORE_EXECUTOR_H

#include <algorithm>
#include <functional>
#include <set>
#include "globals.h"

namespace microcore { namespace core {

template<class Error>
class Executor
{
public:
    class IListener
    {
    public:
        virtual ~IListener() {}
        virtual void onStart() = 0;
        virtual void onFinish() = 0;
        virtual void onError(const Error &error) = 0;
        virtual void onInvalidation(Executor<Error> &source) = 0;
    };
    explicit Executor() = default;
    DISABLE_COPY_DEFAULT_MOVE(Executor);
    virtual ~Executor()
    {
        using namespace std::placeholders;
        std::for_each(std::begin(m_listeners), std::end(m_listeners),
                      std::bind(&IListener::onInvalidation, _1, std::ref(*this)));
    }
    void addListener(IListener &listener)
    {
        m_listeners.insert(&listener);
        if (m_busy) {
            listener.onStart();
        } else if (!m_error.empty()) {
            listener.onError(m_error);
        }
    }
    void removeListener(IListener &listener)
    {
        m_listeners.erase(&listener);
    }
    bool busy()
    {
        return m_busy;
    }
protected:
    bool canStart() const
    {
        return !m_busy;
    }
    bool canFinish() const
    {
        return m_busy;
    }
    void doStart()
    {
        if (!canStart()) {
            return;
        }
        using namespace std::placeholders;
        m_busy = true;
        m_error = Error();
        std::for_each(std::begin(m_listeners), std::end(m_listeners),
                      std::bind(&IListener::onStart, _1));
    }
    void doError(Error &&error)
    {
        using namespace std::placeholders;
        if (!canFinish()) {
            return;
        }
        m_busy = false;
        m_error = std::move(error);
        std::for_each(std::begin(m_listeners), std::end(m_listeners),
                      std::bind(&IListener::onError, _1, std::ref(m_error)));
    }
    void doFinish()
    {
        using namespace std::placeholders;
        if (!canFinish()) {
            return;
        }
        m_busy = false;
        m_error = Error();
        std::for_each(std::begin(m_listeners), std::end(m_listeners),
                      std::bind(&IListener::onFinish, _1));
    }
private:
    std::set<IListener *> m_listeners {};
    bool m_busy {false};
    Error m_error {};
};

}}

#endif // MICROCORE_CORE_EXECUTOR_H
