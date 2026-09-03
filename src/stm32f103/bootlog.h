#pragma once

typedef enum {
    BOOTLOG_MSG_TYPE_INFO,
    BOOTLOG_MSG_TYPE_HIGHLIGHTED,
    BOOTLOG_MSG_TYPE_ERROR,
} bootlog_msg_type_t;

extern void bootlog_add(const char* msg, bootlog_msg_type_t type);
