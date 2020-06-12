/**********************************************************************
 *  This program is free software; you can redistribute it and/or     *
 *  modify it under the terms of the GNU General Public License       *
 *  as published by the Free Software Foundation; either version 2    *
 *  of the License, or (at your option) any later version.            *
 *                                                                    *
 *  This program is distributed in the hope that it will be useful,   *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of    *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the     *
 *  GNU General Public License for more details.                      *
 *                                                                    *
 *  You should have received a copy of the GNU General Public License *
 *  along with this program; if not, see http://gnu.org/licenses/     *
 *  ---                                                               *
 *  Copyright (C) 2009, Justin Davis <tuxdavis@gmail.com>             *
 *  Copyright (C) 2009-2017 ImageWriter developers                    *
 *                 https://sourceforge.net/projects/win32diskimager/  *
 **********************************************************************/

#ifndef WINVER
#define WINVER 0x0601
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <windows.h>
#include <winioctl.h>

#include "diskutilities.h"

DiskUtilities::DiskUtilities()
{
}

HANDLE DiskUtilities::getHandleOnFile(LPCWSTR filelocation, DWORD access, QString &msg)
{
    HANDLE hFile;
    hFile = CreateFileW(filelocation, access, (access == GENERIC_READ) ? FILE_SHARE_READ : 0, nullptr, (access == GENERIC_READ) ? OPEN_EXISTING:CREATE_ALWAYS, 0, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        wchar_t *errormessage = nullptr;
        ::FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, GetLastError(), 0, (LPWSTR)&errormessage, 0, nullptr);
        QString errText = QString::fromUtf16((const ushort *)errormessage);
        msg = QString("File Error;An error occurred when attempting to get a handle on the file.\n Error %1: %2").arg(GetLastError()).arg(errText);
        LocalFree(errormessage);
    }
    return hFile;
}

HANDLE DiskUtilities::getHandleOnDevice(int device, DWORD access, QString &msg)
{
    HANDLE hDevice;
    QString devicename = QString("\\\\.\\PhysicalDrive%1").arg(device);
    hDevice = CreateFile(devicename.toLatin1().data(), access, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hDevice == INVALID_HANDLE_VALUE)
    {
        wchar_t *errormessage = nullptr;
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, GetLastError(), 0, (LPWSTR)&errormessage, 0, nullptr);
        QString errText = QString::fromUtf16((const ushort *)errormessage);
        msg = QString("Device Error;An error occurred when attempting to get a handle on the device.\n Error %1: %2").arg(GetLastError()).arg(errText);
        LocalFree(errormessage);
    }
    return hDevice;
}

HANDLE DiskUtilities::getHandleOnVolume(int volume, DWORD access, QString &msg)
{
    HANDLE hVolume;
    char volumename[] = "\\\\.\\A:";
    volumename[4] += volume;
    hVolume = CreateFile(volumename, access, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hVolume == INVALID_HANDLE_VALUE)
    {
        wchar_t *errormessage=nullptr;
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, GetLastError(), 0, (LPWSTR)&errormessage, 0, nullptr);
        QString errText = QString::fromUtf16((const ushort *)errormessage);
        msg = QString("Volume Error;An error occurred when attempting to get a handle on the volume.\n Error %1: %2").arg(GetLastError()).arg(errText);
        LocalFree(errormessage);
    }
    return hVolume;
}

QString DiskUtilities::getDriveLabel(const char *drv)
{
    // given a drive letter (ending in a slash), return the label for that drive
    // TODO make this more robust by adding input verification
    QString retVal;
    int szNameBuf = MAX_PATH + 1;
    char *nameBuf = nullptr;
    if( (nameBuf = (char *)calloc(szNameBuf, sizeof(char))) != 0 )
    {
        ::GetVolumeInformationA(drv, nameBuf, szNameBuf, nullptr, nullptr, nullptr, nullptr, 0);
    }

    // if malloc fails, nameBuf will be nullptr.
    // if GetVolumeInfo fails, nameBuf will contain empty string
    // if all succeeds, nameBuf will contain label
    if(nameBuf == nullptr)
    {
        retVal = QString("");
    }
    else
    {
        retVal = QString(nameBuf);
        free(nameBuf);
    }

    return retVal;
}

DWORD DiskUtilities::getDeviceID(HANDLE hVolume, QString &msg)
{
    VOLUME_DISK_EXTENTS sd;
    DWORD bytesreturned;
    if (!DeviceIoControl(hVolume, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, nullptr, 0, &sd, sizeof(sd), &bytesreturned, nullptr))
    {
        wchar_t *errormessage=nullptr;
        ::FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, GetLastError(), 0, (LPWSTR)&errormessage, 0, nullptr);
        QString errText = QString::fromUtf16((const ushort *)errormessage);
        msg = QString("Volume Error;An error occurred when attempting to get information on volume.\n Error %1: %2").arg(GetLastError()).arg(errText);
        LocalFree(errormessage);
    }
    return sd.Extents[0].DiskNumber;
}

bool DiskUtilities::getLockOnVolume(HANDLE handle, QString& msg)
{
    DWORD bytesreturned;
    BOOL bResult;
    bResult = DeviceIoControl(handle, FSCTL_LOCK_VOLUME, nullptr, 0, nullptr, 0, &bytesreturned, nullptr);
    if (!bResult)
    {
        wchar_t *errormessage = nullptr;
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, GetLastError(), 0, (LPWSTR)&errormessage, 0, nullptr);
        QString errText = QString::fromUtf16((const ushort *)errormessage);
        msg = QString("Lock Error;An error occurred when attempting to lock the volume.\n Error %1: %2").arg(GetLastError()).arg(errText);
        LocalFree(errormessage);
    }
    return bResult;
}

bool DiskUtilities::removeLockOnVolume(HANDLE handle, QString& msg)
{
    DWORD junk;
    BOOL bResult;
    bResult = DeviceIoControl(handle, FSCTL_UNLOCK_VOLUME, nullptr, 0, nullptr, 0, &junk, nullptr);
    if (!bResult)
    {
        wchar_t *errormessage=nullptr;
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, GetLastError(), 0, (LPWSTR)&errormessage, 0, nullptr);
        QString errText = QString::fromUtf16((const ushort *)errormessage);
        msg = QString("Unlock Error;An error occurred when attempting to unlock the volume.\n Error %1: %2").arg(GetLastError()).arg(errText);
        LocalFree(errormessage);
    }
    return bResult;
}

bool DiskUtilities::unmountVolume(HANDLE handle, QString& msg)
{
    DWORD junk;
    BOOL bResult;
    bResult = DeviceIoControl(handle, FSCTL_DISMOUNT_VOLUME, nullptr, 0, nullptr, 0, &junk, nullptr);
    if (!bResult)
    {
        wchar_t *errormessage=nullptr;
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, GetLastError(), 0, (LPWSTR)&errormessage, 0, nullptr);
        QString errText = QString::fromUtf16((const ushort *)errormessage);
        msg = QString("Unmount Error;An error occurred when attempting to unmount the volume.\n Error %1: %2").arg(GetLastError()).arg(errText);
        LocalFree(errormessage);
    }
    return bResult;
}

bool DiskUtilities::isVolumeUnmounted(HANDLE handle)
{
    DWORD junk;
    BOOL bResult;
    bResult = DeviceIoControl(handle, FSCTL_IS_VOLUME_MOUNTED, nullptr, 0, nullptr, 0, &junk, nullptr);
    return !bResult;
}

QByteArray DiskUtilities::readSectorDataFromHandle(HANDLE handle, const quint64 startSector, const quint64 numSectors, const quint64 sectorSize, QString& msg)
{
    unsigned long bytesRead;
    QByteArray dataArray;
    dataArray.resize(sectorSize * numSectors);
    LARGE_INTEGER li;
    li.QuadPart = startSector * sectorSize;
    SetFilePointer(handle, li.LowPart, &li.HighPart, FILE_BEGIN);
    if (!ReadFile(handle, dataArray.data(), sectorSize * numSectors, &bytesRead, nullptr))
    {
        wchar_t *errormessage = nullptr;
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, GetLastError(), 0, (LPWSTR)&errormessage, 0, nullptr);
        QString errText = QString::fromUtf16((const ushort *)errormessage);
        msg = QString("Read Error;An error occurred when attempting to read data from handle.\n Error %1: %2").arg(GetLastError()).arg(errText);
        LocalFree(errormessage);
    }

    if (!dataArray.isNull() && bytesRead < (sectorSize * numSectors))
    {
        memset(dataArray.data() + bytesRead, 0, (sectorSize * numSectors) - bytesRead);
    }
    return dataArray;
}

bool DiskUtilities::writeSectorDataToHandle(HANDLE handle, const QByteArray& data, const quint64 startSector, const quint64 numSectors, const quint64 sectorSize, QString& msg)
{
    unsigned long bytesWritten;
    BOOL bResult;
    LARGE_INTEGER li;
    li.QuadPart = startSector * sectorSize;
    SetFilePointer(handle, li.LowPart, &li.HighPart, FILE_BEGIN);
    bResult = WriteFile(handle, data.data(), sectorSize * numSectors, &bytesWritten, nullptr);
    if (!bResult)
    {
        wchar_t *errormessage = nullptr;
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, GetLastError(), 0, (LPWSTR)&errormessage, 0, nullptr);
        QString errText = QString::fromUtf16((const ushort *)errormessage);
        msg = QString("Write Error;An error occurred when attempting to write data to handle.\n Error %1: %2").arg(GetLastError()).arg(errText);
        LocalFree(errormessage);
    }
    return bResult;
}

quint64 DiskUtilities::getNumberOfSectors(HANDLE handle, quint64& sectorsize, QString& msg)
{
    DWORD junk;
    DISK_GEOMETRY_EX diskgeometry;
    BOOL bResult;
    bResult = DeviceIoControl(handle, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, nullptr, 0, &diskgeometry, sizeof(diskgeometry), &junk, nullptr);
    if (!bResult)
    {
        wchar_t *errormessage=nullptr;
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, GetLastError(), 0, (LPWSTR)&errormessage, 0, nullptr);
        QString errText = QString::fromUtf16((const ushort *)errormessage);
        msg = QString("Device Error;An error occurred when attempting to get the device's geometry.\n Error %1: %2").arg(GetLastError()).arg(errText);
        LocalFree(errormessage);
        return 0;
    }

    sectorsize = (quint64)diskgeometry.Geometry.BytesPerSector;

    return (quint64)diskgeometry.DiskSize.QuadPart / (quint64)diskgeometry.Geometry.BytesPerSector;
}

quint64 DiskUtilities::getFileSizeInSectors(HANDLE handle, const quint64 sectorsize, QString& msg)
{
    quint64 retVal = 0;

    // avoid divide by 0
    if (sectorsize)
    {
        LARGE_INTEGER filesSize;
        if(GetFileSizeEx(handle, &filesSize) == 0)
        {
            // error
            wchar_t *errormessage = nullptr;
            FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, GetLastError(), 0, (LPWSTR)&errormessage, 0, nullptr);
            QString errText = QString::fromUtf16((const ushort *)errormessage);
            msg = QString("File Error;An error occurred while getting the file size.\n Error %1: %2").arg(GetLastError()).arg(errText);
            LocalFree(errormessage);
            retVal = 0;
        }
        else
        {
            retVal = ((quint64)filesSize.QuadPart / sectorsize ) + (((quint64)filesSize.QuadPart % sectorsize ) ? 1 : 0);
        }
    }
    return retVal;
}

bool DiskUtilities::spaceAvailable(char* location, quint64 spaceneeded, QString& msg)
{
    ULARGE_INTEGER freespace;
    BOOL bResult;
    bResult = GetDiskFreeSpaceEx(location, nullptr, nullptr, &freespace);
    if (!bResult)
    {
        wchar_t *errormessage = nullptr;
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, GetLastError(), 0, (LPWSTR)&errormessage, 0, nullptr);
        QString errText = QString::fromUtf16((const ushort *)errormessage);
        msg = QString("Free Space Error;Failed to get the free space on drive %1.\n Error %2: %3\n Checking of free space will be skipped.").arg(location).arg(GetLastError()).arg(errText);
        return true;
    }
    return (spaceneeded <= freespace.QuadPart);
}

bool DiskUtilities::checkDriveType(char* name, ULONG* pid, QString& msg)
{
    HANDLE hDevice;
    PSTORAGE_DEVICE_DESCRIPTOR pDevDesc;
    DEVICE_NUMBER deviceInfo;
    bool retVal = false;
    char *nameWithSlash;
    char *nameNoSlash;
    int driveType;
    DWORD cbBytesReturned;

    // some calls require no tailing slash, some require a trailing slash...
    if (!slashify(name, &nameWithSlash, &nameNoSlash))
    {
        return retVal;
    }

    driveType = GetDriveType(nameWithSlash);
    switch( driveType )
    {
    case DRIVE_REMOVABLE: // The media can be removed from the drive.
    case DRIVE_FIXED:     // The media cannot be removed from the drive. Some USB drives report as this.
        hDevice = CreateFile(nameNoSlash, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
        if (hDevice == INVALID_HANDLE_VALUE)
        {
            wchar_t *errormessage=nullptr;
            FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, GetLastError(), 0, (LPWSTR)&errormessage, 0, nullptr);
            QString errText = QString::fromUtf16((const ushort *)errormessage);
            msg = QString("Volume Error;An error occurred when attempting to get a handle on %3.\n Error %1: %2").arg(GetLastError()).arg(errText).arg(nameWithSlash);
            LocalFree(errormessage);
        }
        else
        {
            int arrSz = sizeof(STORAGE_DEVICE_DESCRIPTOR) + 512 - 1;
            pDevDesc = (PSTORAGE_DEVICE_DESCRIPTOR)new BYTE[arrSz];
            pDevDesc->Size = arrSz;

            // get the device number if the drive is
            // removable or (fixed AND on the usb bus, SD, or MMC (undefined in XP/mingw))
            if(getMediaType(hDevice) && getDisksProperty(hDevice, pDevDesc, &deviceInfo, msg) &&
                    ( ((driveType == DRIVE_REMOVABLE) && (pDevDesc->BusType != BusTypeSata))
                      || ( (driveType == DRIVE_FIXED) && ((pDevDesc->BusType == BusTypeUsb)
                      || (pDevDesc->BusType == BusTypeSd ) || (pDevDesc->BusType == BusTypeMmc )) ) ) )
            {
                // ensure that the drive is actually accessible
                // multi-card hubs were reporting "removable" even when empty
                if(DeviceIoControl(hDevice, IOCTL_STORAGE_CHECK_VERIFY2, nullptr, 0, nullptr, 0, &cbBytesReturned, (LPOVERLAPPED) nullptr))
                {
                    *pid = deviceInfo.DeviceNumber;
                    retVal = true;
                }
                else
                {
                    // IOCTL_STORAGE_CHECK_VERIFY2 fails on some devices under XP/Vista, try the other (slower) method, just in case.
                    CloseHandle(hDevice);
                    hDevice = CreateFile(nameNoSlash, FILE_READ_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
                    if(DeviceIoControl(hDevice, IOCTL_STORAGE_CHECK_VERIFY, nullptr, 0, nullptr, 0, &cbBytesReturned, (LPOVERLAPPED) nullptr))
                    {
                        *pid = deviceInfo.DeviceNumber;
                        retVal = true;
                    }
                }
            }

            delete[] pDevDesc;
            CloseHandle(hDevice);
        }

        break;
    default:
        retVal = false;
    }

    // free the strings allocated by slashify
    free(nameWithSlash);
    free(nameNoSlash);

    return retVal;
}

BOOL DiskUtilities::getDisksProperty(HANDLE hDevice, PSTORAGE_DEVICE_DESCRIPTOR pDevDesc, DEVICE_NUMBER *devInfo, QString& msg)
{
    STORAGE_PROPERTY_QUERY Query; // input param for query
    DWORD dwOutBytes; // IOCTL output length
    BOOL bResult; // IOCTL return val
    BOOL retVal = true;
    DWORD cbBytesReturned;

    // specify the query type
    Query.PropertyId = StorageDeviceProperty;
    Query.QueryType = PropertyStandardQuery;

    // Query using IOCTL_STORAGE_QUERY_PROPERTY
    bResult = ::DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
                &Query, sizeof(STORAGE_PROPERTY_QUERY), pDevDesc,
                pDevDesc->Size, &dwOutBytes, (LPOVERLAPPED)nullptr);
    if (bResult)
    {
        bResult = ::DeviceIoControl(hDevice, IOCTL_STORAGE_GET_DEVICE_NUMBER,
                    nullptr, 0, devInfo, sizeof(DEVICE_NUMBER), &dwOutBytes,
                    (LPOVERLAPPED)nullptr);
        if (!bResult)
        {
            retVal = false;
            wchar_t *errormessage=nullptr;
            FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, GetLastError(), 0, (LPWSTR)&errormessage, 0, nullptr);
            QString errText = QString::fromUtf16((const ushort *)errormessage);
            msg = QString("File Error;An error occurred while getting the device number.\n This usually means something is currently accessing the device; please close all applications and try again.\n Error %1: %2").arg(GetLastError()).arg(errText);
            LocalFree(errormessage);
        }
    }
    else
    {
        if (DeviceIoControl(hDevice, IOCTL_STORAGE_CHECK_VERIFY2, nullptr, 0, nullptr, 0, &cbBytesReturned,
                            (LPOVERLAPPED) nullptr))
        {
            wchar_t *errormessage=nullptr;
            FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, GetLastError(), 0, (LPWSTR)&errormessage, 0, nullptr);
            QString errText = QString::fromUtf16((const ushort *)errormessage);
            msg = QString("File Error;An error occurred while querying the properties.\n This usually means something is currently accessing the device; Please close all applications and try again.\n Error %1: %2").arg(GetLastError()).arg(errText);
            LocalFree(errormessage);
        }
            retVal = false;
    }

    return(retVal);
}

bool DiskUtilities::getMediaType(HANDLE hDevice)
{
    DISK_GEOMETRY diskGeo;
    DWORD cbBytesReturned;
    if (DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY, nullptr, 0, &diskGeo, sizeof(diskGeo), &cbBytesReturned, nullptr))
    {
        if ((diskGeo.MediaType == FixedMedia) || (diskGeo.MediaType == RemovableMedia))
        {
            return true; // Not a floppy
        }
    }
    return false;
}

bool DiskUtilities::slashify(char *str, char **slash, char **noSlash)
{
    // some routines fail if there's no trailing slash in a name,
    // 		others fail if there is.  So this routine takes a name (trailing
    // 		slash or no), and creates 2 versions - one with the slash, and one w/o
    //
    // 		CALLER MUST FREE THE 2 RETURNED STRINGS
    bool retVal = false;
    int strLen = strlen(str);
    if ( strLen > 0 )
    {
        if ( *(str + strLen - 1) == '\\' )
        {
            // trailing slash exists
            if (( (*slash = (char *)calloc( (strLen + 1), sizeof(char))) != nullptr) &&
                    ( (*noSlash = (char *)calloc(strLen, sizeof(char))) != nullptr))
            {
                strncpy(*slash, str, strLen);
                strncpy(*noSlash, *slash, (strLen - 1));
                retVal = true;
            }
        }
        else
        {
            // no trailing slash exists
            if ( ((*slash = (char *)calloc( (strLen + 2), sizeof(char))) != nullptr) &&
                 ((*noSlash = (char *)calloc( (strLen + 1), sizeof(char))) != nullptr) )
            {
                strncpy(*noSlash, str, strLen);
                sprintf(*slash, "%s\\", *noSlash);
                retVal = true;
            }
        }
    }
    return retVal;
}
