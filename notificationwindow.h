#ifndef NOTIFICATIONWINDOW_H
#define NOTIFICATIONWINDOW_H

#include "structs.h"
#include "settingsmanager.h"
#include <QDialog>
#include <QTimer>

namespace Ui {
class NotificationWindow;
}

class NotificationWindow : public QDialog
{
    Q_OBJECT

public:
    explicit NotificationWindow(QWidget *parent = 0, snippet_container* psn = 0, int cnt = 0, SettingsManager *sm = 0);
    ~NotificationWindow();

private slots:
    void TimerFunc();
    void on_button_Command_clicked();
    void on_button_Ignore_clicked();
    void on_button_Close_clicked();

private:
    Ui::NotificationWindow *ui;

protected:
    void paintEvent(QPaintEvent *event);
    QTimer *timer;
    int cntdown;
    char *pcommandlist;
    SettingsManager *sm_saved;
};

#endif // NOTIFICATIONWINDOW_H
