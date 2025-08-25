#ifndef UDPFILESENDER_H
#define UDPFILESENDER_H

#include "IFileSenderPLC.h"
#include <QUdpSocket>
#include <QHostAddress>
#include <QDataStream>

class UdpFileSender : public IFileSenderPLC
{
public:
    UdpFileSender(const QString& filePath, const QString& address, quint16 port);
    ~UdpFileSender() override;

    bool send() override;
    void setDestination(const QString& destination) override;

private:
    bool sendFile(const QByteArray& fileData);

private:
    QString filePath;
    QHostAddress hostAddress;
    quint16 port;
    quint32 chunkSize;
    QUdpSocket udpSocket;
};

#endif // UDPFILESENDER_H
