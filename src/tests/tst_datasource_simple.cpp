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
#include "core/datasource.h"
#include "mockjob.h"
#include "mockjobfactory.h"

using namespace ::testing;
using namespace ::microcore::core;

class Request
{
public:
    explicit Request() = default;
    explicit Request(int v) : value(v) {}
    DEFAULT_COPY_DEFAULT_MOVE(Request);
    bool operator==(const Request &other) const
    {
        return other.value == value;
    }
    int value {0};
};

class ResultIr
{
public:
    explicit ResultIr() = default;
    explicit ResultIr(int v) : value(v) {}
    DEFAULT_COPY_DEFAULT_MOVE(ResultIr);
    bool operator==(const ResultIr &other) const
    {
        return other.value == value;
    }
    int value {0};
};

class Result
{
public:
    explicit Result() = default;
    explicit Result(int v) : value(v) {}
    DEFAULT_COPY_DEFAULT_MOVE(Result);
    bool operator==(const Result &other) const
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

class Container
{
public:
    void setResult(Result &&result)
    {
        setResult(result);
    }
    void setError(Error &&error)
    {
        setError(error);
    }

    MOCK_METHOD1(setResult, void (const Result &result));
    MOCK_METHOD1(setError, void (const Error &error));
};

using TestDataSource = DataSource<Container, Request, ResultIr, Result, Error>;
using ResultIrJob = MockJob<ResultIr, Error>;
using ResultJob = MockJob<Result, Error>;
using RequestToIrJobFactory = MockJobFactory<Request, ResultIr, Error>;
using IrToResultJobFactory = MockJobFactory<ResultIr, Result, Error>;

class TstDataSourceSimple: public Test
{
protected:
    void SetUp() override
    {
        m_requestToIrFactory = new RequestToIrJobFactory();
        m_irToResultFactory = new IrToResultJobFactory();
        m_dataSource.reset(new TestDataSource(std::unique_ptr<RequestToIrJobFactory>(m_requestToIrFactory),
                                              std::unique_ptr<IrToResultJobFactory>(m_irToResultFactory)));
    }
    std::unique_ptr<TestDataSource> m_dataSource {};
    RequestToIrJobFactory *m_requestToIrFactory {nullptr};
    IrToResultJobFactory *m_irToResultFactory {nullptr};
};

TEST_F(TstDataSourceSimple, TestSimpleSuccess)
{
    // Mock
    Container container {};
    EXPECT_CALL(*m_requestToIrFactory, create(_)).Times(0);
    EXPECT_CALL(*m_requestToIrFactory, create(Request(1))).Times(1).WillRepeatedly(Invoke([](Request) {
        std::unique_ptr<ResultIrJob> returned (new ResultIrJob);
        EXPECT_CALL(*returned, execute(_)).Times(1).WillRepeatedly(Invoke([](ResultIrJob::ICallback &callback) {
           callback.onResult(ResultIr(2));
        }));
        return returned;
    }));
    EXPECT_CALL(*m_irToResultFactory, create(_)).Times(0);
    EXPECT_CALL(*m_irToResultFactory, create(ResultIr(2))).Times(1).WillRepeatedly(Invoke([](ResultIr) {
        std::unique_ptr<ResultJob> returned (new ResultJob);
        EXPECT_CALL(*returned, execute(_)).Times(1).WillRepeatedly(Invoke([](ResultJob::ICallback &callback) {
           callback.onResult(Result(3));
        }));
        return returned;
    }));
    EXPECT_CALL(container, setResult(_)).Times(0);
    EXPECT_CALL(container, setResult(Result(3))).Times(1);
    EXPECT_CALL(container, setError(_)).Times(0);

    // Test
    m_dataSource->applyRequest(container, Request(1));
}

TEST_F(TstDataSourceSimple, TestSimpleError1)
{
    // Mock
    Container container {};
    EXPECT_CALL(*m_requestToIrFactory, create(_)).Times(0);
    EXPECT_CALL(*m_requestToIrFactory, create(Request(1))).Times(1).WillRepeatedly(Invoke([](Request) {
        std::unique_ptr<ResultIrJob> returned (new ResultIrJob);
        EXPECT_CALL(*returned, execute(_)).Times(1).WillRepeatedly(Invoke([](ResultIrJob::ICallback &callback) {
           callback.onError(Error(2));
        }));
        return returned;
    }));
    EXPECT_CALL(*m_irToResultFactory, create(_)).Times(0);
    EXPECT_CALL(container, setResult(_)).Times(0);
    EXPECT_CALL(container, setError(_)).Times(0);
    EXPECT_CALL(container, setError(Error(2))).Times(1);

    // Test
    m_dataSource->applyRequest(container, Request(1));
}

TEST_F(TstDataSourceSimple, TestSimpleError2)
{
    // Mock
    Container container {};
    EXPECT_CALL(*m_requestToIrFactory, create(_)).Times(0);
    EXPECT_CALL(*m_requestToIrFactory, create(Request(1))).Times(1).WillRepeatedly(Invoke([](Request) {
        std::unique_ptr<ResultIrJob> returned (new ResultIrJob);
        EXPECT_CALL(*returned, execute(_)).Times(1).WillRepeatedly(Invoke([](ResultIrJob::ICallback &callback) {
           callback.onResult(ResultIr(2));
        }));
        return returned;
    }));
    EXPECT_CALL(*m_irToResultFactory, create(_)).Times(0);
    EXPECT_CALL(*m_irToResultFactory, create(ResultIr(2))).Times(1).WillRepeatedly(Invoke([](ResultIr) {
        std::unique_ptr<ResultJob> returned (new ResultJob);
        EXPECT_CALL(*returned, execute(_)).Times(1).WillRepeatedly(Invoke([](ResultJob::ICallback &callback) {
           callback.onError(Error(3));
        }));
        return returned;
    }));
    EXPECT_CALL(container, setResult(_)).Times(0);
    EXPECT_CALL(container, setError(_)).Times(0);
    EXPECT_CALL(container, setError(Error(3))).Times(1);

    // Test
    m_dataSource->applyRequest(container, Request(1));
}
