#include "commandeditdialog.h"
#include "ui_commandeditdialog.h"

#include <QMetaEnum>
#include <QStringListModel>

CommandEditDialog::CommandEditDialog(TSCCommandPtr cmd, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CommandEditDialog)
{
    ui->setupUi(this);

    paramStuff += QPair<QComboBox*, QSpinBox*>(ui->cbParamType1, ui->sbParamLen1);
    paramStuff += QPair<QComboBox*, QSpinBox*>(ui->cbParamType2, ui->sbParamLen2);
    paramStuff += QPair<QComboBox*, QSpinBox*>(ui->cbParamType3, ui->sbParamLen3);
    paramStuff += QPair<QComboBox*, QSpinBox*>(ui->cbParamType4, ui->sbParamLen4);

    ui->leCode->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    QListIterator<QPair<TSCCommand::ParameterType, QString>> it(TSCCommand::paramTypeNames);
    for (int i = 0; i < paramStuff.size(); i++) {
        paramStuff[i].first->clear();
        it.toFront();
        while (it.hasNext()) {
            const QPair<TSCCommand::ParameterType, QString> &next = it.next();
            paramStuff[i].first->addItem(next.second, next.first);
        }
    }

    ui->leCode->setText(cmd->code);
    ui->leName->setText(cmd->name);
    ui->teDescription->setPlainText(cmd->description);

    for (int i = 0; i < paramStuff.size(); i++) {
        paramStuff[i].first->setCurrentIndex(paramStuff[i].first->findData(cmd->params[i].first));
        paramStuff[i].second->setValue(cmd->params[i].second);
    }

    ui->cbEndsEvent->setChecked(cmd->endsEvent);
    ui->cbClearsTextbox->setChecked(cmd->clearsTextbox);
    ui->cbParamsAreSeparated->setChecked(cmd->paramsAreSeparated);
}

CommandEditDialog::~CommandEditDialog()
{
    delete ui;
}

void CommandEditDialog::on_btnCancel_clicked()
{
    reject();
}

void CommandEditDialog::on_btnOK_clicked()
{
    TSCCommandPtr newCmd = TSCCommandPtr(new TSCCommand);

    newCmd->code = ui->leCode->text();
    newCmd->name = ui->leName->text();
    newCmd->description = ui->teDescription->toPlainText();

    for (int i = 0; i < paramStuff.size(); i++) {
        newCmd->params[i].first = static_cast<TSCCommand::ParameterType>(paramStuff[i].first->currentData().toUInt());
        newCmd->params[i].second = paramStuff[i].second->value();
    }

    newCmd->endsEvent = ui->cbEndsEvent->isChecked();
    newCmd->clearsTextbox = ui->cbClearsTextbox->isChecked();
    newCmd->paramsAreSeparated = ui->cbParamsAreSeparated->isChecked();

    emit commandReady(this, newCmd);
}

void CommandEditDialog::on_cbParamType1_currentIndexChanged(int index)
{
    (void)index;
    paramStuffToggle(0, ui->cbParamType1->currentData() != TSCCommand::None);
}

void CommandEditDialog::on_cbParamType2_currentIndexChanged(int index)
{
    (void)index;
    paramStuffToggle(1, ui->cbParamType2->currentData() != TSCCommand::None);
}

void CommandEditDialog::on_cbParamType3_currentIndexChanged(int index)
{
    (void)index;
    paramStuffToggle(2, ui->cbParamType3->currentData() != TSCCommand::None);
}

void CommandEditDialog::on_cbParamType4_currentIndexChanged(int index)
{
    (void)index;
    paramStuffToggle(3, ui->cbParamType4->currentData() != TSCCommand::None);
}

void CommandEditDialog::paramStuffToggle(int i, bool b)
{
    paramStuff[i].second->setEnabled(b);
    if (i < 3) {
        paramStuff[i + 1].first->setEnabled(b);
        if (!b)
            paramStuffToggle(i + 1, false);
        else
            paramStuffToggle(i + 1, paramStuff[i + 1].first->currentData() != TSCCommand::None);
    }
}
