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

#ifndef INDEXEDMODEL_H
#define INDEXEDMODEL_H

#include <microcore/data/imutablemodel.h>
#include <microcore/data/iindexeddatastore.h>
#include <microcore/core/globals.h>
#include <microcore/core/listenerrepository.h>
#include <algorithm>
#include <deque>
#include <functional>
#include <memory>
#include <set>
#include <QtCore/QtGlobal>

namespace microcore { namespace data {

template<class V, class M, class S = std::deque<const V *>>
class IndexedModel: public IMutableModel<V, S>
{
public:
    explicit IndexedModel(IIndexedDataStore<typename M::KeyType, V> &dataStore)
        : m_listener {new DataStoreListener(*this)}, m_dataStore {&dataStore}
    {
        m_dataStore->addListener(m_listener);
    }
    DISABLE_COPY_DEFAULT_MOVE(IndexedModel);
    typename S::iterator begin() noexcept override final
    {
        return m_data.begin();
    }
    typename S::iterator end() noexcept override final
    {
        return m_data.end();
    }
    typename S::const_iterator begin() const noexcept override final
    {
        return m_data.begin();
    }
    typename S::const_iterator end() const noexcept override final
    {
        return m_data.end();
    }
    bool empty() const noexcept override final
    {
        return m_data.empty();
    }
    typename S::size_type size() const noexcept override final
    {
        return m_data.size();
    }
    virtual const V * operator[](typename S::size_type index) const override final
    {
        if (index >= m_data.size()) {
            return nullptr;
        }
        return m_data[index];
    }
    void addListener(const typename IModel<V, S>::IListener::Ptr &listener) override final
    {
        if (!listener) {
            return;
        }

        m_listenerRepository.addListener(listener);
        if (!m_data.empty()) {
            listener->onAppend(std::vector<const V *>(std::begin(m_data), std::end(m_data)));
        }
    }
    void removeListener(const typename IModel<V, S>::IListener::Ptr &listener) override final
    {
        m_listenerRepository.removeListener(listener);
    }
    void append(std::vector<V> &&values) override final
    {
        insert(std::end(m_data), std::move(values), &IModel<V, S>::IListener::onAppend);
    }
    void prepend(std::vector<V> &&values) override final
    {
        insert(std::begin(m_data), std::move(values), &IModel<V, S>::IListener::onPrepend);
    }
    void insert(typename S::size_type index, std::vector<V> &&values) override final
    {
        if (index > m_data.size()) {
            return;
        }
        using namespace std::placeholders;
        insert(std::begin(m_data) + index, std::move(values),
               std::bind(&IModel<V, S>::IListener::onInsert, _1, index, _2));
    }
    void remove(typename S::size_type index) override final
    {
        ListenBlockerLock lock {m_listeningDataStore};
        if (m_dataStore == nullptr) {
            return;
        }

        if (index > m_data.size()) {
            return;
        }

        auto it = std::begin(m_data) + index;
        const V *value {*it};
        m_data.erase(it);
        m_dataStore->remove(m_mapper(*value));

        using namespace std::placeholders;
        m_listenerRepository.notify(std::bind(&IModel<V, S>::IListener::onRemove, _1, index));
    }
    void update(typename S::size_type index, arg_rvalue_reference<V> value) override final
    {
        ListenBlockerLock lock {m_listeningDataStore};
        if (m_dataStore == nullptr) {
            return;
        }

        if (index > m_data.size()) {
            return;
        }

        const V *storedValue {m_data[index]};
        const typename M::KeyType &key {m_mapper(*storedValue)};
        if (key != m_mapper(value)) {
            return;
        }

        m_dataStore->update(key, std::move(value));

        using namespace std::placeholders;
        m_listenerRepository.notify(std::bind(&IModel<V, S>::IListener::onUpdate, _1, index, std::ref(*storedValue)));
    }
    void move(typename S::size_type oldIndex, typename S::size_type newIndex) override final
    {
        if (oldIndex >= m_data.size() || newIndex > m_data.size()) {
            return;
        }

        if (newIndex == oldIndex || newIndex == oldIndex + 1) {
            return;
        }

        typename S::size_type toIndex = (newIndex < oldIndex) ? newIndex : newIndex - 1;

        const V *storedValue {m_data[oldIndex]};
        m_data.erase(std::begin(m_data) + oldIndex);
        m_data.insert(std::begin(m_data) + toIndex, storedValue);

        using namespace std::placeholders;
        m_listenerRepository.notify(std::bind(&IModel<V, S>::IListener::onMove, _1, oldIndex, newIndex));
    }
private:
    class DataStoreListener: public IIndexedDataStore<typename M::KeyType, V>::IListener
    {
    public:
        using ValuePtr = std::shared_ptr<V>;
        explicit DataStoreListener(IndexedModel<V, M, S> &parent)
            : m_parent {parent}
        {
        }
        void onAdd(arg_const_reference<typename M::KeyType> key,
                   const ValuePtr &value) override final
        {
            Q_UNUSED(key)
            Q_UNUSED(value)
        }
        void onRemove(arg_const_reference<typename M::KeyType> key) override final
        {
            if (!m_parent.m_listeningDataStore) {
                return;
            }

            auto it = std::find_if(std::begin(m_parent.m_data), std::end(m_parent.m_data), [&key, this](const V *v) {
                return m_parent.m_mapper(*v) == key;
            });

            if (it == std::end(m_parent.m_data)) {
                return;
            }
            typename S::size_type index = it - std::begin(m_parent.m_data);
            m_parent.m_data.erase(it);

            using namespace std::placeholders;
            m_parent.m_listenerRepository.notify(std::bind(&IModel<V, S>::IListener::onRemove, _1, index));
        }
        void onUpdate(arg_const_reference<typename M::KeyType> key,
                      const ValuePtr & value) override final
        {
            Q_UNUSED(value)
            if (!m_parent.m_listeningDataStore) {
                return;
            }

            auto it = std::find_if(std::begin(m_parent.m_data), std::end(m_parent.m_data), [&key, this](const V *v) {
                return m_parent.m_mapper(*v) == key;
            });

            if (it == std::end(m_parent.m_data)) {
                return;
            }
            typename S::size_type index = it - std::begin(m_parent.m_data);

            using namespace std::placeholders;
            m_parent.m_listenerRepository.notify(std::bind(&IModel<V, S>::IListener::onUpdate, _1, index, std::ref(*(*it))));
        }
        void onInvalidation() override final
        {
            m_parent.m_dataStore = nullptr;
            m_parent.m_data.clear();

            using namespace std::placeholders;
            m_parent.m_listenerRepository.notify(std::bind(&IModel<V, S>::IListener::onInvalidation, _1));
        }
    private:
        IndexedModel<V, M, S> &m_parent;
    };
    class ListenBlockerLock
    {
    public:
        explicit ListenBlockerLock(bool &listeningDataStore)
            : m_listeningDataStore {listeningDataStore}
        {
            m_listeningDataStore = false;
        }
        ~ListenBlockerLock()
        {
            m_listeningDataStore = true;
        }
    private:
        bool &m_listeningDataStore;
    };
    void insert(typename S::iterator index, std::vector<V> &&values,
                std::function<void (typename IModel<V, S>::IListener &, const std::vector<const V *> &)> &&function)
    {
        if (m_dataStore == nullptr) {
            return;
        }

        std::vector<const V *> buffer {};
        buffer.reserve(values.size());
        std::for_each(std::begin(values), std::end(values), [&buffer, this](V &value) {
            const std::shared_ptr<V> &addedValue {m_dataStore->addUnique(m_mapper(value), std::move(value))};
            if (addedValue) {
                buffer.emplace_back(addedValue.get());
            }
        });
        m_data.insert(index, std::begin(buffer), std::end(buffer));

        using namespace std::placeholders;
        m_listenerRepository.notify(std::bind(function, _1, std::ref(buffer)));
    }
    typename DataStoreListener::Ptr m_listener;
    IIndexedDataStore<typename M::KeyType, V> *m_dataStore {nullptr};
    M m_mapper {};
    S m_data {};
    std::unique_ptr<std::vector<const V *>> m_buffer {};
    bool m_listeningDataStore {true};
    ::microcore::core::ListenerRepository<typename IModel<V, S>::IListener> m_listenerRepository {};
};

}}

#endif // INDEXEDMODEL_H
