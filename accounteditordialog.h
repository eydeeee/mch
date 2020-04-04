#ifndef ACCOUNTEDITORDIALOG_H
#define ACCOUNTEDITORDIALOG_H

#include <QDialog>

#include "accountmanager.h"

namespace Ui {
class AccountEditorDialog;
}

class AccountEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AccountEditorDialog(QWidget *parent = 0, acclist *account = 0);
    ~AccountEditorDialog();

private slots:
    void on_button_Cancel_clicked();

    void on_button_Save_clicked();

    void on_line_Port_editingFinished();

    void on_button_Browse_clicked();

    void on_button_gmail_clicked();

    void on_button_outlook_clicked();

private:
    Ui::AccountEditorDialog *ui;
    acclist *account_saved;
};

#endif // ACCOUNTEDITORDIALOG_H
