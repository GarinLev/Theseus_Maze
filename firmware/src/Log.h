#ifndef FIRMWARE_LOG_H
#define FIRMWARE_LOG_H

// 1. Добавляем макрос отключения логов
#define DEBUGLOG_DISABLE

// Это можно оставить или закомментировать — при DEBUGLOG_DISABLE оно игнорируется
#ifndef DEBUGLOG_DEFAULT_LOG_LEVEL_TRACE
#define DEBUGLOG_DEFAULT_LOG_LEVEL_TRACE
#endif

// 2. Подключаем саму библиотеку
#include <DebugLog.h>

#endif
