#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>

#include "accountmanager.h"
#include "settingsmanager.h"
#include "accounteditordialog.h"

namespace Ui {
class OptionsDialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(QWidget *parent = 0, AccountManager *am = 0, SettingsManager *sm = 0);
    ~OptionsDialog();
    int m_timer;

private slots:
    void on_button_Cancel_clicked();
    void on_button_Edit_clicked();
    void on_button_Save_clicked();
    void on_button_Add_clicked();
    void on_button_Delete_clicked();
    void on_button_Browse_clicked();

    void on_button_Play_clicked();

private:
    Ui::OptionsDialog *ui;
    AccountManager *am_stored;
    SettingsManager *sm_stored;
    void UpdateAcclist();

};

#endif // OPTIONSDIALOG_H
