#include <QDebug>
#include <QFileInfo>
#include <QElapsedTimer>
#include <QCoreApplication>
#include <QDir>
#include <QSettings>

#include "guimanager.h"
#include "deviceevent.h"

const int ONE_SEC_IN_MS = 1000;
const int MEGA_BYTES = 1024 * 1024;
const int CHUNK_SIZE = 4096;

GuiManager::GuiManager(QObject *parent) : QObject(parent),
    m_deviceIndex(-1),
    m_canCancel(false),
    m_canRead(false),
    m_canWrite(false),
    m_busy(false),
    m_verify(false),
    m_progress(0),
    m_version(VERSION_NUMBER)
{
    m_devices = new QQmlObjectListModel<DeviceItem>(this, "label", "deviceId");
    connect(this, &GuiManager::deviceIndexChanged, this, &GuiManager::onDeviceIndexChanged);
    connect(this, &GuiManager::imageFileUrlChanged, this, &GuiManager::onImageFileUrlChanged);
    connect(this, &GuiManager::imageFilePathChanged, this, &GuiManager::onImageFilePathChanged);

    removableDevices();
    loadSettings();

    update_message("Please choose create or restore image");
}

bool GuiManager::event(QEvent* ev)
{
    if (ev->type() == DeviceEvent::eventType)
    {
        DeviceEvent* devEv = static_cast<DeviceEvent*>(ev);
        if ((devEv->action() == DeviceEvent::AddDevice) || (devEv->action() == DeviceEvent::RemoveDevice))
        {
            removableDevices();
        }
        return true;
    }
    return QObject::event(ev);
}

void GuiManager::createImage()
{
    // Clear message
    update_message("");

    QString error;
    if (m_imageFilePath.isEmpty())
    {
        error = QString("Write Error;Please specify a image file to save data to.");
        setError(error);
        return;
    }

    DeviceItem* deviceItem = m_devices->at(m_deviceIndex);
    if (!deviceItem)
    {
        error = QString("Write Error;Please select a device.");
        setError(error);
        return;
    }

    // Check if image file is located on a volume on the selected device
    if (!checkFileLocation(deviceItem))
    {
        return;
    }

    setBusy(true);

    // Lock  all volumes
    if (!lockVolumes(deviceItem))
    {
        setBusy(false);
        update_message("Create disk image failed");
        return;
    }

    // Get the handle of the target raw disk
    m_rawDiskHandle = DiskUtilities::getHandleOnDevice(deviceItem->get_deviceId().toUInt(), GENERIC_READ, error);
    if (m_rawDiskHandle == INVALID_HANDLE_VALUE)
    {
        setError(error);
        setBusy(false);
        update_message("Create disk image failed");
        return;
    }

    quint64 sectorSize = 0;
    quint64 numSectors = DiskUtilities::getNumberOfSectors(m_rawDiskHandle, sectorSize, error);
    if (!error.isEmpty())
    {
        setError(error);
        setBusy(false);
        update_message("Create disk image failed");
        return;
    }

    // Read MBR partition table
    QByteArray sectorData = DiskUtilities::readSectorDataFromHandle(m_rawDiskHandle, 0, 1, 512, error);
    if (!error.isEmpty())
    {
        setError(error);
        setBusy(false);
        update_message("Create disk image failed");
        return;
    }
    numSectors = 1;

    // Read partition information
    for (quint64 i = 0; i < 4; i++)
    {
        quint32 partitionStartSector = *((quint32*)(sectorData.data() + 0x1BE + 8 + 16 * i));
        quint32 partitionNumSectors = *((quint32*)(sectorData.data() + 0x1BE + 12 + 16 * i));
        // Set numsectors to end of last partition
        if (partitionStartSector + partitionNumSectors > numSectors)
        {
            numSectors = partitionStartSector + partitionNumSectors;
        }
    }

    QFile imageFile(m_imageFilePath);
    if (!imageFile.open(QIODevice::WriteOnly))
    {
        error = QString("Write Error;Cannot open image file.");
        setError(error);
        setBusy(false);
        update_message("Create disk image failed");
        return;
    }

    QElapsedTimer elapsedTimer;
    elapsedTimer.start();
    quint64 lastI = 0;
    bool cancelled = false;
    QDataStream out;
    out.setDevice(&imageFile);

    // Store the total size of the disk (partitions) at the begining of the file
    quint64 totalSize = numSectors * sectorSize;
    out << totalSize;

    for (quint64 i = 0; i < numSectors; i += CHUNK_SIZE)
    {
        if (!m_busy)
        {
            // Cancelled
            cancelled = true;
            break;
        }

        // Read sectors from disk
        sectorData = DiskUtilities::readSectorDataFromHandle(m_rawDiskHandle, i, (numSectors - i >= CHUNK_SIZE) ? CHUNK_SIZE : (numSectors - i), sectorSize, error);
        if (!error.isEmpty())
        {
            setError(error);
            setBusy(false);
            update_message("Create disk image failed");
            return;
        }

        // Write sectors to file
        out << qCompress(sectorData, 9);

        if (elapsedTimer.elapsed() >= ONE_SEC_IN_MS)
        {
            // Calculate speed
            double mbPerSec = ((static_cast<double>(sectorSize) * (i - lastI)) * (static_cast<double>(ONE_SEC_IN_MS) / elapsedTimer.elapsed())) / static_cast<double>(MEGA_BYTES);
            update_message(QString("Reading speed %1 MB/s").arg(formatDouble(mbPerSec, 2)));

            // Calculate percentage
            update_progress(static_cast<double>(i) / static_cast<double>(numSectors));

            lastI = i;
            elapsedTimer.restart();
        }
        QCoreApplication::processEvents();
    }
    imageFile.close();

    // Verify file when needed
    if (m_verify)
    {
        // Data verification
        if (!verifyImage(deviceItem, sectorSize))
        {
            return;
        }
    }

    setBusy(false);
    if (cancelled)
    {
        update_message("Create disk image cancelled.");

        // TODO remove unfinished file
    }
    else
    {
        update_message("Create disk image succeeded.");
    }
}

void GuiManager::restoreImage()
{
    // Clear message
    update_message("");
    QString error;

    if (m_imageFilePath.isEmpty())
    {
        error = QString("Write Error;Please specify a image file to save data to.");
        setError(error);
        return;
    }

    DeviceItem* deviceItem = m_devices->at(m_deviceIndex);
    if (!deviceItem)
    {
        error = QString("Write Error;Please select a device.");
        setError(error);
        return;
    }

    // Check the image file
    QFileInfo fileInfo(m_imageFilePath);
    if (!fileInfo.exists() || !fileInfo.isFile())
    {
        error = QString("File Error;The selected file does not exist.");
        setError(error);
        return;
    }
    else if (!fileInfo.isReadable())
    {
        error = QString("File Error;You do not have permision to read the selected file.");
        setError(error);
        return;
    }
    else if (fileInfo.size() == 0)
    {
        error = QString("File Error;The specified file contains no data.");
        setError(error);
        return;
    }

    QFile imageFile(m_imageFilePath);
    if (!imageFile.open(QIODevice::ReadOnly))
    {
        error = QString("File Error;Cannot open specified image file.");
        setError(error);
        return;
    }

    // Check if image file is located on a volume on the selected device
    if (!checkFileLocation(deviceItem))
    {
        return;
    }

    setBusy(true);

    // Lock and unmount volumes on this device
    if (!lockAndUnmountVolumes(deviceItem))
    {
        setBusy(false);
        update_message("Restore disk image failed");
        return;
    }

    // Get the handle of the target raw disk
    m_rawDiskHandle = DiskUtilities::getHandleOnDevice(deviceItem->get_deviceId().toUInt(), GENERIC_WRITE, error);
    if (m_rawDiskHandle == INVALID_HANDLE_VALUE)
    {
        setError(error);
        setBusy(false);
        update_message("Restore disk image failed");
        return;
    }

    quint64 sectorSize = 0;
    DiskUtilities::getNumberOfSectors(m_rawDiskHandle, sectorSize, error);
    if (!error.isEmpty())
    {
        setError(error);
        setBusy(false);
        update_message("Restore disk image failed");
        return;
    }

    quint64 targetDiskSize = rawDiskSize(m_rawDiskHandle);
    if (0 == targetDiskSize)
    {
        //For external card readers you may not get device change notification when you remove the card/flash.
        //(So no WM_DEVICECHANGE signal). Device stays but size goes to 0. [Is there special event for this on Windows??]
        setBusy(false);
        update_message("Restore disk image failed");
        return;
    }

    QElapsedTimer elapsedTimer;
    elapsedTimer.start();
    quint64 lastI = 0;
    bool cancelled = false;
    QDataStream out;
    out.setDevice(&imageFile);
    quint64 imageDiskSize = 0;
    out >> imageDiskSize;

    // Check if image disk size is larger than target disk size
    if (imageDiskSize > targetDiskSize)
    {
        error = QString("Write Error;Content in selected image file is larger than the size of the selected device.");
        setError(error);
        setBusy(false);
        update_message("Restore disk image failed");
        return;
    }

    quint64 i = 0;
    while (!out.atEnd())
    {
        if (!m_busy)
        {
            // Cancelled
            cancelled = true;
            break;
        }

        // Read from file
        QByteArray data;
        out >> data;
        QByteArray uncompressed = qUncompress(data);
        quint64 wirteSectors = uncompressed.size() / sectorSize;
        if (!DiskUtilities::writeSectorDataToHandle(m_rawDiskHandle, uncompressed, i, wirteSectors, sectorSize, error))
        {
            setError(error);
            setBusy(false);
            update_message("Restore disk image failed");
            return;
        }

        if (elapsedTimer.elapsed() >= ONE_SEC_IN_MS)
        {
            // Calculate speed
            double mbPerSec = ((static_cast<double>(sectorSize) * (i - lastI)) * (static_cast<double>(ONE_SEC_IN_MS) / elapsedTimer.elapsed())) / static_cast<double>(MEGA_BYTES);
            update_message(QString("Writing speed %1 MB/s").arg(formatDouble(mbPerSec, 2)));

            // Calculate percentage
            update_progress(static_cast<double>(imageFile.pos()) / static_cast<double>(imageFile.size()));

            lastI = i;
            elapsedTimer.restart();
        }
        i += wirteSectors;
        QCoreApplication::processEvents();
    }
    imageFile.close();

    // Verify file when needed
    if (m_verify)
    {
        // Data verification
        if (!verifyImage(deviceItem, sectorSize))
        {
            return;
        }
    }

    setBusy(false);
    if (cancelled)
    {
        update_message("Restore disk image cancelled.");
    }
    else
    {
        update_message("Restore disk image succeeded.");
    }
}

bool GuiManager::verifyImage(const DeviceItem* deviceItem, const quint64 sectorSize)
{
    QString error;
    update_progress(0);
    update_message("");

    if (m_rawDiskHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_rawDiskHandle);
        m_rawDiskHandle = INVALID_HANDLE_VALUE;
    }

    // Get raw disk handle in read mode
    m_rawDiskHandle = DiskUtilities::getHandleOnDevice(deviceItem->get_deviceId().toUInt(), GENERIC_READ, error);
    if (m_rawDiskHandle == INVALID_HANDLE_VALUE)
    {
        setError(error);
        setBusy(false);
        update_message("Verify disk image failed");
        return false;
    }

    QFile imageFile(m_imageFilePath);
    if (!imageFile.open(QIODevice::ReadOnly))
    {
        error = QString("Read Error;Cannot open image file.");
        setError(error);
        setBusy(false);
        update_message("Verify disk image failed");
        return false;
    }

    QElapsedTimer elapsedTimer;
    elapsedTimer.start();
    quint64 lastI = 0;
    bool cancelled = false;
    QDataStream out;
    out.setDevice(&imageFile);

    quint64 totalSize = 0;
    out >> totalSize;

    quint64 i = 0;
    while (!out.atEnd())
    {
        if (!m_busy)
        {
            // Cancelled
            cancelled = true;
            break;
        }

        // Read from file
        QByteArray data;
        out >> data;
        QByteArray uncompressed = qUncompress(data);
        quint64 readSectors = uncompressed.size() / sectorSize;

        // Read sectors from disk
        QByteArray sectorData = DiskUtilities::readSectorDataFromHandle(m_rawDiskHandle, i, readSectors, sectorSize, error);
        if (!error.isEmpty())
        {
            setError(error);
            setBusy(false);
            update_message("Verify disk image failed");
            return false;
        }

        if (uncompressed != sectorData)
        {
            error = QString("Verify Error;Data from image file and disk is NOT identical.");
            setError(error);
            setBusy(false);
            update_message("Verify disk image failed");
            return false;
        }

        if (elapsedTimer.elapsed() >= ONE_SEC_IN_MS)
        {
            // Calculate speed
            double mbPerSec = ((static_cast<double>(sectorSize) * (i - lastI)) * (static_cast<double>(ONE_SEC_IN_MS) / elapsedTimer.elapsed())) / static_cast<double>(MEGA_BYTES);
            update_message(QString("Verifying speed %1 MB/s").arg(formatDouble(mbPerSec, 2)));

            // Calculate percentage
            update_progress(static_cast<double>(imageFile.pos()) / static_cast<double>(imageFile.size()));

            lastI = i;
            elapsedTimer.restart();
        }
        i += readSectors;
        QCoreApplication::processEvents();
    }
    imageFile.close();

    if (cancelled)
    {
        update_message("Verify disk image cancelled.");
        return false;
    }
    else
    {
        update_message("Verify disk image succeeded.");
        return true;
    }
}


void GuiManager::cancel()
{
    // Do not use setBusy here
    // The loop in create/restore function will determine the busy status
    setBusy(false);
}

void GuiManager::onDeviceIndexChanged(const int index)
{
    Q_UNUSED(index)
    enableReadWrite();
}

void GuiManager::onImageFileUrlChanged(const QUrl& path)
{
    set_imageFilePath(path.toLocalFile());
}

void GuiManager::onImageFilePathChanged(const QString &path)
{
    // Save the file dir as home dir
    QFileInfo fileInfo(path);
    if (fileInfo.exists())
    {
        QDir dir = fileInfo.absoluteDir();
        QString url = QUrl::fromLocalFile(dir.absolutePath()).toString();
        update_homeDir(url);
        saveSettings();
    }
    enableReadWrite();
}

void GuiManager::removableDevices()
{
    // GetLogicalDrives returns 0 on failure, or a bitmask representing
    // the drives available on the system (bit 0 = A:, bit 1 = B:, etc)
    ulong driveMask = GetLogicalDrives();
    ULONG pID;

    m_devices->clear();
    set_deviceIndex(-1);

    int i = 0;
    QString error;
    while (driveMask != 0)
    {
        if (driveMask & 1)
        {
            // the "A" in drivename will get incremented by the # of bits
            // we've shifted
            char driveName[] = "\\\\.\\A:\\";
            driveName[4] += i;
            QString msg;

            // Only deal with removable devices
            if (DiskUtilities::checkDriveType(driveName, &pID, msg))
            {
                QString driveLabel = QString("%1:\\").arg(driveName[4]);

                HANDLE volumeHandle = DiskUtilities::getHandleOnVolume(volumeId(driveLabel), GENERIC_READ, error);
                if (volumeHandle == INVALID_HANDLE_VALUE)
                {
                    setError(error);
                    return;
                }

                DWORD deviceId = DiskUtilities::getDeviceID(volumeHandle, error);
                setError(error);
                CloseHandle(volumeHandle);

                // Device ID does not exist yet
                QString devIdStr = QString().setNum(deviceId);
                if (!m_devices->getByUid(devIdStr))
                {
                    // Get the handle of the target raw disk
                    HANDLE rawDiskHandle = DiskUtilities::getHandleOnDevice(deviceId, GENERIC_WRITE, error);
                    if (rawDiskHandle == INVALID_HANDLE_VALUE)
                    {
                        setError(error);
                        return;
                    }
                    QString diskSize = formatDiskSize(rawDiskSize(rawDiskHandle));
                    QString devLabel = QString("RM %1 (%2)").arg(deviceId).arg(diskSize);
                    m_devices->append(new DeviceItem(devLabel, devIdStr));
                    CloseHandle(rawDiskHandle);
                }

                DeviceItem* devItem = m_devices->getByUid(devIdStr);
                if (devItem)
                {
                    devItem->appendDrive(driveLabel);
                }
            }
        }
        driveMask >>= 1;
        ++i;
    }

    if (m_devices->count() > 0)
    {
        set_deviceIndex(0);
    }
}

void GuiManager::setBusy(const bool busy)
{
    // Close all handle and set to invalid when it is not busy
    if (!busy)
    {
        unlockVolumes();
        if (m_rawDiskHandle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(m_rawDiskHandle);
            m_rawDiskHandle = INVALID_HANDLE_VALUE;
        }

        // Reset progress to 0
        update_progress(0);
    }

    update_busy(busy);
    if (m_busy)
    {
        update_canCancel(true);
        update_canRead(false);
        update_canWrite(false);
    }
    else
    {
        update_canCancel(false);
        enableReadWrite();
    }
}

void GuiManager::enableReadWrite()
{
    bool fileSelected = !(m_imageFilePath.isEmpty());
    bool deviceSelected = (m_devices->count() > 0) && (m_deviceIndex >= 0);
    QFileInfo fileInfo (m_imageFilePath);

    // set read and write buttons according to status of file/device
    update_canRead(deviceSelected && fileSelected && (fileInfo.exists() ? fileInfo.isWritable() : true));
    update_canWrite(deviceSelected && fileSelected && fileInfo.isReadable());
}

void GuiManager::setError(QString &error)
{
    if (!error.isEmpty())
    {
        QStringList list = error.split(';');
        if (list.size() == 2)
        {
            set_errorType(list.first());
            set_error(list.last());
            showErrorMsg();
        }
        error = QString("");
    }
}

int GuiManager::volumeId(const QString& driveLabel)
{
    return driveLabel.at(0).toLatin1() - 'A';
}

void GuiManager::saveSettings()
{
    QSettings settings;
    settings.setValue("Settings/HomeDir", m_homeDir);
}

void GuiManager::loadSettings()
{
    QSettings settings;
    update_homeDir(settings.value("Settings/HomeDir").toString());
}

bool GuiManager::checkFileLocation(const DeviceItem* deviceItem)
{
    if (deviceItem)
    {
        // Image file cannot be put to on target usb drive
        QString error;
        if (deviceItem->hasDrive(m_imageFilePath.at(0)))
        {
            error = QString("Write Error;Image file cannot be located on the target device.");
            setError(error);
            return false;
        }
        return true;
    }
    return false;
}

bool GuiManager::lockVolumes(const DeviceItem *deviceItem)
{
    if (deviceItem)
    {
        QString error;
        QStringList drvs = deviceItem->drives();
        foreach(QString drv, drvs)
        {
            HANDLE hVolume = DiskUtilities::getHandleOnVolume(volumeId(drv), GENERIC_READ, error);
            if (hVolume == INVALID_HANDLE_VALUE)
            {
                setError(error);
                return false;
            }
            // Get lock on the volume
            if (!DiskUtilities::getLockOnVolume(hVolume, error))
            {
                setError(error);
                return false;
            }

            m_lockedVolumes << hVolume;
        }
        return true;
    }
    return false;
}

bool GuiManager::lockAndUnmountVolumes(const DeviceItem* deviceItem)
{
    if (deviceItem)
    {
        QString error;
        QStringList drvs = deviceItem->drives();
        foreach(QString drv, drvs)
        {
            HANDLE hVolume = DiskUtilities::getHandleOnVolume(volumeId(drv), GENERIC_WRITE, error);
            if (hVolume == INVALID_HANDLE_VALUE)
            {
                setError(error);
                return false;
            }

            // To prevent access denied bug, lock is disabled
            // Get lock on the volume
//            if (!DiskUtilities::getLockOnVolume(hVolume, error))
//            {
//                setError(error);
//                return false;
//            }

//            m_lockedVolumes << hVolume;

            // Unmount volume
            if (!DiskUtilities::unmountVolume(hVolume, error))
            {
                setError(error);
                return false;
            }
        }
        return true;
    }
    return false;
}

bool GuiManager::unlockVolumes()
{
    QString error;
    foreach(HANDLE hVolume, m_lockedVolumes)
    {
        // Unlock all locked volumes
        if (!DiskUtilities::removeLockOnVolume(hVolume, error))
        {
            setError(error);
            return false;
        }
        CloseHandle(hVolume);
    }
    m_lockedVolumes.clear();
    return true;
}

quint64 GuiManager::rawDiskSize(const HANDLE handle)
{
    QString error;
    quint64 sectorSize = 0;
    quint64 availableSectors = DiskUtilities::getNumberOfSectors(handle, sectorSize, error);
    setError(error);
    return availableSectors * sectorSize;
}

QString GuiManager::formatDiskSize(const quint64 size)
{
    QString sizeStr;
    double sizeMb = static_cast<double>(size) / static_cast<double>(MEGA_BYTES);
    if (sizeMb < 1000.0)
    {
        return QString("%1 MB").arg(formatDouble(sizeMb, 2));
    }
    else
    {
        double sizeGb = sizeMb / 1024.0;
        return QString("%1 GB").arg(formatDouble(sizeGb, 2));
    }
}

QString GuiManager::formatDouble(const double value, const int precision)
{
    QString s;
    double p = pow(10.0, static_cast<double>(precision));
    if (!qIsNaN(value))
    {
        double rounded = floor(value * p + 0.5) / p;
        s = QString().setNum(rounded, 'f', precision);
    }
    return s;
}
