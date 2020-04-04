#include "optionsdialog.h"
#include "ui_optionsdialog.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QSound>
#include <QProcess>


OptionsDialog::OptionsDialog(QWidget *parent, AccountManager *am, SettingsManager *sm) :
    QDialog(parent),
    ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);
    am_stored = am;
    sm_stored = sm;

    ui->line_Sound->setText(QString::fromUtf8(sm->GetSoundFile()));
    ui->spinBox->setValue(sm->GetTimerInterval());
    ui->spin_Waitfirst->setValue(sm->GetFirstCheckInterval());
    ui->spin_Notificationtimeout->setValue(sm->GetNotificationTimeout());
    ui->comboBox->setCurrentIndex(sm->GetLocation());

    UpdateAcclist();
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

void OptionsDialog::on_button_Cancel_clicked()
{
    this->close();
}

void OptionsDialog::on_button_Edit_clicked()
{
    int idx;
    if(ui->listWidget->currentItem()) idx = ui->listWidget->currentIndex().row();
    else
    {
        QMessageBox::warning(0, QString::fromUtf8("Impossibru!"), QString::fromUtf8("No account selected!"));
        return;
    }

    AccountEditorDialog *dlg = new AccountEditorDialog(this, am_stored->GetAccountPtr(idx));
    dlg->setModal(true);
    if(dlg->exec()) sm_stored->t_ignore = 0;
    delete dlg;

    UpdateAcclist();
}

void OptionsDialog::UpdateAcclist()
{
    ui->listWidget->clear();
    for(int i = 0; i<am_stored->GetCount(); i++)
    {
        char *name = am_stored->GetFriendlyName(i);
        ui->listWidget->addItem(QString::fromUtf8(name));
        if(name) free(name);
    }
}

void OptionsDialog::on_button_Save_clicked()
{
    am_stored->SaveAccountData();

    sm_stored->SetTimerInterval(ui->spinBox->value());
    sm_stored->SetFirstCheckInterval(ui->spin_Waitfirst->value());
    sm_stored->SetNotificationTimeout(ui->spin_Notificationtimeout->value());
    sm_stored->SetSoundFile((char*)ui->line_Sound->text().toStdString().c_str());
    sm_stored->SetLocation(ui->comboBox->currentIndex());

    sm_stored->SaveSettings();
    this->accept();
}

void OptionsDialog::on_button_Add_clicked()
{
    acclist list; memset(&list, 0, sizeof(list));
    AccountEditorDialog *dlg = new AccountEditorDialog(this, &list);
    dlg->setModal(true);
    if(dlg->exec())
    {
        am_stored->AddAccount(&list);
        UpdateAcclist();
    }

    delete dlg;
}

void OptionsDialog::on_button_Delete_clicked()
{
    int idx;
    if(ui->listWidget->currentItem()) idx = ui->listWidget->currentIndex().row();
    else
    {
        QMessageBox::warning(0, QString::fromUtf8("Impossibru!"), QString::fromUtf8("No account selected!"));
        return;
    }

    am_stored->DeleteAccount(idx);

    UpdateAcclist();
}

void OptionsDialog::on_button_Browse_clicked()
{
    QString result;

    result = QFileDialog::getOpenFileName(this, QString(), QString(),
        QString::fromUtf8("WAV file (*.wav *.wave);;All files (*.*)"),
        0, 0);
    if(result != "") ui->line_Sound->setText(result.toUtf8());
}

void OptionsDialog::on_button_Play_clicked()
{
    QString s = ui->line_Sound->text().toUtf8();
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
