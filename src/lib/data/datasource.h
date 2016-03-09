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
    explicit DataSource(GetKeyFunction_t &&getKeyFunction)
        : m_getKeyFunction {std::move(getKeyFunction)}
    {
        Q_ASSERT(m_getKeyFunction);
    }
    DISABLE_COPY_DEFAULT_MOVE(DataSource);
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
        const Key_t &key = m_getKeyFunction(item);
        auto it = m_data.find(key);

        if (it != m_data.end()) {
            // Update the already added item
            Item_t &newItem = *(it->second);
            newItem = std::move(item);
            return newItem;
        } else {
            auto result = m_data.emplace(key, StoredItem_t(new Item_t(std::move(item))));
            Q_ASSERT(result.second); // Should be added
            return *(result.first->second);
        }
    }
    bool update(const Item_t &source, Item_t &&value)
    {
        const Key_t &sourceKey = m_getKeyFunction(source);
        const Key_t &valueKey = m_getKeyFunction(value);
        if (sourceKey != valueKey) {
            return false;
        }

        auto it = m_data.find(sourceKey);

        if (it == m_data.end()) {
            return false;
        }

        Item_t &newItem = *(it->second);
        newItem = std::move(value);
        return true;
    }

    bool remove(const Item_t &item)
    {
        const Key_t &key = m_getKeyFunction(item);
        auto it = m_data.find(key);

        if (it == m_data.end()) {
            return false;
        }

        m_data.erase(it);
        return true;
    }
private:
    using StoredItem_t = std::unique_ptr<Item_t>;
    using Storage_t = std::map<Key_t, StoredItem_t>;
    GetKeyFunction_t m_getKeyFunction {};
    Storage_t m_data {};
};

}}

#endif // MICROCORE_DATA_DATASOURCE_H
