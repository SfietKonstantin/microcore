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
#include "mockmodellistener.h"

using namespace ::testing;
using namespace ::microcore::core;
using namespace ::microcore::data;

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
        Append,
        Prepend,
        Remove,
        Update,
        Move,
        Invalidation
    };
    explicit ListenerData() = default;
    void onAppend(const std::vector<const Result *> &items)
    {
        clear();
        currentType = Type::Append;
        currentItems = items;
    }
    void onPrepend(const std::vector<const Result *> &items)
    {
        clear();
        currentType = Type::Prepend;
        currentItems = items;
    }
    void onRemove(std::size_t index)
    {
        clear();
        currentType = Type::Remove;
        currentIndex = static_cast<int>(index);
    }
    void onUpdate(std::size_t index, const Result *item)
    {
        Q_UNUSED(item)
        clear();
        currentType = Type::Update;
        currentIndex = static_cast<int>(index);
    }
    void onMove(std::size_t first, std::size_t second)
    {
        clear();
        currentType = Type::Move;
        currentIndex = static_cast<int>(first);
        currentIndex2 = static_cast<int>(second);
    }
    void onInvalidation()
    {
        clear();
        currentType = Type::Invalidation;
    }
    Type currentType {Type::None};
    std::vector<const Result *> currentItems {};
    int currentIndex {-1};
    int currentIndex2 {-1};
private:
    void clear()
    {
        currentType = Type::None;
        currentItems.clear();
        currentIndex = -1;
        currentIndex2 = -1;
    }
};

}

using TestViewModel = Model<Result>;
using MockTestModelListener = MockModelListener<Result, ModelData<Result>>;

class TstModel: public Test
{
protected:
    void SetUp()
    {
        m_model.reset(new TestViewModel());
        EXPECT_CALL(m_listener, onDestroyed()).WillRepeatedly(Invoke(static_cast<TstModel *>(this), &TstModel::invalidateOnDestroyed));
        ON_CALL(m_listener, onAppend(_)).WillByDefault(Invoke(&m_listenerData, &ListenerData::onAppend));
        ON_CALL(m_listener, onPrepend(_)).WillByDefault(Invoke(&m_listenerData, &ListenerData::onPrepend));
        ON_CALL(m_listener, onUpdate(_, _)).WillByDefault(Invoke(&m_listenerData, &ListenerData::onUpdate));
        ON_CALL(m_listener, onRemove(_)).WillByDefault(Invoke(&m_listenerData, &ListenerData::onRemove));
        ON_CALL(m_listener, onMove(_, _)).WillByDefault(Invoke(&m_listenerData, &ListenerData::onMove));
        ON_CALL(m_listener, onInvalidation()).WillByDefault(Invoke(&m_listenerData, &ListenerData::onInvalidation));
    }
    void invalidateOnDestroyed()
    {
        if (!m_invalidated) {
            m_model->removeListener(m_listener);
        }
    }
    std::unique_ptr<TestViewModel> m_model {};
    NiceMock<MockTestModelListener> m_listener {};
    ListenerData m_listenerData {};
    bool m_invalidated {false};
};


TEST_F(TstModel, BaseOperations)
{
    {
        EXPECT_TRUE(m_model->empty());
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(0));
    }
    m_model->append({Result(1), Result(2)});
    {
        EXPECT_FALSE(m_model->empty());
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(2));

        auto it = std::begin(*m_model);
        EXPECT_EQ((*it)->value, 1);
        ++it;
        EXPECT_EQ((*it)->value, 2);
        ++it;
        EXPECT_TRUE(it == std::end(*m_model));
    }
    m_model->append({Result(3), Result(4), Result(5)});
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
    m_model->remove(1);
    {
        EXPECT_FALSE(m_model->empty());
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(4));

        auto it = std::begin(*m_model);
        EXPECT_EQ((*it)->value, 1);
        ++it;
        EXPECT_EQ((*it)->value, 3);
        ++it;
        EXPECT_EQ((*it)->value, 4);
        ++it;
        EXPECT_EQ((*it)->value, 5);
        ++it;
        EXPECT_TRUE(it == std::end(*m_model));
    }
    m_model->prepend({Result(6)});
    {
        EXPECT_FALSE(m_model->empty());
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(5));

        auto it = std::begin(*m_model);
        EXPECT_EQ((*it)->value, 6);
        ++it;
        EXPECT_EQ((*it)->value, 1);
        ++it;
        EXPECT_EQ((*it)->value, 3);
        ++it;
        EXPECT_EQ((*it)->value, 4);
        ++it;
        EXPECT_EQ((*it)->value, 5);
        ++it;
        EXPECT_TRUE(it == std::end(*m_model));
    }
    EXPECT_TRUE(m_model->move(0, 2));
    {
        EXPECT_FALSE(m_model->empty());
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(5));
        auto it = std::begin(*m_model);
        EXPECT_EQ((*it)->value, 1);
        ++it;
        EXPECT_EQ((*it)->value, 6);
        ++it;
        EXPECT_EQ((*it)->value, 3);
        ++it;
        EXPECT_EQ((*it)->value, 4);
        ++it;
        EXPECT_EQ((*it)->value, 5);
        ++it;
        EXPECT_TRUE(it == std::end(*m_model));
    }
    EXPECT_TRUE(m_model->move(2, 0));
    {
        EXPECT_FALSE(m_model->empty());
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(5));
        auto it = std::begin(*m_model);
        EXPECT_EQ((*it)->value, 3);
        ++it;
        EXPECT_EQ((*it)->value, 1);
        ++it;
        EXPECT_EQ((*it)->value, 6);
        ++it;
        EXPECT_EQ((*it)->value, 4);
        ++it;
        EXPECT_EQ((*it)->value, 5);
        ++it;
        EXPECT_TRUE(it == std::end(*m_model));
    }
    EXPECT_TRUE(m_model->update(2, Result(2)));
    {
        EXPECT_FALSE(m_model->empty());
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(5));
        auto it = std::begin(*m_model);
        EXPECT_EQ((*it)->value, 3);
        ++it;
        EXPECT_EQ((*it)->value, 1);
        ++it;
        EXPECT_EQ((*it)->value, 2);
        ++it;
        EXPECT_EQ((*it)->value, 4);
        ++it;
        EXPECT_EQ((*it)->value, 5);
        ++it;
        EXPECT_TRUE(it == std::end(*m_model));
    }
}

TEST_F(TstModel, InvalidOperations)
{
    m_model->append({Result(1), Result(2)});
    EXPECT_FALSE(m_model->update(2, Result(3)));
    EXPECT_FALSE(m_model->remove(2));
}

template<class Data>
class MockStore
{
public:
    std::vector<const Data *> add(std::vector<Data> &&data)
    {
        return std::accumulate(std::begin(data), std::end(data), std::vector<const Data *>(),
                               [](std::vector<const Data *> &input, const Data &) {
            input.push_back(nullptr);
            return input;
        });
    }
    bool remove(const Data *data)
    {
        Q_UNUSED(data)
        return false;
    }
    bool update(const Data *key, Data &&value)
    {
        Q_UNUSED(key)
        Q_UNUSED(value)
        return false;
    }
};

class InvalidModel final : public ModelBase<Result, MockStore<Result>>
{
public:
    explicit InvalidModel()
        : ModelBase<Result, MockStore<Result>>(MockStore<Result>())
    {
    }
};

TEST_F(TstModel, InvalidOperations2)
{
    InvalidModel model {};
    model.append({Result(1), Result(2)});
    EXPECT_FALSE(model.update(0, Result(3)));
    EXPECT_FALSE(model.remove(0));
}

TEST_F(TstModel, Move)
{
    m_model->append({Result(1), Result(2), Result(3)});
    EXPECT_FALSE(m_model->move(0, 0));
    EXPECT_FALSE(m_model->move(0, 1));
    EXPECT_TRUE(m_model->move(0, 2));
    {
        auto it = std::begin(*m_model);
        EXPECT_EQ((*it)->value, 2);
        ++it;
        EXPECT_EQ((*it)->value, 1);
        ++it;
        EXPECT_EQ((*it)->value, 3);
        ++it;
        EXPECT_TRUE(it == std::end(*m_model));
    }
    EXPECT_TRUE(m_model->move(0, 3));
    {
        auto it = std::begin(*m_model);
        EXPECT_EQ((*it)->value, 1);
        ++it;
        EXPECT_EQ((*it)->value, 3);
        ++it;
        EXPECT_EQ((*it)->value, 2);
        ++it;
        EXPECT_TRUE(it == std::end(*m_model));
    }

    EXPECT_TRUE(m_model->move(1, 0));
    {
        auto it = std::begin(*m_model);
        EXPECT_EQ((*it)->value, 3);
        ++it;
        EXPECT_EQ((*it)->value, 1);
        ++it;
        EXPECT_EQ((*it)->value, 2);
        ++it;
        EXPECT_TRUE(it == std::end(*m_model));
    }
    EXPECT_FALSE(m_model->move(1, 1));
    EXPECT_FALSE(m_model->move(1, 2));
    EXPECT_TRUE(m_model->move(1, 3));
    {
        auto it = std::begin(*m_model);
        EXPECT_EQ((*it)->value, 3);
        ++it;
        EXPECT_EQ((*it)->value, 2);
        ++it;
        EXPECT_EQ((*it)->value, 1);
        ++it;
        EXPECT_TRUE(it == std::end(*m_model));
    }

    EXPECT_TRUE(m_model->move(2, 0));
    {
        auto it = std::begin(*m_model);
        EXPECT_EQ((*it)->value, 1);
        ++it;
        EXPECT_EQ((*it)->value, 3);
        ++it;
        EXPECT_EQ((*it)->value, 2);
        ++it;
        EXPECT_TRUE(it == std::end(*m_model));
    }
    EXPECT_TRUE(m_model->move(2, 1));
    {
        auto it = std::begin(*m_model);
        EXPECT_EQ((*it)->value, 1);
        ++it;
        EXPECT_EQ((*it)->value, 2);
        ++it;
        EXPECT_EQ((*it)->value, 3);
        ++it;
        EXPECT_TRUE(it == std::end(*m_model));
    }
    EXPECT_FALSE(m_model->move(2, 2));
    EXPECT_FALSE(m_model->move(2, 3));

    EXPECT_FALSE(m_model->move(3, 0));
    EXPECT_FALSE(m_model->move(3, 1));
    EXPECT_FALSE(m_model->move(3, 2));
    EXPECT_FALSE(m_model->move(3, 3));
}

TEST_F(TstModel, TestListener)
{
    m_model->addListener(m_listener);
    {
        EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::None);
    }
    m_model->append({Result(1), Result(2)});
    {
        EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::Append);
        auto modelIt = std::begin(*m_model);
        auto listenerIt = std::begin(m_listenerData.currentItems);
        EXPECT_EQ(m_listenerData.currentItems.size(), static_cast<std::size_t>(2));
        EXPECT_EQ(*modelIt, *listenerIt);
        ++modelIt;
        ++listenerIt;
        EXPECT_EQ(*modelIt, *listenerIt);
        ++modelIt;
        ++listenerIt;
        EXPECT_TRUE(modelIt == std::end(*m_model));
        EXPECT_TRUE(listenerIt == std::end(m_listenerData.currentItems));
    }
    m_model->append({Result(3), Result(4), Result(5)});
    {
        EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::Append);
        auto modelIt = std::begin(*m_model);
        auto listenerIt = std::begin(m_listenerData.currentItems);
        ++modelIt;
        ++modelIt;
        EXPECT_EQ(m_listenerData.currentItems.size(), static_cast<std::size_t>(3));
        EXPECT_EQ(*modelIt, *listenerIt);
        ++modelIt;
        ++listenerIt;
        EXPECT_EQ(*modelIt, *listenerIt);
        ++modelIt;
        ++listenerIt;
        EXPECT_EQ(*modelIt, *listenerIt);
        ++modelIt;
        ++listenerIt;
        EXPECT_TRUE(modelIt == std::end(*m_model));
        EXPECT_TRUE(listenerIt == std::end(m_listenerData.currentItems));
    }
    m_model->remove(1);
    {
        EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::Remove);
        EXPECT_EQ(m_listenerData.currentIndex, 1);
    }
    m_model->prepend({Result(6)});
    {
        EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::Prepend);
        auto modelIt = std::begin(*m_model);
        auto listenerIt = std::begin(m_listenerData.currentItems);
        EXPECT_EQ(m_listenerData.currentItems.size(), static_cast<std::size_t>(1));
        EXPECT_EQ(*modelIt, *listenerIt);
        ++modelIt;
        ++listenerIt;
        EXPECT_TRUE(listenerIt == std::end(m_listenerData.currentItems));
    }
    m_model->move(0, 2);
    {
        EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::Move);
        EXPECT_EQ(m_listenerData.currentIndex, 0);
        EXPECT_EQ(m_listenerData.currentIndex2, 2);
    }
    m_model->move(2, 0);
    {
        EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::Move);
        EXPECT_EQ(m_listenerData.currentIndex, 2);
        EXPECT_EQ(m_listenerData.currentIndex2, 0);
    }
    m_model->update(2, Result(2));
    {
        EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::Update);
        EXPECT_EQ(m_listenerData.currentIndex, 2);
    }
}

TEST_F(TstModel, TestListenerDelayAdd)
{
    m_model->append({Result(1), Result(2)});
    m_model->addListener(m_listener);
    EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::Append);
    auto modelIt = std::begin(*m_model);
    auto listenerIt = std::begin(m_listenerData.currentItems);
    EXPECT_EQ(m_listenerData.currentItems.size(), static_cast<std::size_t>(2));
    EXPECT_EQ(*modelIt, *listenerIt);
    ++modelIt;
    ++listenerIt;
    EXPECT_EQ(*modelIt, *listenerIt);
    ++modelIt;
    ++listenerIt;
    EXPECT_TRUE(modelIt == std::end(*m_model));
    EXPECT_TRUE(listenerIt == std::end(m_listenerData.currentItems));
}

TEST_F(TstModel, TestListenerInvalidation)
{
    m_model->addListener(m_listener);
    EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::None);

    m_model.reset();
    EXPECT_EQ(m_listenerData.currentType, ListenerData::Type::Invalidation);
    m_invalidated = true;
}
