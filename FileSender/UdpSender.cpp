#include "UdpSender.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>

UdpSender::UdpSender(std::vector<NORAD_SCHEDULE>& data,
                     uint32_t satelliteNumber,
                     const QHostAddress& targetAddress,
                     quint16 targetPort)
    : m_targetAddress(targetAddress), m_targetPort(targetPort)
{
    qDebug() << "[UdpSender]: Constructor started";

    m_udpSocket = new QUdpSocket(this);

    if(data.empty()) {
        qCritical() << "[UdpSender][Constructor]: No data to copy";
        return;
    }

    try {
        // Размер = данные + 2 строки заголовка
        m_data.resize(data.size() + 2);

        // Текущее время для заголовков
        uint32_t currentTime = static_cast<uint32_t>(libsgp4::DateTime::Now().Ticks());

        // Первая строка заголовка [0]
        m_data[0].col0 = currentTime;           // Время сервера
        m_data[0].col1 = data.size();          // Количество точек
        m_data[0].col2 = 0;                    // Место под контрольную сумму

        // Вторая строка заголовка [1]
        m_data[1].col0 = currentTime;          // Время запроса
        m_data[1].col1 = satelliteNumber;      // Номер спутника
        m_data[1].col2 = 0;                    // Код ошибки

        // Заполняем данные точек
        for (size_t i = 0; i < data.size(); i++) {
            m_data[i + 2].col0 = static_cast<uint32_t>(data[i].onDate.Ticks());
            m_data[i + 2].col1 = data[i].azm;
            m_data[i + 2].col2 = data[i].elv;
        }

        calculateCheckSum();
        qDebug() << "[UdpSender]: Data prepared successfully";
        qDebug() << "[UdpSender]: Total points:" << data.size();
        qDebug() << "[UdpSender]: Total size:" << getDataSize() << "bytes";

    } catch (const std::exception& e) {
        qCritical() << "[UdpSender]: Exception in constructor:" << e.what();
        throw;
    }
}

void UdpSender::calculateCheckSum()
{
    if (m_data.size() < 2) return;

    // XOR второй строки заголовка
    uint32_t checksum = m_data[1].col0;  // Время запроса
    checksum ^= static_cast<uint32_t>(m_data[1].col1);  // Номер спутника
    checksum ^= static_cast<uint32_t>(m_data[1].col2);  // Код ошибки

    // Записываем контрольную сумму в первую строку
    m_data[0].col2 = checksum;

    qDebug() << "[UdpSender]: Checksum calculated:" << checksum;
}

QByteArray UdpSender::prepareDataForSend() const
{
    if (m_data.empty()) {
        qWarning() << "[UdpSender]: No data to prepare";
        return QByteArray();
    }

    const size_t totalSize = m_data.size() * sizeof(DataRow);
    QByteArray data(totalSize, 0);

    memcpy(data.data(), m_data.constData(), totalSize);

    qDebug() << "[UdpSender]: Prepared data size:" << totalSize << "bytes";
    return data;
}

void UdpSender::debugPrintData() const
{
    QFile file("NoradSchedule.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCritical() << "Could not open NoradSchedule.txt for writing";
        return;
    }

    QTextStream out(&file);
    out.setRealNumberPrecision(3);
    out.setRealNumberNotation(QTextStream::FixedNotation);

    // Специальная обработка для заголовков (первые две строки)
    // Первая строка заголовка - все значения целые
    out << QString("[0],[0] %1 [0],[1] %2 [0],[2] %3\n")
               .arg(m_data[0].col0)          // uint32_t - время
               .arg(static_cast<uint32_t>(m_data[0].col1))  // количество точек
               .arg(static_cast<uint32_t>(m_data[0].col2)); // контрольная сумма

    // Вторая строка заголовка - все значения целые
    out << QString("[1],[0] %1 [1],[1] %2 [1],[2] %3\n")
               .arg(m_data[1].col0)          // uint32_t - время
               .arg(static_cast<uint32_t>(m_data[1].col1))  // номер спутника
               .arg(static_cast<uint32_t>(m_data[1].col2)); // код ошибки

    // Пустая строка для разделения
    out << "\n";

    // Данные точек - время целое, азимут и угол места - float
    for (int i = 2; i < m_data.size(); i++) {
        out << QString("[%1],[0] %2 [%1],[1] %3 [%1],[2] %4\n")
                   .arg(i)
                   .arg(m_data[i].col0)      // uint32_t - время
                   .arg(m_data[i].col1, 0, 'f', 3)  // float - азимут
                   .arg(m_data[i].col2, 0, 'f', 3); // float - угол места
    }

    file.close();
    qDebug() << "[UdpSender]: Debug data written to NoradSchedule.txt";
}

bool UdpSender::sendData()
{
    if (m_data.empty()) {
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

    qInfo() << "[UdpSender]: Successfully sent" << bytesSent << "bytes to"
            << m_targetAddress.toString() << ":" << m_targetPort;
    emit dataSent(true, bytesSent);
    return true;
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
