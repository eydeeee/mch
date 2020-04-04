#define _CRT_SECURE_NO_WARNINGS

#include "notificationwindow.h"
#include "ui_notificationwindow.h"

#include <QRect>
#include <QDesktopWidget>
#include <QPainter>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QFont>

NotificationWindow::NotificationWindow(QWidget *parent, snippet_container *psn, int cnt, SettingsManager *sm) :
    QDialog(parent),
    ui(new Ui::NotificationWindow)
{
    ui->setupUi(this);

    if(sm->GetDecreaseFont())
    {
        QFont *f = new QFont(ui->tableWidget->font());
        f->setPointSize(8);
        ui->tableWidget->setFont(*f);
        delete f;
    }

    pcommandlist = 0;
    sm_saved = sm;

    this->setAttribute(Qt::WA_ShowWithoutActivating);

    this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint |
        Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint | Qt::SplashScreen);

#ifdef _WIN32
    ShowWindow((HWND)this->winId(), SW_HIDE);
    SetWindowLong((HWND)this->winId(), GWL_EXSTYLE, GetWindowLong((HWND)this->winId(), GWL_EXSTYLE)
                  | WS_EX_NOACTIVATE);
    SetWindowLong((HWND)this->winId(), GWL_EXSTYLE, GetWindowLong((HWND)this->winId(), GWL_EXSTYLE)
                  & ~WS_EX_CLIENTEDGE);
    SetWindowLong((HWND)this->winId(), GWL_EXSTYLE, GetWindowLong((HWND)this->winId(), GWL_EXSTYLE)
                  & ~WS_EX_WINDOWEDGE);
    ShowWindow((HWND)this->winId(), SW_SHOW);
#endif

    QRect pos = frameGeometry();
    switch(sm->GetLocation())
    {
    case 1:
        pos.moveBottomRight(QPoint(QDesktopWidget().availableGeometry().width(), QDesktopWidget().availableGeometry().height()+this->height()));
        pos.setLeft(QDesktopWidget().availableGeometry().width()-this->width());
        pos.setTop(0);
        this->move(pos.topLeft());
        break;
    case 2:
        pos.moveBottomRight(QPoint(this->width(), QDesktopWidget().availableGeometry().height()-this->height()));
        pos.setLeft(0);
        pos.setTop(QDesktopWidget().availableGeometry().height()-this->height());
        this->move(pos.topLeft());
        break;
    case 3:
        pos.moveBottomRight(QPoint(this->width(), this->height()));
        pos.setLeft(0);
        pos.setTop(0);
        this->move(pos.topLeft());
        break;
    case 4:
        pos.moveBottomRight(QPoint(QDesktopWidget().availableGeometry().width()/2 + this->width()/2, QDesktopWidget().availableGeometry().height()/2 + this->height()/2));
        pos.setLeft(QDesktopWidget().availableGeometry().width()/2-this->width()/2);
        pos.setTop(QDesktopWidget().availableGeometry().height()/2-this->height()/2);
        this->move(pos.topLeft());
        break;
    default:
        pos.moveBottomRight(QDesktopWidget().availableGeometry().bottomRight());
        pos.setLeft(QDesktopWidget().availableGeometry().width()-this->width());
        pos.setTop(QDesktopWidget().availableGeometry().height()-this->height());
        this->move(pos.topLeft());
        break;
    }

    ui->tableWidget->setColumnWidth(0, 65);
    ui->tableWidget->setColumnWidth(1, 100);
    ui->tableWidget->setColumnWidth(2, 150);
    ui->tableWidget->setColumnWidth(3, 65);

    if(psn && cnt>0)
    {
        for(int i=0; i<cnt; i++)
        {
            if(psn[i].count > 0)
            {
                if(!pcommandlist)
                {
                    pcommandlist = (char*)calloc(strlen(psn[i].cmd)+2, sizeof(char));
                    if(pcommandlist)
                    {
                        strcat(pcommandlist, psn[i].cmd);
                        strcat(pcommandlist, "\n");
                    }
                }
                else
                {
                    if(!strstr(pcommandlist, psn[i].cmd))
                    {
                        pcommandlist = (char*)realloc(pcommandlist, strlen(pcommandlist)+1 + strlen(psn[i].cmd)+2);
                        if(pcommandlist)
                        {
                            strcat(pcommandlist, psn[i].cmd);
                            strcat(pcommandlist, "\n");
                        }
                    }
                }

                sm_saved->SetLastCommand(pcommandlist);


                for(int j=0; j<psn[i].count; j++)
                {
                    ui->tableWidget->setRowCount(ui->tableWidget->rowCount()+1);
                    ui->tableWidget->setRowHeight(ui->tableWidget->rowCount()-1, 20);
                    ui->tableWidget->setItem(ui->tableWidget->rowCount()-1,
                        0, new QTableWidgetItem(QString::fromUtf8(psn[i].accname)));
                    ui->tableWidget->setItem(ui->tableWidget->rowCount()-1,
                        1, new QTableWidgetItem(QString::fromUtf8(psn[i].snippets[j].from)));
                    ui->tableWidget->setItem(ui->tableWidget->rowCount()-1,
                        2, new QTableWidgetItem(QString::fromUtf8(psn[i].snippets[j].subject)));
                    ui->tableWidget->setItem(ui->tableWidget->rowCount()-1,
                        3, new QTableWidgetItem(QString::fromUtf8(psn[i].snippets[j].size)));
                }
            }
        }
    }

    cntdown = sm_saved->GetNotificationTimeout();
    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(TimerFunc()));
    timer->start(1000);
}

NotificationWindow::~NotificationWindow()
{
    if(pcommandlist) free(pcommandlist);
    timer->stop();
    delete timer;
    delete ui;
}

void NotificationWindow::on_button_Command_clicked()
{
    if(!pcommandlist) return;
    char cmd[256];
    char *p, *start = pcommandlist;
    while(1)
    {
        memset(cmd, 0, 256);
        p = strchr(start, '\n');
        strncpy(cmd, start, p-start);

        if(tolower(cmd[0]) == 'h' && tolower(cmd[1]) == 't' && tolower(cmd[2]) == 't' &&
                tolower(cmd[3]) == 'p' && tolower(cmd[4]) == ':' &&
                tolower(cmd[5]) == '/' && tolower(cmd[6]) == '/')
            QDesktopServices::openUrl(QUrl(QString::fromUtf8(cmd)));
        else QProcess::execute(QString::fromUtf8(cmd));

        if((*(p+1)) == '\0') break;
        else start = p+1;
    }
    on_button_Ignore_clicked();

    time((time_t*)&sm_saved->t_ignore);
    this->accept();
}

void NotificationWindow::on_button_Ignore_clicked()
{
    time((time_t*)&sm_saved->t_ignore);

    this->hide();
}

void NotificationWindow::on_button_Close_clicked()
{
    on_button_Ignore_clicked();
}

void NotificationWindow::paintEvent(QPaintEvent *event)
{
    QDialog::paintEvent(event);

    QPainter painter(this);

    painter.setPen(QColor(10,10,10));
    for(int i=0; i<3; i++)
    {
        painter.drawLine(0, i, this->width(), i);
        painter.drawLine(0, this->height()-i, this->width(), this->height()-i);

        painter.drawLine(0+i, 0, 0+i, this->height());
        painter.drawLine(this->width()-1, 0, this->width()-i, this->height());
    }
}

void NotificationWindow::TimerFunc()
{
    char s[32];
    sprintf(s, "Closing in %d s...", cntdown);
    ui->label_Counter->setText(QString::fromUtf8(s));
    cntdown--;

    if(cntdown<=0) on_button_Ignore_clicked();
}
