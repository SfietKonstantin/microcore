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

}

using ResultSource = DataSource<int, Result>;

class TstDataSource: public Test
{
public:
    explicit TstDataSource()
        : m_data(std::function<int (const Result &)>(&Result::getKey))
    {
    }
protected:
   ResultSource m_data;
};

TEST_F(TstDataSource, TestSize)
{
    m_data.add(Result(1, 1));
    EXPECT_FALSE(m_data.empty());
    EXPECT_EQ(m_data.size(), 1);
    m_data.add(Result(2, 2));
    EXPECT_FALSE(m_data.empty());
    EXPECT_EQ(m_data.size(), 2);
    m_data.add(Result(1, 2));
    EXPECT_FALSE(m_data.empty());
    EXPECT_EQ(m_data.size(), 2);
    m_data.remove(Result(1, 123));
    EXPECT_FALSE(m_data.empty());
    EXPECT_EQ(m_data.size(), 1);
    m_data.remove(Result(2, 123));
    EXPECT_TRUE(m_data.empty());
    EXPECT_EQ(m_data.size(), 0);
}

TEST_F(TstDataSource, TestRemoveFail)
{
    m_data.add(Result(1, 1));
    m_data.remove(Result(2, 1));
    EXPECT_EQ(m_data.size(), 1);
}

TEST_F(TstDataSource, TestUpdate)
{
    const Result &result1 = m_data.add(Result(1, 1));
    EXPECT_EQ(result1, Result(1, 1));
    m_data.add(Result(1, 2));

    const Result &result1Update = m_data.add(Result(1, 2));
    EXPECT_EQ(result1, Result(1, 2));
    EXPECT_EQ(result1Update, Result(1, 2));
    EXPECT_EQ(&result1, &result1Update);
}

TEST_F(TstDataSource, TestUpdate2)
{
    const Result &result1 = m_data.add(Result(1, 1));
    EXPECT_EQ(result1, Result(1, 1));
    EXPECT_TRUE(m_data.update(result1, Result(1, 2)));
    EXPECT_FALSE(m_data.update(result1, Result(2, 2)));
    EXPECT_FALSE(m_data.update(Result(2, 2), Result(2, 1)));
}
