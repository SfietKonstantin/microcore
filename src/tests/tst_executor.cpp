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
#include "core/executor.h"
#include "error/error.h"
#include "mockexecutorlistener.h"

using namespace ::testing;
using namespace ::microcore::core;
using namespace ::microcore::error;

namespace {

class TestExecutor : public Executor<Error>
{
public:
    bool testCanStart() const
    {
        return canStart();
    }
    bool testCanFinish() const
    {
        return canFinish();
    }
    void testStart()
    {
        doStart();
    }
    void testError(Error &&error)
    {
        doError(std::move(error));
    }
    void testFinish()
    {
        doFinish();
    }
};

class ListenerData
{
public:
    enum class Type
    {
        None,
        Start,
        Finish,
        Error,
        Invalidation
    };
    explicit ListenerData() = default;
    void onStart()
    {
        clear();
        currentType = Type::Start;
    }
    void onFinish()
    {
        clear();
        currentType = Type::Finish;
    }
    void onError(const Error &error)
    {
        clear();
        currentType = Type::Error;
        currentError = std::move(error);
    }
    void onInvalidation(Executor<Error> &source)
    {
        clear();
        currentType = Type::Invalidation;
        invalidatedSource = &source;
    }
    void clear()
    {
        currentType = Type::None;
        currentError = Error();
        invalidatedSource = nullptr;
    }
    Type currentType {Type::None};
    Error currentError {};
    Executor<Error> *invalidatedSource {nullptr};
};

}
using MockTestExecutorListener = MockExecutorListener<Error>;

class TstExecutor: public Test
{
protected:
    void SetUp() override final
    {
        m_executor.reset(new TestExecutor());
        EXPECT_CALL(m_listener, onDestroyed()).WillRepeatedly(Invoke(static_cast<TstExecutor *>(this), &TstExecutor::invalidateOnDestroyed));
        ON_CALL(m_listener, onStart()).WillByDefault(Invoke(&m_listenerData, &ListenerData::onStart));
        ON_CALL(m_listener, onFinish()).WillByDefault(Invoke(&m_listenerData, &ListenerData::onFinish));
        ON_CALL(m_listener, onError(_)).WillByDefault(Invoke(&m_listenerData, &ListenerData::onError));
        ON_CALL(m_listener, onInvalidation(_)).WillByDefault(Invoke(&m_listenerData, &ListenerData::onInvalidation));

    }
    void invalidateOnDestroyed()
    {
        if (!m_invalidated) {
            m_executor->removeListener(m_listener);
        }
    }
    std::unique_ptr<TestExecutor> m_executor {};
    NiceMock<MockTestExecutorListener> m_listener {};
    ListenerData m_listenerData {};
    bool m_invalidated {false};
};

TEST_F(TstExecutor, TestStart)
{
    // Mock
    m_executor->addListener(m_listener);

    // Test
    EXPECT_TRUE(m_executor->testCanStart());
    EXPECT_FALSE(m_executor->testCanFinish());
    m_executor->testStart();
    EXPECT_FALSE(m_executor->testCanStart());
    EXPECT_TRUE(m_executor->testCanFinish());

    EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::Start);
    m_listenerData.clear();

    m_executor->testStart();
    EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::None);
}

TEST_F(TstExecutor, TestFinish)
{
    // Mock
    m_executor->addListener(m_listener);

    // Test
    EXPECT_TRUE(m_executor->testCanStart());
    EXPECT_FALSE(m_executor->testCanFinish());
    m_executor->testStart();
    EXPECT_FALSE(m_executor->testCanStart());
    EXPECT_TRUE(m_executor->testCanFinish());
    m_executor->testFinish();
    EXPECT_TRUE(m_executor->testCanStart());
    EXPECT_FALSE(m_executor->testCanFinish());

    EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::Finish);
    m_listenerData.clear();

    m_executor->testFinish();
    EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::None);
}

TEST_F(TstExecutor, TestError)
{
    // Mock
    m_executor->addListener(m_listener);

    // Test
    EXPECT_TRUE(m_executor->testCanStart());
    EXPECT_FALSE(m_executor->testCanFinish());
    m_executor->testStart();
    EXPECT_FALSE(m_executor->testCanStart());
    EXPECT_TRUE(m_executor->testCanFinish());
    m_executor->testError(Error("test", QLatin1String("Error message"), QByteArray("Error data")));
    EXPECT_TRUE(m_executor->testCanStart());
    EXPECT_FALSE(m_executor->testCanFinish());


    EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::Error);
    EXPECT_EQ(m_listenerData.currentError.id(), "test");
    EXPECT_EQ(m_listenerData.currentError.message(), QLatin1String("Error message"));
    EXPECT_EQ(m_listenerData.currentError.data(), QByteArray("Error data"));

    m_listenerData.clear();

    m_executor->testError(Error("test", QLatin1String("Error message"), QByteArray("Error data")));
    EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::None);
}

TEST_F(TstExecutor, TestListenerDelayAddStart)
{
    m_executor->testStart();
    m_executor->addListener(m_listener);
    EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::Start);
}

TEST_F(TstExecutor, TestListenerDelayAddError)
{
    m_executor->testStart();
    m_executor->testError(Error("test", QLatin1String("Error message"), QByteArray("Error data")));
    m_executor->addListener(m_listener);
    EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::Error);
    EXPECT_EQ(m_listenerData.currentError.id(), "test");
    EXPECT_EQ(m_listenerData.currentError.message(), QLatin1String("Error message"));
    EXPECT_EQ(m_listenerData.currentError.data(), QByteArray("Error data"));
}

TEST_F(TstExecutor, TestListenerInvalidation)
{
    m_executor->addListener(m_listener);
    EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::None);

    TestExecutor *oldExecutor = m_executor.get();
    m_executor.reset();
    EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::Invalidation);
    EXPECT_EQ(m_listenerData.invalidatedSource, oldExecutor);
    m_invalidated = true;
}
