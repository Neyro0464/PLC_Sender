#include "UdpSender.h"
#include <QDebug>
#include <QByteArray>
#include <cstring> // для memcpy

// Модифицируем конструктор
UdpSender::UdpSender(std::vector<NORAD_SCHEDULE>& data,
                     const QHostAddress& targetAddress,
                     quint16 targetPort)
    : m_targetAddress(targetAddress), m_targetPort(targetPort)
{
    m_udpSocket = new QUdpSocket(this);

    if(data.empty()) {
        qCritical() << "[UdpSender][Constructor]: No data to copy";
        return;
    }

    try {
        m_result.resize(data.size());

        for (size_t i = 0; i < data.size(); i++) {
            // Время всегда первое в каждой структуре
            uint32_t timestamp = static_cast<uint32_t>(data[i].onDate.Ticks());

            // Point (сохраняем REAL как есть)
            m_result[i].point.Ti = timestamp;
            m_result[i].point.Az = data[i].azm;
            m_result[i].point.El = data[i].elv;

            // Atribute
            m_result[i].atr.Ti = timestamp;
            m_result[i].atr.d1 = i;
            m_result[i].atr.d2 = data.size();

            // Sum (для расчёта контрольной суммы)
            m_result[i].sum.Ti = timestamp;
            m_result[i].sum.Az = static_cast<uint32_t>(data[i].azm * 1000);
            m_result[i].sum.El = static_cast<uint32_t>(data[i].elv * 1000);
        }

        calculateCheckSum();

    } catch (const std::exception& e) {
        qCritical() << "[UdpSender]: Exception in constructor:" << e.what();
        throw;
    }
}

UdpSender::~UdpSender()
{
    qDebug() << "[UdpSender]: Destructor called";
    if (m_udpSocket) {
        m_udpSocket->close();
        delete m_udpSocket;
        m_udpSocket = nullptr;
    }
}

void UdpSender::calculateCheckSum()
{
    m_checkSum = 0;
    if (m_result.empty()) return;

    // Используем XOR как в Codesys
    for (const auto& item : m_result) {
        // Только для структуры Sum
        m_checkSum ^= item.sum.Ti ^ item.sum.Az ^ item.sum.El;
    }
}

QByteArray UdpSender::prepareDataForSend() const
{
    if (m_result.empty()) {
        qWarning() << "[UdpSender]: No data to prepare";
        return QByteArray();
    }

    const size_t headerSize = sizeof(uint32_t) * 2;
    const size_t dataSize = m_result.size() * sizeof(Mas);

    QByteArray data;
    data.resize(headerSize + dataSize);
    char* ptr = data.data();

    // Записываем заголовок
    *reinterpret_cast<uint32_t*>(ptr) = m_checkSum;
    *reinterpret_cast<uint32_t*>(ptr + sizeof(uint32_t)) = m_result.size();

    // Копируем данные структур
    memcpy(ptr + headerSize, m_result.constData(), dataSize);

    return data;
}

void UdpSender::debugPrintData() const
{
    qDebug() << "=== UDP Sender Debug ===";
    qDebug() << "Header:";
    qDebug() << "- Checksum:" << m_checkSum;
    qDebug() << "- Items count:" << m_result.size();

    for (size_t i = 0; i < m_result.size(); i++) {
        qDebug() << "Item" << i << ":";
        qDebug() << "  Point: Ti =" << m_result[i].point.Ti
                 << "Az =" << m_result[i].point.Az
                 << "El =" << m_result[i].point.El;
        qDebug() << "  Sum: Ti =" << m_result[i].sum.Ti
                 << "Az =" << m_result[i].sum.Az
                 << "El =" << m_result[i].sum.El;
    }
}
bool UdpSender::sendData()
{

    if (m_result.empty()) {
        qWarning() << "[UdpSender]: No data to send";
        emit errorOccurred("No data to send");
        return false;
    }

    debugPrintData();

    QByteArray dataToSend = prepareDataForSend();

    if (dataToSend.isEmpty()) {
        qWarning() << "[UdpSender]: Prepared data is empty";
        emit errorOccurred("Prepared data is empty");
        return false;
    }

    qint64 bytesSent = m_udpSocket->writeDatagram(dataToSend, m_targetAddress, m_targetPort);

    if (bytesSent == -1) {
        QString errorMsg = QString("Failed to send UDP data: %1").arg(m_udpSocket->errorString());
        qCritical() << "[UdpSender]:" << errorMsg;
        emit errorOccurred(errorMsg);
        emit dataSent(false, bytesSent);
        return false;
    }

    qInfo() << "[UdpSender]: Successfully sent" << bytesSent << "bytes";
    emit dataSent(true, bytesSent);
    return true;
}
