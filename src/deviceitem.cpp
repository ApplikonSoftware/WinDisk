#include "deviceitem.h"

DeviceItem::DeviceItem(QObject *parent) : QObject(parent)
{
}

DeviceItem::DeviceItem(const QString& label, const QString& deviceId, QObject *parent) : QObject(parent),
    m_label(label),
    m_deviceId(deviceId)
{
}

void DeviceItem::appendDrive(const QString &drive)
{
    if (!m_driveList.contains(drive))
    {
        m_driveList.append(drive);
        update_label(QString("%1 [%2]").arg(m_label).arg(drive));
    }
}

bool DeviceItem::hasDrive(const QString &drive) const
{
    return m_driveList.contains(drive);
}

QStringList DeviceItem::drives() const
{
    return m_driveList;
}
