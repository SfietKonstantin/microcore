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

#ifndef IDATASTORE_H
#define IDATASTORE_H

#include "data/type_helper.h"

namespace microcore { namespace data {

/**
 * @brief Interface for a data store
 *
 * This interface describes a data store.
 *
 * A data store is used to store key-value pairs
 * and notify that content of the store has changed.
 *
 * To add, remove or update the content of the store,
 * the following methods must be implemented
 * - add()
 * - remove()
 * - update()
 *
 * In addition, the data store use listeners to perform
 * notifications. The following methods must be implemented
 * to handle them
 * - addListener()
 * - removeListener()
 */
template<class K, class V>
class IDataStore
{
public:
    class IListener
    {
    public:
        virtual ~IListener() {}
        virtual void onAdd(arg_const_reference<K> key, arg_const_reference<V> value) = 0;
        virtual void onRemove(arg_const_reference<K> key) = 0;
        virtual void onUpdate(arg_const_reference<K> key, arg_const_reference<V> value) = 0;
        virtual void onInvalidation() = 0;
    };
    virtual ~IDataStore() {}
    virtual const V * addUnique(arg_rvalue_reference<K> key, arg_rvalue_reference<V> value) = 0;
    virtual const V & add(arg_rvalue_reference<K> key, arg_rvalue_reference<V> value) = 0;
    virtual const V * update(arg_const_reference<K> key, arg_rvalue_reference<V> value) = 0;
    virtual bool remove(arg_const_reference<K> key) = 0;
    virtual void addListener(IDataStore<K, V>::IListener &listener) = 0;
    virtual void removeListener(IDataStore<K, V>::IListener &listener) = 0;
};

}}

#endif // IDATASTORE_H