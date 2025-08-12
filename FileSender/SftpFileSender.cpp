#include "SftpFileSender.h"

#include <fcntl.h>      // Для O_WRONLY, O_CREAT, O_TRUNC
#include <sys/stat.h>    // Для S_IRWXU, S_IRUSR, S_IWUSR и др.

#include <QFile>
#include <QDebug>



SftpFileSender::SftpFileSender(const QString& host, const quint16 port, const QString& user, const QString& password, const QString& localPath, const QString& remotePath)
    :m_localPath(localPath), m_remotePath(remotePath), m_host(host), m_port(port), m_user(user), m_password(password)
{}

void SftpFileSender::setDestination(const QString& host){
    m_host = host;
}

bool SftpFileSender::connectToPlc(const ssh_session& session){

    // Подключение
    if (ssh_connect(session) != SSH_OK) {
        qCritical() << "[SftpFileSender::connectToPlc]: Connection failed: " << ssh_get_error(session);
        ssh_free(session);
        return false;
    }

    // Аутентификация по паролю
    if (ssh_userauth_password(session, nullptr, m_password.toUtf8().constData()) != SSH_AUTH_SUCCESS) {
        qCritical() << "[SftpFileSender::connectToPlc]: Authentication failed: " << ssh_get_error(session) << '\n';
        ssh_disconnect(session);
        ssh_free(session);
        return false;
    }
    return true;
};

std::optional<ssh_session> SftpFileSender::makeSshSession(){
    ssh_session session = ssh_new();
    if (!session) {
        qCritical() << "[SftpFileSender::makeSshSession]: Failed to create SSH session: " << ssh_get_error(session) << '\n';
        return std::nullopt;
    }
    // Настройка SSH-сессии
    ssh_options_set(session, SSH_OPTIONS_HOST, m_host.toUtf8());
    ssh_options_set(session, SSH_OPTIONS_PORT, &m_port);
    ssh_options_set(session, SSH_OPTIONS_USER, m_user.toUtf8());

    if(!connectToPlc(session)){
        return std::nullopt;
    }
    return session;
}

std::optional<sftp_session> SftpFileSender::makeSftpSession(const ssh_session& session){
    sftp_session sftp = sftp_new(session);
    if (!sftp) {
        qCritical() << "[SftpFileSender::makeSftpSession]: SFTP session creation failed: " << sftp_get_error(sftp) << "\n";
        ssh_disconnect(session);
        ssh_free(session);
        return std::nullopt;
    }

    if (sftp_init(sftp) != SSH_OK) {
        qCritical() << "[SftpFileSender::makeSftpSession]: SFTP initialization failed" << sftp_get_error(sftp);
        sftp_free(sftp);
        ssh_disconnect(session);
        ssh_free(session);
        return std::nullopt;
    }
    return sftp;
}

bool SftpFileSender::send(){

    //Создание SSH-сессии
    std::optional<ssh_session> session{makeSshSession()};
    if (!session.has_value()){
        return false;
    }

    // Создание SFTP-сессии
    std::optional<sftp_session> sftp{makeSftpSession(session.value())};
    if (!sftp.has_value()){
        return false;
    }

    // Создание файла для записи на сервере
    int access_type = O_WRONLY | O_CREAT | O_TRUNC;
    sftp_file remoteFile = sftp_open(sftp.value(), m_remotePath.toUtf8().constData(), access_type, S_IRWXU);
    if (remoteFile == nullptr) {
        qCritical() << "[SftpFileSender::send]: Failed to open/create remote file for writing" << ssh_get_error(session.value());
        sftp_free(sftp.value());
        ssh_disconnect(session.value());
        ssh_free(session.value());
        return false;
    }

    // Чтение локального файла
    QFile localFile(m_localPath);
    if (!localFile.open(QIODevice::ReadOnly)) {
        qCritical() << "[SftpFileSender::send]: Failed to open local file" << localFile.errorString();
        sftp_close(remoteFile);
        sftp_free(sftp.value());
        ssh_disconnect(session.value());
        ssh_free(session.value());
        return false;
    }

    // Передача данных
    qint64 totalSize = localFile.size();
    qint64 bytesSent = 0;
    qint64 chunkSize = 10* 1024* 1024; // 10 Mb

    while (bytesSent < totalSize) {
        // Чтение куска файла
        QByteArray chunk = localFile.read(chunkSize);
        if (chunk.isEmpty() && !localFile.atEnd()) {
            qCritical() << "[SftpFileSender::send]: Error reading file chunk";
            break;
        }

        // Запись куска на сервер
        ssize_t written = sftp_write(remoteFile, chunk.constData(), chunk.size());
        if (written != chunk.size()) {
            qCritical() << "[SftpFileSender::send]: SFTP write error: " << sftp_get_error(sftp.value());
            break;
        }

        bytesSent += written;
    }

    // Закрытие всех ресурсов
    localFile.close();
    sftp_close(remoteFile);
    sftp_free(sftp.value());
    ssh_disconnect(session.value());
    ssh_free(session.value());

    qInfo() << "[SftpFileSender::send]: File uploaded successfully!";
    return true;

}
