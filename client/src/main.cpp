/*
* Author: Montee
 * CreateData: 2024-12-20
 * UpdateData: 2024-12-20
 * Description: 客户端主函数
 */

#include <QApplication>

#include "mainWindow.h"

int main(int argc, char* argv[])
{
 QApplication app(argc, argv);
 mainWindow w;
 w.show();

 return app.exec();
}
