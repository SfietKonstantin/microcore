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
#include "microgen/test.h"
#include "microgen/testobject.h"
#include "microgen/testrequestfactory.h"

TEST(MicroGen, Bean)
{
    ::microcore::test::Test test1 {};
    ::microcore::test::Test test2 {QString("id"), QString("name"), QString("description"), 123};
    ::microcore::test::Test test3 {test2};
    ::microcore::test::Test test4 {};
    test4 = test2;
    EXPECT_EQ(test4.id(), test2.id());
    EXPECT_EQ(test4.name(), test2.name());
    EXPECT_EQ(test4.description(), test2.description());
    EXPECT_EQ(test4.integer(), test2.integer());
}

TEST(MicroGen, BeanQt)
{
    ::microcore::test::Test test {QString("id"), QString("name"), QString("description"), 123};
    ::microcore::test::qt::TestObject test1 {};
    ::microcore::test::qt::TestObject test2 {::microcore::test::Test(test)};
    EXPECT_EQ(test.id(), test2.id());
    EXPECT_EQ(test.name(), test2.name());
    EXPECT_EQ(test.description(), test2.description());
    EXPECT_EQ(test.integer(), test2.integer());
    EXPECT_EQ(test.id(), test2.data().id());
    EXPECT_EQ(test.name(), test2.data().name());
    EXPECT_EQ(test.description(), test2.data().description());
    EXPECT_EQ(test.integer(), test2.data().integer());


    ::microcore::test::Test newTest {QString("newId"), QString("newName"), QString("newDescription"), 456};
    test2.update(::microcore::test::Test(newTest)); // Not updatable
    EXPECT_EQ(test.id(), test2.id());
    EXPECT_EQ(test.name(), test2.name());
    EXPECT_EQ(test.description(), test2.description());
    EXPECT_EQ(test.integer(), test2.integer());
    EXPECT_EQ(test.id(), test2.data().id());
    EXPECT_EQ(test.name(), test2.data().name());
    EXPECT_EQ(test.description(), test2.data().description());
    EXPECT_EQ(test.integer(), test2.data().integer());
}

TEST(MicroGen, Factory)
{
    ::microcore::test::TestRequestFactory factory {};
    EXPECT_TRUE(factory.create(::microcore::json::JsonResult()));
}
