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
#include <microcore/data/indexeddatastore.h>
#include <microcore/data/indexedmodel.h>
#include "mockmodellistener.h"

using namespace ::testing;
using namespace ::microcore::data;

namespace {

class Result
{
public:
    explicit Result() = default;
    explicit Result(int v) : key {v}, value {v} {}
    explicit Result (int k, int v) : key {k}, value {v} {}
    DEFAULT_COPY_DEFAULT_MOVE(Result);
    int key {0};
    int value {0};
};

class ResultMapper
{
public:
    using KeyType = int;
    int operator()(const Result &result) const
    {
        return result.key;
    }
};

class ResultDataStore: public IndexedDataStore<int, Result>
{
public:
    explicit ResultDataStore() = default;
    std::map<int, std::shared_ptr<Result>> & data()
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
        Append,
        Prepend,
        Insert,
        Remove,
        Update,
        Move,
        Invalidation
    };
    explicit ListenerData() = default;
    explicit ListenerData(Type t)
        : type(t)
    {
    }
    explicit ListenerData(Type t, const std::vector<const Result *> &v)
        : type(t), values(v)
    {
    }
    explicit ListenerData(Type t, int i)
        : type(t), index1(i)
    {
    }
    explicit ListenerData(Type t, int i, const Result &v)
        : type(t), value(&v), index1(i)
    {
    }
    explicit ListenerData(Type t, int i, const std::vector<const Result *> &v)
        : type(t), values(v), index1(i)
    {
    }
    explicit ListenerData(Type t, int i1, int i2)
        : type(t), index1(i1), index2(i2)
    {
    }
    Type type {Type::None};
    const Result *value {nullptr};
    std::vector<const Result *> values {};
    int index1 {-1};
    int index2 {-1};
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
    void onAppend(const std::vector<const Result *> &values)
    {
        m_data.emplace_back(ListenerData::Type::Append, values);
    }
    void onPrepend(const std::vector<const Result *> &values)
    {
        m_data.emplace_back(ListenerData::Type::Prepend, values);
    }
    void onInsert(std::size_t index, const std::vector<const Result *> &values)
    {
        m_data.emplace_back(ListenerData::Type::Insert, static_cast<int>(index), values);
    }
    void onRemove(std::size_t index)
    {
        m_data.emplace_back(ListenerData::Type::Remove, static_cast<int>(index));
    }
    void onUpdate(std::size_t index, const Result &value)
    {
        m_data.emplace_back(ListenerData::Type::Update, static_cast<int>(index), value);
    }
    void onMove(std::size_t oldIndex, std::size_t newIndex)
    {
        m_data.emplace_back(ListenerData::Type::Move, static_cast<int>(oldIndex), static_cast<int>(newIndex));
    }
    void onInvalidation()
    {
        m_data.emplace_back(ListenerData::Type::Invalidation);
    }
private:
    std::vector<ListenerData> m_data {};
};

using ResultModel = IndexedModel<Result, ResultMapper>;
using ResultModelListener = MockModelListener<Result>;

}

class TstIndexedModel: public Test
{
public:
    explicit TstIndexedModel()
        : m_listener {new NiceMock<ResultModelListener>()}
    {
    }
protected:
    void SetUp()
    {
        m_dataStore.reset(new ResultDataStore());
        m_model.reset(new ResultModel(*m_dataStore));
        ON_CALL(*m_listener, onAppend(_)).WillByDefault(Invoke(&m_watcher, &ListenerWatcher::onAppend));
        ON_CALL(*m_listener, onPrepend(_)).WillByDefault(Invoke(&m_watcher, &ListenerWatcher::onPrepend));
        ON_CALL(*m_listener, onInsert(_, _)).WillByDefault(Invoke(&m_watcher, &ListenerWatcher::onInsert));
        ON_CALL(*m_listener, onRemove(_)).WillByDefault(Invoke(&m_watcher, &ListenerWatcher::onRemove));
        ON_CALL(*m_listener, onUpdate(_, _)).WillByDefault(Invoke(&m_watcher, &ListenerWatcher::onUpdate));
        ON_CALL(*m_listener, onMove(_, _)).WillByDefault(Invoke(&m_watcher, &ListenerWatcher::onMove));
        ON_CALL(*m_listener, onInvalidation()).WillByDefault(Invoke(&m_watcher, &ListenerWatcher::onInvalidation));
        m_model->addListener(m_listener);
    }
    std::unique_ptr<ResultDataStore> m_dataStore {};
    std::unique_ptr<ResultModel> m_model {};
    std::shared_ptr<NiceMock<ResultModelListener>> m_listener {};
    ListenerWatcher m_watcher {};
    bool m_invalidated {false};
};

TEST_F(TstIndexedModel, InvalidationChain1)
{
    m_model.reset();
    m_invalidated = true;
    m_dataStore.reset();
}

TEST_F(TstIndexedModel, InvalidationChain2)
{
    m_dataStore.reset();
    m_model.reset();
    m_invalidated = true;
}

TEST_F(TstIndexedModel, Append)
{
    m_model->append({Result(1), Result(2)});
    {
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(2));
        EXPECT_EQ(m_watcher.count(), 1);
        EXPECT_EQ(m_watcher[0].type, ListenerData::Type::Append);
        EXPECT_EQ(m_watcher[0].values.size(), static_cast<std::size_t>(2));

        auto it = std::begin(*m_model);
        EXPECT_EQ(*it, m_dataStore->data().at(1).get());
        EXPECT_EQ(*it, m_watcher[0].values[0]);
        ++it;
        EXPECT_EQ(*it, m_dataStore->data().at(2).get());
        EXPECT_EQ(*it, m_watcher[0].values[1]);
        ++it;
        EXPECT_TRUE(it == std::end(*m_model));
    }
    m_model->append({Result(3), Result(4), Result(5)});
    {
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(5));
        EXPECT_EQ(m_watcher.count(), 2);
        EXPECT_EQ(m_watcher[1].type, ListenerData::Type::Append);
        EXPECT_EQ(m_watcher[1].values.size(), static_cast<std::size_t>(3));

        auto it = std::begin(*m_model);
        EXPECT_EQ(*it, m_dataStore->data().at(1).get());
        ++it;
        EXPECT_EQ(*it, m_dataStore->data().at(2).get());
        ++it;
        EXPECT_EQ(*it, m_dataStore->data().at(3).get());
        EXPECT_EQ(*it, m_watcher[1].values[0]);
        ++it;
        EXPECT_EQ(*it, m_dataStore->data().at(4).get());
        EXPECT_EQ(*it, m_watcher[1].values[1]);
        ++it;
        EXPECT_EQ(*it, m_dataStore->data().at(5).get());
        EXPECT_EQ(*it, m_watcher[1].values[2]);
        ++it;
        EXPECT_TRUE(it == std::end(*m_model));
    }
}

TEST_F(TstIndexedModel, AppendAfterInvalidation)
{
    m_model->append({Result(1), Result(2)});
    {
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(2));
    }
    m_dataStore.reset();
    EXPECT_TRUE(m_model->empty());
    m_model->append({Result(3), Result(4), Result(5)});
    {
        EXPECT_TRUE(m_model->empty());
        EXPECT_EQ(m_watcher.count(), 2);
        EXPECT_EQ(m_watcher[1].type, ListenerData::Type::Invalidation);
    }
}

TEST_F(TstIndexedModel, Prepend)
{
    m_model->prepend({Result(1), Result(2)});
    {
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(2));
        EXPECT_EQ(m_watcher.count(), 1);
        EXPECT_EQ(m_watcher[0].type, ListenerData::Type::Prepend);
        EXPECT_EQ(m_watcher[0].values.size(), static_cast<std::size_t>(2));

        auto it = std::begin(*m_model);
        EXPECT_EQ(*it, m_dataStore->data().at(1).get());
        EXPECT_EQ(*it, m_watcher[0].values[0]);
        ++it;
        EXPECT_EQ(*it, m_dataStore->data().at(2).get());
        EXPECT_EQ(*it, m_watcher[0].values[1]);
        ++it;
        EXPECT_TRUE(it == std::end(*m_model));
    }
    m_model->prepend({Result(3), Result(4), Result(5)});
    {
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(5));
        EXPECT_EQ(m_watcher.count(), 2);
        EXPECT_EQ(m_watcher[1].type, ListenerData::Type::Prepend);
        EXPECT_EQ(m_watcher[1].values.size(), static_cast<std::size_t>(3));

        auto it = std::begin(*m_model);
        EXPECT_EQ(*it, m_dataStore->data().at(3).get());
        EXPECT_EQ(*it, m_watcher[1].values[0]);
        ++it;
        EXPECT_EQ(*it, m_dataStore->data().at(4).get());
        EXPECT_EQ(*it, m_watcher[1].values[1]);
        ++it;
        EXPECT_EQ(*it, m_dataStore->data().at(5).get());
        EXPECT_EQ(*it, m_watcher[1].values[2]);
        ++it;
        EXPECT_EQ(*it, m_dataStore->data().at(1).get());
        ++it;
        EXPECT_EQ(*it, m_dataStore->data().at(2).get());
        ++it;
        EXPECT_TRUE(it == std::end(*m_model));
    }
}

TEST_F(TstIndexedModel, PrependAfterInvalidation)
{
    m_model->append({Result(1), Result(2)});
    {
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(2));
    }
    m_dataStore.reset();
    EXPECT_TRUE(m_model->empty());
    m_model->prepend({Result(3), Result(4), Result(5)});
    {
        EXPECT_TRUE(m_model->empty());
        EXPECT_EQ(m_watcher.count(), 2);
        EXPECT_EQ(m_watcher[1].type, ListenerData::Type::Invalidation);
    }
}

TEST_F(TstIndexedModel, Insert)
{
    m_model->insert(0, {Result(1), Result(2)});
    {
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(2));
        EXPECT_EQ(m_watcher.count(), 1);
        EXPECT_EQ(m_watcher[0].type, ListenerData::Type::Insert);
        EXPECT_EQ(m_watcher[0].values.size(), static_cast<std::size_t>(2));

        auto it = std::begin(*m_model);
        EXPECT_EQ(*it, m_dataStore->data().at(1).get());
        EXPECT_EQ(*it, m_watcher[0].values[0]);
        ++it;
        EXPECT_EQ(*it, m_dataStore->data().at(2).get());
        EXPECT_EQ(*it, m_watcher[0].values[1]);
        ++it;
        EXPECT_TRUE(it == std::end(*m_model));
    }
    m_model->insert(2, {Result(3), Result(4), Result(5)});
    {
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(5));
        EXPECT_EQ(m_watcher.count(), 2);
        EXPECT_EQ(m_watcher[1].type, ListenerData::Type::Insert);
        EXPECT_EQ(m_watcher[1].values.size(), static_cast<std::size_t>(3));

        auto it = std::begin(*m_model);
        EXPECT_EQ(*it, m_dataStore->data().at(1).get());
        ++it;
        EXPECT_EQ(*it, m_dataStore->data().at(2).get());
        ++it;
        EXPECT_EQ(*it, m_dataStore->data().at(3).get());
        EXPECT_EQ(*it, m_watcher[1].values[0]);
        ++it;
        EXPECT_EQ(*it, m_dataStore->data().at(4).get());
        EXPECT_EQ(*it, m_watcher[1].values[1]);
        ++it;
        EXPECT_EQ(*it, m_dataStore->data().at(5).get());
        EXPECT_EQ(*it, m_watcher[1].values[2]);
        ++it;
        EXPECT_TRUE(it == std::end(*m_model));

    }
    m_model->insert(3, {Result(6)});
    {
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(6));
        EXPECT_EQ(m_watcher.count(), 3);
        EXPECT_EQ(m_watcher[2].type, ListenerData::Type::Insert);
        EXPECT_EQ(m_watcher[2].values.size(), static_cast<std::size_t>(1));

        auto it = std::begin(*m_model);
        EXPECT_EQ(*it, m_dataStore->data().at(1).get());
        ++it;
        EXPECT_EQ(*it, m_dataStore->data().at(2).get());
        ++it;
        EXPECT_EQ(*it, m_dataStore->data().at(3).get());
        ++it;
        EXPECT_EQ(*it, m_dataStore->data().at(6).get());
        EXPECT_EQ(*it, m_watcher[2].values[0]);
        ++it;
        EXPECT_EQ(*it, m_dataStore->data().at(4).get());
        ++it;
        EXPECT_EQ(*it, m_dataStore->data().at(5).get());
        ++it;
        EXPECT_TRUE(it == std::end(*m_model));
    }
}

TEST_F(TstIndexedModel, InsertFailed)
{
    m_model->insert(1, {Result(0)});
    {
        EXPECT_TRUE(m_model->empty());
        EXPECT_EQ(m_watcher.count(), 0);
    }
    m_model->append({Result(1), Result(2)});
    m_model->insert(3, {Result(3), Result(4), Result(5)});
    {
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(2));
        EXPECT_EQ(m_watcher.count(), 1);
    }
}

TEST_F(TstIndexedModel, InsertAfterInvalidation)
{
    m_model->append({Result(1), Result(2)});
    {
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(2));
    }
    m_dataStore.reset();
    EXPECT_TRUE(m_model->empty());
    m_model->insert(1, {Result(3), Result(4), Result(5)});
    {
        EXPECT_TRUE(m_model->empty());
        EXPECT_EQ(m_watcher.count(), 2);
        EXPECT_EQ(m_watcher[1].type, ListenerData::Type::Invalidation);
    }
}

TEST_F(TstIndexedModel, Update)
{
    m_model->append({Result(1), Result(2)});
    m_model->update(1, Result(2, 3));
    {
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(2));
        EXPECT_EQ(m_watcher.count(), 2);
        EXPECT_EQ(m_watcher[1].type, ListenerData::Type::Update);

        auto it = std::begin(*m_model);
        EXPECT_EQ(*it, m_dataStore->data().at(1).get());
        EXPECT_EQ((*it)->value, 1);
        ++it;
        EXPECT_EQ(*it, m_dataStore->data().at(2).get());
        EXPECT_EQ(*it, m_watcher[1].value);
        EXPECT_EQ((*it)->value, 3);
        ++it;
        EXPECT_TRUE(it == std::end(*m_model));
    }
}

TEST_F(TstIndexedModel, UpdateFailed)
{
    m_model->update(1, Result(0));
    {
        EXPECT_TRUE(m_model->empty());
        EXPECT_EQ(m_watcher.count(), 0);
    }
    m_model->append({Result(1), Result(2)});
    m_model->update(3, Result(3));
    {
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(2));
        EXPECT_EQ(m_watcher.count(), 1);
    }
}

TEST_F(TstIndexedModel, UpdateImpactKeys)
{
    m_model->append({Result(1), Result(2)});
    m_model->update(1, Result(3));
    {
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(2));
        EXPECT_EQ(m_watcher.count(), 1);
    }
}

TEST_F(TstIndexedModel, UpdateAfterInvalidation)
{
    m_model->append({Result(1), Result(2)});
    {
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(2));
    }
    m_dataStore.reset();
    EXPECT_TRUE(m_model->empty());
    m_model->update(1, Result(2, 3));
    {
        EXPECT_TRUE(m_model->empty());
        EXPECT_EQ(m_watcher.count(), 2);
        EXPECT_EQ(m_watcher[1].type, ListenerData::Type::Invalidation);
    }
}

TEST_F(TstIndexedModel, Remove)
{
    m_model->append({Result(1), Result(2)});
    m_model->remove(1);
    {
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(1));
        EXPECT_EQ(m_watcher.count(), 2);
        EXPECT_EQ(m_watcher[1].type, ListenerData::Type::Remove);
        EXPECT_EQ(m_watcher[1].index1, 1);

        auto it = std::begin(*m_model);
        EXPECT_EQ(*it, m_dataStore->data().at(1).get());
        ++it;
        EXPECT_TRUE(it == std::end(*m_model));
    }
}

TEST_F(TstIndexedModel, RemoveFailed)
{
    m_model->remove(1);
    {
        EXPECT_TRUE(m_model->empty());
        EXPECT_EQ(m_watcher.count(), 0);
    }
    m_model->append({Result(1), Result(2)});
    m_model->remove(3);
    {
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(2));
        EXPECT_EQ(m_watcher.count(), 1);
    }
}

TEST_F(TstIndexedModel, RemoveAfterInvalidation)
{
    m_model->append({Result(1), Result(2)});
    {
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(2));
    }
    m_dataStore.reset();
    EXPECT_TRUE(m_model->empty());
    m_model->remove(1);
    {
        EXPECT_TRUE(m_model->empty());
        EXPECT_EQ(m_watcher.count(), 2);
        EXPECT_EQ(m_watcher[1].type, ListenerData::Type::Invalidation);
    }
}

TEST_F(TstIndexedModel, Move)
{
    m_model->append({Result(1), Result(2), Result(3), Result(4), Result(5)});
    m_model->move(0, 2);
    {
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(5));
        EXPECT_EQ(m_watcher.count(), 2);
        EXPECT_EQ(m_watcher[1].type, ListenerData::Type::Move);
        EXPECT_EQ(m_watcher[1].index1, 0);
        EXPECT_EQ(m_watcher[1].index2, 2);

        auto it = std::begin(*m_model);
        EXPECT_EQ(*it, m_dataStore->data().at(2).get());
        ++it;
        EXPECT_EQ(*it, m_dataStore->data().at(1).get());
        ++it;
        EXPECT_EQ(*it, m_dataStore->data().at(3).get());
        ++it;
        EXPECT_EQ(*it, m_dataStore->data().at(4).get());
        ++it;
        EXPECT_EQ(*it, m_dataStore->data().at(5).get());
        ++it;
        EXPECT_TRUE(it == std::end(*m_model));
    }
    m_model->move(2, 0);
    {
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(5));
        EXPECT_EQ(m_watcher.count(), 3);
        EXPECT_EQ(m_watcher[2].type, ListenerData::Type::Move);
        EXPECT_EQ(m_watcher[2].index1, 2);
        EXPECT_EQ(m_watcher[2].index2, 0);

        auto it = std::begin(*m_model);
        EXPECT_EQ(*it, m_dataStore->data().at(3).get());
        ++it;
        EXPECT_EQ(*it, m_dataStore->data().at(2).get());
        ++it;
        EXPECT_EQ(*it, m_dataStore->data().at(1).get());
        ++it;
        EXPECT_EQ(*it, m_dataStore->data().at(4).get());
        ++it;
        EXPECT_EQ(*it, m_dataStore->data().at(5).get());
        ++it;
        EXPECT_TRUE(it == std::end(*m_model));
    }
}

TEST_F(TstIndexedModel, MoveFailed)
{
    m_model->append({Result(1), Result(2), Result(3), Result(4), Result(5)});
    m_model->move(5, 2);
    {
        EXPECT_EQ(m_watcher.count(), 1);
    }
    m_model->move(2, 6);
    {
        EXPECT_EQ(m_watcher.count(), 1);
    }
    m_model->move(2, 2);
    {
        EXPECT_EQ(m_watcher.count(), 1);
    }
    m_model->move(2, 3);
    {
        EXPECT_EQ(m_watcher.count(), 1);
    }
}

TEST_F(TstIndexedModel, ExternalUpdate)
{
    m_model->append({Result(1), Result(2)});
    m_dataStore->update(2, Result(2, 3));
    {
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(2));
        EXPECT_EQ(m_watcher.count(), 2);
        EXPECT_EQ(m_watcher[1].type, ListenerData::Type::Update);

        auto it = std::begin(*m_model);
        EXPECT_EQ(*it, m_dataStore->data().at(1).get());
        EXPECT_EQ((*it)->value, 1);
        ++it;
        EXPECT_EQ(*it, m_dataStore->data().at(2).get());
        EXPECT_EQ(*it, m_watcher[1].value);
        EXPECT_EQ((*it)->value, 3);
        ++it;
        EXPECT_TRUE(it == std::end(*m_model));
    }
}

TEST_F(TstIndexedModel, ExternalUpdateFailed)
{
    m_model->append({Result(1), Result(2)});
    m_dataStore->addUnique(3, Result(3));
    m_dataStore->update(3, Result(3, 4));
    {
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(2));
        EXPECT_EQ(m_watcher.count(), 1);
    }
}

TEST_F(TstIndexedModel, ExternalRemove)
{
    m_model->append({Result(1), Result(2)});
    m_dataStore->remove(2);
    {
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(1));
        EXPECT_EQ(m_watcher.count(), 2);
        EXPECT_EQ(m_watcher[1].type, ListenerData::Type::Remove);
        EXPECT_EQ(m_watcher[1].index1, 1);

        auto it = std::begin(*m_model);
        EXPECT_EQ(*it, m_dataStore->data().at(1).get());
        ++it;
        EXPECT_TRUE(it == std::end(*m_model));
    }
}

TEST_F(TstIndexedModel, ExternalRemoveFailed)
{
    m_model->append({Result(1), Result(2)});
    m_dataStore->addUnique(3, Result(3));
    m_dataStore->remove(3);
    {
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(2));
        EXPECT_EQ(m_watcher.count(), 1);
    }
}

TEST_F(TstIndexedModel, Accessors)
{
    m_model->append({Result(1), Result(2)});
    {
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(2));

        const ResultModel &constModel = *m_model;
        auto it = std::begin(*m_model);
        auto constIt = std::begin(constModel);
        EXPECT_EQ(*it, m_dataStore->data().at(1).get());
        EXPECT_EQ((*constIt), m_dataStore->data().at(1).get());
        EXPECT_EQ(constModel[0], m_dataStore->data().at(1).get());
        ++it;
        ++constIt;
        EXPECT_EQ(*it, m_dataStore->data().at(2).get());
        EXPECT_EQ((*constIt), m_dataStore->data().at(2).get());
        EXPECT_EQ(constModel[1], m_dataStore->data().at(2).get());
        ++it;
        ++constIt;
        EXPECT_TRUE(it == std::end(*m_model));
        EXPECT_TRUE(it == std::end(constModel));
        EXPECT_EQ(constModel[2], nullptr);
    }
}

TEST_F(TstIndexedModel, ListenerDelayAdd)
{
    m_model->removeListener(m_listener);
    m_model->append({Result(1), Result(2)});
    m_model->addListener(m_listener);
    {
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(2));
        EXPECT_EQ(m_watcher.count(), 1);
        EXPECT_EQ(m_watcher[0].type, ListenerData::Type::Append);
        EXPECT_EQ(m_watcher[0].values.size(), static_cast<std::size_t>(2));

        auto it = std::begin(*m_model);
        EXPECT_EQ(*it, m_dataStore->data().at(1).get());
        EXPECT_EQ(*it, m_watcher[0].values[0]);
        ++it;
        EXPECT_EQ(*it, m_dataStore->data().at(2).get());
        EXPECT_EQ(*it, m_watcher[0].values[1]);
        ++it;
        EXPECT_TRUE(it == std::end(*m_model));
    }
}

TEST_F(TstIndexedModel, ListenerInvalidation)
{
    EXPECT_EQ(m_watcher.count(), 0);

    m_model.reset();
    EXPECT_EQ(m_watcher.count(), 1);
    EXPECT_EQ(m_watcher[0].type, ListenerData::Type::Invalidation);
    m_invalidated = true;
}
