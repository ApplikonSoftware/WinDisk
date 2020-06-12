#ifndef QQMLSORTFILTERPROXYMODEL_H
#define QQMLSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class QQmlSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit QQmlSortFilterProxyModel(QObject* parent = 0);

    void setSourceModel(QAbstractItemModel* sourceModel);

signals:

public slots:
    QObject* get(int idx) const;
    void remove(int idx);
    int indexOf(const QString& uid) const;

    void addFilter(const QString& roleName, const QString& pattern);
    void addFilter(const QString& roleName, const QStringList& patterns);
    void clearFilter(const QString& roleName);
    void clearFilter();

private slots:
    void onSourceModelReset();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const;

private:
    QMultiMap<qint32, QString> m_rolePatterns;
};

#endif // QQMLSORTFILTERPROXYMODEL_H
