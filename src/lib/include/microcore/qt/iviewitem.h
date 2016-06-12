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

#ifndef MICROCORE_QT_IVIEWITEM_H
#define MICROCORE_QT_IVIEWITEM_H

#include <microcore/core/globals.h>
#include <QtCore/QAbstractListModel>
#include <QtQml/QQmlParserStatus>

namespace microcore { namespace qt {

class IViewItem : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QObject * controller READ controller WRITE setController NOTIFY controllerChanged)
    Q_PROPERTY(QObject * item READ item NOTIFY itemChanged)
public:
    DISABLE_COPY_DISABLE_MOVE(IViewItem);
    virtual ~IViewItem() {}
    virtual QObject * controller() const = 0;
    virtual QObject * item() const = 0;
    virtual void setController(QObject *controller) = 0;
Q_SIGNALS:
    void controllerChanged();
    void itemChanged();
protected:
    explicit IViewItem(QObject *parent = nullptr) : QObject(parent), QQmlParserStatus() {}
};

}}

#endif // MICROCORE_QT_IVIEWITEM_H
