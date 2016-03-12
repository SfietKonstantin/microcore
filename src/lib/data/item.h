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

#ifndef MICROCORE_DATA_ITEM_H
#define MICROCORE_DATA_ITEM_H

#include "core/globals.h"
#include <algorithm>
#include <functional>
#include <set>

namespace microcore { namespace data {

template<class Data>
class Item
{
public:
    using SourceItem_t = Data;
    class IListener
    {
    public:
         virtual ~IListener() {}
         virtual void onModified(const Data &data) = 0;
         virtual void onInvalidation() = 0;
    };
    DISABLE_COPY_DEFAULT_MOVE(Item);
    explicit Item() = default;
    virtual ~Item()
    {
        using namespace std::placeholders;
        std::for_each(std::begin(m_listeners), std::end(m_listeners),
                      std::bind(&IListener::onInvalidation, _1));
    }
    const Data & data() const
    {
        return m_data;
    }
    void setData(Data &&data)
    {
        using namespace std::placeholders;
        m_data = std::move(data);
        std::for_each(std::begin(m_listeners), std::end(m_listeners),
                      std::bind(&IListener::onModified, _1, m_data));
    }
    void addListener(IListener &listener)
    {
        m_listeners.insert(&listener);
        listener.onModified(m_data);
    }
    void removeListener(IListener &listener)
    {
        m_listeners.erase(&listener);
    }
private:
    Data m_data {};
    std::set<IListener *> m_listeners {};
};

}}

#endif // MICROCORE_DATA_ITEM_H
