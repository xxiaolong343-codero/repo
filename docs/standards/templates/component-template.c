/**
 * @file <模块名>.c
 * @brief <模块> 组件实现
 *
 * @version 1.0
 * @date YYYY-MM-DD
 * @author 作者姓名
 */

/* ---- 对应头文件（最先） ---- */
#include "<模块名>.h"

/* ---- 标准库头文件 ---- */
#include <string.h>

/* ---- 项目头文件 ---- */
#include "TaskManager.h"
/* #include "Alarm.h" */  /* 如有依赖 */

/* ============================================================================
 *  私有宏定义
 * ============================================================================ */

#define <模块名>_MAX_COUNT    100
#define <模块名>_TIMEOUT_MS   1000

/* ============================================================================
 *  私有类型定义
 * ============================================================================ */

/**
 * @brief 内部状态结构
 */
typedef struct {
    <模块名>State_t state;
    uint32_t        counter;
    uint16_t        timer;
    bool            flag;
} <模块名>Internal_t;

/* ============================================================================
 *  私有变量
 * ============================================================================ */

static <模块名>Internal_t s_internal;
static <模块名>Params_t   s_params = {
    .param1 = 1000,
    .param2 = 100,
    .param3 = 10
};

/* ============================================================================
 *  私有函数
 * ============================================================================ */

/**
 * @brief 状态转换处理
 * @param new_state 新状态
 */
static void transition_state(<模块名>State_t new_state)
{
    if (s_internal.state == new_state) {
        return;
    }

    /* 退出旧状态 */
    switch (s_internal.state) {
    case <模块名>_STATE_IDLE:
        /* 清理 */
        break;
    case <模块名>_STATE_ACTIVE:
        /* 清理 */
        break;
    default:
        break;
    }

    /* 进入新状态 */
    s_internal.state = new_state;
    s_internal.timer = 0;

    switch (new_state) {
    case <模块名>_STATE_IDLE:
        /* 初始化 */
        break;
    case <模块名>_STATE_ACTIVE:
        /* 初始化 */
        break;
    default:
        break;
    }
}

/**
 * @brief 内部处理函数
 */
static void process_internal(void)
{
    /* 实现细节 */
}

/* ============================================================================
 *  公开函数实现
 * ============================================================================ */

void <模块名>_Init(void)
{
    memset(&s_internal, 0, sizeof(s_internal));
    s_internal.state = <模块名>_STATE_IDLE;

    /* 其他初始化 */
}

void <模块名>_Process(void)
{
    switch (s_internal.state) {
    case <模块名>_STATE_IDLE:
        /* 空闲处理 */
        break;

    case <模块名>_STATE_ACTIVE:
        process_internal();
        break;

    case <模块名>_STATE_ERROR:
        /* 错误处理 */
        break;

    default:
        s_internal.state = <模块名>_STATE_IDLE;
        break;
    }
}

<模块名>State_t <模块名>_GetState(void)
{
    return s_internal.state;
}

HAL_Status_t <模块名>_SetParams(const <模块名>Params_t *params)
{
    if (params == NULL) {
        return HAL_ERROR;
    }

    /* 参数校验 */
    if (params->param1 == 0) {
        return HAL_ERROR;
    }

    s_params = *params;
    return HAL_OK;
}

const <模块名>Params_t* <模块名>_GetParams(void)
{
    return &s_params;
}