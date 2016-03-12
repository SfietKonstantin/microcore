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

#ifndef MICROCORE_DATA_MOCKMODELAPPENDERLISTENER_H
#define MICROCORE_DATA_MOCKMODELAPPENDERLISTENER_H

#include <gmock/gmock.h>
#include "data/modelappender.h"

namespace microcore { namespace data {

template<class Model, class Request, class Error>
class MockModelAppenderListener: public ModelAppender<Model, Request, Error>::Listener_t
{
public:
    ~MockModelAppenderListener()
    {
        onDestroyed();
    }
    MOCK_METHOD0_T(onDestroyed, void ());
    MOCK_METHOD0_T(onStart, void ());
    MOCK_METHOD0_T(onFinish, void ());
    MOCK_METHOD1_T(onError, void (const Error &error));
    MOCK_METHOD1_T(onInvalidation, void (typename ModelAppender<Model, Request, Error>::Executor_t &source));
};

}}

#endif // MICROCORE_DATA_MOCKMODELAPPENDERLISTENER_H