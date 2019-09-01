#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "console.h"
#include <QLabel>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), m_console(new Console)
{
    QTextEdit *textEdit = new QTextEdit(this);

    ui->setupUi(this);

    ui->textEdit->setStyleSheet("background-color:black;");
    ui->textEdit->setTextColor("yellow");

    ui->textEdit->append("QT program test1");
    ui->textEdit->append("QT program test1");
    ui->textEdit->append("QT program test1");
    ui->textEdit->append("QT program test1");
    ui->textEdit->append("QT program test1");
}

MainWindow::~MainWindow()
{
    delete ui;
}
