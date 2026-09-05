#include "vfs.h"
#include <libk.h>

static vfs_node_t nodes[VFS_NODE_MAX];
static int vfs_initialized = 0;

void vfs_init(void) {
    if (vfs_initialized) return;
    for (int i = 0; i < VFS_NODE_MAX; i++) {
        nodes[i].active = 0;
        nodes[i].id = 0;
        nodes[i].is_console = 0;
    }
    vfs_initialized = 1;
}

int vfs_register(vfs_node_id_t id, const char *name, drv_type_t type, device_t *dev) {
    int i;
    for (i = 0; i < VFS_NODE_MAX; i++) {
        if (!nodes[i].active) break;
    }
    if (i >= VFS_NODE_MAX) return -1;

    nodes[i].id = id;
    nodes[i].type = type;
    nodes[i].device = dev;
    nodes[i].active = 1;
    nodes[i].is_console = 0;

    int j = 0;
    while (name[j] && j < VFS_NAME_MAX - 1) {
        nodes[i].name[j] = name[j];
        j++;
    }
    nodes[i].name[j] = 0;

    if (id == VFS_NODE_STDOUT || id == VFS_NODE_STDERR || id == VFS_NODE_CONSOLE) {
        nodes[i].is_console = 1;
    }

    return 0;
}

vfs_node_t *vfs_get(vfs_node_id_t id) {
    for (int i = 0; i < VFS_NODE_MAX; i++) {
        if (nodes[i].active && nodes[i].id == id) {
            return &nodes[i];
        }
    }
    return 0;
}

vfs_node_t *vfs_get_by_name(const char *name) {
    if (!name) return 0;
    for (int i = 0; i < VFS_NODE_MAX; i++) {
        if (!nodes[i].active) continue;
        int j = 0;
        while (name[j] && nodes[i].name[j] == name[j] && j < VFS_NAME_MAX - 1) j++;
        if (name[j] == 0 && nodes[i].name[j] == 0) {
            return &nodes[i];
        }
    }
    return 0;
}

int vfs_write(vfs_node_id_t id, const void *buf, uint64_t len) {
    vfs_node_t *node = vfs_get(id);
    if (!node || !node->device) return -1;

    if (node->is_console) {
        device_t *dev = node->device;
        if (!dev) return -1;

        int total = 0;

        device_t *vga = dev_get_by_type(DRV_TYPE_DISPLAY);
        if (vga) {
            int n = drv_call_write(vga, buf, len);
            if (n > 0) total = n;
        }

        device_t *serial = dev_get_by_type(DRV_TYPE_SERIAL);
        if (serial) {
            int n = drv_call_write(serial, buf, len);
            if (n > 0) total = n;
        }

        return total;
    }

    return drv_call_write(node->device, buf, len);
}

int vfs_read(vfs_node_id_t id, void *buf, uint64_t len) {
    vfs_node_t *node = vfs_get(id);
    if (!node || !node->device) return -1;
    return drv_call_read(node->device, buf, len);
}

int vfs_ioctl(vfs_node_id_t id, uint32_t cmd, uint64_t arg) {
    vfs_node_t *node = vfs_get(id);
    if (!node || !node->device) return -1;
    return drv_call_ioctl(node->device, cmd, arg);
}

int vfs_write_str(vfs_node_id_t id, const char *s) {
    int len = 0;
    while (s[len]) len++;
    return vfs_write(id, s, len);
}

void vfs_list(void) {
    kset_color(0x0F);
    kputs("VFS Nodes:\n");
    for (int i = 0; i < VFS_NODE_MAX; i++) {
        if (nodes[i].active) {
            const char *dev_name = "none";
            if (nodes[i].device) {
                int j = 0;
                while (nodes[i].device->name[j] && j < 31) j++;
                dev_name = nodes[i].device->name;
            }
            kprintf("  [%d] /dev/%s -> %s%s\n",
                    nodes[i].id,
                    nodes[i].name,
                    dev_name,
                    nodes[i].is_console ? " (console)" : "");
        }
    }
}
