// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QObject>
#include <QPointer>
#include <QProcess>
#include <QQueue>

namespace wpe {

// Drives steamcmd without a terminal emulator.
//  * login(): answers the password / Steam Guard prompts from the UI so credentials get cached by steamcmd.
//  * download(): runs one Workshop download at a time (queued), streaming progress lines.
// Passwords go over stdin only, never argv (argv is world-readable in /proc).
class SteamCmd : public QObject {
    Q_OBJECT
public:
    explicit SteamCmd(QObject *parent = nullptr);

    bool available() const;
    bool busy() const { return m_proc != nullptr; }

    void login(const QString &user, const QString &password);
    void submitGuardCode(const QString &code);
    void download(const QString &itemId, const QString &user, const QString &installDir);
    void installAssets(const QString &user, const QString &installDir);

    static QString stripAnsi(QString s);
    static QString explainError(const QString &line);

signals:
    void progress(QString itemId, QString line);
    void downloadFinished(QString itemId, bool ok, QString pathOrError);
    void guardCodeRequired(QString kind); // "steam-guard" | "two-factor"
    void loginFinished(bool ok, QString message);
    void assetsFinished(bool ok, QString message);

private:
    struct Job { enum Type { Download, Assets } type; QString id, user, dir; };
    void pump();
    void startJob(const Job &j);
    void onReadyRead();

    QProcess *m_proc = nullptr;
    QQueue<Job> m_queue;
    Job m_job{};
    QString m_buf;      // unterminated tail for prompt matching
    QStringList m_lines;

    // login state
    bool m_loginMode = false;
    QString m_password;
    bool m_sentPassword = false;
};

} // namespace wpe
