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
#include <QSignalSpy>
#include "qt/viewmodel.h"

using namespace ::testing;
using namespace ::microcore;
using namespace ::microcore::data;
using namespace ::microcore::qt;

namespace {

class Result
{
public:
    explicit Result() = default;
    explicit Result(int v) : value {v} {}
    DEFAULT_COPY_DEFAULT_MOVE(Result);
    int value {0};
};

class ResultObject final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)
public:
    explicit ResultObject(QObject *parent = nullptr)
        : QObject(parent)
    {
    }
    static ResultObject * create(const Result &result, QObject *parent)
    {
        ResultObject *returned {new ResultObject(parent)};
        returned->m_value = result.value;
        return returned;
    }
    int value() const
    {
        return m_value;
    }
    void setValue(int value)
    {
        if (m_value != value) {
            m_value = value;
            emit valueChanged();
        }
    }
    void update(const Result &result)
    {
        setValue(result.value);
    }

signals:
    void valueChanged();
private:
    int m_value {-1};
};

}

using TestModel = Model<Result>;

class TestViewModel final : public ViewModel<TestModel, ResultObject>
{
public:
    enum Roles {
        ObjectRole = Qt::UserRole + 1
    };
    explicit TestViewModel(QObject *parent = nullptr)
        : ViewModel<TestModel, ResultObject>(parent)
    {
    }
    QVariant data(const QModelIndex &index, int role) const override
    {
        int row = index.row();
        if (row < 0 || row >= rowCount()) {
            return QVariant();
        }
        const QObjectPtr<ResultObject> &item = m_items[row];
        switch (role) {
        case ObjectRole:
            return QVariant::fromValue(item.get());
            break;
        default:
            return QVariant();
            break;
        }
    }
private:
     QHash<int, QByteArray> roleNames() const override
     {
        return {{ObjectRole, "object"}};
     }
};

class TestViewModelController final : public ViewModelController<TestModel>
{
public:
    explicit TestViewModelController(QObject *parent = nullptr)
        : ViewModelController<TestModel>(parent)
    {
    }
    TestModel & model() override
    {
        return m_model;
    }
private:
    TestModel m_model {};
};

class TstQtViewModel: public Test
{
protected:
    void SetUp()
    {
        m_controller.reset(new TestViewModelController());
        m_controller->classBegin();
        m_controller->componentComplete();
    }
    TestViewModel m_model {};
    std::unique_ptr<TestViewModelController> m_controller {};
    bool m_invalidated {false};
};

TEST_F(TstQtViewModel, Controller)
{
    QSignalSpy spy {&m_model, SIGNAL(controllerChanged())};
    m_model.setController(m_controller.get());

    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(m_model.controller(), m_controller.get());
}

TEST_F(TstQtViewModel, ControllerInvalid)
{
    QObjectPtr<QObject> object {new QObject()};
    QSignalSpy spy {&m_model, SIGNAL(controllerChanged())};
    m_model.setController(object.get());

    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(m_model.controller(), nullptr);
}

TEST_F(TstQtViewModel, Append)
{
    m_model.classBegin();
    m_model.componentComplete();
    m_model.setController(m_controller.get());
    m_controller->model().append({Result(1), Result(2), Result(3)});
    ResultObject *result1 = m_model.data(m_model.index(0), TestViewModel::ObjectRole).value<ResultObject *>();
    ResultObject *result2 = m_model.data(m_model.index(1), TestViewModel::ObjectRole).value<ResultObject *>();
    ResultObject *result3 = m_model.data(m_model.index(2), TestViewModel::ObjectRole).value<ResultObject *>();

    QSignalSpy spy1 {&m_model, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int))};
    QSignalSpy spy2 {&m_model, SIGNAL(rowsInserted(const QModelIndex &, int, int))};
    m_controller->model().append({Result(4), Result(5)});


    EXPECT_EQ(spy1.count(), 1);
    EXPECT_EQ(spy1[0][1], 3);
    EXPECT_EQ(spy1[0][2], 4);
    EXPECT_EQ(spy2.count(), 1);
    EXPECT_EQ(spy2[0][1], 3);
    EXPECT_EQ(spy2[0][2], 4);

    EXPECT_EQ(m_model.count(), 5);
    EXPECT_EQ(m_model.data(m_model.index(0), TestViewModel::ObjectRole).value<ResultObject *>(), result1);
    EXPECT_EQ(m_model.data(m_model.index(1), TestViewModel::ObjectRole).value<ResultObject *>(), result2);
    EXPECT_EQ(m_model.data(m_model.index(2), TestViewModel::ObjectRole).value<ResultObject *>(), result3);
    EXPECT_EQ(m_model.data(m_model.index(0), TestViewModel::ObjectRole).value<ResultObject *>()->value(), 1);
    EXPECT_EQ(m_model.data(m_model.index(1), TestViewModel::ObjectRole).value<ResultObject *>()->value(), 2);
    EXPECT_EQ(m_model.data(m_model.index(2), TestViewModel::ObjectRole).value<ResultObject *>()->value(), 3);
    EXPECT_EQ(m_model.data(m_model.index(3), TestViewModel::ObjectRole).value<ResultObject *>()->value(), 4);
    EXPECT_EQ(m_model.data(m_model.index(4), TestViewModel::ObjectRole).value<ResultObject *>()->value(), 5);
}

TEST_F(TstQtViewModel, Prepend)
{
    m_model.classBegin();
    m_model.componentComplete();
    m_model.setController(m_controller.get());
    m_controller->model().append({Result(1), Result(2), Result(3)});
    ResultObject *result1 = m_model.data(m_model.index(0), TestViewModel::ObjectRole).value<ResultObject *>();
    ResultObject *result2 = m_model.data(m_model.index(1), TestViewModel::ObjectRole).value<ResultObject *>();
    ResultObject *result3 = m_model.data(m_model.index(2), TestViewModel::ObjectRole).value<ResultObject *>();


    QSignalSpy spy1 {&m_model, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int))};
    QSignalSpy spy2 {&m_model, SIGNAL(rowsInserted(const QModelIndex &, int, int))};
    m_controller->model().prepend({Result(4), Result(5)});

    EXPECT_EQ(spy1.count(), 1);
    EXPECT_EQ(spy1[0][1], 0);
    EXPECT_EQ(spy1[0][2], 1);
    EXPECT_EQ(spy2.count(), 1);
    EXPECT_EQ(spy2[0][1], 0);
    EXPECT_EQ(spy2[0][2], 1);

    EXPECT_EQ(m_model.count(), 5);
    EXPECT_EQ(m_model.data(m_model.index(2), TestViewModel::ObjectRole).value<ResultObject *>(), result1);
    EXPECT_EQ(m_model.data(m_model.index(3), TestViewModel::ObjectRole).value<ResultObject *>(), result2);
    EXPECT_EQ(m_model.data(m_model.index(4), TestViewModel::ObjectRole).value<ResultObject *>(), result3);
    EXPECT_EQ(m_model.data(m_model.index(0), TestViewModel::ObjectRole).value<ResultObject *>()->value(), 4);
    EXPECT_EQ(m_model.data(m_model.index(1), TestViewModel::ObjectRole).value<ResultObject *>()->value(), 5);
    EXPECT_EQ(m_model.data(m_model.index(2), TestViewModel::ObjectRole).value<ResultObject *>()->value(), 1);
    EXPECT_EQ(m_model.data(m_model.index(3), TestViewModel::ObjectRole).value<ResultObject *>()->value(), 2);
    EXPECT_EQ(m_model.data(m_model.index(4), TestViewModel::ObjectRole).value<ResultObject *>()->value(), 3);
}

TEST_F(TstQtViewModel, Update)
{
    m_model.classBegin();
    m_model.componentComplete();
    m_model.setController(m_controller.get());
    m_controller->model().append({Result(1), Result(2), Result(3)});
    ResultObject *result1 = m_model.data(m_model.index(0), TestViewModel::ObjectRole).value<ResultObject *>();
    ResultObject *result2 = m_model.data(m_model.index(1), TestViewModel::ObjectRole).value<ResultObject *>();
    ResultObject *result3 = m_model.data(m_model.index(2), TestViewModel::ObjectRole).value<ResultObject *>();

    QSignalSpy spy {&m_model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &))};
    m_controller->model().update(1, Result(4));

    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy[0][0], m_model.index(1));
    EXPECT_EQ(spy[0][1], m_model.index(1));

    EXPECT_EQ(m_model.count(), 3);
    EXPECT_EQ(m_model.data(m_model.index(0), TestViewModel::ObjectRole).value<ResultObject *>(), result1);
    EXPECT_EQ(m_model.data(m_model.index(1), TestViewModel::ObjectRole).value<ResultObject *>(), result2);
    EXPECT_EQ(m_model.data(m_model.index(2), TestViewModel::ObjectRole).value<ResultObject *>(), result3);
    EXPECT_EQ(m_model.data(m_model.index(0), TestViewModel::ObjectRole).value<ResultObject *>()->value(), 1);
    EXPECT_EQ(m_model.data(m_model.index(1), TestViewModel::ObjectRole).value<ResultObject *>()->value(), 4);
    EXPECT_EQ(m_model.data(m_model.index(2), TestViewModel::ObjectRole).value<ResultObject *>()->value(), 3);
}

TEST_F(TstQtViewModel, Remove)
{
    m_model.classBegin();
    m_model.componentComplete();
    m_model.setController(m_controller.get());
    m_controller->model().append({Result(1), Result(2), Result(3)});
    ResultObject *result1 = m_model.data(m_model.index(0), TestViewModel::ObjectRole).value<ResultObject *>();
    ResultObject *result3 = m_model.data(m_model.index(2), TestViewModel::ObjectRole).value<ResultObject *>();

    QSignalSpy spy1 {&m_model, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int))};
    QSignalSpy spy2 {&m_model, SIGNAL(rowsRemoved(const QModelIndex &, int, int))};
    m_controller->model().remove(1);

    EXPECT_EQ(spy1.count(), 1);
    EXPECT_EQ(spy1[0][1], 1);
    EXPECT_EQ(spy1[0][2], 1);
    EXPECT_EQ(spy2.count(), 1);
    EXPECT_EQ(spy2[0][1], 1);
    EXPECT_EQ(spy2[0][2], 1);

    EXPECT_EQ(m_model.count(), 2);
    EXPECT_EQ(m_model.data(m_model.index(0), TestViewModel::ObjectRole).value<ResultObject *>(), result1);
    EXPECT_EQ(m_model.data(m_model.index(1), TestViewModel::ObjectRole).value<ResultObject *>(), result3);
    EXPECT_EQ(m_model.data(m_model.index(0), TestViewModel::ObjectRole).value<ResultObject *>()->value(), 1);
    EXPECT_EQ(m_model.data(m_model.index(1), TestViewModel::ObjectRole).value<ResultObject *>()->value(), 3);
}

TEST_F(TstQtViewModel, Move)
{
    m_model.classBegin();
    m_model.componentComplete();
    m_model.setController(m_controller.get());
    m_controller->model().append({Result(1), Result(2), Result(3)});
    ResultObject *result1 = m_model.data(m_model.index(0), TestViewModel::ObjectRole).value<ResultObject *>();
    ResultObject *result2 = m_model.data(m_model.index(1), TestViewModel::ObjectRole).value<ResultObject *>();
    ResultObject *result3 = m_model.data(m_model.index(2), TestViewModel::ObjectRole).value<ResultObject *>();

    QSignalSpy spy1 {&m_model, SIGNAL(rowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int))};
    QSignalSpy spy2 {&m_model, SIGNAL(rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int))};
    m_controller->model().move(0, 2);

    EXPECT_EQ(spy1.count(), 1);
    EXPECT_EQ(spy1[0][1], 0);
    EXPECT_EQ(spy1[0][2], 0);
    EXPECT_EQ(spy1[0][4], 2);
    EXPECT_EQ(spy2.count(), 1);
    EXPECT_EQ(spy2[0][1], 0);
    EXPECT_EQ(spy2[0][2], 0);
    EXPECT_EQ(spy2[0][4], 2);

    EXPECT_EQ(m_model.count(), 3);
    EXPECT_EQ(m_model.data(m_model.index(0), TestViewModel::ObjectRole).value<ResultObject *>(), result2);
    EXPECT_EQ(m_model.data(m_model.index(1), TestViewModel::ObjectRole).value<ResultObject *>(), result1);
    EXPECT_EQ(m_model.data(m_model.index(2), TestViewModel::ObjectRole).value<ResultObject *>(), result3);
    EXPECT_EQ(m_model.data(m_model.index(0), TestViewModel::ObjectRole).value<ResultObject *>()->value(), 2);
    EXPECT_EQ(m_model.data(m_model.index(1), TestViewModel::ObjectRole).value<ResultObject *>()->value(), 1);
    EXPECT_EQ(m_model.data(m_model.index(2), TestViewModel::ObjectRole).value<ResultObject *>()->value(), 3);
}

TEST_F(TstQtViewModel, Invalidation)
{
    m_model.setController(m_controller.get());
    EXPECT_EQ(m_model.controller(), m_controller.get());

    QSignalSpy spy {&m_model, SIGNAL(controllerChanged())};
    m_controller.reset();

    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(m_model.controller(), nullptr);
}

#include "tst_qtviewmodel.moc"
