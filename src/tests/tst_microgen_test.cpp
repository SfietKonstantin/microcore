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
#include <microgen/test.h>
#include <microgen/testobject.h>
#include <microgen/testrequestfactory.h>
#include <QSignalSpy>
#include <QJsonObject>

using namespace ::testing;

TEST(MicroGenTest, Bean)
{
    // Empty constructor
    {
        ::microcore::test::Test empty {};
        Q_UNUSED(empty)
    }
    // Copy constructor
    {
        ::microcore::test::Test test {QString("constant"), QString("read_only"), QString("read_write"), 123, 456.};
        ::microcore::test::Test copyConstructed {test};
        EXPECT_EQ(copyConstructed.constant(), test.constant());
        EXPECT_EQ(copyConstructed.readOnly(), test.readOnly());
        EXPECT_EQ(copyConstructed.readWrite(), test.readWrite());
        EXPECT_EQ(copyConstructed.integer(), test.integer());
        EXPECT_EQ(copyConstructed.doubleValue(), test.doubleValue());
        EXPECT_EQ(copyConstructed, test);
    }
    // Move constructor
    {
        ::microcore::test::Test test {QString("constant"), QString("read_only"), QString("read_write"), 123, 456.};
        ::microcore::test::Test moveConstructed {::microcore::test::Test(test)};
        EXPECT_EQ(moveConstructed.constant(), test.constant());
        EXPECT_EQ(moveConstructed.readOnly(), test.readOnly());
        EXPECT_EQ(moveConstructed.readWrite(), test.readWrite());
        EXPECT_EQ(moveConstructed.integer(), test.integer());
        EXPECT_EQ(moveConstructed.doubleValue(), test.doubleValue());
        EXPECT_EQ(moveConstructed, test);
    }
    // Assignment operator
    {
        ::microcore::test::Test test {QString("constant"), QString("read_only"), QString("read_write"), 123, 456.};
        ::microcore::test::Test assigned {};
        assigned = test;
        EXPECT_EQ(assigned.constant(), test.constant());
        EXPECT_EQ(assigned.readOnly(), test.readOnly());
        EXPECT_EQ(assigned.readWrite(), test.readWrite());
        EXPECT_EQ(assigned.integer(), test.integer());
        EXPECT_EQ(assigned.doubleValue(), test.doubleValue());
        EXPECT_EQ(assigned, test);
    }
    // Move assignment operator
    {
        ::microcore::test::Test test {QString("constant"), QString("read_only"), QString("read_write"), 123, 456.};
        ::microcore::test::Test cloned {test};
        ::microcore::test::Test moveAssigned {};
        moveAssigned = std::move(cloned);
        EXPECT_EQ(moveAssigned.constant(), test.constant());
        EXPECT_EQ(moveAssigned.readOnly(), test.readOnly());
        EXPECT_EQ(moveAssigned.readWrite(), test.readWrite());
        EXPECT_EQ(moveAssigned.integer(), test.integer());
        EXPECT_EQ(moveAssigned.doubleValue(), test.doubleValue());
        EXPECT_EQ(moveAssigned, test);
    }
    // Setter
    {
        ::microcore::test::Test test {QString("constant"), QString("read_only"), QString("read_write"), 123, 456.};
        test.setReadWrite("read_write2");
        EXPECT_EQ(test.constant(), test.constant());
        EXPECT_EQ(test.readOnly(), test.readOnly());
        EXPECT_EQ(test.readWrite(), "read_write2");
        EXPECT_EQ(test.integer(), test.integer());
        EXPECT_EQ(test.doubleValue(), test.doubleValue());
    }
    // Comparisons
    {
        {
            ::microcore::test::Test first {QString("constant"), QString("read_only"), QString("read_write"), 123, 456.};
            ::microcore::test::Test second {QString("constant2"), QString("read_only"), QString("read_write"), 123, 456.};
            EXPECT_NE(first, second);
        }
        {
            ::microcore::test::Test first {QString("constant"), QString("read_only"), QString("read_write"), 123, 456.};
            ::microcore::test::Test second {QString("constant"), QString("read_only2"), QString("read_write"), 123, 456.};
            EXPECT_NE(first, second);
        }
        {
            ::microcore::test::Test first {QString("constant"), QString("read_only"), QString("read_write"), 123, 456.};
            ::microcore::test::Test second {QString("constant"), QString("read_only"), QString("read_write2"), 123, 456.};
            EXPECT_NE(first, second);
        }
        {
            ::microcore::test::Test first {QString("constant"), QString("read_only"), QString("read_write"), 123, 456.};
            ::microcore::test::Test second {QString("constant"), QString("read_only"), QString("read_write"), 1234, 456.};
            EXPECT_NE(first, second);
        }
        {
            ::microcore::test::Test first {QString("constant"), QString("read_only"), QString("read_write"), 123, 456.};
            ::microcore::test::Test second {QString("constant"), QString("read_only"), QString("read_write"), 123, 4567.};
            EXPECT_NE(first, second);
        }
    }
}

TEST(MicroGenTest, BeanQt)
{
    // Empty constructor
    {
        ::microcore::test::Test empty {};
        ::microcore::test::qt::TestObject emptyObject {};
        EXPECT_EQ(emptyObject.constant(), empty.constant());
        EXPECT_EQ(emptyObject.readOnly(), empty.readOnly());
        EXPECT_EQ(emptyObject.readWrite(), empty.readWrite());
        EXPECT_EQ(emptyObject.integer(), empty.integer());
        EXPECT_EQ(emptyObject.doubleValue(), empty.doubleValue());
        EXPECT_EQ(emptyObject.data(), empty);
    }
    // Default constructor
    {
        ::microcore::test::Test test {QString("constant"), QString("read_only"), QString("read_write"), 123, 456.};
        ::microcore::test::qt::TestObject testObject {::microcore::test::Test(test)};
        EXPECT_EQ(testObject.constant(), test.constant());
        EXPECT_EQ(testObject.readOnly(), test.readOnly());
        EXPECT_EQ(testObject.readWrite(), test.readWrite());
        EXPECT_EQ(testObject.integer(), test.integer());
        EXPECT_EQ(testObject.doubleValue(), test.doubleValue());
        EXPECT_EQ(testObject.data(), test);
    }
    // Update
    {
        ::microcore::test::Test test {QString("constant"), QString("read_only"), QString("read_write"), 123, 456.};
        ::microcore::test::qt::TestObject testObject {::microcore::test::Test(test)};

        QSignalSpy readOnlySpy (&testObject, SIGNAL(readOnlyChanged()));
        QSignalSpy readWriteSpy (&testObject, SIGNAL(readWriteChanged()));
        ::microcore::test::Test newTest {QString("constant2"), QString("read_only2"), QString("read_write2"), 1234, 4567.};
        testObject.update(::microcore::test::Test(newTest));
        EXPECT_EQ(readOnlySpy.count(), 1);
        EXPECT_EQ(readWriteSpy.count(), 1);
        EXPECT_EQ(testObject.constant(), test.constant());
        EXPECT_EQ(testObject.readOnly(), newTest.readOnly());
        EXPECT_EQ(testObject.readWrite(), newTest.readWrite());
        EXPECT_EQ(testObject.integer(), test.integer());
        EXPECT_EQ(testObject.doubleValue(), test.doubleValue());
    }
    // Partial update 1
    {
        ::microcore::test::Test test {QString("constant"), QString("read_only"), QString("read_write"), 123, 456.};
        ::microcore::test::qt::TestObject testObject {::microcore::test::Test(test)};

        QSignalSpy readOnlySpy (&testObject, SIGNAL(readOnlyChanged()));
        QSignalSpy readWriteSpy (&testObject, SIGNAL(readWriteChanged()));
        ::microcore::test::Test newTest {QString("constant2"), QString("read_only2"), QString("read_write"), 1234, 4567.};
        testObject.update(::microcore::test::Test(newTest));
        EXPECT_EQ(readOnlySpy.count(), 1);
        EXPECT_EQ(readWriteSpy.count(), 0);
    }
    // Partial update 2
    {
        ::microcore::test::Test test {QString("constant"), QString("read_only"), QString("read_write"), 123, 456.};
        ::microcore::test::qt::TestObject testObject {::microcore::test::Test(test)};

        QSignalSpy readOnlySpy (&testObject, SIGNAL(readOnlyChanged()));
        QSignalSpy readWriteSpy (&testObject, SIGNAL(readWriteChanged()));
        ::microcore::test::Test newTest {QString("constant2"), QString("read_only"), QString("read_write2"), 1234, 4567.};
        testObject.update(::microcore::test::Test(newTest));
        EXPECT_EQ(readOnlySpy.count(), 0);
        EXPECT_EQ(readWriteSpy.count(), 1);
    }
    // No update
    {
        ::microcore::test::Test test {QString("constant"), QString("read_only"), QString("read_write"), 123, 456.};
        ::microcore::test::qt::TestObject testObject {::microcore::test::Test(test)};

        QSignalSpy readOnlySpy (&testObject, SIGNAL(readOnlyChanged()));
        QSignalSpy readWriteSpy (&testObject, SIGNAL(readWriteChanged()));
        ::microcore::test::Test newTest {QString("constant2"), QString("read_only"), QString("read_write"), 1234, 4567.};
        testObject.update(::microcore::test::Test(newTest));
        EXPECT_EQ(readOnlySpy.count(), 0);
        EXPECT_EQ(readWriteSpy.count(), 0);
    }
}

class JobExecutionHandler
{
public:
    void onResult(::microcore::test::Test &&test)
    {
        onResultImpl(test);
    }
    MOCK_METHOD1(onResultImpl, void (const ::microcore::test::Test &test));
    void onError(::microcore::error::Error &&error)
    {
        onErrorImpl(error);
    }
    MOCK_METHOD1(onErrorImpl, void (const ::microcore::error::Error &error));
};

TEST(MicroGenTest, Factory)
{
    using namespace std::placeholders;
    JobExecutionHandler jobExecutionHandler {};
    ::microcore::test::Test expected {
        QLatin1String("constant_value"),
        QLatin1String("read_only_value"),
        QLatin1String(),
        0,
        0.
    };
    EXPECT_CALL(jobExecutionHandler, onResultImpl(_)).Times(0);
    EXPECT_CALL(jobExecutionHandler, onResultImpl(expected)).Times(1);
    EXPECT_CALL(jobExecutionHandler, onErrorImpl(_)).Times(0);

    ::microcore::test::TestRequestFactory factory {};
    QJsonDocument document {QJsonObject {
        {QLatin1String("constants"), QJsonObject {
            {QLatin1String("constant"), QLatin1String("constant_value")}
        }},
        {QLatin1String("read_only"), QJsonObject {
            {QLatin1String("read_only"), QLatin1String("read_only_value")}
        }}
    }};

    std::unique_ptr< ::microcore::test::TestJob> job {factory.create(std::move(document))};
    EXPECT_TRUE(job);

    job->execute(std::bind(&JobExecutionHandler::onResult, &jobExecutionHandler, _1),
                 std::bind(&JobExecutionHandler::onError, &jobExecutionHandler, _1));

}

TEST(MicroGenTest, FactoryFailure1)
{
    using namespace std::placeholders;
    JobExecutionHandler jobExecutionHandler {};
    EXPECT_CALL(jobExecutionHandler, onResultImpl(_)).Times(0);
    EXPECT_CALL(jobExecutionHandler, onErrorImpl(_)).Times(1);

    ::microcore::test::TestRequestFactory factory {};
    std::unique_ptr< ::microcore::test::TestJob> job {factory.create(std::move(QJsonDocument()))};
    EXPECT_TRUE(job);

    job->execute(std::bind(&JobExecutionHandler::onResult, &jobExecutionHandler, _1),
                 std::bind(&JobExecutionHandler::onError, &jobExecutionHandler, _1));

}

TEST(MicroGenTest, FactoryFailure2)
{
    using namespace std::placeholders;
    JobExecutionHandler jobExecutionHandler {};
    EXPECT_CALL(jobExecutionHandler, onResultImpl(_)).Times(0);
    EXPECT_CALL(jobExecutionHandler, onErrorImpl(_)).Times(1);

    ::microcore::test::TestRequestFactory factory {};
    QJsonDocument document {QJsonObject {
        {QLatin1String("constants"), QJsonObject {
            {QLatin1String("constant"), QLatin1String("constant_value")}
        }}
    }};

    std::unique_ptr< ::microcore::test::TestJob> job {factory.create(std::move(document))};
    EXPECT_TRUE(job);

    job->execute(std::bind(&JobExecutionHandler::onResult, &jobExecutionHandler, _1),
                 std::bind(&JobExecutionHandler::onError, &jobExecutionHandler, _1));

}
