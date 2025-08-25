#include "UdpFileSender.h"
#include <QFile>
#include <QDebug>

UdpFileSender::UdpFileSender(const QString& filePath, const QString& address, quint16 port)
    : filePath(filePath), port(port)
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

    QByteArray fileData = file.readAll();
    file.close();

    bool success = sendFile(fileData);


    return success;
}

bool UdpFileSender::sendFile(const QByteArray& fileData)
{
    // Отправляем файл как один датаграммный пакет
    qint64 bytesSent = udpSocket.writeDatagram(fileData, hostAddress, port);
    if (bytesSent == -1) {
        qWarning() << "[UdpFileSender]:Failed to send file:" << udpSocket.errorString();
        return false;
    }

    qDebug() << "[UdpFileSender]:File sent successfully, size:" << bytesSent << "bytes";
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
