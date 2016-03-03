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

#include "httprequestfactory.h"
#include <QNetworkReply>

using namespace ::microcore::core;
using namespace ::microcore::error;

namespace microcore { namespace http {

class HttpRequestJob: public HttpJob
{
public:
    explicit HttpRequestJob(QNetworkAccessManager &network, HttpRequest &&request)
        : m_network(network)
        , m_request(std::move(request))
    {
    }
    void execute(ICallback &callback) override
    {
        QNetworkReply *reply {nullptr};
        switch (m_request.type()) {
        case HttpRequest::Type::Get:
            reply = m_network.get(m_request.request());
            break;
        case HttpRequest::Type::Post:
            reply = m_network.post(m_request.request(), m_request.postData());
            break;
        case HttpRequest::Type::Delete:
            reply = m_network.deleteResource(m_request.request());
            break;
        default:
            break;
        }
        Q_ASSERT_X(reply != nullptr, "execute", "HttpRequest::Type must be Get, Post or Delete");

        reply->setParent(nullptr);
        m_result.reset(reply);

        QObject::connect(reply, &QNetworkReply::finished, [this, reply, &callback]() {
            if (reply->error() != QNetworkReply::NoError) {
                callback.onError(Error("http", reply->errorString()));
            } else {
                callback.onResult(std::move(m_result));
            }
        });
    }
private:
    QNetworkAccessManager &m_network;
    HttpRequest m_request {};
    HttpResult m_result {};
};

HttpRequestFactory::HttpRequestFactory(QNetworkAccessManager &network)
    : m_network(network)
{
}

std::unique_ptr<HttpJob> HttpRequestFactory::create(HttpRequest &&request) const
{
    return std::unique_ptr<HttpJob>{new HttpRequestJob(m_network, std::move(request))};
}

}}
