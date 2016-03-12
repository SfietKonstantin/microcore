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

#ifndef MICROCORE_QT_VIEWITEM_H
#define MICROCORE_QT_VIEWITEM_H

#include "iviewitem.h"
#include "viewitemcontroller.h"
#include "core/qobjectptr.h"
#include "data/model.h"

namespace microcore { namespace qt {

template<class Data, class DataObject>
class ViewItem: public IViewItem
{
public:
    void classBegin() override
    {
    }
    void componentComplete() override
    {
        m_complete = true;
        refreshData();
    }
    QObject * controller() const override final
    {
        return m_controller;
    }
    QObject * item() const override final
    {
        return m_data.get();
    }
    void setController(QObject *controllerObject) override final
    {
        Controller_t *controller = dynamic_cast<Controller_t *>(controllerObject);
        if (m_controller != controller) {
            if (m_controller) {
                disconnect(m_controller, &Controller_t::finished, this, &ViewItem<Data, DataObject>::refreshData);
            }
            m_controller = controller;
            if (m_controller) {
                connect(m_controller, &Controller_t::finished, this, &ViewItem<Data, DataObject>::refreshData);
            }
            Q_EMIT controllerChanged();
            refreshData();
        }
    }
protected:
    using Data_t = QObjectPtr<DataObject>;
    explicit ViewItem(QObject *parent = nullptr)
        : IViewItem(parent)
    {
    }
    Data_t m_data {};
private:
    using Controller_t = ViewItemController<Data>;
    void refreshData()
    {
        if (!m_complete) {
            return;
        }

        if (m_controller == nullptr) {
            m_data.reset();
        } else {
            m_data.reset(new DataObject(m_controller->data()));
        }
        Q_EMIT itemChanged();
    }
    bool m_complete {false};
    Controller_t *m_controller {nullptr};
};

}}


#endif // MICROCORE_QT_VIEWITEM_H
