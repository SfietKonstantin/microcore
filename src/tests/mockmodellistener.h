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

#ifndef MICROCORE_DATA_MOCKMODELLISTENER_H
#define MICROCORE_DATA_MOCKMODELLISTENER_H

#include <gmock/gmock.h>
#include "data/model.h"

namespace microcore { namespace data {

template<class Request, class Data, class Store>
class MockModelListener: public ModelBase<Request, Data, Store>::IListener
{
public:
    ~MockModelListener()
    {
        onDestroyed();
    }
    MOCK_METHOD0_T(onDestroyed, void ());
    MOCK_METHOD1_T(onAppend, void (const std::vector<const Data *> &items));
    MOCK_METHOD1_T(onPrepend, void (const std::vector<const Data *> &items));
    MOCK_METHOD1_T(onUpdate, void (std::size_t index));
    MOCK_METHOD1_T(onRemove, void (std::size_t index));
    MOCK_METHOD2_T(onMove, void (std::size_t from, std::size_t to));
    MOCK_METHOD0_T(onInvalidation, void ());
    MOCK_METHOD0_T(onStart, void ());
    MOCK_METHOD1_T(onError, void (const ::microcore::error::Error &error));
    MOCK_METHOD0_T(onFinish, void ());
};

}}

#endif // MICROCORE_DATA_MOCKMODELLISTENER_H
