#ifndef QQMLOBJECTLISTMODELREGISTER_H
#define QQMLOBJECTLISTMODELREGISTER_H

#include <qqml.h>

#include "qqmlobjectlistmodel.h"

static void registerQtQmlObjectListModel()
{
    const char* uri = "QtQmlObjectListModel";
    const int    maj = 1;
    const int    min = 0;

    qmlRegisterUncreatableType<QAbstractItemModel> (uri, maj, min, "AbstractItemModel", "!!!");
    qmlRegisterUncreatableType<QAbstractListModel> (uri, maj, min, "AbstractListModel", "!!!");
    qmlRegisterUncreatableType<QQmlObjectListModelBase> (uri, maj, min, "ObjectListModelBase",  "!!!");
}

#endif // QQMLOBJECTLISTMODELREGISTER_H
