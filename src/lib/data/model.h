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

#ifndef MICROCORE_DATA_MODEL_H
#define MICROCORE_DATA_MODEL_H

#include "modeldata.h"
#include "core/ijobfactory.h"
#include "core/pipe.h"
#include "error/error.h"
#include <set>
#include <deque>

namespace microcore { namespace data {

/**
 * @brief A generic container
 */
template<class Request, class Data, class Store>
class ModelBase
{
public:

   /**
    * @brief An interface for a Model listener
    *
    * This interface is used to implement models-like classes, that listens to a
    * Model and want to be notified when the Model changes.
    *
    * This listener can be used to get notifications when some items are inserted,
    * removed, or updated. This is done via onAppend(), onPrepend(), onUpdate() and
    * onRemove().
    *
    * This interface also handle the status of the asynchronous loading operation
    * that takes places in the Model. This is done via onStart(), onError() and
    * onFinished().
    */
   class IListener
   {
   public:
        virtual ~IListener() {}
        /**
         * @brief Notify that new items are appended
         * @param items items to be appended.
         */
        virtual void onAppend(const std::vector<const Data *> &items) = 0;
        /**
         * @brief Notify that new items are prepended
         * @param items items to be prepended.
         */
        virtual void onPrepend(const std::vector<const Data *> &items) = 0;
        /**
         * @brief Notify that an item is updated
         * @param index index of the item that is updated.
         * @param item the new value of the item.
         */
        virtual void onUpdate(int index, const Data &item) = 0;
        /**
         * @brief Notify that an item is removed
         * @param index index of the item that is removed.
         */
        virtual void onRemove(int index) = 0;
        /**
         * @brief Notify that an item has moved
         * @param from index of the item to move.
         * @param to the item's new index.
         */
        virtual void onMove(int from, int to) = 0;
        /**
         * @brief Notify that the listened object is now invalid
         *
         * When called on this method, the listener should stop
         * listening.
         */
        virtual void onInvalidation() = 0;
        /**
         * @brief Notify that an asynchronous operation has started
         */
        virtual void onStart() = 0;
        /**
         * @brief Notify that an asynchronous operation has failed
         * @param error error message.
         */
        virtual void onError(const ::microcore::error::Error &error) = 0;
        /**
         * @brief Notify that an asynchronous operation has finished
         */
        virtual void onFinish() = 0;
    };
    using List = std::deque<const Data *>;
    using Factory = ::microcore::core::IJobFactory<Request, std::vector<Data>, ::microcore::error::Error>;
    DISABLE_COPY_DEFAULT_MOVE(ModelBase);
    ~ModelBase()
    {
        for (IListener *listener : m_listeners) {
            listener->onInvalidation();
        }
    }
    typename List::const_iterator begin() const
    {
        return std::begin(m_data);
    }
    typename List::const_iterator end() const
    {
        return std::end(m_data);
    }
    bool empty() const
    {
        return m_data.empty();
    }
    std::size_t size() const
    {
        return m_data.size();
    }
    void append(std::vector<Data> &&data)
    {
        using namespace std::placeholders;
        const std::vector<const Data *> &stored = m_store.add(std::move(data));
        m_data.insert(std::end(m_data), std::begin(stored), std::end(stored));
        std::for_each(std::begin(m_listeners), std::end(m_listeners),
                      std::bind(&IListener::onAppend, _1, std::ref(stored)));
    }
    void prepend(std::vector<Data> &&data)
    {
        using namespace std::placeholders;
        const std::vector<const Data *> &stored = m_store.add(std::move(data));
        m_data.insert(std::begin(m_data), std::begin(stored), std::end(stored));
        std::for_each(std::begin(m_listeners), std::end(m_listeners),
                      std::bind(&IListener::onPrepend, _1, std::ref(stored)));
    }
    void update(std::size_t index, Data &&data)
    {
        using namespace std::placeholders;
        if (index >= m_data.size()) {
            return;
        }
        *m_data[index] = std::move(data);
        std::for_each(std::begin(m_listeners), std::end(m_listeners),
                      std::bind(&IListener::onUpdate, _1, std::ref(*(m_data[index]))));
    }
    void remove(std::size_t index)
    {
        using namespace std::placeholders;
        if (index >= m_data.size()) {
            return;
        }
        m_store.remove(m_data.at(index));
        m_data.erase(std::begin(m_data) + index);
        std::for_each(std::begin(m_listeners), std::end(m_listeners),
                      std::bind(&IListener::onRemove, _1, index));
    }
    void move(std::size_t from, std::size_t to)
    {
        using namespace std::placeholders;
        if (from >= m_data.size() || to > m_data.size()) {
            return;
        }

        if (to == from || to == from + 1) {
            return;
        }

        std::size_t toIndex = (to < from) ? to : to - 1;

        const Data *data {*(std::begin(m_data) + from)};
        m_data.erase(std::begin(m_data) + from);
        m_data.insert(std::begin(m_data) + toIndex, data);
        std::for_each(std::begin(m_listeners), std::end(m_listeners),
                      std::bind(&IListener::onMove, _1, from, to));
    }
    void start(Request &&request)
    {
        using namespace std::placeholders;
        if (m_pipe) {
            return;
        }

        m_error = ::microcore::error::Error();

        OnResult_t onResult {[this](std::vector<Data> &&data) {
            append(std::move(data));
            finish();
        }};
        OnError_t onError {std::bind(&ModelBase<Request, Data, Store>::error, this, _1)};

        m_pipe.reset(new Pipe(m_factory, std::move(onResult), std::move(onError)));
        m_pipe->send(std::move(request));
        std::for_each(std::begin(m_listeners), std::end(m_listeners),
                      std::bind(&IListener::onStart, _1));
    }
    void error(::microcore::error::Error &&error)
    {
        using namespace std::placeholders;
        m_pipe.reset();
        m_error = std::move(error);
        std::for_each(std::begin(m_listeners), std::end(m_listeners),
                      std::bind(&IListener::onError, _1, std::ref(m_error)));
    }
    void finish()
    {
        using namespace std::placeholders;
        m_pipe.reset();
        m_error = ::microcore::error::Error();
        std::for_each(std::begin(m_listeners), std::end(m_listeners),
                      std::bind(&IListener::onFinish, _1));
    }
    void addListener(IListener &listener)
    {
        m_listeners.insert(&listener);
        if (m_pipe) {
            listener.onStart();
        } else if (!m_error.empty()){
            listener.onError(m_error);
        }
    }
    void removeListener(IListener &listener)
    {
        m_listeners.erase(&listener);
    }
protected:
    explicit ModelBase(const Factory &factory, Store &&store)
        : m_store(std::move(store)), m_factory(factory)
    {
    }
private:
    using Pipe = ::microcore::core::Pipe<Request, std::vector<Data>, ::microcore::error::Error>;
    using OnResult_t = typename ::microcore::core::IJob<std::vector<Data>, ::microcore::error::Error>::OnResult_t;
    using OnError_t = typename ::microcore::core::IJob<std::vector<Data>, ::microcore::error::Error>::OnError_t;

    Store m_store {};
    std::deque<const Data *> m_data {};
    const Factory &m_factory;
    std::set<IListener *> m_listeners {};

    std::unique_ptr<Pipe>  m_pipe {};
    ::microcore::error::Error m_error {};
};

template<class Request, class Data>
class Model : public ModelBase<Request, Data, ModelData<Data>>
{
public:
    explicit Model(const typename ModelBase<Request, Data, ModelData<Data>>::Factory &factory)
        : ModelBase<Request, Data, ModelData<Data>>(factory, ModelData<Data>())
    {
    }
};

}}

#endif // MICROCORE_DATA_MODEL_H
