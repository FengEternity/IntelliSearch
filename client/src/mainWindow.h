/*
* Author: Montee
 * CreateData: 2024-12-20
 * UpdateData: 2024-12-20
 * Description: 项目客户端主窗口
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>


QT_BEGIN_NAMESPACE

namespace Ui
{
    class mainWindow;
}

QT_END_NAMESPACE

class mainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit mainWindow(QWidget* parent = nullptr);
    ~mainWindow() override;

private:
    Ui::mainWindow* ui;
};


#endif //MAINWINDOW_H
