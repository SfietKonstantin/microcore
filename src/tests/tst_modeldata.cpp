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
#include "data/modeldata.h"

using namespace std::placeholders;
using namespace ::testing;
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

}

class TstModelData: public Test
{
protected:
    ModelData<Result> m_modelData {};
};

static bool getMatching(const ModelData<Result>::Map::value_type &ptr, int value)
{
    return ptr.first->value == value;
}

TEST_F(TstModelData, BaseOperations)
{
    std::vector<const Result *> result1 {m_modelData.add({Result(1), Result(2)})};
    {
        EXPECT_EQ(result1.size(), static_cast<std::size_t>(2));
        EXPECT_EQ(result1[0]->value, 1);
        EXPECT_EQ(result1[1]->value, 2);

        auto it = std::find_if(std::begin(m_modelData), std::end(m_modelData), std::bind(getMatching, _1, 1));
        EXPECT_FALSE(it == std::end(m_modelData));
        EXPECT_EQ(result1[0], it->first);

        it = std::find_if(std::begin(m_modelData), std::end(m_modelData), std::bind(getMatching, _1, 2));
        EXPECT_FALSE(it == std::end(m_modelData));
        EXPECT_EQ(result1[1], it->first);
    }
    std::vector<const Result *> result2 {m_modelData.add({Result(3)})};
    {
        EXPECT_EQ(result2.size(), static_cast<std::size_t>(1));
        EXPECT_EQ(result2[0]->value, 3);

        auto it = std::find_if(std::begin(m_modelData), std::end(m_modelData), std::bind(getMatching, _1, 1));
        EXPECT_FALSE(it == std::end(m_modelData));
        EXPECT_EQ(result1[0], it->first);

        it = std::find_if(std::begin(m_modelData), std::end(m_modelData), std::bind(getMatching, _1, 2));
        EXPECT_FALSE(it == std::end(m_modelData));
        EXPECT_EQ(result1[1], it->first);

        it = std::find_if(std::begin(m_modelData), std::end(m_modelData), std::bind(getMatching, _1, 3));
        EXPECT_FALSE(it == std::end(m_modelData));
        EXPECT_EQ(result2[0], it->first);
    }
    EXPECT_TRUE(m_modelData.remove(result1[0]));
    {
        auto it = std::find_if(std::begin(m_modelData), std::end(m_modelData), std::bind(getMatching, _1, 1));
        EXPECT_TRUE(it == std::end(m_modelData));

        it = std::find_if(std::begin(m_modelData), std::end(m_modelData), std::bind(getMatching, _1, 2));
        EXPECT_FALSE(it == std::end(m_modelData));
        EXPECT_EQ(result1[1], it->first);

        it = std::find_if(std::begin(m_modelData), std::end(m_modelData), std::bind(getMatching, _1, 3));
        EXPECT_FALSE(it == std::end(m_modelData));
        EXPECT_EQ(result2[0], it->first);
    }
    EXPECT_TRUE(m_modelData.update(result1[1], Result(4)));
    {
        auto it = std::find_if(std::begin(m_modelData), std::end(m_modelData), std::bind(getMatching, _1, 1));
        EXPECT_TRUE(it == std::end(m_modelData));

        it = std::find_if(std::begin(m_modelData), std::end(m_modelData), std::bind(getMatching, _1, 4));
        EXPECT_FALSE(it == std::end(m_modelData));
        EXPECT_EQ(result1[1], it->first);

        it = std::find_if(std::begin(m_modelData), std::end(m_modelData), std::bind(getMatching, _1, 3));
        EXPECT_FALSE(it == std::end(m_modelData));
        EXPECT_EQ(result2[0], it->first);
    }
}

TEST_F(TstModelData, InvalidOperations)
{
    m_modelData.add({Result(1), Result(2)});
    EXPECT_FALSE(m_modelData.remove(nullptr));
    EXPECT_FALSE(m_modelData.update(nullptr, Result(5)));

}
