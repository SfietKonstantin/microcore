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
#include <microcore/data/indexeddatastore.h>
#include <QtCore/QtGlobal>

using namespace ::testing;
using namespace ::microcore::data;

namespace {

class Result
{
public:
    using ConstPtr = std::shared_ptr<const Result>;
    explicit Result() = default;
    explicit Result(int v) : value {v} {}
    DISABLE_COPY_DEFAULT_MOVE(Result);
    int value {0};
};

class ResultDataStore: public IndexedDataStore<int, Result>
{
public:
    explicit ResultDataStore() = default;
    const std::map<int, std::shared_ptr<Result>> & internalStorage() const
    {
        return m_data;
    }
};

class ListenerData
{
public:
    enum class Type
    {
        None,
        Add,
        Remove,
        Update,
        Invalidation
    };
    explicit ListenerData() = default;
    explicit ListenerData(Type t)
        : type(t)
    {
    }
    explicit ListenerData(Type t, int k)
        : type(t), key(k)
    {
    }
    explicit ListenerData(Type t, int k, const Result::ConstPtr &v)
        : type(t), key(k), value(v)
    {
    }
    Type type {Type::None};
    int key {-1};
    Result::ConstPtr value {};
};

class ListenerWatcher
{
public:
    explicit ListenerWatcher() = default;
    const ListenerData & operator[](std::size_t index) const
    {
        return m_data[index];
    }
    int count() const
    {
        return static_cast<int>(m_data.size());
    }
    void clear()
    {
        m_data.clear();
    }
    void onAdd(int key, const Result::ConstPtr &value)
    {
        m_data.emplace_back(ListenerData::Type::Add, key, value);
    }
    void onRemove(int key)
    {
        m_data.emplace_back(ListenerData::Type::Remove, key);
    }
    void onUpdate(int key, const Result::ConstPtr &value)
    {
        m_data.emplace_back(ListenerData::Type::Update, key, value);
    }
    void onInvalidation()
    {
        m_data.emplace_back(ListenerData::Type::Invalidation);
    }
private:
    std::vector<ListenerData> m_data {};
};

template<class K, class V>
class MockIDataStoreListener: public IIndexedDataStore<K, V>::IListener
{
public:
    using ValuePtr = std::shared_ptr<V>;
    MOCK_METHOD2_T(onAdd, void (arg_const_reference<K> key, const ValuePtr &value));
    MOCK_METHOD1_T(onRemove, void (arg_const_reference<K> key));
    MOCK_METHOD2_T(onUpdate, void (arg_const_reference<K> key, const ValuePtr &value));
    MOCK_METHOD0_T(onInvalidation, void ());
};

}

class TstDataStore: public Test
{
public:
    explicit TstDataStore()
        : m_listener {new NiceMock<MockIDataStoreListener<int, Result>>()}
    {
    }
protected:
    void SetUp()
    {
        m_dataStore.reset(new ResultDataStore());
        ON_CALL(*m_listener, onAdd(_, _)).WillByDefault(Invoke(&m_watcher, &ListenerWatcher::onAdd));
        ON_CALL(*m_listener, onRemove(_)).WillByDefault(Invoke(&m_watcher, &ListenerWatcher::onRemove));
        ON_CALL(*m_listener, onUpdate(_, _)).WillByDefault(Invoke(&m_watcher, &ListenerWatcher::onUpdate));
        ON_CALL(*m_listener, onInvalidation()).WillByDefault(Invoke(&m_watcher, &ListenerWatcher::onInvalidation));
        m_dataStore->addListener(m_listener);
    }
    std::unique_ptr<ResultDataStore> m_dataStore;
    std::shared_ptr<NiceMock<MockIDataStoreListener<int, Result>>> m_listener {};
    ListenerWatcher m_watcher {};
    bool m_invalidated {false};
};

TEST_F(TstDataStore, AddUnique)
{
    const Result::ConstPtr &result1 {m_dataStore->addUnique(1, Result(1))};
    {
        EXPECT_NE(result1, nullptr);
        EXPECT_EQ(m_dataStore->internalStorage().size(), static_cast<std::size_t>(1));
        EXPECT_EQ(m_watcher.count(), 1);
        EXPECT_EQ(m_watcher[0].type, ListenerData::Type::Add);
        EXPECT_EQ(m_watcher[0].key, 1);
        EXPECT_FALSE(m_dataStore->internalStorage().find(1) == std::end(m_dataStore->internalStorage()));
        EXPECT_EQ(m_dataStore->internalStorage().find(1)->second, m_watcher[0].value);
        EXPECT_EQ(m_dataStore->internalStorage().find(1)->second, result1);
        EXPECT_EQ(m_dataStore->internalStorage().find(1)->second->value, 1);
    }
    const Result::ConstPtr &result2 {m_dataStore->addUnique(2, Result(2))};
    {
        EXPECT_NE(result2, nullptr);
        EXPECT_EQ(m_dataStore->internalStorage().size(), static_cast<std::size_t>(2));
        EXPECT_EQ(m_watcher.count(), 2);
        EXPECT_EQ(m_watcher[1].type, ListenerData::Type::Add);
        EXPECT_EQ(m_watcher[1].key, 2);
        EXPECT_FALSE(m_dataStore->internalStorage().find(2) == std::end(m_dataStore->internalStorage()));
        EXPECT_EQ(m_dataStore->internalStorage().find(2)->second, m_watcher[1].value);
        EXPECT_EQ(m_dataStore->internalStorage().find(2)->second, result2);
        EXPECT_EQ(m_dataStore->internalStorage().find(2)->second->value, 2);
    }
}

TEST_F(TstDataStore, AddUniqueExisting)
{
    const Result::ConstPtr &result1 {m_dataStore->addUnique(1, Result(1))};
    {
        EXPECT_NE(result1, nullptr);
        EXPECT_EQ(m_dataStore->internalStorage().size(), static_cast<std::size_t>(1));
        EXPECT_EQ(m_watcher.count(), 1);
        EXPECT_EQ(m_watcher[0].type, ListenerData::Type::Add);
        EXPECT_EQ(m_watcher[0].key, 1);
        EXPECT_FALSE(m_dataStore->internalStorage().find(1) == std::end(m_dataStore->internalStorage()));
        EXPECT_EQ(m_dataStore->internalStorage().find(1)->second, m_watcher[0].value);
        EXPECT_EQ(m_dataStore->internalStorage().find(1)->second->value, 1);
    }
    const Result::ConstPtr &result2 {m_dataStore->addUnique(1, Result(2))};
    {
        EXPECT_EQ(result2, nullptr);
        EXPECT_EQ(m_watcher.count(), 1);
        EXPECT_FALSE(m_dataStore->internalStorage().find(1) == std::end(m_dataStore->internalStorage()));
        EXPECT_EQ(m_dataStore->internalStorage().find(1)->second->value, 1);
    }
}

TEST_F(TstDataStore, Add)
{
    const Result::ConstPtr &result1 {m_dataStore->add(1, Result(1))};
    {
        EXPECT_EQ(m_dataStore->internalStorage().size(), static_cast<std::size_t>(1));
        EXPECT_EQ(m_watcher.count(), 1);
        EXPECT_EQ(m_watcher[0].type, ListenerData::Type::Add);
        EXPECT_EQ(m_watcher[0].key, 1);
        EXPECT_FALSE(m_dataStore->internalStorage().find(1) == std::end(m_dataStore->internalStorage()));
        EXPECT_EQ(m_dataStore->internalStorage().find(1)->second, m_watcher[0].value);
        EXPECT_EQ(m_dataStore->internalStorage().find(1)->second, result1);
        EXPECT_EQ(m_dataStore->internalStorage().find(1)->second->value, 1);
    }
    const Result::ConstPtr &result2 {m_dataStore->add(2, Result(2))};
    {
        EXPECT_EQ(m_dataStore->internalStorage().size(), static_cast<std::size_t>(2));
        EXPECT_EQ(m_watcher.count(), 2);
        EXPECT_EQ(m_watcher[1].type, ListenerData::Type::Add);
        EXPECT_EQ(m_watcher[1].key, 2);
        EXPECT_FALSE(m_dataStore->internalStorage().find(2) == std::end(m_dataStore->internalStorage()));
        EXPECT_EQ(m_dataStore->internalStorage().find(2)->second, m_watcher[1].value);
        EXPECT_EQ(m_dataStore->internalStorage().find(2)->second, result2);
        EXPECT_EQ(m_dataStore->internalStorage().find(2)->second->value, 2);
    }
}

TEST_F(TstDataStore, AddAsUpdate)
{
    const Result::ConstPtr &result1 {m_dataStore->add(1, Result(1))};
    {
        EXPECT_EQ(m_dataStore->internalStorage().size(), static_cast<std::size_t>(1));
        EXPECT_EQ(m_watcher.count(), 1);
        EXPECT_EQ(m_watcher[0].type, ListenerData::Type::Add);
        EXPECT_EQ(m_watcher[0].key, 1);
        EXPECT_FALSE(m_dataStore->internalStorage().find(1) == std::end(m_dataStore->internalStorage()));
        EXPECT_EQ(m_dataStore->internalStorage().find(1)->second, m_watcher[0].value);
        EXPECT_EQ(m_dataStore->internalStorage().find(1)->second, result1);
        EXPECT_EQ(m_dataStore->internalStorage().find(1)->second->value, 1);
    }
    const Result::ConstPtr &result2 {m_dataStore->add(1, Result(2))};
    {
        EXPECT_EQ(m_dataStore->internalStorage().size(), static_cast<std::size_t>(1));
        EXPECT_EQ(m_watcher.count(), 2);
        EXPECT_EQ(m_watcher[1].type, ListenerData::Type::Update);
        EXPECT_EQ(m_watcher[1].key, 1);
        EXPECT_FALSE(m_dataStore->internalStorage().find(1) == std::end(m_dataStore->internalStorage()));
        EXPECT_EQ(m_dataStore->internalStorage().find(1)->second, m_watcher[0].value);
        EXPECT_EQ(m_dataStore->internalStorage().find(1)->second, m_watcher[1].value);
        EXPECT_EQ(m_dataStore->internalStorage().find(1)->second, result1);
        EXPECT_EQ(m_dataStore->internalStorage().find(1)->second, result2);
        EXPECT_EQ(m_dataStore->internalStorage().find(1)->second->value, 2);
    }
}

TEST_F(TstDataStore, Update)
{
    m_dataStore->add(1, Result(1));
    const Result::ConstPtr &result2 {m_dataStore->update(1, Result(2))};
    {
        EXPECT_NE(result2, nullptr);
        EXPECT_EQ(m_dataStore->internalStorage().size(), static_cast<std::size_t>(1));
        EXPECT_EQ(m_watcher.count(), 2);
        EXPECT_EQ(m_watcher[1].type, ListenerData::Type::Update);
        EXPECT_EQ(m_watcher[1].key, 1);
        EXPECT_FALSE(m_dataStore->internalStorage().find(1) == std::end(m_dataStore->internalStorage()));
        EXPECT_EQ(m_dataStore->internalStorage().find(1)->second, m_watcher[0].value);
        EXPECT_EQ(m_dataStore->internalStorage().find(1)->second, m_watcher[1].value);
        EXPECT_EQ(m_dataStore->internalStorage().find(1)->second, result2);
        EXPECT_EQ(m_dataStore->internalStorage().find(1)->second->value, 2);
    }
}

TEST_F(TstDataStore, UpdateInexisting)
{
    m_dataStore->add(1, Result(1));
    const Result::ConstPtr &result2 {m_dataStore->update(2, Result(2))};
    {
        EXPECT_EQ(result2, nullptr);
        EXPECT_EQ(m_watcher.count(), 1);
        EXPECT_FALSE(m_dataStore->internalStorage().find(1) == std::end(m_dataStore->internalStorage()));
        EXPECT_EQ(m_dataStore->internalStorage().find(1)->second->value, 1);
    }
}

TEST_F(TstDataStore, Remove)
{
    m_dataStore->add(1, Result(1));
    EXPECT_TRUE(m_dataStore->remove(1));
    {
        EXPECT_EQ(m_dataStore->internalStorage().size(), static_cast<std::size_t>(0));
        EXPECT_EQ(m_watcher.count(), 2);
        EXPECT_EQ(m_watcher[1].type, ListenerData::Type::Remove);
        EXPECT_EQ(m_watcher[1].key, 1);
    }
}

TEST_F(TstDataStore, RemoveInexisting)
{
    m_dataStore->add(1, Result(1));
    EXPECT_FALSE(m_dataStore->remove(2));
    {
        EXPECT_EQ(m_watcher.count(), 1);
        EXPECT_FALSE(m_dataStore->internalStorage().find(1) == std::end(m_dataStore->internalStorage()));
        EXPECT_EQ(m_dataStore->internalStorage().find(1)->second->value, 1);
    }
}

TEST_F(TstDataStore, ListenerInvalidation)
{
    EXPECT_EQ(m_watcher.count(), 0);

    m_dataStore.reset();
    EXPECT_EQ(m_watcher.count(), 1);
    EXPECT_EQ(m_watcher[0].type, ListenerData::Type::Invalidation);
    m_invalidated = true;
}
