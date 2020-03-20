#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QCloseEvent>
#include <QFileDialog>
#include <QTextStream>
#include <QMetaEnum>
#include <QDebug>
#include "htmldelegate.h"
#include "commandeditdialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    newFile();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::newFile()
{
    fileLoaded = true;
    commands.clear();
    lastSaveLocation = nullptr;
    updateWidgetStates();
}

static const QStringList cmdParts = {
    "Code",
    "Parameter count",
    "Parameter types",
    "Name",
    "Description",
};

static const QStringList cmdPartsExtended = {
    "'Ends event' flag",
    "'Clears textbox' flag",
    "'Parameters are separated' flag",
    "Parameter 1 length",
    "Parameter 2 length",
    "Parameter 3 length",
    "Parameter 4 length",
};

bool MainWindow::loadFile(QFile *src, QString *fail)
{
    if (!src->open(QFile::ReadOnly)) {
        *fail = "Could not open file for reading";
        return false;
    }
    QTextStream ts(src);
    QString line;
    // look for header
    bool ok;
    bool gotHeader = false;
    bool extendedFormat;
    uint cmdCount;
    QRegExp reHeader("\\[(CE|BL)_TSC\\]\\s+(\\d+)");
    while (ts.readLineInto(&line)) {
        if (reHeader.exactMatch(line)) {
            extendedFormat = QString("BL").compare(reHeader.cap(1)) == 0;
            cmdCount = reHeader.cap(2).toUInt(&ok);
            if (!ok) {
                *fail = QString("Couldn't read command count (\"%1\") in header").arg(reHeader.cap(2));
                src->close();
                return false;
            }
            gotHeader = true;
            break;
        }
    }
    if (!gotHeader) {
        *fail = "Could not find [CE_TSC]/[BL_TSC] header";
        src->close();
        return false;
    }
    // setup some misc stuff
    QStringList codes;
    int partCount = cmdParts.size();
    QStringList partNames;
    partNames += cmdParts;
    if (extendedFormat) {
        partCount += cmdPartsExtended.size();
        partNames += cmdPartsExtended;
    }
    QMetaEnum paramTypeMeta = QMetaEnum::fromType<TSCCommand::ParameterType>();
    // start reading commands
    commands.clear();
    commands.reserve(cmdCount);
    TSCCommandPtr newCmd;
    for (uint i = 0; i < cmdCount; i++) {
        if (ts.atEnd()) {
            *fail = QString("Incorrect command count; claims there are %1 commands, but only has %2").arg(cmdCount).arg(i);
            src->close();
            return false;
        }
        newCmd = TSCCommandPtr(new TSCCommand);
        ts.readLineInto(&line);
        QStringList parts = line.split('\t');
        if (parts.size() < partCount) {
            QString cmdId = QString("#%1").arg(i + 1);
            if (parts.size() >= 1)
                cmdId = parts[0];
            *fail = QString("Command %1 has missing parts: %2").arg(cmdId).arg(partNames.mid(parts.size()).join(", "));
            src->close();
            return false;
        }
        newCmd->code = parts[0];
        int conflict;
        if ((conflict = codes.indexOf(newCmd->code)) < 0)
            codes += newCmd->code;
        else {
            *fail = QString("Commands #%1 and #%2 have same code %3").arg(conflict + 1).arg(i + 1).arg(newCmd->code);
            src->close();
            return false;
        }
        uint paramCount = parts[1].toUInt(&ok);
        if (!ok) {
            *fail = QString("Command %1 has unparsable number %2 in part %3").arg(newCmd->code).arg(parts[1]).arg(partNames[1]);
            src->close();
            return false;
        }
        if (paramCount > 4) {
            *fail = QString("Command %1 has too many parameters (%2 > 4)").arg(newCmd->code).arg(paramCount);
            src->close();
            return false;
        }
        QByteArray paramTypes = parts[2].toLatin1();
        for (uint j = 0; j < paramCount; j++) {
            char type = paramTypes[j];
            if (!paramTypeMeta.valueToKey(type)) {
                *fail = QString("Command %1 has unknown parameter type '%2' for parameter #%3").arg(newCmd->code).arg(type).arg(j + 1);
                src->close();
                return false;
            }
            newCmd->params[j].first = static_cast<TSCCommand::ParameterType>(type);
        }
        newCmd->name = parts[3];
        newCmd->description = parts[4];
        if (!extendedFormat) {
            commands += newCmd;
            continue;
        }
        newCmd->endsEvent = parts[5].toUInt(&ok) > 0;
        if (!ok) {
            *fail = QString("Command %1 has unparsable number %2 in part %3").arg(newCmd->code).arg(parts[5]).arg(partNames[5]);
            src->close();
            return false;
        }
        newCmd->clearsTextbox = parts[6].toUInt(&ok) > 0;
        if (!ok) {
            *fail = QString("Command %1 has unparsable number %2 in part %3").arg(newCmd->code).arg(parts[6]).arg(partNames[6]);
            src->close();
            return false;
        }
        newCmd->paramsAreSeparated = parts[7].toUInt(&ok) > 0;
        if (!ok) {
            *fail = QString("Command %1 has unparsable number %2 in part %3").arg(newCmd->code).arg(parts[7]).arg(partNames[7]);
            src->close();
            return false;
        }
        for (uint j = 0; j < paramCount; j++) {
            newCmd->params[j].second = parts[8 + j].toUInt(&ok);
            if (!ok) {
                *fail = QString("Command %1 has unparsable number %2 in part %3").arg(newCmd->code).arg(parts[8 + j]).arg(partNames[8 + j]);
                src->close();
                return false;
            }
            if (newCmd->params[j].second == 0 || newCmd->params[j].second > 4) {
                *fail = QString("Command %1 has bad parameter length for parameter #%2 (%3 == 0 or %3 > 4)").arg(newCmd->code).arg(j + 1).arg(newCmd->params[j].second);
                src->close();
                return false;
            }
        }
        commands += newCmd;
    }
    // done!
    src->close();
    lastSaveLocation = src;
    fileLoaded = true;
    updateWidgetStates();
    *fail = QString("Successfully loaded %1 commands").arg(commands.size());
    return true;
}

bool MainWindow::saveFile(QFile *dst, QString *fail)
{
    if (!dst->open(QFile::WriteOnly)) {
        *fail = "Could not open file for writing";
        return false;
    }
    QTextStream ts(dst);
    QList<TSCCommandPtr> commands = this->commands;
    commands.removeAll(nullptr);
    // write header
    ts << "[BL_TSC]\t" << commands.size() << endl;
    // write commands
    TSCCommandPtr cmd;
    foreach (cmd, commands) {
        ts << cmd->code << '\t';
        uint paramCount = 0;
        QByteArray paramTypes(4, '-');
        for (uint i = 0; i < 4; i++) {
            if (cmd->params[i].first == TSCCommand::None)
                break;
            paramCount++;
            paramTypes[i] = cmd->params[i].first;
        }
        ts << paramCount << '\t' << paramTypes << '\t';
        ts << cmd->name << '\t' << cmd->description;
        ts << '\t';
        if (cmd->endsEvent)
            ts << 1;
        else
            ts << 0;
        ts << '\t';
        if (cmd->clearsTextbox)
            ts << 1;
        else
            ts << 0;
        ts << '\t';
        if (cmd->paramsAreSeparated)
            ts << 1;
        else
            ts << 0;
        for (uint i = 0; i < 4; i++) {
            ts << '\t' << cmd->params[i].second;
        }
        ts << endl;
    }
    *fail = QString("Successfully saved %1 commands").arg(commands.size());
    dst->close();
    unsavedMods = false;
    return true;
}

void MainWindow::unloadFile()
{
    fileLoaded = false;
    commands.clear();
    lastSaveLocation = nullptr;
    updateWidgetStates();
}

void MainWindow::updateWidgetStates()
{
    syncCommandsModel();
    ui->actionSave->setEnabled(fileLoaded);
    ui->actionSaveAs->setEnabled(fileLoaded);
    ui->actionUnload->setEnabled(fileLoaded);
    ui->lvCmds->setEnabled(fileLoaded);
    ui->btnAdd->setEnabled(fileLoaded);
    ui->btnRemove->setEnabled(fileLoaded);
    ui->btnEdit->setEnabled(fileLoaded);
}

void MainWindow::syncCommandsModel()
{
    lvCmdsModel = new QStandardItemModel();
    for (int i = 0; i < commands.size(); i++) {
        TSCCommandPtr cmd = commands[i];
        QStandardItem *item = new QStandardItem;
        item->setText(QString("<code>%1</code> - %2").arg(cmd->code.toHtmlEscaped()).arg(cmd->name.toHtmlEscaped()));
        item->setToolTip(cmd->description);
        item->setData(i);
        lvCmdsModel->appendRow(item);
    }
    lvCmdsModel->sort(0);
    ui->lvCmds->setModel(lvCmdsModel);
    ui->lvCmds->setItemDelegate(new HTMLDelegate);
}

bool MainWindow::promptUnsavedMods()
{
    if (!fileLoaded || !unsavedMods)
        return true;
    switch (QMessageBox::question(this, "Unsaved changes", "Save changes to file?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel)) {
    case QMessageBox::Yes:
        on_actionSave_triggered();
        return true;
    case QMessageBox::No:
        return true;
    default:
        return false;
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->setAccepted(promptUnsavedMods());
}


void MainWindow::on_actionNew_triggered()
{
    if (!promptUnsavedMods())
        return;
    newFile();
}

void MainWindow::on_actionOpen_triggered()
{
    if (!promptUnsavedMods())
        return;
    QString filename = QFileDialog::getOpenFileName(this, "Open TSC list", "", "TSC list files (*.txt)");
    if (filename.isNull())
        return;
    QFile *file = new QFile(filename, this);
    QString fail;
    if (!loadFile(file, &fail))
        QMessageBox::critical(this, "Error while loading file", QString("Could not load TSC file:\n%1").arg(fail));
}

void MainWindow::on_actionSave_triggered()
{
    if (!lastSaveLocation) {
        on_actionSaveAs_triggered();
        return;
    }
    QString fail;
    if (!saveFile(lastSaveLocation, &fail))
        QMessageBox::critical(this, "Error while saving file", QString("Could not save TSC file:\n%1").arg(fail));
}

void MainWindow::on_actionSaveAs_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save TSC list", "", "TSC list files (*.txt)");
    if (filename.isNull())
        return;
    lastSaveLocation = new QFile(filename, this);
    QString fail;
    if (!saveFile(lastSaveLocation, &fail))
        QMessageBox::critical(this, "Error while saving file", QString("Could not save TSC file:\n%1").arg(fail));
}

#include <QDebug>

void MainWindow::on_btnAdd_clicked()
{
    qDebug() << 1;
    int i = commands.size();
    qDebug() << 2;
    TSCCommandPtr newCmd = TSCCommandPtr(new TSCCommand);
    qDebug() << 3;
    newCmd->code = "<NEW";
    qDebug() << 4;
    newCmd->name = "NEW command";
    qDebug() << 5;
    commands += newCmd;
    qDebug() << 6;
    QStandardItem *item = new QStandardItem;
    qDebug() << 7;
    item->setText(QString("<code>%1</code> - %2").arg(newCmd->code.toHtmlEscaped()).arg(newCmd->name.toHtmlEscaped()));
    qDebug() << 8;
    item->setToolTip(newCmd->description);
    qDebug() << 9;
    item->setData(i);
    qDebug() << 10;
    lvCmdsModel->appendRow(item);
    qDebug() << 11;
    QModelIndex ni = lvCmdsModel->indexFromItem(item);
    qDebug() << 12;
    ui->lvCmds->selectionModel()->select(ni, QItemSelectionModel::ClearAndSelect);
    qDebug() << 13;
    on_btnEdit_clicked();
    qDebug() << 14;
    lvCmdsModel->sort(0);
}

void MainWindow::on_btnRemove_clicked()
{
    QModelIndex di = ui->lvCmds->selectionModel()->selectedIndexes()[0];
    int i = di.data(Qt::UserRole + 1).toInt();
    commands[i] = nullptr;
    lvCmdsModel->removeRow(di.row());
}

void MainWindow::on_btnEdit_clicked()
{
    qDebug() << 1;
    int i = ui->lvCmds->selectionModel()->selectedIndexes()[0].data(Qt::UserRole + 1).toInt();
    qDebug() << 2;
    CommandEditDialog *ced = new CommandEditDialog(commands[i], this);
    qDebug() << 3;
    connect(ced, &CommandEditDialog::commandReady, this, &MainWindow::commandReady);
    qDebug() << 4;
    if (ced->exec() == QDialog::Accepted)
        lvCmdsModel->sort(0);
}

void MainWindow::on_actionUnload_triggered()
{
    if (!promptUnsavedMods())
        return;
    unloadFile();
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

void MainWindow::on_lvCmds_doubleClicked(const QModelIndex &index)
{
    ui->lvCmds->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
    on_btnEdit_clicked();
}

void MainWindow::commandReady(TSCCommandPtr newCmd)
{
    int i = ui->lvCmds->selectionModel()->selectedIndexes()[0].data(Qt::UserRole + 1).toInt();
    commands[i] = newCmd;
    QStandardItem *item = lvCmdsModel->item(i);
    item->setText(QString("<code>%1</code> - %2").arg(newCmd->code.toHtmlEscaped()).arg(newCmd->name.toHtmlEscaped()));
    item->setToolTip(newCmd->description);
    lvCmdsModel->sort(0);
    unsavedMods = true;
}
