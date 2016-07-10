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
#include <microcore/core/listenerrepository.h>

using namespace ::testing;

class MockListener
{
public:
    using Ptr = std::shared_ptr<MockListener>;
    static Ptr create()
    {
        return Ptr(new MockListener());
    }
    void onInvalidation()
    {
        invalidated = true;
    }
    MOCK_METHOD1(onEvent, void (int arg1));
    bool invalidated {false};
};

using MockListenerRepository = ::microcore::core::ListenerRepository<MockListener>;

TEST(TstListenerRepository, EmptySize)
{
    // Mock
    MockListenerRepository listenerRepository {};
    MockListener::Ptr listener1 {MockListener::create()};
    MockListener::Ptr listener2 {MockListener::create()};

    // Test
    EXPECT_TRUE(listenerRepository.isEmpty());
    EXPECT_EQ(listenerRepository.count(), static_cast<std::size_t>(0));
    listenerRepository.addListener(listener1);
    listenerRepository.addListener(listener2);
    EXPECT_FALSE(listenerRepository.isEmpty());
    EXPECT_EQ(listenerRepository.count(), static_cast<std::size_t>(2));
}


TEST(TstListenerRepository, Notify)
{
    // Mock
    using namespace std::placeholders;
    MockListenerRepository listenerRepository {};
    MockListener::Ptr listener1 {MockListener::create()};
    EXPECT_CALL(*listener1, onEvent(_)).Times(0);
    EXPECT_CALL(*listener1, onEvent(123)).Times(1);
    EXPECT_CALL(*listener1, onEvent(234)).Times(1);
    MockListener::Ptr listener2 {MockListener::create()};
    EXPECT_CALL(*listener2, onEvent(_)).Times(0);
    EXPECT_CALL(*listener2, onEvent(123)).Times(1);
    EXPECT_CALL(*listener2, onEvent(234)).Times(1);

    // Test
    listenerRepository.addListener(listener1);
    listenerRepository.addListener(listener2);
    const MockListenerRepository &constListenerRepository {listenerRepository};
    constListenerRepository.notify(std::bind(&MockListener::onEvent, _1, 123));
    listenerRepository.notify(std::bind(&MockListener::onEvent, _1, 234));
}

TEST(TstListenerRepository, NotifyDestroyed)
{
    // Mock
    using namespace std::placeholders;
    MockListenerRepository listenerRepository {};
    MockListener::Ptr listener1 {MockListener::create()};
    EXPECT_CALL(*listener1, onEvent(_)).Times(0);
    EXPECT_CALL(*listener1, onEvent(123)).Times(1);
    EXPECT_CALL(*listener1, onEvent(234)).Times(1);
    EXPECT_CALL(*listener1, onEvent(345)).Times(1);
    EXPECT_CALL(*listener1, onEvent(456)).Times(1);
    MockListener::Ptr listener2 {MockListener::create()};
    EXPECT_CALL(*listener2, onEvent(_)).Times(0);
    EXPECT_CALL(*listener2, onEvent(123)).Times(1);
    EXPECT_CALL(*listener2, onEvent(234)).Times(1);

    // Test
    listenerRepository.addListener(listener1);
    listenerRepository.addListener(listener2);
    const MockListenerRepository &constListenerRepository {listenerRepository};
    constListenerRepository.notify(std::bind(&MockListener::onEvent, _1, 123));
    listenerRepository.notify(std::bind(&MockListener::onEvent, _1, 234));
    listener2.reset();
    constListenerRepository.notify(std::bind(&MockListener::onEvent, _1, 345));
    EXPECT_EQ(listenerRepository.count(), static_cast<std::size_t>(2));
    listenerRepository.notify(std::bind(&MockListener::onEvent, _1, 456));
    EXPECT_EQ(listenerRepository.count(), static_cast<std::size_t>(1));
}

TEST(TstListenerRepository, NotifyMultipleTimes)
{
    // Mock
    using namespace std::placeholders;
    MockListenerRepository listenerRepository {};
    MockListener::Ptr listener {MockListener::create()};
    EXPECT_CALL(*listener, onEvent(_)).Times(0);
    EXPECT_CALL(*listener, onEvent(123)).Times(2);

    // Test
    listenerRepository.addListener(listener);
    listenerRepository.addListener(listener);
    EXPECT_EQ(listenerRepository.count(), static_cast<std::size_t>(2));
    listenerRepository.notify(std::bind(&MockListener::onEvent, _1, 123));
}

TEST(TstListenerRepository, Remove)
{
    // Mock
    MockListenerRepository listenerRepository {};
    MockListener::Ptr listener {MockListener::create()};

    // Test
    listenerRepository.addListener(listener);
    EXPECT_EQ(listenerRepository.count(), static_cast<std::size_t>(1));

    listenerRepository.removeListener(listener);
    EXPECT_EQ(listenerRepository.count(), static_cast<std::size_t>(0));
}

TEST(TstListenerRepository, RemoveMultiple)
{
    // Mock
    MockListenerRepository listenerRepository {};
    MockListener::Ptr listener {MockListener::create()};

    // Test
    listenerRepository.addListener(listener);
    listenerRepository.addListener(listener);
    EXPECT_EQ(listenerRepository.count(), static_cast<std::size_t>(2));

    listenerRepository.removeListener(listener);
    EXPECT_EQ(listenerRepository.count(), static_cast<std::size_t>(0));
}

TEST(TstListenerRepository, RemoveEmpty)
{
    // Mock
    MockListenerRepository listenerRepository {};
    MockListener::Ptr listener {MockListener::create()};

    // Test
    listenerRepository.addListener(listener);
    EXPECT_EQ(listenerRepository.count(), static_cast<std::size_t>(1));
    listener.reset();

    listenerRepository.removeListener(MockListener::Ptr());
    EXPECT_EQ(listenerRepository.count(), static_cast<std::size_t>(0));
}

TEST(TstListenerRepository, Invalidation)
{
    // Mock
    std::unique_ptr<MockListenerRepository> listenerRepository {new MockListenerRepository()};
    MockListener::Ptr listener {MockListener::create()};

    // Test
    listenerRepository->addListener(listener);
    listenerRepository.reset();

    EXPECT_TRUE(listener->invalidated);
}
