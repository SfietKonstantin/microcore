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

#ifndef INDEXEDDATASTORE_H
#define INDEXEDDATASTORE_H

#include <microcore/data/iindexeddatastore.h>
#include <microcore/core/globals.h>
#include <microcore/core/listenerrepository.h>
#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <set>

namespace microcore { namespace data {

template<class K, class V>
class IndexedDataStore: public IIndexedDataStore<K, V>
{
public:
    using ValuePtr = std::shared_ptr<V>;
    explicit IndexedDataStore() = default;
    DISABLE_COPY_DEFAULT_MOVE(IndexedDataStore);
    ValuePtr addUnique(arg_rvalue_reference<K> key, arg_rvalue_reference<V> value) override final
    {
        auto it = m_data.find(key);
        if (it != std::end(m_data)) {
            return ValuePtr();
        }
        it = m_data.emplace(std::move(key), ValuePtr(new V(std::move(value)))).first;
        using namespace std::placeholders;
        m_listenerRepository.notify(std::bind(&IndexedDataStore::IListener::onAdd, _1,
                                              std::ref(it->first), std::ref(it->second)));
        return it->second;
    }
    ValuePtr add(arg_rvalue_reference<K> key, arg_rvalue_reference<V> value) override final
    {
        auto it = m_data.find(key);
        if (it != std::end(m_data)) {
            update(it, std::move(value));
            return it->second;
        }
        it = m_data.emplace(std::move(key), ValuePtr(new V(std::move(value)))).first;
        using namespace std::placeholders;
        m_listenerRepository.notify(std::bind(&IndexedDataStore::IListener::onAdd, _1,
                                              std::ref(it->first), std::ref(it->second)));
        return it->second;
    }
    ValuePtr update(arg_const_reference<K> key, arg_rvalue_reference<V> value) override final
    {
        auto it = m_data.find(key);
        if (it == std::end(m_data)) {
            return nullptr;
        }
        update(it, std::move(value));
        return it->second;
    }
    bool remove(arg_const_reference<K> key) override final
    {
        auto it = m_data.find(key);
        if (it == std::end(m_data)) {
            return false;
        }
        using namespace std::placeholders;
        m_listenerRepository.notify(std::bind(&IndexedDataStore::IListener::onRemove, _1, std::ref(key)));

        m_data.erase(it);
        return true;
    }

    void addListener(const typename IndexedDataStore::IListener::Ptr &listener) override final
    {
        if (!listener) {
            return;
        }

        m_listenerRepository.addListener(listener);
    }
    void removeListener(const typename IndexedDataStore::IListener::Ptr &listener) override final
    {
        m_listenerRepository.removeListener(listener);
    }
protected:
    std::map<K, ValuePtr> m_data {};
private:
    void update(typename std::map<K, ValuePtr>::iterator &it, arg_rvalue_reference<V> value)
    {
        *(it->second) = std::move(value);

        using namespace std::placeholders;
        m_listenerRepository.notify(std::bind(&IndexedDataStore::IListener::onUpdate, _1,
                                              std::ref(it->first), std::ref(it->second)));
    }
    ::microcore::core::ListenerRepository<typename IndexedDataStore::IListener> m_listenerRepository {};
};

}}

#endif // INDEXEDDATASTORE_H
