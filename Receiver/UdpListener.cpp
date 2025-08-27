#include "UdpListener.h"
#include <QDebug>

UdpListener::UdpListener(quint16 port, QObject *parent)
    : QObject(parent), listeningPort(port)
{
    socket = new QUdpSocket(this);
}

UdpListener::~UdpListener() {
    if (socket) {
        socket->close();
        delete socket;
    }
}

bool UdpListener::startListening() {
    if (!socket->bind(QHostAddress::Any, listeningPort)) {
        qCritical() << "[UdpListener] [startListening]: Failed to bind to port" << listeningPort;
        return false;
    }

    connect(socket, &QUdpSocket::readyRead, this, &UdpListener::processPendingDatagrams);
    qInfo() << "[UdpListener] [startListening]: UDP Listener started on port" << listeningPort;
    return true;
}

void UdpListener::processPendingDatagrams() {
    while (socket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(socket->pendingDatagramSize());

        QHostAddress sender;
        quint16 senderPort;

        socket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        qDebug() << "\n-------------------------------\n" << "Received UDP packet:\n" << datagram << "\n-------------------------------";
        emit dataReceived(datagram, sender, senderPort);
    }
}

