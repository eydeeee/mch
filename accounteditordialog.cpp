#define _CRT_SECURE_NO_WARNINGS

#include "accounteditordialog.h"
#include "ui_accounteditordialog.h"

#include <QMessageBox>
#include <QFileDialog>

AccountEditorDialog::AccountEditorDialog(QWidget *parent, acclist *account) :
    QDialog(parent),
    ui(new Ui::AccountEditorDialog)
{
    ui->setupUi(this);
    account_saved = account;

    if(account)
    {
        ui->line_FriendlyName->setText(QString::fromUtf8(account->s_friendlyname));
        ui->line_LoginName->setText(QString::fromUtf8(account->s_loginname));
        ui->line_Password->setText(QString::fromUtf8(account->s_password));
        ui->line_Domain->setText(QString::fromUtf8(account->s_domain));
        ui->line_Command->setText(QString::fromUtf8(account->s_command));
        char tmp[256]; sprintf(tmp, "%d", account->i_port);
        ui->line_Port->setText(QString::fromUtf8(tmp));
        if(account->i_ssl) ui->check_Secure->setChecked(true);
        else ui->check_Secure->setChecked(false);
    }
}

AccountEditorDialog::~AccountEditorDialog()
{
    delete ui;
}

void AccountEditorDialog::on_button_Cancel_clicked()
{
    this->setResult(0);
    this->close();
}

void AccountEditorDialog::on_button_Save_clicked()
{
    account_saved->i_port = ui->line_Port->text().toInt();
    if(ui->check_Secure->isChecked()) account_saved->i_ssl = 1;
    else account_saved->i_ssl = 0;
    strcpy(account_saved->s_domain, (char*)ui->line_Domain->text().toStdString().c_str());
    strcpy(account_saved->s_command, (char*)ui->line_Command->text().toStdString().c_str());
    strcpy(account_saved->s_friendlyname, (char*)ui->line_FriendlyName->text().toStdString().c_str());
    strcpy(account_saved->s_loginname, (char*)ui->line_LoginName->text().toStdString().c_str());
    strcpy(account_saved->s_password, (char*)ui->line_Password->text().toStdString().c_str());

    this->accept();
}

void AccountEditorDialog::on_line_Port_editingFinished()
{
    char s[256] = "";
    strcpy(s, ui->line_Port->text().toStdString().c_str());
    for(int i=0; i<(int)strlen(s); i++)
        if(!isdigit(s[i]))
        {
            QMessageBox::warning(this, QString::fromUtf8("Warning"),
                QString::fromUtf8("Port should be a number!"), QMessageBox::Ok, 0);
            ui->button_Save->setEnabled(false);
            break;
        }
        else ui->button_Save->setEnabled(true);
}

void AccountEditorDialog::on_button_Browse_clicked()
{
    QString result;

    result = QFileDialog::getOpenFileName(this, QString(), QString(),
        QString::fromUtf8("All files (*)"),
        0, 0);
    if(result != "") ui->line_Command->setText(result.toUtf8());
}

void AccountEditorDialog::on_button_gmail_clicked()
{
    ui->line_FriendlyName->setText(QString::fromUtf8("GMail"));
    ui->line_Domain->setText(QString::fromUtf8("imap.gmail.com"));
    ui->line_Port->setText(QString::fromUtf8("993"));
    ui->check_Secure->setChecked(true);
    ui->line_Command->setText(QString::fromUtf8("http://mail.google.com"));
}




void AccountEditorDialog::on_button_outlook_clicked()
{
    ui->line_FriendlyName->setText(QString::fromUtf8("Outlook.com"));
    ui->line_Domain->setText(QString::fromUtf8("imap-mail.outlook.com"));
    ui->line_Port->setText(QString::fromUtf8("993"));
    ui->check_Secure->setChecked(true);
    ui->line_Command->setText(QString::fromUtf8("http://www.hotmail.com"));
}
