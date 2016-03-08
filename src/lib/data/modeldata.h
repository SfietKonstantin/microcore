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

#ifndef MICROCORE_DATA_MODELDATA_H
#define MICROCORE_DATA_MODELDATA_H

#include "core/globals.h"
#include <map>
#include <vector>
#include <algorithm>
#include <memory>
#include <QtGlobal>

namespace microcore { namespace data {

template<class Data>
class ModelData
{
public:
    using Key_t = const Data *;
    using Item_t = Data;
    using StoredItem_t = std::unique_ptr<Data>;
    using Storage_t = std::map<Key_t, StoredItem_t>;
    using SourceItems_t = std::vector<Item_t>;
    using OutputItems_t = std::vector<Key_t>;
    explicit ModelData()
    {
    }
    DISABLE_COPY_DEFAULT_MOVE(ModelData);
    typename Storage_t::const_iterator begin() const
    {
        return std::begin(m_data);
    }
    typename Storage_t::const_iterator end() const
    {
        return std::end(m_data);
    }
    OutputItems_t add(SourceItems_t &&data)
    {
        OutputItems_t returned (data.size(), nullptr);
        typename OutputItems_t::iterator it = std::begin(returned);
        std::for_each(std::begin(data), std::end(data), [this, &it](const Item_t &item) {
            StoredItem_t data {new Data(item)};
            Key_t key = data.get();
            auto result = m_data.emplace(key, std::move(data));
            Q_ASSERT(result.second); // Should be added
            *it = result.first->first;
            ++it;
        });
        return returned;
    }
    bool remove(Key_t item)
    {
        auto it = m_data.find(item);
        if (it == std::end(m_data)) {
            return false;
        }
        m_data.erase(it);
        return true;
    }
    bool update(Key_t key, Item_t &&value)
    {
        auto it = m_data.find(key);
        if (it == std::end(m_data)) {
            return false;
        }
        *it->second = std::move(value);
        return true;
    }

private:
    Storage_t m_data {};
};

}}

#endif // MICROCORE_DATA_MODELDATA_H
