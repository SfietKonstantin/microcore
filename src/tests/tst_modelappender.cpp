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
#include "error/error.h"
#include "data/model.h"
#include "mockjob.h"
#include "mockjobfactory.h"
#include "mockmodelappenderlistener.h"

using namespace std::placeholders;
using namespace ::testing;
using namespace ::microcore::core;
using namespace ::microcore::data;
using namespace ::microcore::error;

namespace {

class Result
{
public:
    explicit Result() = default;
    explicit Result(int v) : value {v} {}
    DEFAULT_COPY_DEFAULT_MOVE(Result);
    int value {0};
};

}

using TestViewModel = Model<Result>;
using TestModelAppender = ModelAppender<TestViewModel, int, Error>;
using MockTestModelAppenderListener = MockModelAppenderListener<TestViewModel, int, Error>;
using AppenderJob = MockJob<TestViewModel::SourceItems_t, Error>;

namespace {

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
    void onInvalidation(TestModelAppender::Executor_t &source)
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
    TestModelAppender::Executor_t *invalidatedSource {nullptr};
};

}

class TstModelAppender: public Test
{
protected:
    void SetUp()
    {
        std::unique_ptr<Factory_t> appenderFactory (new Factory_t());
        m_factory = appenderFactory.get();

        m_model.reset(new TestViewModel());
        m_appender.reset(new TestModelAppender(*m_model, std::move(appenderFactory)));
        EXPECT_CALL(m_listener, onDestroyed()).WillRepeatedly(Invoke(static_cast<TstModelAppender *>(this), &TstModelAppender::invalidateOnDestroyed));
        ON_CALL(m_listener, onStart()).WillByDefault(Invoke(&m_listenerData, &ListenerData::onStart));
        ON_CALL(m_listener, onFinish()).WillByDefault(Invoke(&m_listenerData, &ListenerData::onFinish));
        ON_CALL(m_listener, onError(_)).WillByDefault(Invoke(&m_listenerData, &ListenerData::onError));
        ON_CALL(m_listener, onInvalidation(_)).WillByDefault(Invoke(&m_listenerData, &ListenerData::onInvalidation));
    }
    void invalidateOnDestroyed()
    {
        if (!m_invalidated) {
            m_appender->removeListener(m_listener);
        }
    }
    using Factory_t = NiceMock<MockJobFactory<int, TestViewModel::SourceItems_t, Error>>;
    std::unique_ptr<TestViewModel> m_model {};
    Factory_t *m_factory {nullptr};
    std::unique_ptr<TestModelAppender> m_appender {};
    NiceMock<MockTestModelAppenderListener> m_listener {};
    ListenerData m_listenerData {};
    AppenderJob::OnResult_t m_onResult {};
    AppenderJob::OnError_t m_onError {};
    bool m_invalidated {false};
};

TEST_F(TstModelAppender, TestSuccess)
{
    // Mock
    EXPECT_CALL(*m_factory, mockCreate(_)).Times(0);
    EXPECT_CALL(*m_factory, mockCreate(1)).Times(1).WillRepeatedly(Invoke([this](const int &) {
        std::unique_ptr<AppenderJob> returned (new AppenderJob);
        EXPECT_CALL(*returned, executeImpl(_, _)).Times(1).WillRepeatedly(Invoke([this](AppenderJob::OnResult_t onResult, AppenderJob::OnError_t onError) {
            m_onResult = onResult;
            m_onError = onError;
        }));
        return returned;
    }));
    m_appender->addListener(m_listener);
    m_model->append({Result(1), Result(2)});

    // Test
    EXPECT_TRUE(m_appender->start(1));

    EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::Start);
    EXPECT_EQ(m_model->size(), static_cast<std::size_t>(2));
    m_onResult({Result(3), Result(4), Result(5)});

    EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::Finish);
    {
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(5));

        auto it = std::begin(*m_model);
        EXPECT_EQ((*it)->value, 1);
        ++it;
        EXPECT_EQ((*it)->value, 2);
        ++it;
        EXPECT_EQ((*it)->value, 3);
        ++it;
        EXPECT_EQ((*it)->value, 4);
        ++it;
        EXPECT_EQ((*it)->value, 5);
        ++it;
        EXPECT_TRUE(it == std::end(*m_model));
    }
}

TEST_F(TstModelAppender, TestError)
{
    // Mock
    EXPECT_CALL(*m_factory, mockCreate(_)).Times(0);
    EXPECT_CALL(*m_factory, mockCreate(1)).Times(1).WillRepeatedly(Invoke([this](const int &) {
        std::unique_ptr<AppenderJob> returned (new AppenderJob);
        EXPECT_CALL(*returned, executeImpl(_, _)).Times(1).WillRepeatedly(Invoke([this](AppenderJob::OnResult_t onResult, AppenderJob::OnError_t onError) {
            m_onResult = onResult;
            m_onError = onError;
        }));
        return returned;
    }));
    m_appender->addListener(m_listener);

    // Test
    EXPECT_TRUE(m_appender->start(1));

    EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::Start);
    EXPECT_TRUE(m_model->empty());
    m_onError(Error("test", QLatin1String("Error message"), QByteArray("Error data")));

    EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::Error);
    EXPECT_EQ(m_listenerData.currentError.id(), "test");
    EXPECT_EQ(m_listenerData.currentError.message(), QLatin1String("Error message"));
    EXPECT_EQ(m_listenerData.currentError.data(), QByteArray("Error data"));

    EXPECT_TRUE(m_model->empty());
}

TEST_F(TstModelAppender, TestBusy)
{
    // Mock
    ON_CALL(*m_factory, mockCreate(_)).WillByDefault(Invoke([this](const int &) {
        std::unique_ptr<AppenderJob> returned (new AppenderJob);
        EXPECT_CALL(*returned, executeImpl(_, _)).Times(1).WillRepeatedly(Invoke([this](AppenderJob::OnResult_t onResult, AppenderJob::OnError_t onError) {
            m_onResult = onResult;
            m_onError = onError;
        }));
        return returned;
    }));

    // Test
    EXPECT_TRUE(m_appender->start(1));
    EXPECT_FALSE(m_appender->start(2));
    m_onResult({Result(1), Result(2)});
    EXPECT_TRUE(m_appender->start(3));
}

TEST_F(TstModelAppender, TestListenerDelayAddStart)
{
    // Mock
    ON_CALL(*m_factory, mockCreate(_)).WillByDefault(Invoke([this](const int &) {
        std::unique_ptr<AppenderJob> returned (new AppenderJob);
        EXPECT_CALL(*returned, executeImpl(_, _)).Times(1).WillRepeatedly(Invoke([this](AppenderJob::OnResult_t onResult, AppenderJob::OnError_t onError) {
            m_onResult = onResult;
            m_onError = onError;
        }));
        return returned;
    }));

    // Test
    EXPECT_TRUE(m_appender->start(1));
    m_appender->addListener(m_listener);
    EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::Start);
}

TEST_F(TstModelAppender, TestListenerDelayAddError)
{
    // Mock
    ON_CALL(*m_factory, mockCreate(_)).WillByDefault(Invoke([this](const int &) {
        std::unique_ptr<AppenderJob> returned (new AppenderJob);
        EXPECT_CALL(*returned, executeImpl(_, _)).Times(1).WillRepeatedly(Invoke([this](AppenderJob::OnResult_t onResult, AppenderJob::OnError_t onError) {
            m_onResult = onResult;
            m_onError = onError;
        }));
        return returned;
    }));

    // Test
    EXPECT_TRUE(m_appender->start(1));
    m_onError(Error("test", QLatin1String("Error message"), QByteArray("Error data")));
    m_appender->addListener(m_listener);
    EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::Error);
    EXPECT_EQ(m_listenerData.currentError.id(), "test");
    EXPECT_EQ(m_listenerData.currentError.message(), QLatin1String("Error message"));
    EXPECT_EQ(m_listenerData.currentError.data(), QByteArray("Error data"));
}

TEST_F(TstModelAppender, TestListenerInvalidation)
{
    m_appender->addListener(m_listener);
    EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::None);

    TestModelAppender::Executor_t *oldAppender = m_appender.get();
    m_appender.reset();
    EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::Invalidation);
    EXPECT_EQ(m_listenerData.invalidatedSource, oldAppender);
    m_invalidated = true;
}
