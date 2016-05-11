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

#ifndef MICROCORE_QT_VIEWMODEL_H
#define MICROCORE_QT_VIEWMODEL_H

#include "iviewmodel.h"
#include "viewmodelcontroller.h"
#include "data/model.h"
#include "qt/qobjectptr.h"

namespace microcore { namespace qt {

template<class Model, class DataObject>
class ViewModel: public IViewModel, public Model::IListener
{
public:
    ~ViewModel()
    {
        if (m_controller != nullptr) {
            m_controller->model().removeListener(*this);
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
    QObject * controller() const override final
    {
        return m_controller;
    }
    void setController(QObject *controllerObject) override final
    {
        Controller_t *controller = dynamic_cast<Controller_t *>(controllerObject);
        if (m_controller != controller) {
            if (m_controller) {
                m_controller->model().removeListener(*this);
            }
            m_controller = controller;
            if (m_controller) {
                m_controller->model().addListener(*this);
            }
            Q_EMIT controllerChanged();
            refreshData();
        }
    }
    int count() const override
    {
        return rowCount();
    }
protected:
    using StoredItem_t = QObjectPtr<DataObject>;
    using Storage_t = std::deque<StoredItem_t>;
    explicit ViewModel(QObject *parent = nullptr)
        : IViewModel(parent)
    {
    }
    Storage_t m_items {};
private:
    using Controller_t = ViewModelController<Model>;
    void onAppend(const typename Model::NotificationItems_t &items) override final
    {
        beginInsertRows(QModelIndex(), rowCount(), rowCount() + items.size() - 1);
        std::for_each(std::begin(items), std::end(items), [this](typename Model::NotificationItem_t item) {
            m_items.emplace_back(DataObject::create(*item, this));
        });
        Q_EMIT countChanged();
        endInsertRows();
    }
    void onPrepend(const typename Model::NotificationItems_t &items) override final
    {
        beginInsertRows(QModelIndex(), 0, items.size() - 1);
        std::for_each(items.rbegin(), items.rend(), [this](typename Model::NotificationItem_t item) {
            m_items.emplace_front(DataObject::create(*item, this));
        });
        Q_EMIT countChanged();
        endInsertRows();
    }
    void onUpdate(std::size_t index, typename Model::NotificationItem_t item) override final
    {
        int indexInt = static_cast<int>(index);
        if (indexInt >= rowCount()) {
            return;
        }
        m_items[index]->update(*item);
        Q_EMIT dataChanged(this->index(indexInt), this->index(indexInt));
    }

    void onRemove(std::size_t index) override final
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
    void onMove(std::size_t from, std::size_t to) override final
    {
        performMove(from, to);
    }
    void onInvalidation() override final
    {
        if (m_controller != nullptr) {
            m_controller = nullptr;
            Q_EMIT controllerChanged();
        }
    }
    void refreshData()
    {
        if (!m_complete) {
            return;
        }

        Storage_t newItems;
        if (m_controller != nullptr) {
            Model &model = m_controller->model();
            std::for_each(std::begin(model), std::end(model), [&newItems, this](typename Model::NotificationItem_t item) {
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
        StoredItem_t item = std::move(*(std::begin(m_items) + from));
        m_items.erase(std::begin(m_items) + from);
        m_items.insert(std::begin(m_items) + toIndex, std::move(item));
        endMoveRows();
    }
    bool m_complete {false};
    Controller_t *m_controller {nullptr};
};

}}

#endif // MICROCORE_QT_VIEWMODEL_H
