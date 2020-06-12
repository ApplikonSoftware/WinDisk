#include "qqmlsortfilterproxymodel.h"
#include "qqmlobjectlistmodel.h"


QQmlSortFilterProxyModel::QQmlSortFilterProxyModel(QObject* parent) : QSortFilterProxyModel(parent)
{

}

void QQmlSortFilterProxyModel::setSourceModel(QAbstractItemModel* sourceModel)
{
    connect(sourceModel, &QAbstractItemModel::modelReset, this, &QQmlSortFilterProxyModel::onSourceModelReset);
    QSortFilterProxyModel::setSourceModel(sourceModel);
}

QObject* QQmlSortFilterProxyModel::get(int idx) const
{
    return qobject_cast<QQmlObjectListModelBase*>(sourceModel())->get(mapToSource(index(idx, 0)).row());
}

void QQmlSortFilterProxyModel::remove(int idx)
{
    qobject_cast<QQmlObjectListModelBase*>(sourceModel())->remove(mapToSource(index(idx, 0)).row());
}

int QQmlSortFilterProxyModel::indexOf(const QString& uid) const
{
    QQmlObjectListModelBase* model = qobject_cast<QQmlObjectListModelBase*>(sourceModel());
    return mapFromSource(model->index(model->indexOf(uid))).row();
}

void QQmlSortFilterProxyModel::addFilter(const QString& roleName, const QString& pattern)
{
    int roleNum = qobject_cast<QQmlObjectListModelBase*>(sourceModel())->roleForName(roleName.toUtf8());
    m_rolePatterns.remove(roleNum);
    m_rolePatterns.insertMulti(roleNum, pattern);

    invalidateFilter();

}

void QQmlSortFilterProxyModel::addFilter(const QString& roleName, const QStringList& patterns)
{
    int roleNum = qobject_cast<QQmlObjectListModelBase*>(sourceModel())->roleForName(roleName.toUtf8());
    m_rolePatterns.remove(roleNum);
    foreach (const QString& pattern, patterns)
    {
        m_rolePatterns.insertMulti(roleNum, pattern);
    }

    invalidateFilter();
}

void QQmlSortFilterProxyModel::clearFilter(const QString& roleName)
{
    int roleNum = qobject_cast<QQmlObjectListModelBase*>(sourceModel())->roleForName(roleName.toUtf8());
    m_rolePatterns.remove(roleNum);
}

void QQmlSortFilterProxyModel::clearFilter()
{
    m_rolePatterns.clear();
}

void QQmlSortFilterProxyModel::onSourceModelReset()
{
    qDebug() << "Source model has been reset";
}

bool QQmlSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    if (m_rolePatterns.isEmpty())
    {
        return true;
    }

    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
//    for (QMap<qint32, QString>::const_iterator iter = m_rolePatterns.constBegin(); iter != m_rolePatterns.constEnd(); ++iter)
//    {
//        QString roleData = index.data(iter.key()).toString();
//        QString filterRoleData = iter.value();
//        if (roleData != filterRoleData)
//        {
//            return false;
//        }
//    }
    foreach (qint32 key, m_rolePatterns.keys())
    {
        QString roleData = index.data(key).toString();
        QList<QString> values = m_rolePatterns.values();
        if (!values.contains(roleData))
        {
            return false;
        }
    }

    return true;
}
