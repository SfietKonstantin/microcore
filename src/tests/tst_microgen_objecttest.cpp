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
#include <microgen/object_test.h>
#include <microgen/object_testobject.h>
//#include <microgen/testrequestfactory.h>
#include <microgen/nested_test.h>
#include <QSignalSpy>
#include <QJsonObject>

using namespace ::testing;

TEST(MicroGenObjectTest, Bean)
{
    // Empty constructor
    {
        ::microcore::test::ObjectTest empty {};
        Q_UNUSED(empty)
    }
    // Copy constructor
    {
        ::microcore::test::ObjectTest test {
            ::microcore::test::ObjectTest::ReadWriteContent {
                QLatin1String("read_write/constant"),
                QLatin1String("read_write/read_only"),
                QLatin1String("read_write/read_write")
            },
            ::microcore::test::ObjectTest::ConstantContent {
                QLatin1String("constant/constant"),
                QLatin1String("constant/read_only"),
                QLatin1String("constant/read_write")
            }
        };
        ::microcore::test::ObjectTest copyConstructed {test};
        EXPECT_EQ(copyConstructed.readWriteContent().constant(), test.readWriteContent().constant());
        EXPECT_EQ(copyConstructed.readWriteContent().readOnly(), test.readWriteContent().readOnly());
        EXPECT_EQ(copyConstructed.readWriteContent().readWrite(), test.readWriteContent().readWrite());
        EXPECT_EQ(copyConstructed.readWriteContent(), test.readWriteContent());
        EXPECT_EQ(copyConstructed.constantContent().constant(), test.constantContent().constant());
        EXPECT_EQ(copyConstructed.constantContent().readOnly(), test.constantContent().readOnly());
        EXPECT_EQ(copyConstructed.constantContent().readWrite(), test.constantContent().readWrite());
        EXPECT_EQ(copyConstructed.constantContent(), test.constantContent());
        EXPECT_EQ(copyConstructed, test);
    }
    // Move constructor
    {
        ::microcore::test::ObjectTest test {
            ::microcore::test::ObjectTest::ReadWriteContent {
                QLatin1String("read_write/constant"),
                QLatin1String("read_write/read_only"),
                QLatin1String("read_write/read_write")
            },
            ::microcore::test::ObjectTest::ConstantContent {
                QLatin1String("constant/constant"),
                QLatin1String("constant/read_only"),
                QLatin1String("constant/read_write")
            }
        };
        ::microcore::test::ObjectTest moveConstructed {::microcore::test::ObjectTest(test)};
        EXPECT_EQ(moveConstructed.readWriteContent().constant(), test.readWriteContent().constant());
        EXPECT_EQ(moveConstructed.readWriteContent().readOnly(), test.readWriteContent().readOnly());
        EXPECT_EQ(moveConstructed.readWriteContent().readWrite(), test.readWriteContent().readWrite());
        EXPECT_EQ(moveConstructed.readWriteContent(), test.readWriteContent());
        EXPECT_EQ(moveConstructed.constantContent().constant(), test.constantContent().constant());
        EXPECT_EQ(moveConstructed.constantContent().readOnly(), test.constantContent().readOnly());
        EXPECT_EQ(moveConstructed.constantContent().readWrite(), test.constantContent().readWrite());
        EXPECT_EQ(moveConstructed.constantContent(), test.constantContent());
        EXPECT_EQ(moveConstructed, test);
    }
    // Assignment operator
    {
        ::microcore::test::ObjectTest test {
            ::microcore::test::ObjectTest::ReadWriteContent {
                QLatin1String("read_write/constant"),
                QLatin1String("read_write/read_only"),
                QLatin1String("read_write/read_write")
            },
            ::microcore::test::ObjectTest::ConstantContent {
                QLatin1String("constant/constant"),
                QLatin1String("constant/read_only"),
                QLatin1String("constant/read_write")
            }
        };
        ::microcore::test::ObjectTest assigned {};
        assigned = test;
        EXPECT_EQ(assigned.readWriteContent().constant(), test.readWriteContent().constant());
        EXPECT_EQ(assigned.readWriteContent().readOnly(), test.readWriteContent().readOnly());
        EXPECT_EQ(assigned.readWriteContent().readWrite(), test.readWriteContent().readWrite());
        EXPECT_EQ(assigned.readWriteContent(), test.readWriteContent());
        EXPECT_EQ(assigned.constantContent().constant(), test.constantContent().constant());
        EXPECT_EQ(assigned.constantContent().readOnly(), test.constantContent().readOnly());
        EXPECT_EQ(assigned.constantContent().readWrite(), test.constantContent().readWrite());
        EXPECT_EQ(assigned.constantContent(), test.constantContent());
        EXPECT_EQ(assigned, test);
    }
    // Move assignment operator
    {
        ::microcore::test::ObjectTest test {
            ::microcore::test::ObjectTest::ReadWriteContent {
                QLatin1String("read_write/constant"),
                QLatin1String("read_write/read_only"),
                QLatin1String("read_write/read_write")
            },
            ::microcore::test::ObjectTest::ConstantContent {
                QLatin1String("constant/constant"),
                QLatin1String("constant/read_only"),
                QLatin1String("constant/read_write")
            }
        };
        ::microcore::test::ObjectTest cloned {test};
        ::microcore::test::ObjectTest moveAssigned {};
        moveAssigned = std::move(cloned);
        EXPECT_EQ(moveAssigned.readWriteContent().constant(), test.readWriteContent().constant());
        EXPECT_EQ(moveAssigned.readWriteContent().readOnly(), test.readWriteContent().readOnly());
        EXPECT_EQ(moveAssigned.readWriteContent().readWrite(), test.readWriteContent().readWrite());
        EXPECT_EQ(moveAssigned.readWriteContent(), test.readWriteContent());
        EXPECT_EQ(moveAssigned.constantContent().constant(), test.constantContent().constant());
        EXPECT_EQ(moveAssigned.constantContent().readOnly(), test.constantContent().readOnly());
        EXPECT_EQ(moveAssigned.constantContent().readWrite(), test.constantContent().readWrite());
        EXPECT_EQ(moveAssigned.constantContent(), test.constantContent());
        EXPECT_EQ(moveAssigned, test);
    }
    // Setter
    {
        {
            ::microcore::test::ObjectTest::ReadWriteContent test {
                QLatin1String("read_write/constant"),
                QLatin1String("read_write/read_only"),
                QLatin1String("read_write/read_write")
            };
            test.setReadWrite("read_write/read_write2");
            EXPECT_EQ(test.constant(), test.constant());
            EXPECT_EQ(test.readOnly(), test.readOnly());
            EXPECT_EQ(test.readWrite(), "read_write/read_write2");
        }
        {
            ::microcore::test::ObjectTest::ConstantContent test {
                QLatin1String("constant/constant"),
                QLatin1String("constant/read_only"),
                QLatin1String("constant/read_write")
            };
            test.setReadWrite("constant/read_write2");
            EXPECT_EQ(test.constant(), test.constant());
            EXPECT_EQ(test.readOnly(), test.readOnly());
            EXPECT_EQ(test.readWrite(), "constant/read_write2");
        }
        {
            ::microcore::test::ObjectTest test {
                ::microcore::test::ObjectTest::ReadWriteContent {
                    QLatin1String("read_write/constant"),
                    QLatin1String("read_write/read_only"),
                    QLatin1String("read_write/read_write")
                },
                ::microcore::test::ObjectTest::ConstantContent {
                    QLatin1String("constant/constant"),
                    QLatin1String("constant/read_only"),
                    QLatin1String("constant/read_write")
                }
            };
            test.setReadWriteContent(::microcore::test::ObjectTest::ReadWriteContent {
                QLatin1String("read_write/constant2"),
                QLatin1String("read_write/read_only2"),
                QLatin1String("read_write/read_write2")
            });
            EXPECT_EQ(test.readWriteContent().constant(), "read_write/constant2");
            EXPECT_EQ(test.readWriteContent().readOnly(), "read_write/read_only2");
            EXPECT_EQ(test.readWriteContent().readWrite(), "read_write/read_write2");
            EXPECT_EQ(test.constantContent().constant(), "constant/constant");
            EXPECT_EQ(test.constantContent().readOnly(), "constant/read_only");
            EXPECT_EQ(test.constantContent().readWrite(), "constant/read_write");
        }
    }
    // Comparisons
    {
        {
            ::microcore::test::ObjectTest::ReadWriteContent first {
                QLatin1String("read_write/constant"),
                QLatin1String("read_write/read_only"),
                QLatin1String("read_write/read_write")
            };
            ::microcore::test::ObjectTest::ReadWriteContent second {
                QLatin1String("read_write/constant2"),
                QLatin1String("read_write/read_only"),
                QLatin1String("read_write/read_write")
            };
            EXPECT_NE(first, second);
        }
        {
            ::microcore::test::ObjectTest::ReadWriteContent first {
                QLatin1String("read_write/constant"),
                QLatin1String("read_write/read_only"),
                QLatin1String("read_write/read_write")
            };
            ::microcore::test::ObjectTest::ReadWriteContent second {
                QLatin1String("read_write/constant"),
                QLatin1String("read_write/read_only2"),
                QLatin1String("read_write/read_write")
            };
            EXPECT_NE(first, second);
        }
        {
            ::microcore::test::ObjectTest::ReadWriteContent first {
                QLatin1String("read_write/constant"),
                QLatin1String("read_write/read_only"),
                QLatin1String("read_write/read_write")
            };
            ::microcore::test::ObjectTest::ReadWriteContent second {
                QLatin1String("read_write/constant"),
                QLatin1String("read_write/read_only"),
                QLatin1String("read_write/read_write2")
            };
            EXPECT_NE(first, second);
        }
        {
            ::microcore::test::ObjectTest::ConstantContent first {
                QLatin1String("constant/constant"),
                QLatin1String("constant/read_only"),
                QLatin1String("constant/read_write")
            };
            ::microcore::test::ObjectTest::ConstantContent second {
                QLatin1String("constant/constant2"),
                QLatin1String("constant/read_only"),
                QLatin1String("constant/read_write")
            };
            EXPECT_NE(first, second);
        }
        {
            ::microcore::test::ObjectTest::ConstantContent first {
                QLatin1String("constant/constant"),
                QLatin1String("constant/read_only"),
                QLatin1String("constant/read_write")
            };
            ::microcore::test::ObjectTest::ConstantContent second {
                QLatin1String("constant/constant"),
                QLatin1String("constant/read_only2"),
                QLatin1String("constant/read_write")
            };
            EXPECT_NE(first, second);
        }
        {
            ::microcore::test::ObjectTest::ConstantContent first {
                QLatin1String("constant/constant"),
                QLatin1String("constant/read_only"),
                QLatin1String("constant/read_write")
            };
            ::microcore::test::ObjectTest::ConstantContent second {
                QLatin1String("constant/constant"),
                QLatin1String("constant/read_only"),
                QLatin1String("constant/read_write2")
            };
            EXPECT_NE(first, second);
        }
        {
            ::microcore::test::ObjectTest first {
                ::microcore::test::ObjectTest::ReadWriteContent {
                    QLatin1String("read_write/constant"),
                    QLatin1String("read_write/read_only"),
                    QLatin1String("read_write/read_write")
                },
                ::microcore::test::ObjectTest::ConstantContent {
                    QLatin1String("constant/constant"),
                    QLatin1String("constant/read_only"),
                    QLatin1String("constant/read_write")
                }
            };
            ::microcore::test::ObjectTest second {
                ::microcore::test::ObjectTest::ReadWriteContent {
                    QLatin1String("read_write/constant2"),
                    QLatin1String("read_write/read_only2"),
                    QLatin1String("read_write/read_write2")
                },
                ::microcore::test::ObjectTest::ConstantContent {
                    QLatin1String("constant/constant"),
                    QLatin1String("constant/read_only"),
                    QLatin1String("constant/read_write")
                }
            };
            EXPECT_NE(first, second);
        }
        {
            ::microcore::test::ObjectTest first {
                ::microcore::test::ObjectTest::ReadWriteContent {
                    QLatin1String("read_write/constant"),
                    QLatin1String("read_write/read_only"),
                    QLatin1String("read_write/read_write")
                },
                ::microcore::test::ObjectTest::ConstantContent {
                    QLatin1String("constant/constant"),
                    QLatin1String("constant/read_only"),
                    QLatin1String("constant/read_write")
                }
            };
            ::microcore::test::ObjectTest second {
                ::microcore::test::ObjectTest::ReadWriteContent {
                    QLatin1String("read_write/constant"),
                    QLatin1String("read_write/read_only"),
                    QLatin1String("read_write/read_write")
                },
                ::microcore::test::ObjectTest::ConstantContent {
                    QLatin1String("constant/constant2"),
                    QLatin1String("constant/read_only2"),
                    QLatin1String("constant/read_write2")
                }
            };
            EXPECT_NE(first, second);
        }
    }
}

TEST(MicroGenObjectTest, BeanQt)
{
    // Empty constructor
    {
        ::microcore::test::ObjectTest empty {};
        ::microcore::test::qt::ObjectTestObject emptyObject {};
        EXPECT_TRUE(emptyObject.readWriteContent() != nullptr);
        EXPECT_TRUE(emptyObject.constantContent() != nullptr);
        EXPECT_EQ(emptyObject.readWriteContent()->data(), empty.readWriteContent());
        EXPECT_EQ(emptyObject.constantContent()->data(), empty.constantContent());
        EXPECT_EQ(emptyObject.data(), empty);
    }
    // Default constructor
    {
        ::microcore::test::ObjectTest test {
            ::microcore::test::ObjectTest::ReadWriteContent {
                QLatin1String("read_write/constant"),
                QLatin1String("read_write/read_only"),
                QLatin1String("read_write/read_write")
            },
            ::microcore::test::ObjectTest::ConstantContent {
                QLatin1String("constant/constant"),
                QLatin1String("constant/read_only"),
                QLatin1String("constant/read_write")
            }
        };
        ::microcore::test::qt::ObjectTestObject testObject {::microcore::test::ObjectTest(test)};
        EXPECT_TRUE(testObject.readWriteContent() != nullptr);
        EXPECT_TRUE(testObject.constantContent() != nullptr);
        EXPECT_EQ(testObject.readWriteContent()->data(), test.readWriteContent());
        EXPECT_EQ(testObject.constantContent()->data(), test.constantContent());
        EXPECT_EQ(testObject.data(),test);

    }
    // Update
    {
        ::microcore::test::ObjectTest test {
            ::microcore::test::ObjectTest::ReadWriteContent {
                QLatin1String("read_write/constant"),
                QLatin1String("read_write/read_only"),
                QLatin1String("read_write/read_write")
            },
            ::microcore::test::ObjectTest::ConstantContent {
                QLatin1String("constant/constant"),
                QLatin1String("constant/read_only"),
                QLatin1String("constant/read_write")
            }
        };
        ::microcore::test::qt::ObjectTestObject testObject {::microcore::test::ObjectTest(test)};

        QSignalSpy readWriteContentReadOnlySpy (testObject.readWriteContent(), SIGNAL(readOnlyChanged()));
        QSignalSpy readWriteContentReadWriteSpy (testObject.readWriteContent(), SIGNAL(readWriteChanged()));
        QSignalSpy constantContentReadOnlySpy (testObject.constantContent(), SIGNAL(readOnlyChanged()));
        QSignalSpy constantContentReadWriteSpy (testObject.constantContent(), SIGNAL(readWriteChanged()));
        ::microcore::test::ObjectTest newTest {
            ::microcore::test::ObjectTest::ReadWriteContent {
                QLatin1String("read_write/constant2"),
                QLatin1String("read_write/read_only2"),
                QLatin1String("read_write/read_write2")
            },
            ::microcore::test::ObjectTest::ConstantContent {
                QLatin1String("constant/constant2"),
                QLatin1String("constant/read_only2"),
                QLatin1String("constant/read_write2")
            }
        };
        testObject.update(::microcore::test::ObjectTest(newTest));
        EXPECT_EQ(readWriteContentReadOnlySpy.count(), 1);
        EXPECT_EQ(readWriteContentReadWriteSpy.count(), 1);
        EXPECT_EQ(constantContentReadOnlySpy.count(), 0);
        EXPECT_EQ(constantContentReadWriteSpy.count(), 0);
        EXPECT_EQ(testObject.readWriteContent()->constant(), test.readWriteContent().constant());
        EXPECT_EQ(testObject.readWriteContent()->readOnly(), newTest.readWriteContent().readOnly());
        EXPECT_EQ(testObject.readWriteContent()->readWrite(), newTest.readWriteContent().readWrite());
        EXPECT_EQ(testObject.constantContent()->constant(), test.constantContent().constant());
        EXPECT_EQ(testObject.constantContent()->readOnly(), test.constantContent().readOnly());
        EXPECT_EQ(testObject.constantContent()->readWrite(), test.constantContent().readWrite());
        {
            EXPECT_TRUE(testObject.readWriteContent() != nullptr);
            ::microcore::test::ObjectTest::ReadWriteContent expected {
                QLatin1String("read_write/constant"),
                QLatin1String("read_write/read_only2"),
                QLatin1String("read_write/read_write2")
            };
            EXPECT_EQ(testObject.readWriteContent()->data(), expected);
        }
        {
            EXPECT_TRUE(testObject.constantContent() != nullptr);
            ::microcore::test::ObjectTest::ConstantContent expected {
                QLatin1String("constant/constant"),
                QLatin1String("constant/read_only"),
                QLatin1String("constant/read_write")
            };
            EXPECT_EQ(testObject.constantContent()->data(), expected);
        }
    }
}

/*
class JobExecutionHandler
{
public:
    void onResult(::microcore::test::ObjectTest &&test)
    {
        onResultImpl(test);
    }
    MOCK_METHOD1(onResultImpl, void (const ::microcore::test::ObjectTest &test));
    void onError(::microcore::error::Error &&error)
    {
        onErrorImpl(error);
    }
    MOCK_METHOD1(onErrorImpl, void (const ::microcore::error::Error &error));
};

TEST(MicroGenObjectTest, Factory)
{
    using namespace std::placeholders;
    JobExecutionHandler jobExecutionHandler {};
    ::microcore::test::ObjectTest expected {
        QLatin1String("constant_value"),
        QLatin1String("name_value"),
        QLatin1String(),
        0,
        0.
    };
    EXPECT_CALL(jobExecutionHandler, onResultImpl(_)).Times(0);
    EXPECT_CALL(jobExecutionHandler, onResultImpl(expected)).Times(1);
    EXPECT_CALL(jobExecutionHandler, onErrorImpl(_)).Times(0);

    ::microcore::test::ObjectTestRequestFactory factory {};
    QJsonDocument document {QJsonObject {
        {QLatin1String("constants"), QJsonObject {
            {QLatin1String("constant"), QLatin1String("constant_value")}
        }},
        {QLatin1String("read_only"), QJsonObject {
            {QLatin1String("name"), QLatin1String("name_value")}
        }}
    }};

    std::unique_ptr< ::microcore::test::ObjectTestJob> job {factory.create(std::move(document))};
    EXPECT_TRUE(job);

    job->execute(std::bind(&JobExecutionHandler::onResult, &jobExecutionHandler, _1),
                 std::bind(&JobExecutionHandler::onError, &jobExecutionHandler, _1));

}

TEST(MicroGenObjectTest, FactoryFailure1)
{
    using namespace std::placeholders;
    JobExecutionHandler jobExecutionHandler {};
    EXPECT_CALL(jobExecutionHandler, onResultImpl(_)).Times(0);
    EXPECT_CALL(jobExecutionHandler, onErrorImpl(_)).Times(1);

    ::microcore::test::ObjectTestRequestFactory factory {};
    std::unique_ptr< ::microcore::test::ObjectTestJob> job {factory.create(std::move(QJsonDocument()))};
    EXPECT_TRUE(job);

    job->execute(std::bind(&JobExecutionHandler::onResult, &jobExecutionHandler, _1),
                 std::bind(&JobExecutionHandler::onError, &jobExecutionHandler, _1));

}

TEST(MicroGenObjectTest, FactoryFailure2)
{
    using namespace std::placeholders;
    JobExecutionHandler jobExecutionHandler {};
    EXPECT_CALL(jobExecutionHandler, onResultImpl(_)).Times(0);
    EXPECT_CALL(jobExecutionHandler, onErrorImpl(_)).Times(1);

    ::microcore::test::ObjectTestRequestFactory factory {};
    QJsonDocument document {QJsonObject {
        {QLatin1String("constants"), QJsonObject {
            {QLatin1String("constant"), QLatin1String("constant_value")}
        }}
    }};

    std::unique_ptr< ::microcore::test::ObjectTestJob> job {factory.create(std::move(document))};
    EXPECT_TRUE(job);

    job->execute(std::bind(&JobExecutionHandler::onResult, &jobExecutionHandler, _1),
                 std::bind(&JobExecutionHandler::onError, &jobExecutionHandler, _1));

}
*/
