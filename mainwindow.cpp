#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "seed_relay.h"
#include <QTextEdit>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow), relay(new seed_relay)
{
    ui->setupUi(this);
    QTextEdit *textEdit = new QTextEdit(this);

    ui->textEdit->setStyleSheet("background-color:black;");
    ui->textEdit->setTextColor("yellow");
    relay->port_reset();

//    relay->port_reset();

//    relay->work(CH1_DETECT, CH_ON);
//    relay->work(CH1_DETECT, CH_OFF);

//    relay->work(CH2_WORK, CH_ON);
//    relay->work(CH2_WORK, CH_OFF);

//    relay->work(CH3_THIRD, CH_ON);
//    relay->work(CH3_THIRD, CH_OFF);

//    ui->textEdit->append("QT program test1");
//    ui->textEdit->append("QT program test1");
//    ui->textEdit->append("QT program test1");
//    ui->textEdit->append("QT program test1");
//    ui->textEdit->append("QT program test1");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::PrintMessage(quint8 messageType)
{

}
