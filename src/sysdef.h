#ifndef SYSDEF_H
#define SYSDEF_H

#include <QObject>

class SysDef : public QObject
{
    Q_OBJECT

public:
    enum Status { STATUS_IDLE = 0, STATUS_READING, STATUS_WRITING, STATUS_VERIFYING, STATUS_EXIT, STATUS_CANCELED };
    Q_ENUM(Status)
};

#endif // SYSDEF_H
