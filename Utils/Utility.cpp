#include "Utility.h"
#include <QSettings>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

int32_t Utility::CalcChecksum(const int32_t a, const int32_t b, const int32_t c){
    return a ^ b ^ c;
}

void Utility::CreateSettingsFile(const QString& settingsFilePath){
    QSettings settings(settingsFilePath, QSettings::IniFormat);
    settings.beginGroup("ObserverConfiguration");
    settings.setValue("dt_mks",  15000000);
    settings.setValue("numKA", 44714);
    settings.setValue("Latitude",  51.507406923983446);
    settings.setValue("Longitude", -0.12773752212524414);
    settings.setValue("Altitude", 0.05);
    settings.endGroup();

    settings.beginGroup("SshConnection");
    settings.setValue("host",  "127.0.0.1");
    settings.setValue("port", 99);
    settings.setValue("login",  "login");
    settings.setValue("pass", "password");
    settings.endGroup();

    settings.beginGroup("TLEbySpaceTrack");
    settings.setValue("login", "");
    settings.setValue("pass", "");
    settings.endGroup();

}

void Utility::СustomMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // Открываем файл для добавления (append)
    QFile logFile("ServiceFiles/app.log");
    if (!logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        return; // Если файл не удалось открыть, ничего не делаем
    }

    QTextStream out(&logFile);

    // Форматируем текущую дату и время
    QString dateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    // Формируем сообщение в зависимости от типа
    QString message;
    switch (type) {
    case QtDebugMsg:
        message = QString("[%1] Debug: %2").arg(dateTime).arg(msg);
        break;
    case QtInfoMsg:
        message = QString("[%1] Info: %2").arg(dateTime).arg(msg);
        break;
    case QtWarningMsg:
        message = QString("[%1] Warning: %2").arg(dateTime).arg(msg);
        break;
    case QtCriticalMsg:
        message = QString("[%1] Critical: %2").arg(dateTime).arg(msg);
        break;
    case QtFatalMsg:
        message = QString("[%1] Fatal: %2").arg(dateTime).arg(msg);
        break;
    }

    // Записываем сообщение в файл
    out << message << "\n";
    logFile.close();

    // Опционально: выводим сообщение в консоль
    QTextStream(stdout) << message << "\n";
}

void Utility::ClearOldLogs(const QString &fileName, int days)
{
    // Открываем файл для чтения
    QFile logFile(fileName);
    if (!logFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть файл для чтения:" << fileName;
        return;
    }

    // Читаем все строки
    QStringList validLines;
    QTextStream in(&logFile);
    QDateTime currentTime = QDateTime::currentDateTime();
    QDateTime thresholdTime = currentTime.addDays(-days);

    while (!in.atEnd()) {
        QString line = in.readLine();
        // Предполагаем, что строка начинается с временной метки в формате [yyyy-MM-dd hh:mm:ss]
        if (line.startsWith("[")) {
            QString dateTimeStr = line.mid(1, 19); // Извлекаем [yyyy-MM-dd hh:mm:ss]
            QDateTime logTime = QDateTime::fromString(dateTimeStr, "yyyy-MM-dd hh:mm:ss");
            if (logTime.isValid() && logTime >= thresholdTime) {
                validLines.append(line); // Сохраняем только записи моложе n дней
            }
        } else {
            validLines.append(line); // Сохраняем строки без временной метки (если есть)
        }
    }
    logFile.close();

    // Перезаписываем файл с отфильтрованными строками
    if (!logFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qWarning() << "Не удалось открыть файл для записи:" << fileName;
        return;
    }

    QTextStream out(&logFile);
    for (const QString &line : validLines) {
        out << line << "\n";
    }
    logFile.close();
}


void Utility::СlearLogFile(const QString &fileName){
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        file.resize(0);
        file.close();
    }
}
