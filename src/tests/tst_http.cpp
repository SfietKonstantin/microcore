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
using HttpJob = IJob<HttpResult, HttpError>;

class TstHttp: public Test
{
protected:
    void SetUp() override
    {
        using namespace std::placeholders;
        HttpJob::OnResult_t onResult {std::bind(&TstHttp::onResult, this, _1)};
        HttpJob::OnError_t onError {std::bind(&TstHttp::onError, this, _1)};

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

#ifdef ENABLE_MOCK_SERVER

TEST_F(TstHttp, TestGetSuccess)
{
    // Mock
    bool called {false};
    EXPECT_CALL(*this, mockOnResult(_)).Times(1).WillRepeatedly(Invoke([&called](const HttpResult &result) {
        called = true;
        EXPECT_EQ(result->readAll(), QByteArray("Hello world from /api/get"));
    }));
    QElapsedTimer timer {};

    // Test
    m_pipe->send(HttpRequest(HttpRequest::Type::Get, QNetworkRequest(QUrl("http://localhost:8080/api/get"))));
    timer.start();
    while (!called && !timer.hasExpired(5000)) {
        QTest::qWait(300);
    }
    EXPECT_TRUE(called);
}

TEST_F(TstHttp, TestGetError)
{
    // Mock
    bool called {false};
    EXPECT_CALL(*this, mockOnError(_)).Times(1).WillRepeatedly(Invoke([&called](const HttpError &error) {
        called = true;
        EXPECT_EQ(error.id(), "http");
        EXPECT_TRUE(!error.message().isEmpty());
        EXPECT_EQ(error.data(), QByteArray("Error from /api/geterror"));
    }));
    QElapsedTimer timer {};

    // Test
    m_pipe->send(HttpRequest(HttpRequest::Type::Get, QNetworkRequest(QUrl("http://localhost:8080/api/geterror"))));
    timer.start();
    while (!called && !timer.hasExpired(5000)) {
        QTest::qWait(300);
    }
    EXPECT_TRUE(called);
}

TEST_F(TstHttp, TestPostSuccess)
{
    // Mock
    bool called {false};
    EXPECT_CALL(*this, mockOnResult(_)).Times(1).WillRepeatedly(Invoke([&called](const HttpResult &result) {
        called = true;
        EXPECT_EQ(result->readAll(), QByteArray("Hello world from /api/post"));
    }));
    QElapsedTimer timer {};

    // Test
    m_pipe->send(HttpRequest(HttpRequest::Type::Post, QNetworkRequest(QUrl("http://localhost:8080/api/post"))));
    timer.start();
    while (!called && !timer.hasExpired(5000)) {
        QTest::qWait(300);
    }
    EXPECT_TRUE(called);
}

TEST_F(TstHttp, TestPostError)
{
    // Mock
    bool called {false};
    EXPECT_CALL(*this, mockOnError(_)).Times(1).WillRepeatedly(Invoke([&called](const HttpError &error) {
        called = true;
        EXPECT_EQ(error.id(), "http");
        EXPECT_TRUE(!error.message().isEmpty());
        EXPECT_EQ(error.data(), QByteArray("Error from /api/posterror"));
    }));
    QElapsedTimer timer {};

    // Test
    m_pipe->send(HttpRequest(HttpRequest::Type::Post, QNetworkRequest(QUrl("http://localhost:8080/api/posterror"))));
    timer.start();
    while (!called && !timer.hasExpired(5000)) {
        QTest::qWait(300);
    }
    EXPECT_TRUE(called);
}

TEST_F(TstHttp, TestDeleteSuccess)
{
    // Mock
    bool called {false};
    EXPECT_CALL(*this, mockOnResult(_)).Times(1).WillRepeatedly(Invoke([&called](const HttpResult &result) {
        called = true;
        EXPECT_EQ(result->readAll(), QByteArray("Hello world from /api/delete"));
    }));
    QElapsedTimer timer {};

    // Test
    m_pipe->send(HttpRequest(HttpRequest::Type::Delete, QNetworkRequest(QUrl("http://localhost:8080/api/delete"))));
    timer.start();
    while (!called && !timer.hasExpired(5000)) {
        QTest::qWait(300);
    }
    EXPECT_TRUE(called);
}

TEST_F(TstHttp, TestDeleteError)
{
    // Mock
    bool called {false};
    EXPECT_CALL(*this, mockOnError(_)).Times(1).WillRepeatedly(Invoke([&called](const HttpError &error) {
        called = true;
        EXPECT_EQ(error.id(), "http");
        EXPECT_TRUE(!error.message().isEmpty());
        EXPECT_EQ(error.data(), QByteArray("Error from /api/deleteerror"));
    }));
    QElapsedTimer timer {};

    // Test
    m_pipe->send(HttpRequest(HttpRequest::Type::Delete, QNetworkRequest(QUrl("http://localhost:8080/api/deleteerror"))));
    timer.start();
    while (!called && !timer.hasExpired(5000)) {
        QTest::qWait(300);
    }
    EXPECT_TRUE(called);
}

#endif // ENABLE_MOCK_SERVER
