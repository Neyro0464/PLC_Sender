#ifndef SNMPSENDER_H
#define SNMPSENDER_H

#include <QString>
#include <QTcpSocket>

#include "ISenderPLC.h"


class SnmpSender: public ISenderPLC
{
public:
    SnmpSender();
    void send() override {return;};
    void setDestination() override {return;};
    virtual ~SnmpSender() = default;

private:
    QTcpSocket m_socket;
    QString m_dest_ip;
    quint16 m_dest_port;

};

#endif // SNMPSENDER_H
