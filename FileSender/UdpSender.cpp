#include "UdpSender.h"
#include <QDebug>
#include <QByteArray>

UdpSender::UdpSender(std::vector<NORAD_SCHEDULE>& data,
                     const QHostAddress& targetAddress,
                     quint16 targetPort)
    : m_targetAddress(targetAddress), m_targetPort(targetPort)
{
    m_udpSocket = new QUdpSocket(this);
    qDebug() << "[UdpSender]: UDP socket created";

    if(data.empty()){
        qCritical() << "[UdpSender][Constructor]: No data to copy";
        return;
    }


    try {
        m_result.resize(data.size());

        for (int i = 0; i < int(data.size()); i++){
            // Проверяем валидность данных
            if (i < data.size()) {
                // Преобразуем DateTime в тики (uint32_t)
                m_result[i].point.dt = static_cast<uint32_t>(data[i].onDate.Ticks());
                m_result[i].point.Az = static_cast<float>(data[i].azm);
                m_result[i].point.El = static_cast<float>(data[i].elv);

                // Инициализируем sum структуру
                m_result[i].sum.dt = m_result[i].point.dt;
                m_result[i].sum.Az = static_cast<uint32_t>(data[i].azm);
                m_result[i].sum.El = static_cast<uint32_t>(data[i].elv);

                // Заполняем атрибуты
                if (!data.empty()) {
                    m_result[i].atr.dt = static_cast<uint32_t>(data[0].onDate.Ticks());
                } else {
                    m_result[i].atr.dt = 0;
                }
                m_result[i].atr.d1 = 0;
                m_result[i].atr.d2 = 0;
            }
        }

        calculateCheckSum();
        qDebug() << "[UdpSender]: Checksum calculated:" << m_checkSum;

    } catch (const std::exception& e) {
        qCritical() << "[UdpSender]: Exception in constructor:" << e.what();
        throw;
    }
}

UdpSender::~UdpSender()
{
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

    const uint8_t* dataPtr = reinterpret_cast<const uint8_t*>(m_result.constData());
    size_t dataSize = m_result.size() * sizeof(Mas);

    for (size_t i = 0; i < dataSize; ++i) {
        m_checkSum += dataPtr[i];
    }
}

QByteArray UdpSender::prepareDataForSend() const
{
    QByteArray data;

    if (m_result.empty()) {
        qWarning() << "[UdpSender]: No data to prepare";
        return data;
    }

    // Ручная подготовка данных в little-endian формате
    data.resize(sizeof(uint32_t) * 2 + m_result.size() * sizeof(Mas));

    uint8_t* dataPtr = reinterpret_cast<uint8_t*>(data.data());

    // Записываем контрольную сумму (4 байта, little-endian)
    uint32_t checksumLE = m_checkSum;
    dataPtr[0] = (checksumLE >> 0) & 0xFF;
    dataPtr[1] = (checksumLE >> 8) & 0xFF;
    dataPtr[2] = (checksumLE >> 16) & 0xFF;
    dataPtr[3] = (checksumLE >> 24) & 0xFF;

    // Записываем количество элементов (4 байта, little-endian)
    uint32_t countLE = static_cast<uint32_t>(m_result.size());
    dataPtr[4] = (countLE >> 0) & 0xFF;
    dataPtr[5] = (countLE >> 8) & 0xFF;
    dataPtr[6] = (countLE >> 16) & 0xFF;
    dataPtr[7] = (countLE >> 24) & 0xFF;

    // Копируем данные структур Mas
    size_t offset = 8;
    const uint8_t* masData = reinterpret_cast<const uint8_t*>(m_result.constData());
    size_t masDataSize = m_result.size() * sizeof(Mas);

    for (size_t i = 0; i < masDataSize; ++i) {
        dataPtr[offset + i] = masData[i];
    }
    return data;
}

bool UdpSender::sendData()
{

    if (m_result.empty()) {
        qWarning() << "[UdpSender]: No data to send";
        emit errorOccurred("No data to send");
        return false;
    }

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
