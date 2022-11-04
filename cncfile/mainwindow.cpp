#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->ThisCNC = Q_NULLPTR;
    this->allInfo = "";

    //fanuc
    ui->ipEdit->setText("192.168.44.128");
    ui->comboBox_cnctype->setCurrentIndex(1);
    ui->userLabel->hide();
    ui->userEdit->hide();
    ui->passLabel->hide();
    ui->passEdit->hide();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SetNetInfo(QString info)
{
    allInfo += info;
    if(allInfo.size()>=10000) {
        allInfo = allInfo.mid(allInfo.size()-10000);
    }
    ui->infoEdit->setText(allInfo);
    QTextCursor cursor = ui->infoEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->infoEdit->setTextCursor(cursor);
}

void MainWindow::on_comboBox_cnctype_currentIndexChanged(int index)
{
    if(index == 0 || index == 3)
    {
        ui->userLabel->show();
        ui->userEdit->show();
        ui->passLabel->show();
        ui->passEdit->show();
    }
    else
    {
        ui->userLabel->hide();
        ui->userEdit->hide();
        ui->passLabel->hide();
        ui->passEdit->hide();
    }
}

void MainWindow::on_connectButton_clicked()
{
    if(!CNCconnected)
    {
        cnctype = ui->comboBox_cnctype->currentIndex();
        QString ip = ui->ipEdit->text();
        QString user = ui->userEdit->text();
        QString pass = ui->passEdit->text();
        switch (cnctype) {
            case 0:{
                ThisCNC = new BrotherCNC(ip,user,pass);
            }
            break;
            case 1:{
                ThisCNC = new FanucCNC(ip,user,pass);
            }
            break;
        }
        if(ThisCNC)
        {
            bool tryconnect = ThisCNC->Connect();
            if(tryconnect)
            {
                ui->comboBox_cnctype->setEnabled(false);
                CNCconnected = true;
                SetNetInfo("connect success!\n");
                ui->connectButton->setText("DisConnect");


                QStringList Files = ThisCNC->GetSubItemInfoOfADir("");
                foreach(QString file,Files)
                {
                    QStringList fileinfo = file.split("|");
                    if(fileinfo.count() > 1)
                    {
                        QTreeWidgetItem* item = new QTreeWidgetItem(ui->fileTree);
                        item->setText(0, fileinfo[0]);
                        item->setText(1, fileinfo[1]);
                        ui->fileTree->addTopLevelItem(item);
                    }
                }
            }
            else
            {
                SetNetInfo("connect failed!\n");
            }
        }
    }
    else
    {
        if(ThisCNC)
        {
            ThisCNC->DisConnect();
            ThisCNC = Q_NULLPTR;
            CNCconnected = false;
            ui->comboBox_cnctype->setEnabled(true);
            SetNetInfo("disconnect~\n");
            ui->connectButton->setText("Connect");
            ui->fileTree->clear();
        }
    }
}

void MainWindow::on_downButton_clicked()
{
    QTreeWidgetItem* curItem = ui->fileTree->currentItem();
    if(curItem && CNCconnected)
    {
        QString type = curItem->text(0);
        if(type=="Dir")
            return;
        QString downName = curItem->text(1);
        QString saveDir = QFileDialog::getExistingDirectory(this, "Choose save path");
        QString filecode = ThisCNC->GetNcProgramByPath(downName);
        QFile file;
        file.setFileName(saveDir + "/" + downName);
        if(file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream stream(&file);
            stream << filecode;
            file.close();
        }
        SetNetInfo("Download " + downName + " finished~\n");
    }
}

void MainWindow::on_upButton_clicked()
{
    QString localFile = QFileDialog::getOpenFileName(this, "Choose the file to upload");
    QString filecode = "";
    QFile file;
    file.setFileName(localFile);
    if(file.open(QIODevice::ReadOnly))
    {
        QByteArray bytes = file.readAll();
        filecode = QString::fromLatin1(bytes,bytes.size());
    }
    int p = localFile.lastIndexOf("/");
    QString fileName = localFile.mid(p + 1);
    if(CNCconnected)
    {
        bool success = ThisCNC->SetNcProgramByPath(filecode,fileName);
        if(success)
        {
            SetNetInfo("Upload " + fileName + " finished~\n");
        }

        ui->fileTree->clear();
        QStringList Files = ThisCNC->GetSubItemInfoOfADir("");
        foreach(QString file,Files)
        {
            QStringList fileinfo = file.split("|");
            if(fileinfo.count() > 1)
            {
                QTreeWidgetItem* item = new QTreeWidgetItem(ui->fileTree);
                item->setText(0, fileinfo[0]);
                item->setText(1, fileinfo[1]);
                ui->fileTree->addTopLevelItem(item);
            }
        }
    }
    file.close();
}

void MainWindow::on_deleteButton_clicked()
{
    QTreeWidgetItem* curItem = ui->fileTree->currentItem();
    if(curItem && CNCconnected)
    {
        QString type = curItem->text(0);
        if(type=="Dir")
            return;
        QString downName = curItem->text(1);
        bool success = ThisCNC->DelNcProgramByPath(downName);
        if(success)
        {
            SetNetInfo("Delete " + downName + " finished~\n");
        }

        ui->fileTree->clear();
        QStringList Files = ThisCNC->GetSubItemInfoOfADir("");
        foreach(QString file,Files)
        {
            QStringList fileinfo = file.split("|");
            if(fileinfo.count() > 1)
            {
                QTreeWidgetItem* item = new QTreeWidgetItem(ui->fileTree);
                item->setText(0, fileinfo[0]);
                item->setText(1, fileinfo[1]);
                ui->fileTree->addTopLevelItem(item);
            }
        }
    }
}

void MainWindow::on_fileTree_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    column = column > 0 ? column : 0;
    QString type = item->text(0);
    if(type!="Dir")
        return;
    if(CNCconnected)
    {
        QString path = item->text(1);
        ui->fileTree->clear();
        QStringList Files = ThisCNC->GetSubItemInfoOfADir(path);
        foreach(QString file,Files)
        {
            QStringList fileinfo = file.split("|");
            if(fileinfo.count() > 1)
            {
                QTreeWidgetItem* item = new QTreeWidgetItem(ui->fileTree);
                item->setText(0, fileinfo[0]);
                item->setText(1, fileinfo[1]);
                ui->fileTree->addTopLevelItem(item);
            }
        }
    }
}
