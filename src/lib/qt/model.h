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

#ifndef MICROCORE_QT_MODEL_H
#define MICROCORE_QT_MODEL_H

#include "imodel.h"
#include "core/qobjectptr.h"
#include "data/model.h"

namespace microcore { namespace qt {

template<class Data, class DataObject>
class Model: public IModel, public ::microcore::data::IModelListener< ::microcore::data::Model<Data>>
{
public:
    ~Model()
    {
        if (m_model != nullptr) {
            m_model->removeListener(*this);
        }
    }
    void classBegin() override
    {
    }
    void componentComplete() override
    {
        m_complete = true;
        refreshData();
    }
    int rowCount(const QModelIndex &parent = QModelIndex()) const override final
    {
        Q_UNUSED(parent)
        return m_items.size();
    }
    int count() const override
    {
        return rowCount();
    }
    Status status() const override
    {
        return m_status;
    }
    QString errorMessage() const override
    {
        return m_errorMessage;
    }
public Q_SLOTS:
    void startMove() override
    {
    }
    void move(int from, int to) override
    {
    }
    void endMove()
    {
    }
protected:
    using Content_t = QObjectPtr<DataObject>;
    explicit Model(QObject *parent = 0)
        : IModel(parent)
    {
    }
    std::deque<Content_t> m_items {};
private:
    void onAppend(const std::vector<const Data *> &items) override
    {
        beginInsertRows(QModelIndex(), rowCount(), rowCount() + items.size() - 1);
        for (const Data *entry : items) {
            m_items.emplace_back(DataObject::create(*entry, this));
        }
        Q_EMIT countChanged();
        endInsertRows();
    }
    void onPrepend(const std::vector<const Data *> &items) override
    {
        beginInsertRows(QModelIndex(), 0, items.size() - 1);
        for (auto it = items.rbegin(); it != items.rend(); ++it) {
            m_items.emplace_front(DataObject::create(**it, this));
        }
        Q_EMIT countChanged();
        endInsertRows();
    }
    void onUpdate(std::size_t index, const Data *item) override
    {
        int indexInt = static_cast<int>(index);
        if (indexInt >= rowCount()) {
            return;
        }
        m_items[index]->update(*item);
        Q_EMIT dataChanged(this->index(indexInt), this->index(indexInt));
    }

    void onRemove(std::size_t index) override
    {
        int indexInt = static_cast<int>(index);
        if (indexInt >= rowCount()) {
            return;
        }

        beginRemoveRows(QModelIndex(), indexInt, indexInt);
        m_items.erase(std::begin(m_items) + index);
        Q_EMIT countChanged();
        endRemoveRows();
    }
    void onMove(std::size_t from, std::size_t to) override
    {
        performMove(from, to);
    }
    void onInvalidation() override
    {
        m_model = nullptr;
    }
    void refreshData()
    {
        if (!m_complete) {
            return;
        }

        std::deque<Content_t> newItems;
        if (m_model != nullptr) {
            std::for_each(std::begin(m_model), std::end(m_model), [&newItems, this](const Data *item) {
                newItems.emplace_back(DataObject::create(*item, this));
            });
        }

        int oldSize = m_items.size();
        int newSize = newItems.size();
        int delta = newSize - oldSize;

        if (delta < 0) {
            beginRemoveRows(QModelIndex(), newSize, oldSize - 1);
        } else if (delta > 0) {
            beginInsertRows(QModelIndex(), oldSize, newSize - 1);
        }

        m_items = std::move(newItems);

        Q_EMIT dataChanged(index(0), index(rowCount() - 1));

        if (delta != 0) {
            Q_EMIT countChanged();
        }
        if (delta < 0) {
            endRemoveRows();
        } else if (delta > 0) {
            endInsertRows();
        }
    }
    void setStatusAndErrorMessage(Status status, const QString &errorMessage)
    {
        if (m_status != status) {
            m_status = status;
            Q_EMIT statusChanged();
            switch (m_status) {
            case Idle:
                Q_EMIT finished();
                break;
            case Error:
                Q_EMIT error();
                break;
            default:
                break;
            }
        }

        if (m_errorMessage != errorMessage) {
            m_errorMessage = errorMessage;
            Q_EMIT errorMessageChanged();
        }
    }
    void performMove(std::size_t from, std::size_t to)
    {
        if (from >= m_items.size() || to > m_items.size()) {
            return;
        }

        int fromInt = static_cast<int>(from);
        int toInt = static_cast<int>(to);
        if (!beginMoveRows(QModelIndex(), fromInt, fromInt, QModelIndex(), toInt)) {
            return;
        }

        std::size_t toIndex = (to < from) ? to : to - 1;
        Content_t item = std::move(*(std::begin(m_items) + from));
        m_items.erase(std::begin(m_items) + from);
        m_items.insert(std::begin(m_items) + toIndex, std::move(item));
        endMoveRows();
    }
    bool m_complete {false};
    Status m_status {Idle};
    QString m_errorMessage {};
    ::microcore::data::Model<Data> *m_model {nullptr};
};

}}

#endif // MICROCORE_QT_MODEL_H
