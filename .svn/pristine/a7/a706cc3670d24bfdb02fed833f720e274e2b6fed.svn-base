/*
 *  ion.c
 *
 * Memory Allocator functions for ion
 *
 *   Copyright 2011 Google, Inc
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
//#define LOG_NDEBUG 0

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <ionapi.h>

static int g_ion_fd = -1;

int ion_open()
{
    int fd = open("/dev/ion", O_RDWR);
    if (fd < 0)
        printf("open /dev/ion failed!\n");
    return fd;
}

int ion_close(int fd)
{
    int ret = close(fd);
    if (ret < 0)
        return -errno;
    return ret;
}

static int ion_ioctl(int fd, int req, void *arg)
{
    int ret = ioctl(fd, req, arg);
    if (ret < 0) {
        printf("ioctl %x failed with code %d: %s\n", req,
              ret, strerror(errno));
        return -errno;
    }
    return ret;
}

int ion_alloc(int fd, size_t len, size_t align, unsigned int heap_mask,
              unsigned int flags, ion_user_handle_t *handle)
{
        int ret;
        struct ion_allocation_data data = {
                .len = len,
                .align = align,
                .heap_id_mask = heap_mask,
                .flags = flags,
        };

        if (handle == NULL)
            return -EINVAL;

        ret = ion_ioctl(fd, ION_IOC_ALLOC, &data);
        if (ret < 0){
                printf("ION_IOC_ALLOC failed: len:%zu align:%zu heap:%x flags:%x", len, align, heap_mask, flags);
                return ret;
        }
        *handle = data.handle;
//        ALOGV("%s handle:%p\n", __func__, *handle);
        return ret;
}

int ion_alloc_tiler(int fd, size_t w, size_t h, int fmt, unsigned int size, unsigned int flags,
            ion_user_handle_t *handle, size_t *stride)
{
        int ret;
        struct rtk_phoenix_ion_tiler_alloc_data alloc_data = {
                .w = w,
                .h = h,
                .fmt = fmt,
                .flags = flags,
                .size = size,
        };

        struct ion_custom_data custom_data = {
                .cmd = RTK_PHOENIX_ION_TILER_ALLOC,
                .arg = (unsigned long)(&alloc_data),
        };

        ret = ion_ioctl(fd, ION_IOC_CUSTOM, &custom_data);
        if (ret < 0)
                return ret;
        *stride = alloc_data.stride;
        *handle = alloc_data.handle;

        return ret;
}

int ion_free(int fd, ion_user_handle_t handle)
{
    struct ion_handle_data data = {
        .handle = handle,
    };
    return ion_ioctl(fd, ION_IOC_FREE, &data);
}

int ion_map(int fd, ion_user_handle_t handle, size_t length, int prot,
            int flags, off64_t offset, unsigned char **ptr, int *map_fd)
{
    int ret;
    unsigned char *tmp_ptr;
    struct ion_fd_data data = {
        .handle = handle,
    };

    if (map_fd == NULL)
        return -EINVAL;
    if (ptr == NULL)
        return -EINVAL;

    ret = ion_ioctl(fd, ION_IOC_MAP, &data);
    if (ret < 0)
        return ret;
    if (data.fd < 0) {
        printf("map ioctl returned negative fd\n");
        return -EINVAL;
    }
    tmp_ptr = mmap(NULL, length, prot, flags, data.fd, offset);
    if (tmp_ptr == MAP_FAILED) {
        printf("mmap failed: %s\n", strerror(errno));
        return -errno;
    }
    *map_fd = data.fd;
    *ptr = tmp_ptr;
    return ret;
}

int ion_share(int fd, ion_user_handle_t handle, int *share_fd)
{
    int ret;
    struct ion_fd_data data = {
        .handle = handle,
    };

    if (share_fd == NULL)
        return -EINVAL;

    ret = ion_ioctl(fd, ION_IOC_SHARE, &data);
    if (ret < 0)
        return ret;
    if (data.fd < 0) {
        printf("share ioctl returned negative fd\n");
        return -EINVAL;
    }
    *share_fd = data.fd;
    return ret;
}

int ion_alloc_fd(int fd, size_t len, size_t align, unsigned int heap_mask,
                 unsigned int flags, int *handle_fd) {
    ion_user_handle_t handle;
    int ret;

    ret = ion_alloc(fd, len, align, heap_mask, flags, &handle);
    if (ret < 0)
        return ret;
    ret = ion_share(fd, handle, handle_fd);
    ion_free(fd, handle);
    return ret;
}

int ion_import(int fd, int share_fd, ion_user_handle_t *handle)
{
    int ret;
    struct ion_fd_data data = {
        .fd = share_fd,
    };

    if (handle == NULL)
        return -EINVAL;

    ret = ion_ioctl(fd, ION_IOC_IMPORT, &data);
    if (ret < 0)
        return ret;
    *handle = data.handle;
    return ret;
}

int ion_sync_fd(int fd, int handle_fd)
{
    struct ion_fd_data data = {
        .fd = handle_fd,
    };
    return ion_ioctl(fd, ION_IOC_SYNC, &data);
}

#if 1 //20130208 charleslin
int ion_phys(int fd, ion_user_handle_t handle, unsigned long *addr, unsigned int *size)
{
    int ret;
    struct ion_phys_data data = {
        .handle = handle,
        .addr = 0,
        .len = 0,
    };
    ret = ion_ioctl(fd, ION_IOC_PHYS, &data);
    if (ret < 0){
        printf("ION_IOC_PHYS failed");
        return ret;
    }
    *addr = data.addr;
    *size = data.len;
//    ALOGV("%s handle:%p addr:%lx size:%x\n", __func__, handle, data.addr, data.len);
    return ret;
}

int ion_last_alloc_addr(int fd, unsigned long *addr, unsigned int *_size)
{
    int ret;
    unsigned int buf[2];//alloc_addr;
    struct ion_custom_data custom_data = {
                .cmd = RTK_PHOENIX_ION_GET_LAST_ALLOC_ADDR,
//                .arg = (unsigned long)(&alloc_addr),
                .arg = (unsigned long)(&buf[0]),
        };

        ret = ion_ioctl(fd, ION_IOC_CUSTOM, &custom_data);
        if (ret < 0)
                return ret;
//        *addr = alloc_addr;
        printf("in ion_last_alloc_addr  buf[0]=0x%x buf[1]=0x%x\n", buf[0], buf[1]);
        *addr = buf[0];
        *_size = buf[1];

        return ret;
}

//used to get physical address of ion memory by shared fd
int ion_get_phys_from_shared_fd(int share_fd, unsigned long *addr, unsigned int *size)
{
    int ret;
    ion_user_handle_t handle;

    if(g_ion_fd == -1)
        g_ion_fd = ion_open();
    if(g_ion_fd < 0)
        return g_ion_fd;
    printf("g_ion_fd:%d heapID:%d", g_ion_fd, share_fd);

    ret = ion_import(g_ion_fd, share_fd, &handle);
    if(ret != 0)
        return ret;
//    ALOGV("handle:%p", handle);

    ret = ion_phys(g_ion_fd, handle, addr, size);
    if(ret != 0)
        return ret;
    printf("get ion memory addr:%lx size:%d", *addr, *size);

    return 0;
}
#endif
