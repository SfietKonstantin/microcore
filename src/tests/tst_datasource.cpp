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
#include "data/datasource.h"
#include "mockdatasourcelistener.h"

using namespace ::testing;
using namespace ::microcore::data;

namespace {

class Result
{
public:
    explicit Result() = default;
    explicit Result(int k, int v) : key {k}, value {v} {}
    DISABLE_COPY_DEFAULT_MOVE(Result);
    bool operator==(const Result &other) const
    {
        return other.key == key && other.value == value;
    }
    int getKey() const
    {
       return key;
    }
    int key {0};
    int value {0};
};

class ListenerData
{
public:
    explicit ListenerData() = default;
    void onAdd(const Result &result)
    {
        lastUpdated = nullptr;
        results.insert(&result);
    }
    void onRemove(const Result &result)
    {
        lastUpdated = nullptr;
        results.erase(&result);
    }
    void onUpdate(const Result &result)
    {
        lastUpdated = &result;
    }
    void onInvalidation()
    {
        lastUpdated = nullptr;
        invalidated = true;
    }
    std::set<const Result *> results {};
    bool invalidated {false};
    const Result *lastUpdated {nullptr};
};

}

using ResultSource = DataSource<int, Result>;
using MockResultSourceListener = MockDataSourceListener<int, Result>;

class TstDataSource: public Test
{
protected:
   void SetUp() override
   {
      m_data.reset(new ResultSource(std::function<int (const Result &)>(&Result::getKey)));
      EXPECT_CALL(m_listener, onDestroyed()).WillRepeatedly(Invoke(static_cast<TstDataSource *>(this), &TstDataSource::invalidateOnDestroyed));
      ON_CALL(m_listener, onAdd(_)).WillByDefault(Invoke(&m_listenerData, &ListenerData::onAdd));
      ON_CALL(m_listener, onUpdate(_)).WillByDefault(Invoke(&m_listenerData, &ListenerData::onUpdate));
      ON_CALL(m_listener, onRemove(_)).WillByDefault(Invoke(&m_listenerData, &ListenerData::onRemove));
      ON_CALL(m_listener, onInvalidation()).WillByDefault(Invoke(&m_listenerData, &ListenerData::onInvalidation));
   }
   void invalidateOnDestroyed()
   {
       if (!m_invalidated) {
           m_data->removeListener(m_listener);
       }
   }
   std::unique_ptr<ResultSource> m_data {};
   NiceMock<MockResultSourceListener> m_listener {};
   ListenerData m_listenerData {};
   bool m_invalidated {false};
};

TEST_F(TstDataSource, TestSize)
{
    m_data->add(Result(1, 1));
    EXPECT_FALSE(m_data->empty());
    EXPECT_EQ(m_data->size(), 1);
    m_data->add(Result(2, 2));
    EXPECT_FALSE(m_data->empty());
    EXPECT_EQ(m_data->size(), 2);
    m_data->add(Result(1, 2));
    EXPECT_FALSE(m_data->empty());
    EXPECT_EQ(m_data->size(), 2);
    m_data->remove(Result(1, 123));
    EXPECT_FALSE(m_data->empty());
    EXPECT_EQ(m_data->size(), 1);
    m_data->remove(Result(2, 123));
    EXPECT_TRUE(m_data->empty());
    EXPECT_EQ(m_data->size(), 0);
}

TEST_F(TstDataSource, TestUpdate)
{
    const Result &result1 = m_data->add(Result(1, 1));
    EXPECT_EQ(result1, Result(1, 1));
    m_data->add(Result(1, 2));

    const Result &result1Update = m_data->add(Result(1, 2));
    EXPECT_EQ(result1, Result(1, 2));
    EXPECT_EQ(result1Update, Result(1, 2));
    EXPECT_EQ(&result1, &result1Update);
}

TEST_F(TstDataSource, TestListener)
{
    m_data->addListener(m_listener);
    EXPECT_EQ(m_listenerData.results.size(), static_cast<std::size_t>(0));
    EXPECT_EQ(m_listenerData.lastUpdated, nullptr);
    EXPECT_FALSE(m_listenerData.invalidated);

    const Result &result1 = m_data->add(Result(1, 1));
    EXPECT_EQ(m_listenerData.results.size(), static_cast<std::size_t>(1));
    EXPECT_NE(m_listenerData.results.find(&result1), m_listenerData.results.end());
    EXPECT_EQ(m_listenerData.lastUpdated, nullptr);
    EXPECT_FALSE(m_listenerData.invalidated);

    const Result &result2 = m_data->add(Result(2, 2));
    EXPECT_EQ(m_listenerData.results.size(), static_cast<std::size_t>(2));
    EXPECT_NE(m_listenerData.results.find(&result1), m_listenerData.results.end());
    EXPECT_NE(m_listenerData.results.find(&result2), m_listenerData.results.end());
    EXPECT_EQ(m_listenerData.lastUpdated, nullptr);
    EXPECT_FALSE(m_listenerData.invalidated);

    const Result &result1Update = m_data->add(Result(1, 2));
    EXPECT_EQ(m_listenerData.results.size(), static_cast<std::size_t>(2));
    EXPECT_NE(m_listenerData.results.find(&result1), m_listenerData.results.end());
    EXPECT_NE(m_listenerData.results.find(&result2), m_listenerData.results.end());
    EXPECT_EQ(m_listenerData.lastUpdated, &result1Update);
    EXPECT_FALSE(m_listenerData.invalidated);

    m_data->remove(result1);
    EXPECT_EQ(m_listenerData.results.size(), static_cast<std::size_t>(1));
    EXPECT_NE(m_listenerData.results.find(&result2), m_listenerData.results.end());
    EXPECT_EQ(m_listenerData.lastUpdated, nullptr);
    EXPECT_FALSE(m_listenerData.invalidated);
}

TEST_F(TstDataSource, TestListenerDelayAdd)
{
    const Result &result1 = m_data->add(Result(1, 1));
    const Result &result2 = m_data->add(Result(2, 2));
    m_data->addListener(m_listener);
    EXPECT_EQ(m_listenerData.results.size(), static_cast<std::size_t>(2));
    EXPECT_NE(m_listenerData.results.find(&result1), m_listenerData.results.end());
    EXPECT_NE(m_listenerData.results.find(&result2), m_listenerData.results.end());
    EXPECT_EQ(m_listenerData.lastUpdated, nullptr);
    EXPECT_FALSE(m_listenerData.invalidated);
}

TEST_F(TstDataSource, TestListenerInvalidation)
{
    m_data->addListener(m_listener);
    EXPECT_EQ(m_listenerData.results.size(), static_cast<std::size_t>(0));
    EXPECT_EQ(m_listenerData.lastUpdated, nullptr);
    EXPECT_FALSE(m_listenerData.invalidated);

    m_data.reset();
    EXPECT_EQ(m_listenerData.results.size(), static_cast<std::size_t>(0));
    EXPECT_EQ(m_listenerData.lastUpdated, nullptr);
    EXPECT_TRUE(m_listenerData.invalidated);
    m_invalidated = true;
}

