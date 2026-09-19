#include <stdint.h>

typedef struct __attribute__((packed)) {
    char     hardware_id[10] __attribute__((nonstring));
    char     app_version[10] __attribute__((nonstring));
} app_metadata_t;

#define HARDWARE_ID_SIZE (sizeof(((app_metadata_t*)0)->hardware_id))
#define APP_VERSION_SIZE (sizeof(((app_metadata_t*)0)->app_version))
