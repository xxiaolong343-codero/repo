/**
 * @file TaskConfig.h
 * @brief Task configuration persistence to E2
 */
#ifndef TASK_CONFIG_H
#define TASK_CONFIG_H

#include "hal_common.h"

#define TASK_CONFIG_VERSION  1

void TaskConfig_Save(void);
void TaskConfig_Load(void);

#endif /* TASK_CONFIG_H */
