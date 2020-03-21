#ifndef COMMANDEDITDIALOG_H
#define COMMANDEDITDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include "tsccommand.h"

namespace Ui {
class CommandEditDialog;
}

class CommandEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CommandEditDialog(TSCCommandPtr cmd, QWidget *parent = nullptr);
    ~CommandEditDialog();

signals:
    void commandReady(CommandEditDialog *ced, TSCCommandPtr newCmd);

private slots:
    void on_btnCancel_clicked();
    void on_btnOK_clicked();
    void on_cbParamType1_currentIndexChanged(int index);
    void on_cbParamType2_currentIndexChanged(int index);
    void on_cbParamType3_currentIndexChanged(int index);
    void on_cbParamType4_currentIndexChanged(int index);

private:
    QList<QPair<QComboBox*, QSpinBox*>> paramStuff;
    Ui::CommandEditDialog *ui;
    void paramStuffToggle(int i, bool b);
};

#endif // COMMANDEDITDIALOG_H
