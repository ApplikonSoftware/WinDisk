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

#ifndef DISKUTILITIES_H
#define DISKUTILITIES_H

#ifndef WINVER
#define WINVER 0x0601
#endif

#include <QString>

#include <cstdio>
#include <cstdlib>
#include <windows.h>
#include <winioctl.h>
#ifndef FSCTL_IS_VOLUME_MOUNTED
#define FSCTL_IS_VOLUME_MOUNTED  CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 10, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif // FSCTL_IS_VOLUME_MOUNTED

typedef struct _DEVICE_NUMBER
{
    DEVICE_TYPE  DeviceType;
    ULONG  DeviceNumber;
    ULONG  PartitionNumber;
} DEVICE_NUMBER, *PDEVICE_NUMBER;

// IOCTL control code
#define IOCTL_STORAGE_QUERY_PROPERTY   CTL_CODE(IOCTL_STORAGE_BASE, 0x0500, METHOD_BUFFERED, FILE_ANY_ACCESS)

class DiskUtilities
{
public:
    DiskUtilities();

    static HANDLE     getHandleOnFile(LPCWSTR filelocation, DWORD access, QString& msg);
    static HANDLE     getHandleOnDevice(int device, DWORD access, QString& msg);
    static HANDLE     getHandleOnVolume(int volume, DWORD access, QString& msg);
    static QString    getDriveLabel(const char *drv);
    static DWORD      getDeviceID(HANDLE hVolume, QString& msg);
    static bool       getLockOnVolume(HANDLE handle, QString &msg);
    static bool       removeLockOnVolume(HANDLE handle, QString &msg);
    static bool       unmountVolume(HANDLE handle, QString &msg);
    static bool       isVolumeUnmounted(HANDLE handle);
    static QByteArray readSectorDataFromHandle(HANDLE handle, quint64 startsector, const quint64 numSectors, const quint64 sectorSize, QString& msg);
    static bool       writeSectorDataToHandle(HANDLE handle, const QByteArray& data, const quint64 startSector, const quint64 numSectors, const quint64 sectorSize, QString &msg);
    static quint64    getNumberOfSectors(HANDLE handle, quint64& sectorsize, QString &msg);
    static quint64    getFileSizeInSectors(HANDLE handle, const quint64 sectorsize, QString& msg);
    static bool       spaceAvailable(char *location, quint64 spaceneeded, QString &msg);
    static bool       checkDriveType(char *name, ULONG *pid, QString &msg);

private:
    static BOOL getDisksProperty(HANDLE hDevice, PSTORAGE_DEVICE_DESCRIPTOR pDevDesc, DEVICE_NUMBER *devInfo, QString &msg);
    static bool getMediaType(HANDLE hDevice);
    static bool slashify(char *str, char **slash, char **noSlash);
};


#endif // DISKUTILITIES_H
