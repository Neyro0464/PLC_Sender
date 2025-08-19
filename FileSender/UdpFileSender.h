#ifndef UDPFILESENDER_H
#define UDPFILESENDER_H

#include "IFileSenderPLC.h"
#include <QUdpSocket>
#include <QHostAddress>
#include <QDataStream>

class UdpFileSender : public IFileSenderPLC
{
public:
    UdpFileSender(const QString& filePath, const QString& address, quint16 port, quint32 chunkSize = 32768);
    ~UdpFileSender() override;

    bool send() override;
    void setDestination(const QString& destination) override;
    void setChunkSize(quint32 size);

private:
    bool sendChunk(const QByteArray& chunk, quint32 sequence);

private:
    QString filePath;
    QHostAddress hostAddress;
    quint16 port;
    quint32 chunkSize;
    QUdpSocket udpSocket;
};

#endif // UDPFILESENDER_H
