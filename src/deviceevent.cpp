#include "deviceevent.h"

DeviceEvent::DeviceEvent(char devLabel, Action action) : QEvent(DeviceEvent::eventType),
    m_devLabel(devLabel),
    m_action(action)
{
}

char DeviceEvent::devLabel() const
{
    return m_devLabel;
}

DeviceEvent::Action DeviceEvent::action() const
{
    return m_action;
}

