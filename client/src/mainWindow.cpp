//
// Created by montee on 24-12-20.
//

#include <QMessageBox>

#include "mainWindow.h"
#include "ui_mainWindow.h"


mainWindow::mainWindow(QWidget* parent) :
    QWidget(parent), ui(new Ui::mainWindow)
{
    ui->setupUi(this);

    connect(ui->sendButton, &QPushButton::clicked, this, &mainWindow::on_sendButton_clicked);
}

mainWindow::~mainWindow()
{
    delete ui;
}

/*
 * Summary: 点击发送按钮
 * Parameters:
 * Return: void
 */
void mainWindow::on_sendButton_clicked()
{
    QString message = ui->inputLineEdit->text();

    if (message.isEmpty())
    {
        QMessageBox::warning(this, "警告", "输入不能为空！");
    }

    ui->chatHistoryBrowser->append(message);
    ui->inputLineEdit->clear();
}
