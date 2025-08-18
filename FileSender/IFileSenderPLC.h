#ifndef IFILESENDERPLC_H
#define IFILESENDERPLC_H

#include <QString>

class IFileSenderPLC {
public:
    virtual ~IFileSenderPLC() = default;
    virtual bool send() = 0;
    virtual void setDestination(const QString&) = 0;

};

#endif // IFILESENDERPLC_H
