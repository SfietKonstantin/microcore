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

#ifndef DATASTORE_H
#define DATASTORE_H

#include "core/globals.h"
#include "data/idatastore.h"
#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <set>

namespace microcore { namespace data {

template<class K, class V>
class DataStore: public IDataStore<K, V>
{
public:
    explicit DataStore() = default;
    DISABLE_COPY_DEFAULT_MOVE(DataStore);
    ~DataStore()
    {
        using namespace std::placeholders;
        std::for_each(std::begin(m_listeners), std::end(m_listeners),
                      std::bind(&DataStore::IListener::onInvalidation, _1));
    }
    const V * addUnique(arg_rvalue_reference<K> key, arg_rvalue_reference<V> value) override
    {
        auto it = m_data.find(key);
        if (it != std::end(m_data)) {
            return nullptr;
        }
        it = m_data.emplace(std::move(key), std::move(value)).first;
        using namespace std::placeholders;
        std::for_each(std::begin(m_listeners), std::end(m_listeners),
                      std::bind(&DataStore::IListener::onAdd, _1, std::ref(it->first), std::ref(it->second)));
        return &(it->second);
    }
    const V & add(arg_rvalue_reference<K> key, arg_rvalue_reference<V> value) override
    {
        auto it = m_data.find(key);
        if (it != std::end(m_data)) {
            update(it, std::move(value));
            return it->second;
        }
        it = m_data.emplace(std::move(key), std::move(value)).first;
        using namespace std::placeholders;
        std::for_each(std::begin(m_listeners), std::end(m_listeners),
                      std::bind(&DataStore::IListener::onAdd, _1, std::ref(it->first), std::ref(it->second)));
        return it->second;
    }
    const V * update(arg_const_reference<K> key, arg_rvalue_reference<V> value) override
    {
        auto it = m_data.find(key);
        if (it == std::end(m_data)) {
            return nullptr;
        }
        update(it, std::move(value));
        return &(it->second);
    }
    bool remove(arg_const_reference<K> key) override
    {
        auto it = m_data.find(key);
        if (it == std::end(m_data)) {
            return false;
        }
        m_data.erase(it);

        using namespace std::placeholders;
        std::for_each(std::begin(m_listeners), std::end(m_listeners),
                      std::bind(&DataStore::IListener::onRemove, _1, std::ref(key)));
        return true;
    }

    void addListener(typename DataStore::IListener &listener) override
    {
        m_listeners.insert(&listener);
    }
    void removeListener(typename DataStore::IListener &listener) override
    {
        m_listeners.erase(&listener);
    }
protected:
    void update(typename std::map<K, V>::iterator &it, arg_rvalue_reference<V> value)
    {
        it->second = std::move(value);

        using namespace std::placeholders;
        std::for_each(std::begin(m_listeners), std::end(m_listeners),
                      std::bind(&DataStore::IListener::onUpdate, _1, std::ref(it->first), std::ref(it->second)));
    }

    std::map<K, V> m_data {};
    std::set<typename DataStore::IListener *> m_listeners {};
};

}}

#endif // DATASTORE_H
