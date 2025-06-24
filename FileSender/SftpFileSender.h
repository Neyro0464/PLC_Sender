#ifndef SFTPFILESENDER_H
#define SFTPFILESENDER_H

#include <QString>

#include "IFileSenderPLC.h"

#include <libssh/libssh.h>
#include <libssh/sftp.h>


class SftpFileSender: public IFileSenderPLC
{
public:
    explicit SftpFileSender(const QString& host, const quint16 port,
                            const QString& user, const QString& password,
                            const QString& localPath, const QString& remotePath);
    bool send() override;
    // Function to change host address
    void setDestination(const QString& host) override;

    // The Exclusive Ownability Strategy
    SftpFileSender &operator=(const SftpFileSender&) = delete;
    SftpFileSender (const SftpFileSender&) = delete;
    SftpFileSender(SftpFileSender&&) noexcept;
    SftpFileSender& operator=(SftpFileSender&&) noexcept;

    virtual ~SftpFileSender() = default;

private:
    // Connection to PLC with login and password
    bool connectToPlc(const ssh_session& session);
    // Making SSH session with libssh to make SFTP session in future
    std::optional<ssh_session>makeSshSession();
    // Making SFTP session to send file to PLC
    std::optional<sftp_session> makeSftpSession(const ssh_session& session);

    QString m_localPath;  // path of the sendered file
    QString m_remotePath; // path on remote device
    QString m_host;       // ip address of server
    quint16 m_port;       // port
    QString m_user;       // user login SSH
    QString m_password;   // user password SSH
};

#endif // SFTPFILESENDER_H
