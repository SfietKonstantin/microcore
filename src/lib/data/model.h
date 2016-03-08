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

#ifndef MICROCORE_DATA_MODEL_H
#define MICROCORE_DATA_MODEL_H

#include "modeldata.h"
#include "core/pipe.h"
#include <set>
#include <deque>

namespace microcore { namespace data {

/**
 * @brief A generic container
 */
template<class Data, class Store>
class ModelBase
{
public:
    using Item_t = Data;
    using NotificationItem_t = const Data *;
    using NotificationItems_t = std::vector<NotificationItem_t>;
    using StoredItem_t = const Data *;
    using Storage_t = std::deque<StoredItem_t>;
    using SourceItems_t = std::vector<Item_t>;
    /**
     * @brief An interface for a Model listener
     *
     * This interface is used to implement models-like classes, that listens to a
     * Model and want to be notified when the Model changes.
     *
     * This listener can be used to get notifications when some items are inserted,
     * removed, or updated. This is done via onAppend(), onPrepend(), onUpdate() and
     * onRemove().
     */
    class IListener
    {
    public:
         virtual ~IListener() {}
         /**
          * @brief Notify that new items are appended
          * @param items items to be appended.
          */
         virtual void onAppend(const NotificationItems_t &items) = 0;
         /**
          * @brief Notify that new items are prepended
          * @param items items to be prepended.
          */
         virtual void onPrepend(const NotificationItems_t &items) = 0;
         /**
          * @brief Notify that an item is updated
          * @param index index of the item that is updated.
          */
         virtual void onUpdate(std::size_t index, NotificationItem_t item) = 0;
         /**
          * @brief Notify that an item is removed
          * @param index index of the item that is removed.
          */
         virtual void onRemove(std::size_t index) = 0;
         /**
          * @brief Notify that an item has moved
          * @param from index of the item to move.
          * @param to the item's new index.
          */
         virtual void onMove(std::size_t from, std::size_t to) = 0;
         /**
          * @brief Notify that the listened object is now invalid
          *
          * When called on this method, the listener should stop
          * listening.
          */
         virtual void onInvalidation() = 0;
    };
    DISABLE_COPY_DEFAULT_MOVE(ModelBase);
    virtual ~ModelBase()
    {
        using namespace std::placeholders;
        std::for_each(std::begin(m_listeners), std::end(m_listeners),
                      std::bind(&IListener::onInvalidation, _1));
    }
    typename Storage_t::const_iterator begin() const
    {
        return std::begin(m_data);
    }
    typename Storage_t::const_iterator end() const
    {
        return std::end(m_data);
    }
    bool empty() const
    {
        return m_data.empty();
    }
    std::size_t size() const
    {
        return m_data.size();
    }
    void append(SourceItems_t &&items)
    {
        using namespace std::placeholders;
        const NotificationItems_t &stored = m_store.add(std::move(items));
        m_data.insert(std::end(m_data), std::begin(stored), std::end(stored));
        std::for_each(std::begin(m_listeners), std::end(m_listeners),
                      std::bind(&IListener::onAppend, _1, std::ref(stored)));
    }
    void prepend(SourceItems_t &&items)
    {
        using namespace std::placeholders;
        const NotificationItems_t &stored = m_store.add(std::move(items));
        m_data.insert(std::begin(m_data), std::begin(stored), std::end(stored));
        std::for_each(std::begin(m_listeners), std::end(m_listeners),
                      std::bind(&IListener::onPrepend, _1, std::ref(stored)));
    }
    bool update(std::size_t index, Item_t &&item)
    {
        using namespace std::placeholders;
        if (index >= m_data.size()) {
            return false;
        }
        if (!m_store.update(m_data.at(index), std::move(item))) {
            return false;
        }

        std::for_each(std::begin(m_listeners), std::end(m_listeners),
                      std::bind(&IListener::onUpdate, _1, index, m_data.at(index)));
        return true;
    }
    bool remove(std::size_t index)
    {
        using namespace std::placeholders;
        if (index >= m_data.size()) {
            return false;
        }
        if (!m_store.remove(m_data.at(index))) {
            return false;
        }
        m_data.erase(std::begin(m_data) + index);
        std::for_each(std::begin(m_listeners), std::end(m_listeners),
                      std::bind(&IListener::onRemove, _1, index));
        return true;
    }
    bool move(std::size_t from, std::size_t to)
    {
        using namespace std::placeholders;
        if (from >= m_data.size() || to > m_data.size()) {
            return false;
        }

        if (to == from || to == from + 1) {
            return false;
        }

        std::size_t toIndex = (to < from) ? to : to - 1;

        StoredItem_t data {*(std::begin(m_data) + from)};
        m_data.erase(std::begin(m_data) + from);
        m_data.insert(std::begin(m_data) + toIndex, data);
        std::for_each(std::begin(m_listeners), std::end(m_listeners),
                      std::bind(&IListener::onMove, _1, from, to));
        return true;
    }
    void addListener(IListener &listener)
    {
        m_listeners.insert(&listener);
        if (!m_data.empty()) {
            listener.onAppend(NotificationItems_t(m_data.begin(), m_data.end()));
        }
    }
    void removeListener(IListener &listener)
    {
        m_listeners.erase(&listener);
    }
protected:
    explicit ModelBase(Store &&store)
        : m_store(std::move(store))
    {
    }
private:
    Store m_store {};
    Storage_t m_data {};
    std::set<IListener *> m_listeners {};
};

template<class Data>
class Model : public ModelBase<Data, ModelData<Data>>
{
public:
    explicit Model()
        : ModelBase<Data, ModelData<Data>>(ModelData<Data>())
    {
    }
};

}}

#endif // MICROCORE_DATA_MODEL_H
