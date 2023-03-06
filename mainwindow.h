#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QFileDialog>
#include "abstractcnc.h"
#include "brothercnc.h"
#include "fanuccnc.h"
#include "mitsubishicnc.h"
#include "siemenscnc.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_comboBox_cnctype_currentIndexChanged(int index);

    void on_connectButton_clicked();

    void on_downButton_clicked();

    void on_upButton_clicked();

    void on_deleteButton_clicked();

    void on_fileTree_itemDoubleClicked(QTreeWidgetItem *item, int column);

private:
    Ui::MainWindow *ui;
    AbstractCNC *ThisCNC;
    bool CNCconnected;
    int cnctype;
    QString allInfo;

    void SetNetInfo(QString info);

    QString OldPath;
};
#endif // MAINWINDOW_H
