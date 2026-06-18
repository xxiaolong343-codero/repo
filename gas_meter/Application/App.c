/**
 * @file App.c
 * @brief System entry point
 */
#include "TaskManager.h"

int main(void)
{
    /* HAL init is called inside TaskManager_Init */
    TaskManager_Init();
    TaskManager_Run();   /* Never returns */
    return 0;
}
