#pragma once

enum TYPE {
    HARDLINK,
    SYMBOL_LINK,
    FILE,
    OBJECT,
    LINK
};

typedef struct vfs_entry
{
    char name[255];
    int size;
    char owner[255];
    int mode;
    TYPE type;
    void *data;
    open_t open;
    write_t write;
} vfs_entry_t;

typedef vfs_entry_t *(*open_t)(vfs_entry_t *vfs, const char *path);
typedef int(*write_t)(vfs_entry_t *vfs, void *data, int len);

extern vfs_entry_t *root_node;