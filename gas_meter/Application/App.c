/**
 * @file App.c
 * @brief 系统入口 — 硬件初始化 + 调度器启动
 *
 * 本文件为智能燃气表嵌入式系统的 main() 入口：
 * 1. 调用 TaskManager_Init() 初始化调度器（含 HAL 初始化）
 * 2. 调用 TaskManager_Run() 进入主循环（永不返回）
 */
#include "TaskManager.h"

int main(void)
{
    /* HAL 初始化在 TaskManager_Init 内部完成 */
    TaskManager_Init();

    /* 进入调度器主循环，永不返回 */
    TaskManager_Run();

    return 0;
}
