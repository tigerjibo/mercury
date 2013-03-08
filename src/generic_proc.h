/*
 * Copyright (C) 2013 Argonne National Laboratory, Department of Energy,
 *                    and UChicago Argonne, LLC.
 * Copyright (C) 2013 The HDF Group.
 * All rights reserved.
 *
 * The full copyright notice, including terms governing use, modification,
 * and redistribution, is contained in the COPYING file that can be
 * found at the root of the source code distribution tree.
 */

#ifndef GENERIC_PROC_H
#define GENERIC_PROC_H

#include "shipper_error.h"
#include "shipper_config.h"
#include "bulk_data_shipper.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifndef FS_INLINE
# if __GNUC__ && !__GNUC_STDC_INLINE__
#   define FS_INLINE extern inline
# else
#  define FS_INLINE inline
# endif
#endif

/* TODO using ifdef IOFSL_SHIPPER_HAS_XDR is dangerous for inline functions */

#ifdef IOFSL_SHIPPER_HAS_XDR
#include <rpc/types.h>
#include <rpc/xdr.h>
#endif

typedef void * fs_proc_t;

/*
 * Proc operations.  FS_ENCODE causes the type to be encoded into the
 * stream.  FS_DECODE causes the type to be extracted from the stream.
 * FS_FREE can be used to release the space allocated by an FS_DECODE
 * request.
 */
typedef enum {
    FS_ENCODE,
    FS_DECODE,
    FS_FREE
} fs_proc_op_t;

typedef struct fs_proc_buf {
    void    *buf;
    void    *buf_ptr;
    size_t   size;
    size_t   size_left;
    bool     is_mine;
    bool     is_used;
} fs_proc_buf_t;

typedef struct fs_priv_proc {
    fs_proc_op_t   op;
    fs_proc_buf_t *current_buf;
    fs_proc_buf_t  proc_buf;
    fs_proc_buf_t  extra_buf;
#ifdef IOFSL_SHIPPER_HAS_XDR
    XDR            proc_xdr;
    XDR            extra_xdr;
#endif
} fs_priv_proc_t;

typedef const char * fs_string_t;

#define BDS_MAX_HANDLE_SIZE 32 /* TODO Arbitrary value / may need to be increased depending on implementations */

#ifdef __cplusplus
extern "C" {
#endif

/* Create a new encoding/decoding processor from a given buffer */
int fs_proc_create(void *buf, size_t buf_len, fs_proc_op_t op, fs_proc_t *proc);

/* Free the processor */
int fs_proc_free(fs_proc_t proc);

/* Get total buffer size available for processing */
size_t fs_proc_get_size(fs_proc_t proc);

/* Request a new buffer size */
int fs_proc_set_size(fs_proc_t proc, size_t buf_len);

/* Get size left for processing (info) */
size_t fs_proc_get_size_left(fs_proc_t proc);

/* Get pointer to current buffer (for manual encoding) */
void * fs_proc_get_buf_ptr(fs_proc_t proc);

/* Set new buffer pointer (for manual encoding) */
int fs_proc_set_buf_ptr(fs_proc_t proc, void *buf_ptr);

/*---------------------------------------------------------------------------
 * Function:    fs_proc_string_hash
 *
 * Purpose:     Hash function name for unique ID to register
 *
 * Returns:     Non-negative on success or negative on failure
 *
 *---------------------------------------------------------------------------
 */
FS_INLINE int fs_proc_string_hash(const char *string)
{
    /* This is the djb2 string hash function */

    unsigned int result = 5381;
    unsigned char *p;

    p = (unsigned char *) string;

    while (*p != '\0') {
        result = (result << 5) + result + *p;
        ++p;
    }
    return result;
}

/*---------------------------------------------------------------------------
 * Function:    fs_proc_memcpy
 *
 * Purpose:     Generic processing routines using memcpy
 *
 * Returns:     Non-negative on success or negative on failure
 *
 *---------------------------------------------------------------------------
 */
FS_INLINE int fs_proc_memcpy(fs_proc_t proc, void *data, size_t data_size)
{
    fs_priv_proc_t *priv_proc = (fs_priv_proc_t*) proc;
    const void *src;
    void *dest;
    int ret = S_SUCCESS;

    if (priv_proc->op == FS_FREE) return ret;

    /* If not enough space allocate extra space if encoding or just get extra buffer if decoding */
    if (priv_proc->current_buf->size_left < data_size) {
        fs_proc_set_size(proc, priv_proc->proc_buf.size + priv_proc->extra_buf.size + data_size);
    }

    /* Process data */
    src = (priv_proc->op == FS_ENCODE) ? (const void *) data : (const void *) priv_proc->current_buf->buf_ptr;
    dest = (priv_proc->op == FS_ENCODE) ? priv_proc->current_buf->buf_ptr : data;
    memcpy(dest, src, data_size);
    priv_proc->current_buf->buf_ptr   += data_size;
    priv_proc->current_buf->size_left -= data_size;

    return ret;
}

/*---------------------------------------------------------------------------
 * Function:    fs_proc_int8_t
 *
 * Purpose:     Generic processing routines
 *
 * Returns:     Non-negative on success or negative on failure
 *
 *---------------------------------------------------------------------------
 */
FS_INLINE int fs_proc_int8_t  (fs_proc_t proc, int8_t *data)
{
    fs_priv_proc_t *priv_proc = (fs_priv_proc_t*) proc;
    int ret = S_FAIL;
#ifdef IOFSL_SHIPPER_HAS_XDR
    ret = xdr_int8_t(&priv_proc->xdr, data) ? S_SUCCESS : S_FAIL;
#else
    ret = fs_proc_memcpy(priv_proc, data, sizeof(int8_t));
#endif
    return ret;
}

/*---------------------------------------------------------------------------
 * Function:    fs_proc_uint8_t
 *
 * Purpose:     Generic processing routines
 *
 * Returns:     Non-negative on success or negative on failure
 *
 *---------------------------------------------------------------------------
 */
FS_INLINE int fs_proc_uint8_t  (fs_proc_t proc, uint8_t *data)
{
    fs_priv_proc_t *priv_proc = (fs_priv_proc_t*) proc;
    int ret = S_FAIL;
#ifdef IOFSL_SHIPPER_HAS_XDR
    ret = xdr_uint8_t(&priv_proc->xdr, data) ? S_SUCCESS : S_FAIL;
#else
    ret = fs_proc_memcpy(priv_proc, data, sizeof(uint8_t));
#endif
    return ret;
}

/*---------------------------------------------------------------------------
 * Function:    fs_proc_int16_t
 *
 * Purpose:     Generic processing routines
 *
 * Returns:     Non-negative on success or negative on failure
 *
 *---------------------------------------------------------------------------
 */
FS_INLINE int fs_proc_int16_t  (fs_proc_t proc, int16_t *data)
{
    fs_priv_proc_t *priv_proc = (fs_priv_proc_t*) proc;
    int ret = S_FAIL;
#ifdef IOFSL_SHIPPER_HAS_XDR
    ret = xdr_int16_t(&priv_proc->xdr, data) ? S_SUCCESS : S_FAIL;
#else
    ret = fs_proc_memcpy(priv_proc, data, sizeof(int16_t));
#endif
    return ret;
}

/*---------------------------------------------------------------------------
 * Function:    fs_proc_uint16_t
 *
 * Purpose:     Generic processing routines
 *
 * Returns:     Non-negative on success or negative on failure
 *
 *---------------------------------------------------------------------------
 */
FS_INLINE int fs_proc_uint16_t  (fs_proc_t proc, uint16_t *data)
{
    fs_priv_proc_t *priv_proc = (fs_priv_proc_t*) proc;
    int ret = S_FAIL;
#ifdef IOFSL_SHIPPER_HAS_XDR
    ret = xdr_uint16_t(&priv_proc->xdr, data) ? S_SUCCESS : S_FAIL;
#else
    ret = fs_proc_memcpy(priv_proc, data, sizeof(uint16_t));
#endif
    return ret;
}

/*---------------------------------------------------------------------------
 * Function:    fs_proc_int32_t
 *
 * Purpose:     Generic processing routines
 *
 * Returns:     Non-negative on success or negative on failure
 *
 *---------------------------------------------------------------------------
 */
FS_INLINE int fs_proc_int32_t  (fs_proc_t proc, int32_t *data)
{
    fs_priv_proc_t *priv_proc = (fs_priv_proc_t*) proc;
    int ret = S_FAIL;
#ifdef IOFSL_SHIPPER_HAS_XDR
    ret = xdr_int32_t(&priv_proc->xdr, data) ? S_SUCCESS : S_FAIL;
#else
    ret = fs_proc_memcpy(priv_proc, data, sizeof(int32_t));
#endif
    return ret;
}

/*---------------------------------------------------------------------------
 * Function:    fs_proc_uint32_t
 *
 * Purpose:     Generic processing routines
 *
 * Returns:     Non-negative on success or negative on failure
 *
 *---------------------------------------------------------------------------
 */
FS_INLINE int fs_proc_uint32_t  (fs_proc_t proc, uint32_t *data)
{
    fs_priv_proc_t *priv_proc = (fs_priv_proc_t*) proc;
    int ret = S_FAIL;
#ifdef IOFSL_SHIPPER_HAS_XDR
    ret = xdr_uint32_t(&priv_proc->xdr, data) ? S_SUCCESS : S_FAIL;
#else
    ret = fs_proc_memcpy(priv_proc, data, sizeof(uint32_t));
#endif
    return ret;
}

/*---------------------------------------------------------------------------
 * Function:    fs_proc_int64_t
 *
 * Purpose:     Generic processing routines
 *
 * Returns:     Non-negative on success or negative on failure
 *
 *---------------------------------------------------------------------------
 */
FS_INLINE int fs_proc_int64_t  (fs_proc_t proc, int64_t *data)
{
    fs_priv_proc_t *priv_proc = (fs_priv_proc_t*) proc;
    int ret = S_FAIL;
#ifdef IOFSL_SHIPPER_HAS_XDR
    ret = xdr_int64_t(&priv_proc->xdr, data) ? S_SUCCESS : S_FAIL;
#else
    ret = fs_proc_memcpy(priv_proc, data, sizeof(int64_t));
#endif
    return ret;
}

/*---------------------------------------------------------------------------
 * Function:    fs_proc_uint64_t
 *
 * Purpose:     Generic processing routines
 *
 * Returns:     Non-negative on success or negative on failure
 *
 *---------------------------------------------------------------------------
 */
FS_INLINE int fs_proc_uint64_t  (fs_proc_t proc, uint64_t *data)
{
    fs_priv_proc_t *priv_proc = (fs_priv_proc_t*) proc;
    int ret = S_FAIL;
#ifdef IOFSL_SHIPPER_HAS_XDR
    ret = xdr_uint64_t(&priv_proc->xdr, data) ? S_SUCCESS : S_FAIL;
#else
    ret = fs_proc_memcpy(priv_proc, data, sizeof(uint64_t));
#endif
    return ret;
}

/*---------------------------------------------------------------------------
 * Function:    fs_proc_raw
 *
 * Purpose:     Generic processing routines
 *
 * Returns:     Non-negative on success or negative on failure
 *
 *---------------------------------------------------------------------------
 */
FS_INLINE int fs_proc_raw  (fs_proc_t proc, void *buf, size_t buf_len)
{
    fs_priv_proc_t *priv_proc = (fs_priv_proc_t*) proc;
    uint8_t *buf_ptr;
    int ret = S_FAIL;

    for (buf_ptr = buf; buf_ptr < (uint8_t*)buf + buf_len; buf_ptr++) {
        ret = fs_proc_uint8_t(priv_proc, buf_ptr);
        if (ret != S_SUCCESS) {
            S_ERROR_DEFAULT("Proc error");
            break;
        }
    }

    return ret;
}

/*---------------------------------------------------------------------------
 * Function:    fs_proc_fs_string_t
 *
 * Purpose:     Generic processing routines
 *
 * Returns:     Non-negative on success or negative on failure
 *
 *---------------------------------------------------------------------------
 */
FS_INLINE int fs_proc_fs_string_t(fs_proc_t proc, fs_string_t *string)
{
    fs_priv_proc_t *priv_proc = (fs_priv_proc_t*) proc;
    uint32_t string_len = 0;
    char *string_buf = NULL;
    int ret = S_FAIL;

    switch (priv_proc->op) {
        case FS_ENCODE:
            string_len = strlen(*string) + 1;
            string_buf = (char*) *string;
            ret = fs_proc_uint32_t(priv_proc, &string_len);
            if (ret != S_SUCCESS) {
                S_ERROR_DEFAULT("Proc error");
                ret = S_FAIL;
                return ret;
            }
            ret = fs_proc_raw(priv_proc, string_buf, string_len);
            if (ret != S_SUCCESS) {
                S_ERROR_DEFAULT("Proc error");
                ret = S_FAIL;
                return ret;
            }
            break;
        case FS_DECODE:
            ret = fs_proc_uint32_t(priv_proc, &string_len);
            if (ret != S_SUCCESS) {
                S_ERROR_DEFAULT("Proc error");
                ret = S_FAIL;
                return ret;
            }
            string_buf = malloc(string_len);
            ret = fs_proc_raw(priv_proc, string_buf, string_len);
            if (ret != S_SUCCESS) {
                S_ERROR_DEFAULT("Proc error");
                ret = S_FAIL;
                return ret;
            }
            *string = string_buf;
            break;
        case FS_FREE:
            string_buf = (char*) *string;
            if (!string_buf) {
                S_ERROR_DEFAULT("Already freed");
                ret = S_FAIL;
                return ret;
            }
            free(string_buf);
            *string = NULL;
            ret = S_SUCCESS;
            break;
        default:
            break;
    }

    return ret;
}

/*---------------------------------------------------------------------------
 * Function:    fs_proc_bds_handle_t
 *
 * Purpose:     Generic processing routines
 *
 * Returns:     Non-negative on success or negative on failure
 *
 *---------------------------------------------------------------------------
 */
FS_INLINE int fs_proc_bds_handle_t(fs_proc_t proc, bds_handle_t *handle)
{
    fs_priv_proc_t *priv_proc = (fs_priv_proc_t*) proc;
    int ret = S_FAIL;

    switch (priv_proc->op) {
        case FS_ENCODE:
            /* If not enough space allocate extra space if encoding or just get extra buffer if decoding */
            if (priv_proc->current_buf->size_left < BDS_MAX_HANDLE_SIZE) {
                fs_proc_set_size(proc, priv_proc->proc_buf.size + priv_proc->extra_buf.size + BDS_MAX_HANDLE_SIZE);
            }
            ret = bds_handle_serialize(priv_proc->current_buf->buf_ptr, BDS_MAX_HANDLE_SIZE, *handle);
            fs_proc_set_buf_ptr(proc, priv_proc->current_buf->buf_ptr + BDS_MAX_HANDLE_SIZE);
            break;
        case FS_DECODE:
            ret = bds_handle_deserialize(handle, priv_proc->current_buf->buf_ptr, BDS_MAX_HANDLE_SIZE);
            fs_proc_set_buf_ptr(proc, priv_proc->current_buf->buf_ptr + BDS_MAX_HANDLE_SIZE);
            break;
        case FS_FREE:
            ret = bds_handle_free(*handle);
            *handle = NULL;
            break;
        default:
            break;
    }

    return ret;
}


#ifdef __cplusplus
}
#endif

#endif /* GENERIC_PROC_H */
