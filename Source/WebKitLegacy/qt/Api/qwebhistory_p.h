/*
    Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef QWEBHISTORY_P_H
#define QWEBHISTORY_P_H

#include "BackForwardList.h"
#include "qwebkitglobal.h"
#include <QtCore/qglobal.h>
#include <QtCore/qshareddata.h>
#include <WebCore/HistoryItem.h>
#include <wtf/RefPtr.h>

class QWebPageAdapter;

class QWEBKIT_EXPORT QWebHistoryItemPrivate : public QSharedData {
public:
    static QExplicitlySharedDataPointer<QWebHistoryItemPrivate> get(QWebHistoryItem* q)
    {
        return q->d;
    }
    QWebHistoryItemPrivate(WebCore::HistoryItem* i)
    {
        if (i)
            i->ref();
        item = i;
    }

    QWebHistoryItemPrivate(WebCore::HistoryItem& i)
    {
        i.ref();
        item = &i;
    }

    ~QWebHistoryItemPrivate()
    {
        if (item)
            item->deref();
    }

    static WebCore::HistoryItem* core(const QWebHistoryItem* q);

    WebCore::HistoryItem* item;
};

class QWebHistoryPrivate : public QSharedData {
public:
    QWebHistoryPrivate(BackForwardList* l)
    {
        l->ref();
        lst = l;
    }
    ~QWebHistoryPrivate()
    {
        lst->deref();
    }

    void goToItem(RefPtr<WebCore::HistoryItem>&&);

    QWebPageAdapter* page();

    BackForwardList* lst;
};


#endif
