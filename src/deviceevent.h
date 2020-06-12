#ifndef DEVICEEVENT_H
#define DEVICEEVENT_H

#include <QEvent>

class DeviceEvent : public QEvent
{
public:
    static const QEvent::Type eventType = static_cast<QEvent::Type>(1001);

    enum Action
    {
        InvalidAction = -1,
        AddDevice = 0,
        RemoveDevice = 1
    };

    DeviceEvent(char devLabel, Action action);

    char devLabel() const;
    DeviceEvent::Action action() const;

private:
    char m_devLabel = {'A'};
    Action m_action = {InvalidAction};
};
#endif // DEVICEEVENT_H
