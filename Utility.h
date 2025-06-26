#ifndef UTILITY_H
#define UTILITY_H

#include<QString>

class Utility final
{
public:
    Utility() = delete;
    Utility(const Utility&) = delete;
    Utility &operator=(const Utility&) = delete;
    Utility(Utility&&) noexcept;
    Utility &operator=(const Utility&&) noexcept;

    static void CreateSettingsFile(const QString& settingsFilePath);
    static void СustomMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    // Создает файл если его нет, если есть отчищает
    static void СlearLogFile(const QString &fileName = "app.log");
    // Отчищает сообщения в лог файлы определенной давности (дней)
    static void ClearOldLogs(const QString &fileName, int days);

};

#endif // UTILITY_H
