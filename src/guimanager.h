#ifndef GUIMANAGER_H
#define GUIMANAGER_H

#include <QObject>
#include <QEvent>

#include "qqmlhelpers.h"
#include "qqmlobjectlistmodel.h"
#include "deviceitem.h"
#include "diskutilities.h"

class GuiManager : public QObject
{
    Q_OBJECT
    QML_WRITABLE_PROPERTY(QUrl, imageFileUrl)
    QML_WRITABLE_PROPERTY(QString, imageFilePath)
    QML_WRITABLE_PROPERTY(int, deviceIndex)
    QML_WRITABLE_PROPERTY(QString, error)
    QML_WRITABLE_PROPERTY(QString, errorType)
    QML_READONLY_PROPERTY(QString, message)
    QML_READONLY_PROPERTY(QString, homeDir)
    QML_READONLY_PROPERTY(bool, canCancel)
    QML_READONLY_PROPERTY(bool, canRead)
    QML_READONLY_PROPERTY(bool, canWrite)
    QML_READONLY_PROPERTY(bool, busy)
    QML_WRITABLE_PROPERTY(bool, verify)
    QML_READONLY_PROPERTY(double, progress)
    QML_READONLY_PROPERTY(QString, version)
    QML_OBJMODEL_PROPERTY(DeviceItem, devices)

public:

    explicit GuiManager(QObject *parent = nullptr);

    bool event(QEvent* ev) override;

    Q_INVOKABLE void createImage();
    Q_INVOKABLE void restoreImage();
    Q_INVOKABLE void cancel();

signals:
    void showErrorMsg();

public slots:
    void onDeviceIndexChanged(const int index);
    void onImageFileUrlChanged(const QUrl& url);
    void onImageFilePathChanged(const QString& path);
private:
    void removableDevices();
    void setBusy(const bool busy);
    void enableReadWrite();
    void setError(QString &error);
    int  volumeId(const QString& driveLabel);
    void saveSettings();
    void loadSettings();
    bool checkFileLocation(const DeviceItem* deviceItem);
    bool lockVolumes(const DeviceItem* deviceItem);
    bool lockAndUnmountVolumes(const DeviceItem* deviceItem);
    bool unlockVolumes();
    quint64 rawDiskSize(const HANDLE handle);
    QString formatDiskSize(const quint64 size);
    QString formatDouble(const double value, const int precision);
    bool verifyImage(const DeviceItem *deviceItem, const quint64 sectorSize);

private:
    HANDLE        m_rawDiskHandle = {INVALID_HANDLE_VALUE};
    QList<HANDLE> m_lockedVolumes;
};

#endif // GUIMANAGER_H
