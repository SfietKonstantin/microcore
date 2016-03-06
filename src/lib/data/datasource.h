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

template<class DataSource>
class IDataSourceListener
{
public:
    virtual ~IDataSourceListener() {}
    virtual void onAdd(typename DataSource::Content_t data) = 0;
    virtual void onUpdate(typename DataSource::Content_t data) = 0;
    virtual void onRemove(typename DataSource::Content_t data) = 0;
    virtual void onInvalidation() = 0;
};

/**
 * @brief A generic container
 */
template<class K, class V>
class DataSource
{
public:
    using Data_t = V;
    using Content_t = const Data_t &;
    using GetKeyFunction_t = std::function<K (const Data_t &)>;
    using Listener_t = IDataSourceListener<DataSource<K, V>>;
    explicit DataSource(GetKeyFunction_t &&getKeyFunction)
        : m_getKeyFunction {std::move(getKeyFunction)}
    {
        Q_ASSERT(m_getKeyFunction);
    }
    DISABLE_COPY_DEFAULT_MOVE(DataSource);
    ~DataSource()
    {
        std::for_each(std::begin(m_listeners), std::end(m_listeners),
                      std::function<void (Listener_t *)>(&Listener_t::onInvalidation));
    }
    bool empty() const
    {
        return m_data.empty();
    }
    int size() const
    {
        return m_data.size();
    }
    const Data_t & add(Data_t &&data)
    {
        using namespace std::placeholders;
        const K &key = m_getKeyFunction(data);
        auto it = m_data.find(key);

        if (it != m_data.end()) {
            // Update the already added data
            Data_t &value = *(it->second);
            value = std::move(data);

            std::for_each(std::begin(m_listeners), std::end(m_listeners),
                          std::bind(&Listener_t::onUpdate, _1, std::ref(value)));
            return value;
        } else {
            auto result = m_data.emplace(key, Storage_t(new Data_t(std::move(data))));
            Q_ASSERT(result.second); // Should be added
            Data_t &value = *(result.first->second);

            std::for_each(std::begin(m_listeners), std::end(m_listeners),
                          std::bind(&Listener_t::onAdd, _1, std::ref(value)));
            return value;
        }
    }
    void remove(const Data_t &data)
    {
        using namespace std::placeholders;
        const K &key = m_getKeyFunction(data);
        auto it = m_data.find(key);

        if (it != m_data.end()) {
            Data_t &value = *(it->second);

            std::for_each(std::begin(m_listeners), std::end(m_listeners),
                          std::bind(&Listener_t::onRemove, _1, std::ref(value)));
            m_data.erase(it);
        }
    }
    void addListener(Listener_t &listener)
    {
        using namespace std::placeholders;
        std::for_each(std::begin(m_data), std::end(m_data), [&listener](const typename Container_t::value_type &it) {
            listener.onAdd(*(it.second));
        });
        m_listeners.insert(&listener);
    }
    void removeListener(Listener_t &listener)
    {
        m_listeners.erase(&listener);
    }
private:
    using Storage_t = std::unique_ptr<Data_t>;
    using Container_t = std::map<K, Storage_t>;
    GetKeyFunction_t m_getKeyFunction {};
    Container_t m_data {};
    std::set<Listener_t *> m_listeners {};
};

}}

#endif // MICROCORE_DATA_DATASOURCE_H
