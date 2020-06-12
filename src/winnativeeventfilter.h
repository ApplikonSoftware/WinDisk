#ifndef WINNATIVEEVENTFILTER_H
#define WINNATIVEEVENTFILTER_H

#include <QAbstractNativeEventFilter>
#include <QByteArray>
#include <QObject>

#include <windows.h>

class WinNativeEventFilter : public QAbstractNativeEventFilter
{
public:
    WinNativeEventFilter(QObject* receiver);
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *) override;

private:
    char firstDriveFromMask (ULONG unitmask);

private:
    QObject* m_receiver = {nullptr};
};

#endif // WINNATIVEEVENTFILTER_H
