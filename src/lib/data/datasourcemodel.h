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

#ifndef MICROCORE_DATA_DATASOURCEMODEL_H
#define MICROCORE_DATA_DATASOURCEMODEL_H

#include "datasource.h"
#include "model.h"

namespace microcore { namespace data {

template<class Key, class Data>
class DataSourceModelAdaptor
{
public:
    using DataSource_t = DataSource<Key, Data>;
    using Key_t = const Data *;
    using Item_t = Data;
    using SourceItems_t = std::vector<Item_t>;
    using OutputItems_t = std::vector<Key_t>;
    explicit DataSourceModelAdaptor(std::unique_ptr<DataSource_t> source)
        : m_source(std::move(source))
    {
    }
    OutputItems_t add(SourceItems_t &&data)
    {
        OutputItems_t returned (data.size(), nullptr);
        typename OutputItems_t::iterator it = std::begin(returned);
        std::for_each(std::begin(data), std::end(data), [this, &it](const Item_t &item) {
            *it = &(m_source->add(Item_t(item)));
            ++it;
        });
        return returned;
    }
    bool remove(Key_t item)
    {
        return m_source->remove(*item);
    }
    bool update(Key_t key, Item_t &&value)
    {
        return m_source->update(*key, std::move(value));
    }
private:
    std::unique_ptr<DataSource_t> m_source {};
};

template<class Key, class Data>
class DataSourceModel: public ModelBase<Data, DataSourceModelAdaptor<Key, Data>>
{
public:
    using DataSource_t = DataSource<Key, Data>;
    explicit DataSourceModel(std::unique_ptr<DataSource_t> source)
        : ModelBase<Data, Adaptor_t>(Adaptor_t(std::move(source)))
    {
    }
private:
    using Adaptor_t = DataSourceModelAdaptor<Key, Data>;
};

}}

#endif // MICROCORE_DATA_DATASOURCEMODEL_H
