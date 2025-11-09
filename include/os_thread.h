#pragma once

#ifdef ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef TaskHandle_t os_thread_t;
typedef void *(*os_thread_func_t)(void *);

static inline int os_thread_create(os_thread_t *thread, os_thread_func_t func, void *arg) {
    BaseType_t ret = xTaskCreate(
        (TaskFunction_t)func,
        "task",
        4096,
        arg,
        5,
        thread
    );
    return ret == pdPASS ? 0 : -1;
}

static inline int os_thread_join(os_thread_t thread) {
    while (eTaskGetState(thread) != eDeleted) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    return 0;
}

#else
#include <pthread.h>

typedef pthread_t os_thread_t;
typedef void *(*os_thread_func_t)(void *);

static inline int os_thread_create(os_thread_t *thread, os_thread_func_t func, void *arg) {
    return pthread_create(thread, NULL, func, arg);
}

static inline int os_thread_join(os_thread_t thread) {
    return pthread_join(thread, NULL);
}

#endif
