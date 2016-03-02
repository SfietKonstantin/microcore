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
#include "core/globals.h"
#include "core/pipe.h"
#include "mockjob.h"
#include "mockjobfactory.h"

using namespace ::testing;
using namespace ::microcore::core;

class ResultA
{
public:
    explicit ResultA() = default;
    explicit ResultA(int v) : value(v) {}
    DEFAULT_COPY_DEFAULT_MOVE(ResultA);
    bool operator==(const ResultA &other) const
    {
        return other.value == value;
    }
    int value {0};
};

class ResultB
{
public:
    explicit ResultB() = default;
    explicit ResultB(int v) : value(v) {}
    DEFAULT_COPY_DEFAULT_MOVE(ResultB);
    bool operator==(const ResultB &other) const
    {
        return other.value == value;
    }
    int value {0};
};

class ResultC
{
public:
    explicit ResultC() = default;
    explicit ResultC(int v) : value(v) {}
    DEFAULT_COPY_DEFAULT_MOVE(ResultC);
    bool operator==(const ResultC &other) const
    {
        return other.value == value;
    }
    int value {0};
};

class Error
{
public:
    explicit Error() = default;
    explicit Error(int v) : value(v) {}
    DEFAULT_COPY_DEFAULT_MOVE(Error);
    bool operator==(const Error &other) const
    {
        return other.value == value;
    }
    int value {0};
};

using ABPipe = Pipe<ResultA, ResultB, Error>;
using BCPipe = Pipe<ResultB, ResultC, Error>;
using BJob = MockJob<ResultB, Error>;
using CJob = MockJob<ResultC, Error>;
using ABFactory = MockJobFactory<ResultA, ResultB, Error>;
using BCFactory = MockJobFactory<ResultB, ResultC, Error>;
using Callback = MockJobCallback<ResultC, Error>;

class TstPipe: public Test
{
protected:
    void SetUp() override
    {
       m_bcPipe.reset(new BCPipe(m_bcFactory, m_callback));
       m_abPipe = m_bcPipe->prepend<ResultA>(m_abFactory);
    }
    ABFactory m_abFactory;
    BCFactory m_bcFactory;
    Callback m_callback;
    std::unique_ptr<ABPipe> m_abPipe;
    std::unique_ptr<BCPipe> m_bcPipe;
};

TEST_F(TstPipe, TestSimpleSuccess)
{
    // Mock
    EXPECT_CALL(m_abFactory, create(_)).Times(0);
    EXPECT_CALL(m_abFactory, create(ResultA(1))).Times(1).WillRepeatedly(Invoke([](ResultA) {
        std::unique_ptr<BJob> returned (new BJob);
        EXPECT_CALL(*returned, execute(_)).Times(1).WillRepeatedly(Invoke([](BJob::ICallback &callback) {
           callback.onResult(ResultB(2));
        }));
        return returned;
    }));
    EXPECT_CALL(m_bcFactory, create(_)).Times(0);
    EXPECT_CALL(m_bcFactory, create(ResultB(2))).Times(1).WillRepeatedly(Invoke([](ResultB) {
        std::unique_ptr<CJob> returned (new CJob);
        EXPECT_CALL(*returned, execute(_)).Times(1).WillRepeatedly(Invoke([](CJob::ICallback &callback) {
           callback.onResult(ResultC(3));
        }));
        return returned;
    }));
    EXPECT_CALL(m_callback, mockOnResult(_)).Times(0);
    EXPECT_CALL(m_callback, mockOnResult(ResultC(3))).Times(1);
    EXPECT_CALL(m_callback, mockOnError(_)).Times(0);

    // Test
    m_abPipe->send(ResultA(1));
}

TEST_F(TstPipe, TestSimpleError1)
{
    // Mock
    EXPECT_CALL(m_abFactory, create(_)).Times(0);
    EXPECT_CALL(m_abFactory, create(ResultA(1))).Times(1).WillRepeatedly(Invoke([](ResultA) {
        std::unique_ptr<BJob> returned (new BJob);
        EXPECT_CALL(*returned, execute(_)).Times(1).WillRepeatedly(Invoke([](BJob::ICallback &callback) {
           callback.onResult(ResultB(2));
        }));
        return returned;
    }));
    EXPECT_CALL(m_bcFactory, create(_)).Times(0);
    EXPECT_CALL(m_bcFactory, create(ResultB(2))).Times(1).WillRepeatedly(Invoke([](ResultB) {
        std::unique_ptr<CJob> returned (new CJob);
        EXPECT_CALL(*returned, execute(_)).Times(1).WillRepeatedly(Invoke([](CJob::ICallback &callback) {
           callback.onError(Error(3));
        }));
        return returned;
    }));
    EXPECT_CALL(m_callback, mockOnResult(_)).Times(0);
    EXPECT_CALL(m_callback, mockOnError(_)).Times(0);
    EXPECT_CALL(m_callback, mockOnError(Error(3))).Times(1);

    // Test
    m_abPipe->send(ResultA(1));
}

TEST_F(TstPipe, TestSimpleError2)
{
    // Mock
    EXPECT_CALL(m_abFactory, create(_)).Times(0);
    EXPECT_CALL(m_abFactory, create(ResultA(1))).Times(1).WillRepeatedly(Invoke([](ResultA) {
        std::unique_ptr<BJob> returned (new BJob);
        EXPECT_CALL(*returned, execute(_)).Times(1).WillRepeatedly(Invoke([](BJob::ICallback &callback) {
           callback.onError(Error(2));
        }));
        return returned;
    }));
    EXPECT_CALL(m_bcFactory, create(_)).Times(0);
    EXPECT_CALL(m_callback, mockOnResult(_)).Times(0);
    EXPECT_CALL(m_callback, mockOnError(_)).Times(0);
    EXPECT_CALL(m_callback, mockOnError(Error(2))).Times(1);

    // Test
    m_abPipe->send(ResultA(1));
}

TEST_F(TstPipe, TestSimpleError3)
{
    // Mock
    EXPECT_CALL(m_abFactory, create(_)).Times(0);
    EXPECT_CALL(m_bcFactory, create(_)).Times(0);
    EXPECT_CALL(m_callback, mockOnResult(_)).Times(0);
    EXPECT_CALL(m_callback, mockOnError(_)).Times(0);
    EXPECT_CALL(m_callback, mockOnError(Error(1))).Times(1);

    // Test
    m_abPipe->sendError(Error(1));
}
