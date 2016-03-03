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

class TstPipe: public Test
{
protected:
    void SetUp() override
    {
        using namespace std::placeholders;
        auto onResult {std::bind(&TstPipe::onResult, this, _1)};
        auto onError {std::bind(&TstPipe::onError, this, _1)};

        m_bcPipe.reset(new BCPipe(m_bcFactory, std::move(onResult), std::move(onError)));
        m_abPipe = m_bcPipe->prepend<ResultA>(m_abFactory);
    }
    MOCK_METHOD1(mockOnResult, void (const ResultC &result));
    void onResult(ResultC &&result)
    {
        mockOnResult(result);
    }
    MOCK_METHOD1(mockOnError, void (const Error &error));
    void onError(Error &&error)
    {
        mockOnError(error);
    }
    ABFactory m_abFactory {};
    BCFactory m_bcFactory {};
    std::unique_ptr<ABPipe> m_abPipe {};
    std::unique_ptr<BCPipe> m_bcPipe {};
};

TEST_F(TstPipe, TestSimpleSuccess)
{
    // Mock
    EXPECT_CALL(m_abFactory, mockCreate(_)).Times(0);
    EXPECT_CALL(m_abFactory, mockCreate(ResultA(1))).Times(1).WillRepeatedly(Invoke([](const ResultA &) {
        std::unique_ptr<BJob> returned (new BJob);
        EXPECT_CALL(*returned, execute(_, _)).Times(1).WillRepeatedly(Invoke([](BJob::OnResult_t onResult, BJob::OnError_t) {
           onResult(ResultB(2));
        }));
        return returned;
    }));
    EXPECT_CALL(m_bcFactory, mockCreate(_)).Times(0);
    EXPECT_CALL(m_bcFactory, mockCreate(ResultB(2))).Times(1).WillRepeatedly(Invoke([](const ResultB &) {
        std::unique_ptr<CJob> returned (new CJob);
        EXPECT_CALL(*returned, execute(_, _)).Times(1).WillRepeatedly(Invoke([](CJob::OnResult_t onResult, CJob::OnError_t) {
           onResult(ResultC(3));
        }));
        return returned;
    }));
    EXPECT_CALL(*this, mockOnResult(_)).Times(0);
    EXPECT_CALL(*this, mockOnResult(ResultC(3))).Times(1);
    EXPECT_CALL(*this, mockOnError(_)).Times(0);

    // Test
    m_abPipe->send(ResultA(1));
}

TEST_F(TstPipe, TestSimpleError1)
{
    // Mock
    EXPECT_CALL(m_abFactory, mockCreate(_)).Times(0);
    EXPECT_CALL(m_abFactory, mockCreate(ResultA(1))).Times(1).WillRepeatedly(Invoke([](const ResultA &) {
        std::unique_ptr<BJob> returned (new BJob);
        EXPECT_CALL(*returned, execute(_, _)).Times(1).WillRepeatedly(Invoke([](BJob::OnResult_t onResult, BJob::OnError_t) {
           onResult(ResultB(2));
        }));
        return returned;
    }));
    EXPECT_CALL(m_bcFactory, mockCreate(_)).Times(0);
    EXPECT_CALL(m_bcFactory, mockCreate(ResultB(2))).Times(1).WillRepeatedly(Invoke([](const ResultB &) {
        std::unique_ptr<CJob> returned (new CJob);
        EXPECT_CALL(*returned, execute(_, _)).Times(1).WillRepeatedly(Invoke([](CJob::OnResult_t, CJob::OnError_t onError) {
           onError(Error(3));
        }));
        return returned;
    }));
    EXPECT_CALL(*this, mockOnResult(_)).Times(0);
    EXPECT_CALL(*this, mockOnError(_)).Times(0);
    EXPECT_CALL(*this, mockOnError(Error(3))).Times(1);

    // Test
    m_abPipe->send(ResultA(1));
}

TEST_F(TstPipe, TestSimpleError2)
{
    // Mock
    EXPECT_CALL(m_abFactory, mockCreate(_)).Times(0);
    EXPECT_CALL(m_abFactory, mockCreate(ResultA(1))).Times(1).WillRepeatedly(Invoke([](const ResultA &) {
        std::unique_ptr<BJob> returned (new BJob);
        EXPECT_CALL(*returned, execute(_, _)).Times(1).WillRepeatedly(Invoke([](BJob::OnResult_t, BJob::OnError_t onError) {
           onError(Error(2));
        }));
        return returned;
    }));
    EXPECT_CALL(m_bcFactory, mockCreate(_)).Times(0);
    EXPECT_CALL(*this, mockOnResult(_)).Times(0);
    EXPECT_CALL(*this, mockOnError(_)).Times(0);
    EXPECT_CALL(*this, mockOnError(Error(2))).Times(1);

    // Test
    m_abPipe->send(ResultA(1));
}

TEST_F(TstPipe, TestSimpleError3)
{
    // Mock
    EXPECT_CALL(m_abFactory, mockCreate(_)).Times(0);
    EXPECT_CALL(m_bcFactory, mockCreate(_)).Times(0);
    EXPECT_CALL(*this, mockOnResult(_)).Times(0);
    EXPECT_CALL(*this, mockOnError(_)).Times(0);
    EXPECT_CALL(*this, mockOnError(Error(1))).Times(1);

    // Test
    m_abPipe->sendError(Error(1));
}

class TestOnlyMovable
{
public:
    explicit TestOnlyMovable() = default;
    explicit TestOnlyMovable(int v) : value(v) {}
    DISABLE_COPY_DEFAULT_MOVE(TestOnlyMovable);
    int value {0};
};

static void testOnResult(TestOnlyMovable &&) {}
static void testOnError(TestOnlyMovable &&) {}

class TestOnlyMovableResultJob: public IJob<TestOnlyMovable, TestOnlyMovable>
{
public:
    TestOnlyMovableResultJob(TestOnlyMovable &&result)
        : m_result(std::move(result))
    {
    }
    void execute(OnResult_t onResult, OnError_t) override
    {
        onResult(std::move(m_result));
    }
private:
    TestOnlyMovable m_result {};
};

class TestOnlyMovableResultJobFactory: public IJobFactory<TestOnlyMovable, TestOnlyMovable, TestOnlyMovable>
{
public:
    std::unique_ptr<IJob<TestOnlyMovable, TestOnlyMovable>> create(TestOnlyMovable &&request) const override
    {
        return std::unique_ptr<IJob<TestOnlyMovable, TestOnlyMovable>>(new TestOnlyMovableResultJob(std::move(request)));
    }
};

class TestOnlyMovableErrorJob: public IJob<TestOnlyMovable, TestOnlyMovable>
{
public:
    TestOnlyMovableErrorJob(TestOnlyMovable &&error)
        : m_error(std::move(error))
    {
    }
    void execute(OnResult_t, OnError_t onError) override
    {
        onError(std::move(m_error));
    }
private:
    TestOnlyMovable m_error {};
};

class TestOnlyMovableErrorJobFactory: public IJobFactory<TestOnlyMovable, TestOnlyMovable, TestOnlyMovable>
{
public:
    std::unique_ptr<IJob<TestOnlyMovable, TestOnlyMovable>> create(TestOnlyMovable &&request) const override
    {
        return std::unique_ptr<IJob<TestOnlyMovable, TestOnlyMovable>>(new TestOnlyMovableErrorJob(std::move(request)));
    }
};

using TestOnlyMovablePipe = Pipe<TestOnlyMovable, TestOnlyMovable, TestOnlyMovable>;


TEST_F(TstPipe, OnlyMovableConstructible)
{
    TestOnlyMovableResultJobFactory factory;
    auto onResult {IJob<TestOnlyMovable, TestOnlyMovable>::OnResult_t(testOnResult)};
    auto onError {IJob<TestOnlyMovable, TestOnlyMovable>::OnError_t(testOnError)};

    std::unique_ptr<TestOnlyMovablePipe> pipe3 {new TestOnlyMovablePipe(factory, onResult, onError)};
    std::unique_ptr<TestOnlyMovablePipe> pipe2 {pipe3->prepend<TestOnlyMovable>(factory)};
    std::unique_ptr<TestOnlyMovablePipe> pipe1 {pipe2->prepend<TestOnlyMovable>(factory)};

    pipe1->send(TestOnlyMovable(123));
}

TEST_F(TstPipe, OnlyMovableConstructibleWithError)
{
    TestOnlyMovableResultJobFactory factory;
    TestOnlyMovableErrorJobFactory errorFactory;
    auto onResult {IJob<TestOnlyMovable, TestOnlyMovable>::OnResult_t(testOnResult)};
    auto onError {IJob<TestOnlyMovable, TestOnlyMovable>::OnError_t(testOnError)};

    std::unique_ptr<TestOnlyMovablePipe> pipe3 {new TestOnlyMovablePipe(factory, onResult, onError)};
    std::unique_ptr<TestOnlyMovablePipe> pipe2 {pipe3->prepend<TestOnlyMovable>(errorFactory)};
    std::unique_ptr<TestOnlyMovablePipe> pipe1 {pipe2->prepend<TestOnlyMovable>(factory)};

    pipe1->send(TestOnlyMovable(123));
}
