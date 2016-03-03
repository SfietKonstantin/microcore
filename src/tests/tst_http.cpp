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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QtTest/QTest>
#include <QElapsedTimer>
#include "core/globals.h"
#include "core/pipe.h"
#include "http/httprequestfactory.h"
#include "mockjob.h"

using namespace ::testing;
using namespace ::microcore::core;
using namespace ::microcore::http;

using HttpPipe = Pipe<HttpRequest, HttpResult, HttpError>;

class TstHttp: public Test
{
protected:
    void SetUp() override
    {
        using namespace std::placeholders;
        auto onResult {std::bind(&TstHttp::onResult, this, _1)};
        auto onError {std::bind(&TstHttp::onError, this, _1)};

        m_factory.reset(new HttpRequestFactory(m_network));
        m_pipe.reset(new HttpPipe(*m_factory, std::move(onResult), std::move(onError)));
    }
    MOCK_METHOD1(mockOnResult, void (const HttpResult &result));
    void onResult(HttpResult &&result)
    {
        mockOnResult(result);
    }
    MOCK_METHOD1(mockOnError, void (const HttpError &error));
    void onError(HttpError &&error)
    {
        mockOnError(error);
    }
    QNetworkAccessManager m_network {};
    std::unique_ptr<HttpRequestFactory> m_factory {};
    std::unique_ptr<HttpPipe> m_pipe {};
};

TEST_F(TstHttp, TestSimpleSuccess)
{
    // Mock
    bool called {false};
    EXPECT_CALL(*this, mockOnResult(_)).Times(1).WillRepeatedly(Invoke([&called](const HttpResult &) {
        called = true;
    }));
    QElapsedTimer timer {};

    // Test
    m_pipe->send(HttpRequest(HttpRequest::Type::Get, QNetworkRequest(QUrl("http://www.google.fr"))));
    timer.start();
    while (!called && !timer.hasExpired(30000)) {
        QTest::qWait(300);
    }
    EXPECT_TRUE(called);
}
