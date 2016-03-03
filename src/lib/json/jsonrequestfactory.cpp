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

#include "jsonrequestfactory.h"

using namespace ::microcore::core;
using namespace ::microcore::error;

namespace microcore { namespace json {

class JsonRequestJob: public JsonJob
{
public:
    explicit JsonRequestJob(JsonRequest &&request)
        : m_request(std::move(request))
    {
    }
    void execute(OnResult_t onResult, OnError_t onError) override
    {
        QJsonParseError error {};
        const QByteArray &data {m_request->readAll()};
        QJsonDocument document {QJsonDocument::fromJson(data, &error)};
        if (error.error != QJsonParseError::NoError) {
            onError(Error("json", error.errorString(), data));
        } else {
            onResult(std::move(document));
        }
    }
private:
    JsonRequest m_request {};
};

std::unique_ptr<JsonJob> JsonRequestFactory::create(JsonRequest &&request) const
{
    return std::unique_ptr<JsonJob>(new JsonRequestJob(std::move(request)));
}

}}
