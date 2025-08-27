#include "QueryHandler.h"
#include <QDebug>

QueryHandler::QueryHandler(QObject *parent) : QObject(parent)
{}

QueryHandler::ErrorCodes QueryHandler::validatePacket(const QByteArray &data) const
{
    if (!validateFormat(data)) {
        return INVALID_FORMAT;
    }

    if (!validateChecksum(data)) {
        return CHECKSUM_ERROR;
    }

    // Other checks

    return NO_ERROR;
}

bool QueryHandler::validateFormat(const QByteArray &data) const
{
    // Минимальный размер пакета - заголовок
    const int minHeaderSize = sizeof(ControllerHeader);
    if (data.size() < minHeaderSize) {
        qWarning() << "[QueryHandler]: Packet too small, size:" << data.size();
        return false;
    }

    // Проверяем структуру заголовка
    // Здесь можно добавить дополнительные проверки формата

    return true;
}

bool QueryHandler::validateChecksum(const QByteArray &data) const
{
    // Реализация проверки контрольной суммы
    // Заглушка - всегда возвращаем true для примера
    // В реальной реализации нужно реализовать правильную проверку
    return true;
}
