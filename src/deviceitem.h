#ifndef DEVICEITEM_H
#define DEVICEITEM_H

#include <QStringList>

#include "qqmlhelpers.h"

class DeviceItem : public QObject
{
    Q_OBJECT
    QML_READONLY_PROPERTY(QString, label)
    QML_READONLY_PROPERTY(QString, deviceId)

public:
    explicit DeviceItem(QObject *parent = nullptr);
    explicit DeviceItem(const QString& label, const QString& deviceId, QObject *parent = nullptr);

    void appendDrive(const QString& drive);
    bool hasDrive(const QString& drive) const;
    QStringList drives() const;

signals:

public slots:

private:
    QStringList m_driveList;
};

#endif // DEVICEITEM_H
