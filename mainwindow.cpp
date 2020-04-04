#define BSIZE       2048
//#define _CRT_SECURE_NO_WARNINGS

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#endif

#ifdef __linux__
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    if(IsRunning())
    {
        QMessageBox::warning(nullptr, QString::fromUtf8("Mchecker"), QString::fromUtf8("Only one instance of the application is allowed. The program will now exit."));
        ::exit(-1);
    }

    sm = new SettingsManager();
    sm->LoadSettings();

    i_lasticon = 0;
    i_ischecking = 0;
    i_isanimating = 0;
    i_networkerror = 0;
    i_firstcheck = 1;

    trayMenu = new QMenu();

    QAction *a_show = nullptr;

    if(sm->GetDebugLevel()>0)
    {
        a_show = trayMenu->addAction(QString::fromUtf8("Show Debug Window"));
        trayMenu->addSeparator();
    }
    a_check = trayMenu->addAction(QString::fromUtf8("Check Now"));
    if(sm->GetDebugLevel()<1) trayMenu->addSeparator();
    QAction *a_settings = trayMenu->addAction(QString::fromUtf8("Settings"));
    QAction *a_exit = trayMenu->addAction(QString::fromUtf8("Exit"));

    if(sm->GetDebugLevel()>0) a_show->connect(a_show, SIGNAL(triggered()), this, SLOT(show()));
    a_check->connect(a_check, SIGNAL(triggered()), this, SLOT(on_button_Check_clicked()));
    a_settings->connect(a_settings, SIGNAL(triggered()), this, SLOT(on_button_Settings_clicked()));
    a_exit->connect(a_exit, SIGNAL(triggered()), this, SLOT(on_button_Exit_clicked()));

    systrayIcon = new QSystemTrayIcon();
    connect(systrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(IconClicked(QSystemTrayIcon::ActivationReason)));
    systrayIcon->setIcon(QIcon("./images/tray1.png"));
    systrayIcon->setContextMenu(trayMenu);
    systrayIcon->show();

/*    QRect r = systrayIcon->geometry();
    if(r.width() == 0 && r.height() == 0)
    {
        if(QMessageBox::warning(0, QString::fromUtf8("Mchecker"), QString::fromUtf8("Failed to create tray icon. Continue?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
        {
            ClearLock();
            ::exit(-1);
        }
    }
*/

    qt = new QTimer(this);
    connect(qt, SIGNAL(timeout()), this, SLOT(TimerFunc()));
    qt->start(sm->GetFirstCheckInterval()*1000);
    myprintfs("Program started.", nullptr);

    am = new AccountManager();
    am->LoadAccountData();

    if(am->GetCount() < 1) on_button_Settings_clicked();
}

MainWindow::~MainWindow()
{
    delete trayMenu;
    delete systrayIcon;
    delete qt;
    delete am;
    delete ui;

    ClearLock();
}


void MainWindow::TimerFunc()
{
    myprintfs("Timer fired.", nullptr);
    if(i_firstcheck)
    {
        qt->setInterval(sm->GetTimerInterval()*1000);
        i_firstcheck = 0;
    }
    CheckAll();
}

void MainWindow::AnimTimerFunc()
{
    QString file;

    switch(i_lasticon)
    {
    case 0:
        file = "./images/tray2.png";
        i_lasticon = 2;
        break;
    case 2:
        file = "./images/tray3.png";
        i_lasticon = 3;
        break;
    case 3:
        file = "./images/tray4.png";
        i_lasticon = 4;
        break;
    case 4:
        file = "./images/tray2.png";
        i_lasticon = 2;
        break;
    default:
        file = "./images/tray1.png";
        i_lasticon = 0;
        break;
    }

    systrayIcon->setIcon(QIcon(file));
}

void MainWindow::CheckAll()
{
    if(i_ischecking)
    {
        if(i_ischecking == 1) myprintfs("Another check is in progress. Aborting...", nullptr);
        else myprintfs("Checking is disabled. Aborting...", nullptr);
        return;
    }
    i_ischecking = 1;
    ui->button_Check->setEnabled(false);
    a_check->setEnabled(false);

    int accs = am->GetCount();
    snippet_container *psnippets;

    if(accs>0)
    {
        psnippets = (snippet_container*)calloc(accs, sizeof(snippet_container));
        if(!psnippets)
        {
            myprintfs("Cannot allocate memory for message check. Aborting.", nullptr);
            ui->button_Check->setEnabled(true);
            a_check->setEnabled(true);
            i_ischecking = 0;
            return;
        }
        on_button_Animate_clicked();
        for(int i=0; i<accs; i++)
        {
            char *fname = am->GetFriendlyName(i),
                    *cmd = am->GetCommand(i),
                    *lname = am->GetLoginName(i),
                    *psw = am->GetPassword(i),
                    *dm = am->GetDomain(i);
            psnippets[i].accid = i;
            strcpy(psnippets[i].accname, fname);
            strcpy(psnippets[i].cmd, cmd);
            mailcheck(&psnippets[i], lname, psw, dm, am->GetPort(i), am->GetSSL(i));

            free(fname);
            free(cmd);
            free(lname);
            free(psw);
            free(dm);

            QApplication::processEvents();
        }
        on_button_AnimStop_clicked();

        int hasmessages = 0;
        for(int i=0; i<accs; i++) hasmessages += psnippets[i].count;

        int ignorecount = 0;
        for(int i=0; i<accs; i++) for(int j=0; j<psnippets[i].count; j++)
            ignorecount += psnippets[i].snippets[j].ignored;

        if(hasmessages)
        {
            IconSetMailCount(hasmessages);
            if(ignorecount >= hasmessages)
            {
                myprintfs("You only have ignored messages. Skipping notification...\n", nullptr);
            }
            else
            {
                myprintfs("\nYou have unread mail, showing notification...\n", nullptr);

                char *snd = sm->GetSoundFile();
                if(snd && snd[0] != '\0')
                {
                    QString s =  QString::fromUtf8(sm->GetSoundFile());

                    if(s != "")
                    {
#if QT_VERSION < 0x050000
                        if(QSound::isAvailable())
                        {
                            QSound::play(s);
                        }
                        else
                        {
                            s = QString::fromUtf8("mplayer ") + s;
                            QProcess::startDetached(s);
                        }
#else
                        QSound::play(s);
#endif
                    }
                }

                NotificationWindow *nw = new NotificationWindow(nullptr, psnippets, accs, sm);
                if(nw->exec()) systrayIcon->setIcon(QIcon("./images/tray1.png"));

                delete nw;
            }
        }
        else myprintfs("\nYou have no unread mail.\n", nullptr);

        for(int i=0; i<accs; i++) if(psnippets[i].snippets) free(psnippets[i].snippets);
        free(psnippets);
    }
    else myprintfs("Nothing to check, no accounts configured.\n", nullptr);

    ui->button_Check->setEnabled(true);
    a_check->setEnabled(true);

    i_ischecking = 0;
}

void MainWindow::mailcheck(snippet_container *snippets, char *name, char *psw, char *domain, int port, int ssl)
{
    BIO *bio = nullptr;
    char *response = nullptr;

    char phrase[BSIZE] = "";

    int cmdcnt = 1, unrdcnt = 0;
    i_networkerror = 0;

    time((time_t*)&sm->t_check);
    struct tm* t = localtime((time_t*)&sm->t_check);
    sprintf(phrase, "Last checked: %02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
    systrayIcon->setToolTip(QString(phrase));

    myprintfs("Connecting...\n", nullptr);

    bio = sslConnect(domain, port, ssl);

    if(!bio)
    {
        if(response) free(response);
        response = nullptr;
        myprintfs("Connection failed. Make sure you're connected to the internet and your account settings are right.\n", nullptr);
        return;
    }

    response = messageExchange(bio, nullptr, -1);

    char *p = strstr(response, "* OK");
    if(!p)
    {
        myprintfs("Connection failed. Make sure you're connected to the internet.", nullptr);
        sslDisconnect(bio);
        if(response) free(response);
        response = nullptr;
        return;
    }

    if(response) free(response);
    response = nullptr;

    myprintfs("Connection successful. Checking account: %s", name);
    sprintf(phrase, "%d LOGIN %s %s\r\n", cmdcnt++, name, psw);

    response = messageExchange(bio, phrase, cmdcnt-1);

    p = strstr(response, "1 OK ");
    if(!p)
    {
        myprintfs("Login failed. Check your username and password.\n", nullptr);
        sslDisconnect(bio);
        if(response) free(response);
        response = nullptr;
        return;
    }

    char *mail = responseToEmailAddress(response);
    myprintfs("\nEmail address: %s", mail);
    if(mail) free(mail);

    char *namee = responseToName(response);
    if(!namee)
    {
        myprintfs("Login Successful.", namee);
    }
    else
    {
        myprintfs("Name: %s\nLogin Successful.", namee);
        free(namee);
    }

    if(response) free(response);
    response = nullptr;

    sprintf(phrase, "%d SELECT INBOX\r\n", cmdcnt++);
    response = messageExchange(bio, phrase, cmdcnt-1);

    p = strstr(response, "EXISTS");
    if(!p)
    {
        myprintfs("Error. Cannot select INBOX.", nullptr);
        sslDisconnect(bio);
        if(response) free(response);
        response = nullptr;
        return;
    }

    myprintfi("\nYou have %d messages in your inbox.", responseToAllMsgNumber(response));

    if(response) free(response);
    response = nullptr;

    sprintf(phrase, "%d SEARCH UNSEEN\r\n", cmdcnt++);
    response = messageExchange(bio, phrase, cmdcnt-1);

    p = strstr(phrase, "SEARCH"); p[0] = '\0'; strcat(phrase, "OK");
    p = strstr(response, phrase);
    if(!p)
    {
        myprintfs("Error finding unseen messageas.", nullptr);
        myprintfs("\nLogging out...", nullptr);

        sprintf(phrase, "%d LOGOUT\r\n", cmdcnt++);

        if(response) free(response);
        response = nullptr;

        response = messageExchange(bio, phrase, cmdcnt-1);

        p = strstr(phrase, "LOGOUT"); p[0] = '\0'; strcat(phrase, "OK");
        p = strstr(response, phrase);

        if(p) myprintfs("Successfully logged out.\n", nullptr);
        else myprintfs("Failed to log out.\n", nullptr);

        sslDisconnect(bio);
        if(response) free(response);
        response = nullptr;

        return;
    }

    lista *l = nullptr;

    unrdcnt = responseToMsgList(response, &l);

    if(response) free(response);
    response = nullptr;

    if(!l && unrdcnt != 0)
    {
        myprintfs("Failed to store message list in memory.", nullptr);
        myprintfs("\nLogging out...", nullptr);

        if(response) free(response);
        response = nullptr;

        sprintf(phrase, "%d LOGOUT\r\n", cmdcnt++);

        if(response) free(response);
        response = nullptr;

        response = messageExchange(bio, phrase, cmdcnt-1);

        p = strstr(phrase, "LOGOUT"); p[0] = '\0'; strcat(phrase, "OK");
        p = strstr(response, phrase);

        if(p) myprintfs("Successfully logged out.\n", nullptr);
        else myprintfs("Failed to log out.\n", nullptr);

        sslDisconnect(bio);
        if(response) free(response);
        response = nullptr;

        return;
    }

    if(unrdcnt < 1)
    {
        myprintfs("You have no unseen messages.", nullptr);
        myprintfs("\nLogging out...", nullptr);

        sprintf(phrase, "%d LOGOUT\r\n", cmdcnt++);

        if(response) free(response);
        response = nullptr;

        response = messageExchange(bio, phrase, cmdcnt-1);

        p = strstr(phrase, "LOGOUT"); p[0] = '\0'; strcat(phrase, "OK");
        p = strstr(response, phrase);

        if(p) myprintfs("Successfully logged out.\n", nullptr);
        else myprintfs("Failed to log out.\n", nullptr);

        sslDisconnect(bio);
        if(response) free(response);
        response = nullptr;

        return;
    }

    if(unrdcnt == 1) myprintfs("You have 1 unseen message.", nullptr);
    else myprintfi("You have %d unseen messages.", unrdcnt);

    snippets->count = unrdcnt;
    snippets->snippets = (snippet*)calloc(unrdcnt, sizeof(snippet));
    if(!snippets->snippets)
    {
        myprintfs("Filed to allocate memory for email data. Aborting this account, logging out.", nullptr);

        sprintf(phrase, "%d LOGOUT\r\n", cmdcnt++);

        if(response) free(response);
        response = nullptr;

        response = messageExchange(bio, phrase, cmdcnt-1);

        p = strstr(phrase, "LOGOUT"); p[0] = '\0'; strcat(phrase, "OK");
        p = strstr(response, phrase);

        if(p) myprintfs("Successfully logged out.\n", nullptr);
        else myprintfs("Failed to log out.\n", nullptr);

        sslDisconnect(bio);
        if(response) free(response);
        response = nullptr;

        return;
    }

    lista *lc = l;
    int index = 0;

    while(1)
    {
        QApplication::processEvents();
        if(response) free(response);
        response = nullptr;

        snippets->snippets[index].msgid = lc->msg;

        myprintfi("\nMessage number: %d", lc->msg);
        sprintf(phrase, "%d FETCH %d ALL\r\n", cmdcnt++, lc->msg);

        response = messageExchange(bio, phrase, cmdcnt-1);

        p = strstr(phrase, "FETCH"); p[0] = '\0'; strcat(phrase, "OK");
        p = strstr(response, phrase);

        if(!p)
        {
            myprintfs("Fetching data was unsuccessful!\n", nullptr);
            lc = lc->plist; if(!lc) break;
            continue;
        }

        char *sub = responseToMsgSubject(response);
        if(sub)
        {
            myprintfs("Message Subject: \"%s\"", sub);
            strcpy(snippets->snippets[index].subject, sub);
            free(sub);
        }
        else
        {
            myprintfs("Message Subject: \"%s\"", (char*)"NULL");
            strcpy(snippets->snippets[index].subject, (char*)"NULL");
        }

        snippets->snippets[index].dt = responseToDatetime(response);
        sprintf(phrase, "Message date: %02d-%02d-%04d %02d:%02d:%02d",
            snippets->snippets[index].dt.day,
            snippets->snippets[index].dt.month,
            snippets->snippets[index].dt.year,
            snippets->snippets[index].dt.hour,
            snippets->snippets[index].dt.minute,
            snippets->snippets[index].dt.second);
        myprintfs(phrase, nullptr);

        if(IsIgnored(snippets->snippets[index].dt))
        {
            snippets->snippets[index].ignored = 1;
            myprintfs("Message is ignored based on date.", nullptr);
        }
        else
        {
            snippets->snippets[index].ignored = 0;
            myprintfs("Message is not ignored based on date.", nullptr);
        }

        char *sender = responseToMsgSender(response);
        if(!sender)
        {
            myprintfs("Message Sender: ***not present***", nullptr);
            strcpy(snippets->snippets[index].from, "\n\n\n\n\n");
        }
        else
        {
            myprintfs("Message Sender: \"%s\"", sender);
            strcpy(snippets->snippets[index].from, sender);
            free(sender);
        }

        char *sdemail = responseToSenderEmail(response);
        if(sdemail)
        {
            myprintfs("Sender Email: \"%s\"", sdemail);
            if(!strcmp(snippets->snippets[index].from, "\n\n\n\n\n"))
                strcpy(snippets->snippets[index].from, sdemail);
            free(sdemail);
        }
        else strcpy(snippets->snippets[index].from, "NULL");

        float msize = responseToMsgSize(response);
        myprintff("Message Size: \"%.2f KB\"", msize/1024.0);
        sprintf(snippets->snippets[index].size, "%.2f KB", msize/1024.0);

        lc = lc->plist; index++; if(!lc) break;

        QApplication::processEvents();
    }

    if(response) free(response);
    response = nullptr;

    for(lista *pcurr = l; pcurr != nullptr; pcurr = lc)
    {
        lc = pcurr->plist;
        free(pcurr);
    }

    myprintfs("\nLogging out...", nullptr);

    sprintf(phrase, "%d LOGOUT\r\n", cmdcnt++);

    if(response) free(response);
    response = nullptr;

    response = messageExchange(bio, phrase, cmdcnt-1);

    p = strstr(phrase, "LOGOUT"); p[0] = '\0'; strcat(phrase, "OK");
    p = strstr(response, phrase);

    if(p) myprintfs("Successfully logged out.\n", nullptr);
    else myprintfs("Failed to log out.\n", nullptr);

    sslDisconnect(bio);
    if(response) free(response);
    response = nullptr;

    return;
}

char *MainWindow::messageExchange(BIO *bio, char *phrase, int msgidx)
{
    if(phrase) sslWrite(bio, phrase);
    char *r;
    char msgok[256], msgno[256], msgbad[256];

    sprintf(msgok, "%d OK", msgidx);
    sprintf(msgno, "%d NO", msgidx);
    sprintf(msgbad, "%d BAD", msgidx);

    r = sslRead(bio);
    if(!(strstr(r, msgok) || strstr(r, msgno) || strstr(r, msgbad) ||
         msgidx == -1))
    {
        while(1)
        {
            char *tmp = sslRead(bio);
            if(!tmp) return nullptr;
            strcat(r, tmp);
            free(tmp);
            if(strstr(r, msgok) || strstr(r, msgno) || strstr(r, msgbad) ||
                    msgidx == -1) break;
        }
    }


    if(r[strlen(r)-1] != '\n') while(1)
    {
        char *tmp = sslRead(bio);
        if(!tmp) return nullptr;
        strcat(r, tmp);
        free(tmp);
        if(r[strlen(r)] == '\n') break;
    }

    return r;
}

BIO *MainWindow::sslConnect(char *domain, int port, int secure)
{
    int iport = port;
    SSL* ssl;
    BIO* bio;

    //CRYPTO_malloc_init();
    SSL_library_init();
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    OpenSSL_add_all_algorithms();

    if(secure)
    {
        ctx = SSL_CTX_new(SSLv23_client_method());
        SSL_CTX_set_timeout(ctx, 60);
    }
    else ctx = nullptr;

    if(secure) bio = BIO_new_ssl_connect(ctx);
    else
    {
        char s[BSIZE];
        sprintf(s, "%s:%d", domain, port);
        bio = BIO_new_connect(s);
    }

    if(bio == nullptr)
    {
        myprintfs("Error creating BIO!", nullptr);
        systrayIcon->setToolTip(QString("Network error!"));
        i_networkerror = 1;
        ERR_print_errors_fp(stderr);
        if(ctx) SSL_CTX_free(ctx);
        return nullptr;
    }

    if(secure)
    {
        BIO_get_ssl(bio, &ssl);
        SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
        BIO_set_conn_hostname(bio, domain);
        BIO_set_conn_port(bio, &iport);
    }

    if(BIO_do_connect(bio) <= 0)
    {
        myprintfs("Failed to connect!", nullptr);
        systrayIcon->setToolTip(QString::fromUtf8("Network error!"));
        i_networkerror = 1;
        if(bio) BIO_free_all(bio);
        if(ctx) SSL_CTX_free(ctx);
        return nullptr;
    }

    if(secure)
    {
        if(BIO_do_handshake(bio) <= 0)
        {
            myprintfs("Failed to do SSL handshake!", nullptr);
            systrayIcon->setToolTip(QString::fromUtf8("Network error!"));
            i_networkerror = 1;
            if(bio) BIO_free_all(bio);
            if(ctx) SSL_CTX_free(ctx);
            return nullptr;
        }
    }

    return bio;
}

void MainWindow::sslDisconnect(BIO *bio)
{
    if(bio) BIO_free_all(bio);
    if(ctx) SSL_CTX_free(ctx);
}

char *MainWindow::sslRead(BIO *bio)
{
    int readSize = BSIZE;
    char *rc = nullptr;
    int received, count = 0;
    char buffer[BSIZE+1];

    if(bio)
    {
        while(1)
        {
            QApplication::processEvents();

            if(!rc) rc =(char *) calloc(readSize + 1, sizeof(char));
            else rc =(char *) realloc(rc,(count + 1) * readSize * sizeof(char) + 1);

            received = BIO_read(bio, buffer, readSize);
            if(received >= 0) buffer[received] = '\0';

            if(received > 0) strcat(rc, buffer);

            if(received < readSize) break;

            count++;
        }
    }

    return rc;
}

void MainWindow::sslWrite(BIO *bio, char *text)
{
    if(bio) BIO_write(bio, text, (int)strlen(text));
}

char *MainWindow::responseToEmailAddress(char *response)
{
    char *p1 = strstr(response, "1 OK ");
    char *p2 = &p1[5];
    char *p3 = strchr(p2, ' ');

    char *ret = (char*) calloc(BSIZE, sizeof(char));

    strncpy(ret, p2, (p3-p2)>BSIZE?BSIZE:(p3-p2));

    return ret;
}

char *MainWindow::responseToName(char *response)
{
    char *p1 = strstr(response, "@");
    char *p2 = strstr(p1, " "); p2++;
    p1 = strstr(p2, " auth");

    if(!p1) return nullptr;

    char *ret = (char*) calloc(BSIZE, sizeof(char));

    strncpy(ret, p2, (p1-p2)>BSIZE?BSIZE:(p1-p2));

    return ret;
}

char *MainWindow::responseToMsgSubject(char *response)
{
    char *buffer = (char*) calloc(BSIZE, sizeof(char));
    char *p1 = strstr(response, "ENVELOPE");
    p1 = strstr(p1+1, "\"");
    p1 = strstr(p1+1, "\"");
    p1 = strstr(p1+1, "\""); p1++;
    char *p2 = strstr(p1, "\"");

    strncpy(buffer, p1, (p2-p1)>BSIZE-1?BSIZE-1:(p2-p1));

    char *ret = StringDecoder::DecodeString(buffer);
    if(buffer) free(buffer);

    return ret;
}

char *MainWindow::responseToMsgSender(char *response)
{
    char *buffer = (char*) calloc(BSIZE, sizeof(char));
    char *p1 = strstr(response, "((");

    if(p1[2] == 'N' && p1[3] == 'I' && p1[4] == 'L') return nullptr;

    char *p2 = strstr(p1+3, "\"");

    strncpy(buffer, p1+3, (p2-p1-3)>BSIZE?BSIZE:(p2-p1-3));

    char *ret = StringDecoder::DecodeString(buffer);

    if(buffer) free(buffer);

    return ret;
}

char *MainWindow::responseToSenderEmail(char *response)
{
    char *buffer = (char*) calloc(BSIZE, sizeof(char));
    char *p1 = strstr(response, "NIL \"");
    char *p2 = strstr(p1+5, "\"");

    strncpy(buffer, p1+5, (p2-p1-5)>BSIZE-2?BSIZE-2:(p2-p1-5));
    strcat(buffer, "@");

    p1 = strstr(response, "NIL \"");
    p2 = strstr(p1+5, "\"");
    p1 = strstr(p2+1, "\"");
    p2 = strstr(p1+1, "\"");

    strncat(buffer, p1+1, p2-p1-1);

    char *ret = StringDecoder::DecodeString(buffer);

    if(buffer) free(buffer);

    return ret;
}

int	MainWindow::responseToAllMsgNumber(char *response)
{
    char buffer[BSIZE];
    char *p1 = strstr(response, "EXISTS");

    while(*(p1--) != '*');
    p1+=3;

    char *p2 = strchr(p1, '\r');

    strncpy(buffer, p1, (p2-p1)-7); buffer[(p2-p1)-7] = '\0';

    return atoi(buffer);
}

int	MainWindow::responseToMsgList(char *response, lista **pList)
{
    int cnt = 0;
    char *p1 = strstr(response, "* SEARCH"); p1+=9;

    for(int k=0; k<(int)strlen(p1); k++) if((p1[k] == '\r') || (p1[k] == '\n')) p1[k] = '\0';

    if(!p1 || p1[0] == '\0')
    {
        *pList = nullptr;
        return 0;
    }

    lista *l = (lista *)malloc(sizeof(lista));
    lista *lc = l;

    while(true)
    {
        QApplication::processEvents();

        cnt++;
        char *p2 = strrchr(p1, ' ');

        if(!p2)
        {
            lc->msg = atoi(p1);
            lc->plist = nullptr;
            break;
        }

        lc->msg = atoi(p2+1);
        lc->plist = (lista*)malloc(sizeof(lista));
        lc = lc->plist;

        p2[0] = '\0';
    }

    *pList = l;

    return cnt;
}

float MainWindow::responseToMsgSize(char *response)
{
    char *p1 = strstr(response, "RFC822.SIZE ");
    if(!p1) return 0; else p1+=12;

    return (float) atoi(p1);
}

datetime MainWindow::responseToDatetime(char *response)
{
    datetime dt; memset(&dt, 0, sizeof(dt));

    char *adj;
    char *p = strstr(response, "ENVELOPE");
    p = strchr(p, ',');
    p+=2;
    char *end = strchr(p, ' ');
    char tmp[5]; memset(tmp, 0, 5);

    strncpy(tmp, p, end-p);
    dt.day = atoi(tmp);

    memset(tmp, 0, 5); memcpy(tmp, end+1, 3);
    if(!strcmp(tmp, "Jan")) dt.month = 1;
    else if(!strcmp(tmp, "Feb")) dt.month = 2;
    else if(!strcmp(tmp, "Mar")) dt.month = 3;
    else if(!strcmp(tmp, "Apr")) dt.month = 4;
    else if(!strcmp(tmp, "May")) dt.month = 5;
    else if(!strcmp(tmp, "Jun")) dt.month = 6;
    else if(!strcmp(tmp, "Jul")) dt.month = 7;
    else if(!strcmp(tmp, "Aug")) dt.month = 8;
    else if(!strcmp(tmp, "Sep")) dt.month = 9;
    else if(!strcmp(tmp, "Oct")) dt.month = 10;
    else if(!strcmp(tmp, "Nov")) dt.month = 11;
    else if(!strcmp(tmp, "Dec")) dt.month = 12;

    p = strchr(end+1, ' '); p++;
    end = strchr(p, ' ');
    memset(tmp, 0, 5);
    strncpy(tmp, p, end-p);
    dt.year = atoi(tmp);

    p = end+1;
    end = strchr(p, ':');
    memset(tmp, 0, 5);
    strncpy(tmp, p, end-p);
    dt.hour = atoi(tmp);

    p = end+1;
    end = strchr(p, ':');
    memset(tmp, 0, 5);
    strncpy(tmp, p, end-p);
    dt.minute = atoi(tmp);

    p = end+1;
    end = strchr(p, ' ');
    memset(tmp, 0, 5);
    strncpy(tmp, p, end-p);
    dt.second = atoi(tmp);

    end = strchr(p, '\"');
    adj = strchr(p, '+');
    if(adj && end>adj)
    {
        tmp[0] = adj[1];
        tmp[1] = adj[2];
        tmp[2] = '\0';

        dt.hour -= atoi(tmp);
    }
    adj = strchr(p, '-');
    if(adj && end>adj)
    {
        tmp[0] = adj[1];
        tmp[1] = adj[2];
        tmp[2] = '\0';

        dt.hour += atoi(tmp);
    }

    return dt;
}

void MainWindow::myprintfs(const char *format, char *param)
{
    char buffer[BSIZE]= "";
    sprintf(buffer, format, param);
    ui->plainTextEdit->appendPlainText(QString::fromUtf8(buffer));
}

void MainWindow::myprintfi(const char *format, int param)
{
    char buffer[BSIZE]= "";
    sprintf(buffer, format, param);
    ui->plainTextEdit->appendPlainText(QString::fromUtf8(buffer));
}

void MainWindow::myprintff(const char *format, float param)
{
    char buffer[BSIZE]= "";
    sprintf(buffer, format, param);
    ui->plainTextEdit->appendPlainText(QString::fromUtf8(buffer));
}

void MainWindow::myprintfis(const char *format, int p1, char *p2)
{
    char buffer[BSIZE]= "";
    sprintf(buffer, format, p1, p2);
    ui->plainTextEdit->appendPlainText(QString::fromUtf8(buffer));
}

void MainWindow::myprintfii(const char *format, int p1, int p2)
{
    char buffer[BSIZE]= "";
    sprintf(buffer, format, p1, p2);
    ui->plainTextEdit->appendPlainText(QString::fromUtf8(buffer));
}

void MainWindow::on_button_Clear_clicked()
{
    ui->plainTextEdit->clear();
}

void MainWindow::on_button_Exit_clicked()
{
    systrayIcon->hide();
    QApplication::exit();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    this->hide();
    event->ignore();
}

void MainWindow::on_button_Check_clicked()
{
    CheckAll();
}

void MainWindow::on_button_Settings_clicked()
{
    int chsave = i_ischecking;
    i_ischecking = 2;
    OptionsDialog *pdialog = new OptionsDialog(0, am, sm);
    pdialog->setModal(true);
    if(pdialog->exec()) sm->t_ignore = 0;

    delete pdialog;

    int iv = sm->GetTimerInterval();

    if((iv*1000) != qt->interval())
    {
        qt->setInterval(iv*1000);
        myprintfi("Timer interval changed to %d seconds.", iv);
    }
    i_ischecking = chsave;
}

void MainWindow::on_button_Hide_clicked()
{
    this->close();
}

void MainWindow::on_button_Animate_clicked()
{
    if(i_isanimating) return;
    i_isanimating = 1;
    aqt = new QTimer();
    connect(aqt, SIGNAL(timeout()), this, SLOT(AnimTimerFunc()));
    aqt->start(500);
}

void MainWindow::on_button_AnimStop_clicked()
{
    i_isanimating = 0;
    if(aqt)
    {
        aqt->stop();
        delete aqt; aqt = nullptr;
    }
    if(!i_networkerror) systrayIcon->setIcon(QIcon("./images/tray1.png"));
    else systrayIcon->setIcon(QIcon("./images/tray7.png"));
}

int MainWindow::IsIgnored(datetime dt)
{
    struct tm cmp;
    memset(&cmp, 0, sizeof(cmp));

    time_t rawtime;
    struct tm *timeinfo = nullptr;

    time(&rawtime);
    timeinfo = gmtime(&rawtime);

    cmp.tm_hour = dt.hour;
    if(timeinfo->tm_isdst) cmp.tm_isdst = 1;
    cmp.tm_min = dt.minute;
    cmp.tm_sec = dt.second;

    cmp.tm_year = dt.year-1900;
    cmp.tm_mon = dt.month-1;
    cmp.tm_mday = dt.day;

    time_t t = mktime(&cmp);
    t -= timezone;

    if(t>sm->t_ignore) return 0;
    else return 1;
}

void MainWindow::on_button_Unignore_clicked()
{
    sm->t_ignore = 0;
}

void MainWindow::IconSetMailCount(int count)
{
    if(i_networkerror)
    {
        systrayIcon->setIcon(QIcon("./images/tray7.png"));
        return;
    }
    char iconname[256]; memset(iconname, 0, 20);

    if(count<26) sprintf(iconname, "./images/tray5_%d.png", count);
    else sprintf(iconname, "./images/tray6.png");

    if(count<1) sprintf(iconname, "./images/tray1.png");

    systrayIcon->setIcon(QIcon(iconname));
}

void MainWindow::on_button_Disable_clicked()
{
    i_ischecking = 2;
    myprintfs("Checking is now disabled.", nullptr);
}

void MainWindow::on_button_Enable_clicked()
{
    i_ischecking = 0;
    myprintfs("Checking is now enabled.", nullptr);
}

void MainWindow::IconClicked(QSystemTrayIcon::ActivationReason reason)
{
    if(i_ischecking) return;
    if(reason == QSystemTrayIcon::MiddleClick)
    {
        CheckAll();
    }
    else if(reason == QSystemTrayIcon::DoubleClick)
    {
        char *lcmd = (char*)sm->GetLastCommand();
        if(!lcmd) return;
        char cmd[256];
        char *p, *start = lcmd;
        while(1)
        {
            memset(cmd, 0, 256);
            p = strchr(start, '\n');
            strncpy(cmd, start, p-start);

            if(tolower(cmd[0]) == 'h' && tolower(cmd[1]) == 't' && tolower(cmd[2]) == 't' &&
                    tolower(cmd[3]) == 'p' && tolower(cmd[4]) == ':' &&
                    tolower(cmd[5]) == '/' && tolower(cmd[6]) == '/')
                QDesktopServices::openUrl(QUrl(cmd));
            else QProcess::execute(cmd);

            if((*(p+1)) == '\0') break;
            else start = p+1;
        }
        systrayIcon->setIcon(QIcon("./images/tray1.png"));
    }
}

int MainWindow::IsRunning()
{
    return 0;
/*
    char *ptr;
    QString path = ptr = GetLockFilePath();
    if(ptr) free(ptr);
    QDir *dir = new QDir(path);
    dir->mkdir(path);
    delete dir;
#ifdef _WIN32
    SetFileAttributes(path.toStdWString().c_str(), FILE_ATTRIBUTE_HIDDEN);
#endif

    path += "/locked";

    FILE *fp = fopen(path.toStdString().c_str(), "rt");
    if(!fp)
    {
        fp = fopen(path.toStdString().c_str(), "wt");
        fclose(fp);
        return 0;
    }

    fclose(fp);
    return 1;
*/
}

char* MainWindow::GetLockFilePath()
{
    QString s = QDir::homePath() + "/.mchecker";
    char *ret = (char *)calloc(256, sizeof(char));
    strcpy(ret, s.toStdString().c_str());

    return ret;
}

void MainWindow::ClearLock()
{
    char *ptr;
    QString path = ptr = GetLockFilePath();
    if(ptr) free(ptr);
    QDir *dir = new QDir(path);
    dir->mkdir(path);
    delete dir;
#ifdef _WIN32
    SetFileAttributes(path.toStdWString().c_str(), FILE_ATTRIBUTE_HIDDEN);
#endif

    path += "/locked";

    QFile *f = new QFile(path);
    f->remove();
    delete f;
}

/*char *MainWindow::decode(char *in)
{
    return in;

  char enc[64], *i, *is, *o, *os, *p1, *p2;
  iconv_t handle;
  size_t ileft, oleft, ret;
  int type, index;

  if(in[0] != '=' || in[1] != '?')
  {
      i = (char *)malloc(256 * sizeof(char));
      memset(i, 0, 256);
      strcpy(i, in);
      return i;
  }

  is = i = (char *)malloc(256 * sizeof(char));
  os = o = (char *)malloc(256 * sizeof(char));
  memset(i, 0, 256);
  memset(o, 0, 256);

  printf("input:\t%s\n", in);

  p1 = in+2; p1 = strchr(p1, '?');
  memset(enc, 0, 64);
  strncat(enc, in+2, p1-in-2);

  printf("encoding:\t%s\n", enc);

  if(*(p1+1) == 'q' || *(p1+1) == 'Q')
  {
      type = 0;
      printf("encoding style: Q\n");
  }
  else
  {
      type = 1;
      printf("encoding style: B\n");
  }

  memset(i, 0, 128);

  while(1)
  {
    p2 = strchr(p1+1, '?'); p2++;
    p1 = strstr(p2, "?=");

    strncat(i, p2, p1-p2);

    p1 = strchr(p2, ' ');
    if(!p1)
    {
        p1 = strchr(p2, '\t');
        if(!p1) break;
    }
    p1 = strchr(p1, '?'); p1++;
    p1 = strchr(p1, '?'); if(!p1) break;

  }
  printf("encoded test:\t%s\n", i);

  index = 0;
  if(!type)
  {
    for(size_t x = 0; x<strlen(i); x++)
        if(i[x] == '=')
        {
            char str1[64];
            memset(str1, 0, 64);
            sprintf(str1, "0x%c%c", i[x+1], i[x+2]);
            o[index++] = (int)strtol(str1, 0, 0);
            x+=2;
        }
        else if(i[x] == '_')
        {
            o[index++] = 0x20;
        }
        else
        {
            o[index++] = i[x];
        }
  }
  else
  {
      Base64decode(o, i);
  }

  handle = iconv_open("UTF-8", enc);
  if(handle==(iconv_t)-1)
  {
    printf("iconv error!\n");
    return 0;
  }

  oleft = strlen(o);
  ileft = 256;
  memset(i, 0, 256);

  ret = iconv(handle, &o, &oleft, &i, &ileft);
  if(ret==(size_t)-1)
  {
    printf("conversion error!\n");
    switch(errno)
    {
    case E2BIG:
      printf("There is not sufficient room at *outbuf.\n");
      break;
    case EILSEQ:
      printf("An invalid multibyte sequence has been encountered in the input.\n");
      break;
    case EINVAL:
      printf("An incomplete multibyte sequence has been encountered in the input.\n");
      break;
    default:
      printf("unknown error\n");
      break;
    }
  }

  iconv_close(handle);

  printf("decoded test:\t%s\n", is);

  printf("\n");
  free(os);
  return is;

}

int MainWindow::Base64decode(char *bufplain, const char *bufcoded)
{
    static const unsigned char pr2six[256] =
    {
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
        64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
        64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
    };

    int nbytesdecoded;
    register const unsigned char *bufin;
    register unsigned char *bufout;
    register int nprbytes;

    bufin = (const unsigned char *) bufcoded;
    while (pr2six[*(bufin++)] <= 63);
    nprbytes = (bufin - (const unsigned char *) bufcoded) - 1;
    nbytesdecoded = ((nprbytes + 3) / 4) * 3;

    bufout = (unsigned char *) bufplain;
    bufin = (const unsigned char *) bufcoded;

    while (nprbytes > 4) {
    *(bufout++) =
        (unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
    *(bufout++) =
        (unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
    *(bufout++) =
        (unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
    bufin += 4;
    nprbytes -= 4;
    }

    if (nprbytes > 1) {
    *(bufout++) =
        (unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
    }
    if (nprbytes > 2) {
    *(bufout++) =
        (unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
    }
    if (nprbytes > 3) {
    *(bufout++) =
        (unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
    }

    *(bufout++) = '\0';
    nbytesdecoded -= (4 - nprbytes) & 3;
    return nbytesdecoded;
}
*/
