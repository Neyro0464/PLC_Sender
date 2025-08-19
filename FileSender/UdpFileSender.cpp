#include "UdpFileSender.h"
#include <QFile>
#include <QDebug>

UdpFileSender::UdpFileSender(const QString& filePath, const QString& address, quint16 port, quint32 chunkSize)
    : filePath(filePath), port(port), chunkSize(chunkSize)
{
    if (!hostAddress.setAddress(address)) {
        qWarning() << "Invalid IP address:" << address;
    }
}

UdpFileSender::~UdpFileSender()
{
    udpSocket.close();
}

bool UdpFileSender::send()
{
    if (hostAddress.isNull()) {
        qWarning() << "Invalid destination address";
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open file:" << filePath;
        return false;
    }

    quint32 sequence = 0;
    bool success = true;

    while (!file.atEnd() && success) {
        QByteArray chunk = file.read(chunkSize);
        if (!sendChunk(chunk, sequence++)) {
            qWarning() << "Failed to send chunk" << sequence - 1;
            success = false;
        }
    }

    file.close();

    if (success) {
        QHostAddress localAddress = udpSocket.localAddress();
        quint16 localPort = udpSocket.localPort();

        qDebug() << "Пакет отправлен с адреса:" << localAddress.toString()
                 << "и порта:" << localPort;
    }

    return success;
}

bool UdpFileSender::sendChunk(const QByteArray& chunk, quint32 sequence)
{
    // Добавляем номер последовательности в начало чанка
    QByteArray packet;
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream << sequence << chunk;

    qint64 bytesSent = udpSocket.writeDatagram(packet, hostAddress, port);
    if (bytesSent == -1) {
        qWarning() << "Failed to send chunk:" << udpSocket.errorString();
        return false;
    }

    qDebug() << "Sent chunk" << sequence << "size" << bytesSent << "bytes";
    return true;
}

void UdpFileSender::setDestination(const QString& destination)
{
    QStringList parts = destination.split(':');
    if (parts.size() == 2) {
        if (hostAddress.setAddress(parts[0])) {
            bool ok;
            quint16 newPort = parts[1].toUShort(&ok);
            if (ok) {
                port = newPort;
                return;
            }
        }
    }
    qWarning() << "Invalid destination format. Expected 'address:port'";
}

void UdpFileSender::setChunkSize(quint32 size)
{
    if (size > 0) {
        chunkSize = size;
    } else {
        qWarning() << "Invalid chunk size, must be positive";
    }
}
