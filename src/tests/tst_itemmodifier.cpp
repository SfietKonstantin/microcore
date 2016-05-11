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
#include "data/item.h"
#include "mockjob.h"
#include "mockjobfactory.h"
#include "mockitemmodifierlistener.h"

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

using TestItem = Item<Result>;
using TestItemModifier = ItemModifier<TestItem, int, Error>;
using MockTestItemModifierListener = MockItemModifierListener<TestItem, int, Error>;
using ModifierJob = MockJob<TestItem::SourceItem_t, Error>;

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
    void onInvalidation(TestItemModifier::Executor_t &source)
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
    TestItemModifier::Executor_t *invalidatedSource {nullptr};
};

}

class TstItemModifier: public Test
{
protected:
    void SetUp()
    {
        std::unique_ptr<Factory_t> factory (new Factory_t());
        m_factory = factory.get();

        m_item.reset(new TestItem());
        m_modifier.reset(new TestItemModifier(*m_item, std::move(factory)));
        EXPECT_CALL(m_listener, onDestroyed()).WillRepeatedly(Invoke(static_cast<TstItemModifier *>(this), &TstItemModifier::invalidateOnDestroyed));
        ON_CALL(m_listener, onStart()).WillByDefault(Invoke(&m_listenerData, &ListenerData::onStart));
        ON_CALL(m_listener, onFinish()).WillByDefault(Invoke(&m_listenerData, &ListenerData::onFinish));
        ON_CALL(m_listener, onError(_)).WillByDefault(Invoke(&m_listenerData, &ListenerData::onError));
        ON_CALL(m_listener, onInvalidation(_)).WillByDefault(Invoke(&m_listenerData, &ListenerData::onInvalidation));
    }
    void invalidateOnDestroyed()
    {
        if (!m_invalidated) {
            m_modifier->removeListener(m_listener);
        }
    }
    using Factory_t = NiceMock<MockJobFactory<int, TestItem::SourceItem_t, Error>>;
    std::unique_ptr<TestItem> m_item {};
    Factory_t *m_factory {nullptr};
    std::unique_ptr<TestItemModifier> m_modifier {};
    NiceMock<MockTestItemModifierListener> m_listener {};
    ListenerData m_listenerData {};
    ModifierJob::OnResult_t m_onResult {};
    ModifierJob::OnError_t m_onError {};
    bool m_invalidated {false};
};

TEST_F(TstItemModifier, TestSuccess)
{
    // Mock
    EXPECT_CALL(*m_factory, mockCreate(_)).Times(0);
    EXPECT_CALL(*m_factory, mockCreate(1)).Times(1).WillRepeatedly(Invoke([this](const int &) {
        std::unique_ptr<ModifierJob> returned (new ModifierJob);
        EXPECT_CALL(*returned, executeImpl(_, _)).Times(1).WillRepeatedly(Invoke([this](ModifierJob::OnResult_t onResult, ModifierJob::OnError_t onError) {
            m_onResult = onResult;
            m_onError = onError;
        }));
        return returned;
    }));
    m_modifier->addListener(m_listener);

    // Test
    EXPECT_TRUE(m_modifier->start(1));

    EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::Start);
    EXPECT_EQ(m_item->data().value, 0);
    m_onResult(Result(1));

    EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::Finish);
    EXPECT_EQ(m_item->data().value, 1);
}

TEST_F(TstItemModifier, TestError)
{
    // Mock
    EXPECT_CALL(*m_factory, mockCreate(_)).Times(0);
    EXPECT_CALL(*m_factory, mockCreate(1)).Times(1).WillRepeatedly(Invoke([this](const int &) {
        std::unique_ptr<ModifierJob> returned (new ModifierJob);
        EXPECT_CALL(*returned, executeImpl(_, _)).Times(1).WillRepeatedly(Invoke([this](ModifierJob::OnResult_t onResult, ModifierJob::OnError_t onError) {
            m_onResult = onResult;
            m_onError = onError;
        }));
        return returned;
    }));
    m_modifier->addListener(m_listener);

    // Test
    EXPECT_TRUE(m_modifier->start(1));

    EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::Start);
    EXPECT_EQ(m_item->data().value, 0);
    m_onError(Error("test", QLatin1String("Error message"), QByteArray("Error data")));

    EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::Error);
    EXPECT_EQ(m_listenerData.currentError.id(), "test");
    EXPECT_EQ(m_listenerData.currentError.message(), QLatin1String("Error message"));
    EXPECT_EQ(m_listenerData.currentError.data(), QByteArray("Error data"));

    EXPECT_EQ(m_item->data().value, 0);
}

TEST_F(TstItemModifier, TestBusy)
{
    // Mock
    ON_CALL(*m_factory, mockCreate(_)).WillByDefault(Invoke([this](const int &) {
        std::unique_ptr<ModifierJob> returned (new ModifierJob);
        EXPECT_CALL(*returned, executeImpl(_, _)).Times(1).WillRepeatedly(Invoke([this](ModifierJob::OnResult_t onResult, ModifierJob::OnError_t onError) {
            m_onResult = onResult;
            m_onError = onError;
        }));
        return returned;
    }));

    // Test
    EXPECT_TRUE(m_modifier->start(1));
    EXPECT_FALSE(m_modifier->start(2));
    m_onResult(Result(1));
    EXPECT_TRUE(m_modifier->start(3));
}

TEST_F(TstItemModifier, TestListenerDelayAddStart)
{
    // Mock
    ON_CALL(*m_factory, mockCreate(_)).WillByDefault(Invoke([this](const int &) {
        std::unique_ptr<ModifierJob> returned (new ModifierJob);
        EXPECT_CALL(*returned, executeImpl(_, _)).Times(1).WillRepeatedly(Invoke([this](ModifierJob::OnResult_t onResult, ModifierJob::OnError_t onError) {
            m_onResult = onResult;
            m_onError = onError;
        }));
        return returned;
    }));

    // Test
    EXPECT_TRUE(m_modifier->start(1));
    m_modifier->addListener(m_listener);
    EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::Start);
}

TEST_F(TstItemModifier, TestListenerDelayAddError)
{
    // Mock
    ON_CALL(*m_factory, mockCreate(_)).WillByDefault(Invoke([this](const int &) {
        std::unique_ptr<ModifierJob> returned (new ModifierJob);
        EXPECT_CALL(*returned, executeImpl(_, _)).Times(1).WillRepeatedly(Invoke([this](ModifierJob::OnResult_t onResult, ModifierJob::OnError_t onError) {
            m_onResult = onResult;
            m_onError = onError;
        }));
        return returned;
    }));

    // Test
    EXPECT_TRUE(m_modifier->start(1));
    m_onError(Error("test", QLatin1String("Error message"), QByteArray("Error data")));
    m_modifier->addListener(m_listener);
    EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::Error);
    EXPECT_EQ(m_listenerData.currentError.id(), "test");
    EXPECT_EQ(m_listenerData.currentError.message(), QLatin1String("Error message"));
    EXPECT_EQ(m_listenerData.currentError.data(), QByteArray("Error data"));
}

TEST_F(TstItemModifier, TestListenerInvalidation)
{
    m_modifier->addListener(m_listener);
    EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::None);

    TestItemModifier::Executor_t *oldAppender = m_modifier.get();
    m_modifier.reset();
    EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::Invalidation);
    EXPECT_EQ(m_listenerData.invalidatedSource, oldAppender);
    m_invalidated = true;
}
