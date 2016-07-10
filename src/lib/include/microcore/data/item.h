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

#ifndef MICROCORE_DATA_ITEM_H
#define MICROCORE_DATA_ITEM_H

#include <microcore/data/iitem.h>
#include <microcore/core/globals.h>
#include <microcore/core/listenerrepository.h>
#include <algorithm>
#include <functional>
#include <set>

namespace microcore { namespace data {

template<class T>
class Item: public IItem<T>
{
public:
    explicit Item() = default;
    DISABLE_COPY_DEFAULT_MOVE(Item);
    const T & data() const override
    {
        return m_data;
    }
    void setData(T &&data) override
    {
        using namespace std::placeholders;
        m_data = std::move(data);
        m_listenerRepository.notify(std::bind(&IItem<T>::IListener::onUpdate, _1, m_data));
    }
    void addListener(const typename IItem<T>::IListener::Ptr &listener) override final
    {
        if (!listener) {
            return;
        }
        m_listenerRepository.addListener(listener);
        listener->onModified(m_data);
    }
    void removeListener(const typename IItem<T>::IListener::Ptr &listener) override final
    {
        m_listenerRepository.removeListener(listener);
    }
private:
    T m_data {};
    ::microcore::core::ListenerRepository<typename IItem<T>::IListener> m_listenerRepository {};
};

}}

#endif // MICROCORE_DATA_ITEM_H
