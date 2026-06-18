/**
 * @file <模块名>.h
 * @brief <模块> 组件公开 API
 *
 * @version 1.0
 * @date YYYY-MM-DD
 * @author 作者姓名
 *
 * @copyright Copyright (c) YYYY 公司名称
 */

#ifndef <模块名>_H
#define <模块名>_H

/* ---- 标准库头文件 ---- */
#include <stdint.h>
#include <stdbool.h>

/* ---- 项目头文件 ---- */
#include "hal_common.h"

/* ---- 其他模块头文件（如有依赖） ---- */
/* #include "other_module.h" */

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 *  公开类型定义
 * ============================================================================ */

/**
 * @brief <模块> 状态枚举
 */
typedef enum {
    <模块名>_STATE_IDLE = 0,    /**< 空闲状态 */
    <模块名>_STATE_ACTIVE,      /**< 活动状态 */
    <模块名>_STATE_ERROR        /**< 错误状态 */
} <模块名>State_t;

/**
 * @brief <模块> 配置参数
 */
typedef struct {
    uint32_t param1;    /**< 参数1说明 */
    uint16_t param2;    /**< 参数2说明 */
    uint8_t  param3;    /**< 参数3说明 */
} <模块名>Params_t;

/* ============================================================================
 *  公开函数声明
 * ============================================================================ */

/**
 * @brief 初始化 <模块>
 *
 * 详细描述初始化过程。
 *
 * @note 必须在调用其他 <模块> 函数之前调用。
 */
void <模块名>_Init(void);

/**
 * @brief 处理任务（由 TaskManager 调用）
 */
void <模块名>_Process(void);

/**
 * @brief 获取当前状态
 * @return 当前状态
 */
<模块名>State_t <模块名>_GetState(void);

/**
 * @brief 设置配置参数
 * @param params 参数指针
 * @return HAL_OK 成功，HAL_ERROR 失败
 */
HAL_Status_t <模块名>_SetParams(const <模块名>Params_t *params);

/**
 * @brief 获取配置参数
 * @return 参数指针
 */
const <模块名>Params_t* <模块名>_GetParams(void);

#ifdef __cplusplus
}
#endif

#endif /* <模块名>_H */