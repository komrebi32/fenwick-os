#ifndef VFS_H
#define VFS_H

#include <stddef.h>

#include "drv/drv.h"

#define VFS_NODE_MAX  16
#define VFS_PATH_MAX  64
#define VFS_NAME_MAX  32

typedef enum {
    VFS_NODE_DISPLAY = 1,
    VFS_NODE_SERIAL,
    VFS_NODE_KBD,
    VFS_NODE_RTC,
    VFS_NODE_PIC,
    VFS_NODE_RAM,
    VFS_NODE_CONSOLE,
    VFS_NODE_STDOUT,
    VFS_NODE_STDERR,
    VFS_NODE_KBDIN,
} vfs_node_id_t;

typedef struct vfs_node {
    char           name[VFS_NAME_MAX];
    vfs_node_id_t  id;
    drv_type_t     type;
    device_t      *device;
    uint8_t        active;
    uint8_t        is_console;
    uint8_t        reserved[2];
} vfs_node_t;

void vfs_init(void);
int  vfs_register(vfs_node_id_t id, const char *name, drv_type_t type, device_t *dev);
int  vfs_write(vfs_node_id_t id, const void *buf, uint64_t len);
int  vfs_read(vfs_node_id_t id, void *buf, uint64_t len);
int  vfs_ioctl(vfs_node_id_t id, uint32_t cmd, uint64_t arg);
vfs_node_t *vfs_get(vfs_node_id_t id);
vfs_node_t *vfs_get_by_name(const char *name);

int  vfs_write_str(vfs_node_id_t id, const char *s);

void vfs_list(void);

#endif
