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
#include "mockjobfactory.h"

using namespace std::placeholders;
using namespace ::testing;
using namespace ::microcore::core;
using namespace ::microcore::data;
using namespace ::microcore::error;

class ModelResult
{
public:
    explicit ModelResult() = default;
    explicit ModelResult(int v) : value {v} {}
    DEFAULT_COPY_DEFAULT_MOVE(ModelResult);
    int value {0};
};

using TestModel = Model<int, ModelResult>;

class TstModel: public Test
{
protected:
    void SetUp()
    {
        m_model.reset(new TestModel(m_factory));
    }
    MockJobFactory<int, std::vector<ModelResult>, Error> m_factory {};
    std::unique_ptr<TestModel> m_model {};
};


TEST_F(TstModel, Simple)
{
    {
        EXPECT_TRUE(m_model->empty());
        EXPECT_EQ(m_model->size(), static_cast<std::size_t>(0));
    }
    m_model->append({ModelResult(1), ModelResult(2)});
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
    m_model->append({ModelResult(3), ModelResult(4), ModelResult(5)});
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
    m_model->prepend({ModelResult(6)});
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
    m_model->move(0, 2);
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
    m_model->move(2, 0);
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
}
