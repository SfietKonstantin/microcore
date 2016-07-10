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

#ifndef MICROCORE_CORE_LISTENERREPOSITORY_H
#define MICROCORE_CORE_LISTENERREPOSITORY_H

#include <microcore/core/globals.h>
#include <memory>
#include <vector>
#include <algorithm>

namespace microcore { namespace core {

template<class Listener>
class ListenerRepository
{
public:
    using ListenerPtr = std::shared_ptr<Listener>;
    explicit ListenerRepository() = default;
    DISABLE_COPY_DISABLE_MOVE(ListenerRepository);
    ~ListenerRepository()
    {
        Invoker invoker {&Listener::onInvalidation};
        std::for_each(std::begin(m_listeners), std::end(m_listeners), invoker);
    }
    bool isEmpty() const
    {
        return m_listeners.empty();
    }
    std::size_t count() const
    {
        return m_listeners.size();
    }
    void addListener(const std::shared_ptr<Listener> &listener)
    {
        m_listeners.emplace_back(listener);
    }
    void removeListener(const std::shared_ptr<Listener> &listener)
    {
        const Listener *listenerPtr {listener.get()};
        m_listeners.erase(std::remove_if(std::begin(m_listeners), std::end(m_listeners),
                          [listenerPtr](const std::weak_ptr<Listener> &element) {
            if (element.expired())
            {
                return (listenerPtr == nullptr);
            }
            else
            {
                const std::shared_ptr<Listener> &elementSharedPtr {element.lock()};
                return (listenerPtr == elementSharedPtr.get());
            }
        }), std::end(m_listeners));
    }
    void notify(std::function<void (Listener &)> &&function)
    {
        // This method will purge expired listeners while invoking non-expired ones
        Invoker invoker {std::move(function)};
        m_listeners.erase(std::remove_if(std::begin(m_listeners), std::end(m_listeners), invoker),
                          std::end(m_listeners));
    }
    void notify(std::function<void (Listener &)> &&function) const
    {
        Invoker invoker {std::move(function)};
        std::for_each(std::begin(m_listeners), std::end(m_listeners), invoker);
    }
private:
    // This function will return true for expired listeners
    // It can be used for std::remove_if to purge them from the repository
    class Invoker
    {
    public:
        explicit Invoker(std::function<void (Listener &)> &&function)
            : m_function {function}
        {
        }
        bool operator()(const std::weak_ptr<Listener> &listener) const
        {
            if (listener.expired()) {
                return true;
            }
            const std::shared_ptr<Listener> &sharedListener {listener.lock()};
            m_function(*sharedListener);
            return false;
        }
    private:
        const std::function<void (Listener &)> &m_function;
    };
    std::vector<std::weak_ptr<Listener>> m_listeners {};
};

}}

#endif // MICROCORE_CORE_LISTENERREPOSITORY_H
