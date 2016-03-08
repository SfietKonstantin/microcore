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
#include <algorithm>
#include <functional>
#include <memory>
#include <QtGlobal>

namespace microcore { namespace data {

template<class DataSource>
class IDataSourceListener
{
public:
    virtual ~IDataSourceListener() {}
    virtual void onAdd(typename DataSource::NotificationItem_t data) = 0;
    virtual void onUpdate(typename DataSource::NotificationItem_t data) = 0;
    virtual void onRemove(typename DataSource::NotificationItem_t data) = 0;
    virtual void onInvalidation() = 0;
};

/**
 * @brief A generic container
 */
template<class K, class V>
class DataSource
{
public:
    using Key_t = K;
    using Item_t = V;
    using NotificationItem_t = const Item_t &;
    using GetKeyFunction_t = std::function<Key_t (const Item_t &)>;
    using Listener_t = IDataSourceListener<DataSource<Key_t, Item_t>>;
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
    const Item_t & add(Item_t &&item)
    {
        using namespace std::placeholders;
        const Key_t &key = m_getKeyFunction(item);
        auto it = m_data.find(key);

        if (it != m_data.end()) {
            // Update the already added item
            Item_t &newItem = *(it->second);
            newItem = std::move(item);

            std::for_each(std::begin(m_listeners), std::end(m_listeners),
                          std::bind(&Listener_t::onUpdate, _1, std::ref(newItem)));
            return newItem;
        } else {
            auto result = m_data.emplace(key, StoredItem_t(new Item_t(std::move(item))));
            Q_ASSERT(result.second); // Should be added
            Item_t &existingItem = *(result.first->second);

            std::for_each(std::begin(m_listeners), std::end(m_listeners),
                          std::bind(&Listener_t::onAdd, _1, std::ref(existingItem)));
            return existingItem;
        }
    }
    void remove(const Item_t &item)
    {
        using namespace std::placeholders;
        const Key_t &key = m_getKeyFunction(item);
        auto it = m_data.find(key);

        if (it != m_data.end()) {
            Item_t &existingItem = *(it->second);

            std::for_each(std::begin(m_listeners), std::end(m_listeners),
                          std::bind(&Listener_t::onRemove, _1, std::ref(existingItem)));
            m_data.erase(it);
        }
    }
    void addListener(Listener_t &listener)
    {
        using namespace std::placeholders;
        std::for_each(std::begin(m_data), std::end(m_data), [&listener](const typename Storage_t::value_type &it) {
            listener.onAdd(*(it.second));
        });
        m_listeners.insert(&listener);
    }
    void removeListener(Listener_t &listener)
    {
        m_listeners.erase(&listener);
    }
private:
    using StoredItem_t = std::unique_ptr<Item_t>;
    using Storage_t = std::map<Key_t, StoredItem_t>;
    GetKeyFunction_t m_getKeyFunction {};
    Storage_t m_data {};
    std::set<Listener_t *> m_listeners {};
};

}}

#endif // MICROCORE_DATA_DATASOURCE_H
