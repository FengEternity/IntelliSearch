/*
* Author: Montee
 * CreateData: 2024-12-22
 * UpdateData: 2024-12-22
 * LastUpdateContent: 初始化
 * Description: 搜索引擎核心主函数
 */

#include<iostream>

#include "Logger.h"

int main()
{
    std::cout<<"Hello World!"<<std::endl;

    LogConfig config = {"info", "log/intelliSearch_engine.log", 1048576 * 5, 3};
    INITLOG(config);

    INFOLOG("Hello World!");
    return 0;
}