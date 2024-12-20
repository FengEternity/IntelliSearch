//
// Created by montee on 24-12-20.
//

#include "mainWindow.h"
#include "ui_mainWindow.h"


mainWindow::mainWindow(QWidget* parent) :
    QWidget(parent), ui(new Ui::mainWindow)
{
    ui->setupUi(this);
}

mainWindow::~mainWindow()
{
    delete ui;
}
