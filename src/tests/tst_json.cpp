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
#include <QBuffer>
#include <QJsonObject>
#include <microcore/core/globals.h>
#include <microcore/core/pipe.h>
#include <microcore/json/jsonrequestfactory.h>
#include "mockjob.h"

using namespace ::testing;
using namespace ::microcore;
using namespace ::microcore::core;
using namespace ::microcore::json;

using JsonPipe = Pipe<JsonRequest, JsonResult, JsonError>;
using JsonJob = IJob<JsonResult, JsonError>;

class TstJson: public Test
{
protected:
    void SetUp() override final
    {
        using namespace std::placeholders;
        JsonJob::OnResult onResult {std::bind(&TstJson::onResult, this, _1)};
        JsonJob::OnError onError {std::bind(&TstJson::onError, this, _1)};

        m_factory.reset(new JsonRequestFactory());
        m_pipe.reset(new JsonPipe(*m_factory, std::move(onResult), std::move(onError)));
    }
    MOCK_METHOD1(mockOnResult, void (const JsonResult &result));
    void onResult(JsonResult &&result)
    {
        mockOnResult(result);
    }
    MOCK_METHOD1(mockOnError, void (const JsonError &error));
    void onError(JsonError &&error)
    {
        mockOnError(error);
    }
    std::unique_ptr<JsonRequestFactory> m_factory {};
    std::unique_ptr<JsonPipe> m_pipe {};
};

TEST_F(TstJson, TestSuccess)
{
    // Mock
    bool called {false};
    EXPECT_CALL(*this, mockOnResult(_)).Times(1).WillRepeatedly(Invoke([&called](const JsonResult &result) {
        called = true;
        QJsonObject expected {
            {"hello", "world"},
            {"test", 123.}
        };
        EXPECT_EQ(result.object(), expected);
    }));
    QByteArray data {"{\"hello\": \"world\", \"test\": 123}"};

    // Test
    QObjectPtr<QIODevice> bufferPtr {new QBuffer(&data)};
    bufferPtr->open(QIODevice::ReadOnly);
    m_pipe->send(JsonRequest(std::move(bufferPtr)));
    EXPECT_TRUE(called);
}

TEST_F(TstJson, TestError)
{
    // Mock
    bool called {false};
    EXPECT_CALL(*this, mockOnError(_)).Times(1).WillRepeatedly(Invoke([&called](const JsonError &error) {
        called = true;
        EXPECT_EQ(error.id(), "json");
        EXPECT_TRUE(!error.message().isEmpty());
        EXPECT_EQ(error.data(), QByteArray("{"));
    }));
    QByteArray data {"{"};

    // Test
    QObjectPtr<QIODevice> bufferPtr {new QBuffer(&data)};
    bufferPtr->open(QIODevice::ReadOnly);
    m_pipe->send(JsonRequest(std::move(bufferPtr)));
    EXPECT_TRUE(called);
}
