#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#ifdef _WIN32
#include <windows.h>
#endif

#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QTimer>
#include <QMessageBox>
#include <QSound>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QDir>


#include "accountmanager.h"
#include "settingsmanager.h"
#include "optionsdialog.h"
#include "notificationwindow.h"
#include "stringdecoder.h"

#include "structs.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

protected:
    void closeEvent(QCloseEvent *event);
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    AccountManager* am;
    SettingsManager*sm;
    
private slots:
    void TimerFunc();
    void AnimTimerFunc();
    void IconClicked(QSystemTrayIcon::ActivationReason reason);
    void on_button_Clear_clicked();
    void on_button_Exit_clicked();
    void on_button_Check_clicked();
    void on_button_Settings_clicked();
    void on_button_Hide_clicked();
    void on_button_Animate_clicked();
    void on_button_AnimStop_clicked();
    void on_button_Unignore_clicked();
    void on_button_Disable_clicked();
    void on_button_Enable_clicked();

private:
    Ui::MainWindow* ui;
    int i_ischecking;
    int i_isanimating;
    int i_networkerror;
    int i_firstcheck;

protected:
    QSystemTrayIcon*systrayIcon;
    QMenu*          trayMenu;
    QAction*        a_check;
    QTimer*         qt, *aqt;
    SSL_CTX*        ctx;
    int             i_lasticon;
    void            CheckAll();
    void            mailcheck(snippet_container *snippets, char *name, char *psw, char *domain, int port, int ssl);
    char*           messageExchange(BIO *bio, char *phrase, int msgidx);
    BIO*         	sslConnect(char *domain, int port, int secure);
    void			sslDisconnect(BIO *bio);
    char*			sslRead(BIO *bio);
    void			sslWrite(BIO *bio, char *text);
    char*			responseToEmailAddress(char *response);
    char*			responseToName(char *response);
    char*			responseToMsgSubject(char *response);
    char*			responseToMsgSender(char *response);
    char*			responseToSenderEmail(char *response);
    int				responseToAllMsgNumber(char *response);
    int				responseToMsgList(char *response, lista **pList);
    float			responseToMsgSize(char *response);
    datetime        responseToDatetime(char *response);
    void            myprintfs(const char *, char *);
    void            myprintfi(const char *, int);
    void            myprintff(const char *, float);
    void            myprintfis(const char *, int, char *);
    void            myprintfii(const char *, int, int);
    int             IsIgnored(datetime dt);
    void            IconSetMailCount(int count);
    int             IsRunning();
    char*           GetLockFilePath();
    void            ClearLock();
};

#endif // MAINWINDOW_H
