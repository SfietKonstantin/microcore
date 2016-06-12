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

#ifndef IMODEL_H
#define IMODEL_H

#include <vector>

namespace microcore { namespace data {

template<class T, class S>
class IModel
{
public:
    using Type = T;
    using StorageType = S;
    class IListener
    {
    public:
        virtual ~IListener() {}
        virtual void onAppend(const std::vector<const T *> &values) = 0;
        virtual void onPrepend(const std::vector<const T *> &values) = 0;
        virtual void onInsert(typename S::size_type index, const std::vector<const T *> &values) = 0;
        virtual void onRemove(typename S::size_type index) = 0;
        virtual void onUpdate(typename S::size_type index, const T &value) = 0;
        virtual void onMove(typename S::size_type oldIndex, typename S::size_type newIndex) = 0;
        virtual void onInvalidation() = 0;
    };
    virtual ~IModel() {}
    virtual typename S::iterator begin() noexcept = 0;
    virtual typename S::iterator end() noexcept = 0;
    virtual typename S::const_iterator begin() const noexcept = 0;
    virtual typename S::const_iterator end() const noexcept = 0;
    virtual bool empty() const noexcept = 0;
    virtual typename S::size_type size() const noexcept = 0;
    virtual const T * operator[](typename S::size_type index) const = 0;
    virtual void addListener(IListener &listener) = 0;
    virtual void removeListener(IListener &listener) = 0;
};

}}

#endif // IMODEL_H
