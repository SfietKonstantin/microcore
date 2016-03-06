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
#include "data/model.h"
#include "mockjob.h"
#include "mockjobfactory.h"
#include "mockmodeloperatorlistener.h"

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
    void onInvalidation()
    {
        clear();
        currentType = Type::Invalidation;
    }
    Type currentType {Type::None};
    Error currentError {};
private:
    void clear()
    {
        currentType = Type::None;
        currentError = Error();
    }
};

}

using TestModel = Model<Result>;
using TestModelAppender = ModelAppender<TestModel, int, Error>;
using MockTestModelAppenderListener = MockModelAppenderListener<TestModel, int, Error>;
using AppenderJob = MockJob<TestModel::Source_t, Error>;

class TstModeOperator: public Test
{
protected:
    void SetUp()
    {
        m_model.reset(new TestModel());
        m_appender.reset(new TestModelAppender(*m_model, m_appenderFactory));
        EXPECT_CALL(m_appenderListener, onDestroyed()).WillRepeatedly(Invoke(static_cast<TstModeOperator *>(this), &TstModeOperator::invalidateOnDestroyed));
        ON_CALL(m_appenderListener, onAppendStart()).WillByDefault(Invoke(&m_appenderListenerData, &ListenerData::onStart));
        ON_CALL(m_appenderListener, onAppendFinish()).WillByDefault(Invoke(&m_appenderListenerData, &ListenerData::onFinish));
        ON_CALL(m_appenderListener, onAppendError(_)).WillByDefault(Invoke(&m_appenderListenerData, &ListenerData::onError));
        ON_CALL(m_appenderListener, onAppendInvalidation()).WillByDefault(Invoke(&m_appenderListenerData, &ListenerData::onInvalidation));
    }
    void invalidateOnDestroyed()
    {
        if (!m_invalidated) {
            m_appender->removeListener(m_appenderListener);
        }
    }
    std::unique_ptr<TestModel> m_model {};
    NiceMock<MockJobFactory<int, TestModel::Source_t, Error>> m_appenderFactory {};
    std::unique_ptr<TestModelAppender> m_appender {};
    NiceMock<MockTestModelAppenderListener> m_appenderListener {};
    ListenerData m_appenderListenerData {};
    AppenderJob::OnResult_t m_appenderOnResult {};
    AppenderJob::OnError_t m_appenderOnError {};
    bool m_invalidated {false};
};

TEST_F(TstModeOperator, TestSuccess)
{
    // Mock
    EXPECT_CALL(m_appenderFactory, mockCreate(_)).Times(0);
    EXPECT_CALL(m_appenderFactory, mockCreate(1)).Times(1).WillRepeatedly(Invoke([this](const int &) {
        std::unique_ptr<AppenderJob> returned (new AppenderJob);
        EXPECT_CALL(*returned, execute(_, _)).Times(1).WillRepeatedly(Invoke([this](AppenderJob::OnResult_t onResult, AppenderJob::OnError_t onError) {
            m_appenderOnResult = onResult;
            m_appenderOnError = onError;
        }));
        return returned;
    }));
    m_appender->addListener(m_appenderListener);
    m_model->append({Result(1), Result(2)});

    // Test
    EXPECT_TRUE(m_appender->start(1));

    EXPECT_EQ(m_appenderListenerData.currentType, ListenerData::Type::Start);
    EXPECT_EQ(m_model->size(), static_cast<std::size_t>(2));
    m_appenderOnResult({Result(3), Result(4), Result(5)});

    EXPECT_EQ(m_appenderListenerData.currentType, ListenerData::Type::Finish);
    {
        EXPECT_FALSE(m_model->empty());
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

TEST_F(TstModeOperator, TestError)
{
    // Mock
    EXPECT_CALL(m_appenderFactory, mockCreate(_)).Times(0);
    EXPECT_CALL(m_appenderFactory, mockCreate(1)).Times(1).WillRepeatedly(Invoke([this](const int &) {
        std::unique_ptr<AppenderJob> returned (new AppenderJob);
        EXPECT_CALL(*returned, execute(_, _)).Times(1).WillRepeatedly(Invoke([this](AppenderJob::OnResult_t onResult, AppenderJob::OnError_t onError) {
            m_appenderOnResult = onResult;
            m_appenderOnError = onError;
        }));
        return returned;
    }));
    m_appender->addListener(m_appenderListener);

    // Test
    EXPECT_TRUE(m_appender->start(1));

    EXPECT_EQ(m_appenderListenerData.currentType, ListenerData::Type::Start);
    EXPECT_TRUE(m_model->empty());
    m_appenderOnError(Error("test", QLatin1String("Error message"), QByteArray("Error data")));

    EXPECT_EQ(m_appenderListenerData.currentType, ListenerData::Type::Error);
    EXPECT_EQ(m_appenderListenerData.currentError.id(), "test");
    EXPECT_EQ(m_appenderListenerData.currentError.message(), QLatin1String("Error message"));
    EXPECT_EQ(m_appenderListenerData.currentError.data(), QByteArray("Error data"));

    EXPECT_TRUE(m_model->empty());
}

TEST_F(TstModeOperator, TestBusy)
{
    // Mock
    ON_CALL(m_appenderFactory, mockCreate(_)).WillByDefault(Invoke([this](const int &) {
        std::unique_ptr<AppenderJob> returned (new AppenderJob);
        EXPECT_CALL(*returned, execute(_, _)).Times(1).WillRepeatedly(Invoke([this](AppenderJob::OnResult_t onResult, AppenderJob::OnError_t onError) {
            m_appenderOnResult = onResult;
            m_appenderOnError = onError;
        }));
        return returned;
    }));

    // Test
    EXPECT_TRUE(m_appender->start(1));
    EXPECT_FALSE(m_appender->start(2));
    m_appenderOnResult({Result(1), Result(2)});
    EXPECT_TRUE(m_appender->start(3));
}

TEST_F(TstModeOperator, TestListenerDelayAddStart)
{
    // Mock
    ON_CALL(m_appenderFactory, mockCreate(_)).WillByDefault(Invoke([this](const int &) {
        std::unique_ptr<AppenderJob> returned (new AppenderJob);
        EXPECT_CALL(*returned, execute(_, _)).Times(1).WillRepeatedly(Invoke([this](AppenderJob::OnResult_t onResult, AppenderJob::OnError_t onError) {
            m_appenderOnResult = onResult;
            m_appenderOnError = onError;
        }));
        return returned;
    }));

    // Test
    EXPECT_TRUE(m_appender->start(1));
    m_appender->addListener(m_appenderListener);
    EXPECT_EQ(m_appenderListenerData.currentType, ListenerData::Type::Start);
}

TEST_F(TstModeOperator, TestListenerDelayAddError)
{
    m_appender->error(Error("test", QLatin1String("Error message"), QByteArray("Error data")));
    m_appender->addListener(m_appenderListener);
    EXPECT_EQ(m_appenderListenerData.currentType, ListenerData::Type::Error);
    EXPECT_EQ(m_appenderListenerData.currentError.id(), "test");
    EXPECT_EQ(m_appenderListenerData.currentError.message(), QLatin1String("Error message"));
    EXPECT_EQ(m_appenderListenerData.currentError.data(), QByteArray("Error data"));
}

TEST_F(TstModeOperator, TestListenerInvalidation)
{
    m_appender->addListener(m_appenderListener);
    EXPECT_EQ(m_appenderListenerData.currentType, ListenerData::Type::None);

    m_appender.reset();
    EXPECT_EQ(m_appenderListenerData.currentType, ListenerData::Type::Invalidation);
    m_invalidated = true;
}
