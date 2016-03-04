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

#ifndef MICROCORE_DATA_DATASOURCE_H
#define MICROCORE_DATA_DATASOURCE_H

#include "core/globals.h"
#include <set>
#include <map>
#include <functional>
#include <memory>
#include <QtGlobal>
#include <algorithm>

namespace microcore { namespace data {

/**
 * @brief A generic container
 */
template<class K, class V>
class DataSource
{
public:
    class IListener
    {
    public:
        virtual ~IListener() {}
        virtual void onAdd(const V &data) = 0;
        virtual void onUpdate(const V &data) = 0;
        virtual void onRemove(const V &data) = 0;
        virtual void onInvalidation() = 0;
    };
    using GetKeyFunction_t = std::function<K (const V &)>;
    explicit DataSource(GetKeyFunction_t &&getKeyFunction)
        : m_getKeyFunction {std::move(getKeyFunction)}
    {
        Q_ASSERT(m_getKeyFunction);
    }
    DISABLE_COPY_DEFAULT_MOVE(DataSource);
    ~DataSource()
    {
        std::for_each(std::begin(m_listeners), std::end(m_listeners),
                      std::function<void (IListener *)>(&IListener::onInvalidation));
    }
    bool empty() const
    {
        return m_data.empty();
    }
    int size() const
    {
        return m_data.size();
    }
    const V & add(V &&data)
    {
        using namespace std::placeholders;
        const K &key = m_getKeyFunction(data);
        auto it = m_data.find(key);

        if (it != m_data.end()) {
            // Update the already added data
            V &value = *(it->second);
            value = std::move(data);

            std::for_each(std::begin(m_listeners), std::end(m_listeners),
                          std::bind(&IListener::onUpdate, _1, std::ref(value)));
            return value;
        } else {
            auto result = m_data.emplace(key, std::unique_ptr<V>(new V(std::move(data))));
            Q_ASSERT(result.second); // Should be added
            V &value = *(result.first->second);

            std::for_each(std::begin(m_listeners), std::end(m_listeners),
                          std::bind(&IListener::onAdd, _1, std::ref(value)));
            return value;
        }
    }
    void remove(const V &data)
    {
        using namespace std::placeholders;
        const K &key = m_getKeyFunction(data);
        auto it = m_data.find(key);

        if (it != m_data.end()) {
            V &value = *(it->second);

            std::for_each(std::begin(m_listeners), std::end(m_listeners),
                          std::bind(&IListener::onRemove, _1, std::ref(value)));
            m_data.erase(it);
        }
    }
    void addListener(IListener &listener)
    {
        using namespace std::placeholders;
        std::for_each(std::begin(m_data), std::end(m_data), [&listener](const typename DataMap_t::value_type &it) {
            listener.onAdd(*(it.second));
        });
        m_listeners.insert(&listener);
    }
    void removeListener(IListener &listener)
    {
        m_listeners.erase(&listener);
    }
private:
    using DataMap_t = std::map<K, std::unique_ptr<V>>;
    GetKeyFunction_t m_getKeyFunction {};
    DataMap_t m_data {};
    std::set<IListener *> m_listeners {};
};

}}

#endif // MICROCORE_DATA_DATASOURCE_H
