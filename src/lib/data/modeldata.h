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
    using DataPtr = std::unique_ptr<Data>;
    using Map = std::map<const Data *, DataPtr>;
    explicit ModelData()
    {
    }
    DISABLE_COPY_DEFAULT_MOVE(ModelData);
    typename Map::const_iterator begin() const
    {
        return std::begin(m_data);
    }
    typename Map::const_iterator end() const
    {
        return std::end(m_data);
    }
    std::vector<const Data *> add(std::vector<Data> &&data)
    {
        typename std::vector<const Data *> returned (data.size(), nullptr);
        typename std::vector<const Data *>::iterator it = std::begin(returned);
        std::for_each(std::begin(data), std::end(data), [this, &it](Data &item) {
            std::unique_ptr<Data> data {new Data(item)};
            Data *key = data.get();
            auto result = m_data.emplace(key, std::move(data));
            Q_ASSERT(result.second); // Should be added
            *it = result.first->first;
            ++it;
        });
        return returned;
    }
    bool remove(const Data *data)
    {
        auto it = m_data.find(data);
        if (it == std::end(m_data)) {
            return false;
        }
        m_data.erase(it);
        return true;
    }
    bool update(const Data *key, Data &&value)
    {
        auto it = m_data.find(key);
        if (it == std::end(m_data)) {
            return false;
        }
        *it->second = std::move(value);
        return true;
    }

private:
    Map m_data {};
};

}}

#endif // MICROCORE_DATA_MODELDATA_H
