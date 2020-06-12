
#include <QDebug>
#include <QCoreApplication>

#include <windows.h>
#include <dbt.h>

#include "winnativeeventfilter.h"
#include "diskutilities.h"
#include "deviceevent.h"

WinNativeEventFilter::WinNativeEventFilter(QObject *receiver) :
    m_receiver(receiver)
{

}

bool WinNativeEventFilter::nativeEventFilter(const QByteArray& eventType, void *message, long *)
{
    if (m_receiver && (eventType == "windows_generic_MSG"))
    {
        MSG* msg = static_cast<MSG*>(message);
        if(msg->message == WM_DEVICECHANGE)
        {
            PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)msg->lParam;
            switch(msg->wParam)
            {
            case DBT_DEVICEARRIVAL:
                if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)
                {
                    PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
                    if(DBTF_NET)
                    {
                        char ALET = firstDriveFromMask(lpdbv->dbcv_unitmask);
                        DeviceEvent event(ALET, DeviceEvent::AddDevice);
                        QCoreApplication::sendEvent(m_receiver, &event);
                    }
                }
                break;
            case DBT_DEVICEREMOVECOMPLETE:
                if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)
                {
                    PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
                    if(DBTF_NET)
                    {
                        char ALET = firstDriveFromMask(lpdbv->dbcv_unitmask);
                        DeviceEvent event(ALET, DeviceEvent::RemoveDevice);
                        QCoreApplication::sendEvent(m_receiver, &event);
                    }
                }
                break;
            } // skip the rest
        }
    }
    return false;
}

char WinNativeEventFilter::firstDriveFromMask(ULONG unitmask)
{
    char i;

    for (i = 0; i < 26; ++i)
    {
        if (unitmask & 0x1)
        {
            break;
        }
        unitmask = unitmask >> 1;
    }

    return i + 'A';
}
