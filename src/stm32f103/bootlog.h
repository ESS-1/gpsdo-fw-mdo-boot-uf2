#pragma once

typedef enum {
    BOOTLOG_MSG_TYPE_INFO,
    BOOTLOG_MSG_TYPE_HIGHLIGHTED,
    BOOTLOG_MSG_TYPE_ERROR,
} BootlogMsgType;

extern void bootlog_add(const char* msg, BootlogMsgType type);
