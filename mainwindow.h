#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QFile>
#include "tsccommand.h"
#include "commandeditdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    bool fileLoaded;
    QList<TSCCommandPtr> commands;
    QStandardItemModel *lvCmdsModel;
    bool unsavedMods;
    QFile *lastSaveLocation;

    void newFile();
    bool loadFile(QFile *src, QString *error);
    bool saveFile(QFile *dst, QString *error);
    void unloadFile();

    void updateWidgetStates();
    void syncCommandsModel();

    bool promptUnsavedMods();

    void closeEvent(QCloseEvent *event);

private slots:
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_btnAdd_clicked();
    void on_btnRemove_clicked();
    void on_btnEdit_clicked();
    void on_actionUnload_triggered();
    void on_actionExit_triggered();
    void on_lvCmds_doubleClicked(const QModelIndex &index);

    void commandReady(CommandEditDialog *ced, TSCCommandPtr newCmd);

    void on_btnSort_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
