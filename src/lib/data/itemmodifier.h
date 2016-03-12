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

#ifndef MICROCORE_DATA_ITEMMODIFIER_H
#define MICROCORE_DATA_ITEMMODIFIER_H

#include "data/dataoperator.h"

namespace microcore { namespace data {

template<class Item, class Request, class Error>
class ItemModifier: public DataOperator<Item, Request, Item, Error>
{
public:
    using Factory_t = ::microcore::core::IJobFactory<Request, Item, Error>;
    ItemModifier(Item &item, std::unique_ptr<Factory_t> factory)
        : Parent_t(item, std::move(factory), OperatorFunction_t(&This_t::modify))
    {
    }
private:
    static void modify(Item &dest, Item &&source)
    {
        dest = std::move(source);
    }
    using Parent_t = DataOperator<Item, Request, Item, Error>;
    using This_t = ItemModifier<Item, Request, Error>;
    using OperatorFunction_t = std::function<void (Item &, Item &&)>;
};

}}

#endif // MICROCORE_DATA_ITEMMODIFIER_H