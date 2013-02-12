/*
 * client_fs.c
 */

#include "function_shipper.h"
#include "shipper_error.h"
#include "shipper_test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Dummy function that needs to be shipped */
int bla_initialize(int comm)
{
    const char message[] = "Hi, I'm bla_initialize";
    printf("%s\n", message);
    return strlen(message);
}

/******************************************************************************/
/* Can be automatically generated using macros */
typedef struct bla_initialize_in {
    int comm;
} bla_initialize_in_t;

typedef struct bla_initialize_out {
    int bla_initialize_ret;
} bla_initialize_out_t;

int bla_initialize_enc(void *buf, size_t buf_len, const void *in_struct)
{
    int ret = S_SUCCESS;
    bla_initialize_in_t *bla_initialize_in_struct = (bla_initialize_in_t*) in_struct;

    if (buf_len < sizeof(bla_initialize_in_t)) {
        S_ERROR_DEFAULT("Buffer size too small for serializing parameter");
        ret = S_FAIL;
    } else {
        /* Here safe to do a simple memcpy */
        /* TODO may also want to add a checksum or something */
        memcpy(buf, bla_initialize_in_struct, sizeof(bla_initialize_in_t));
    }
    return ret;
}

int bla_initialize_dec(void *out_struct, const void *buf, size_t buf_len)
{
    int ret = S_SUCCESS;
    bla_initialize_out_t *bla_initialize_out_struct = out_struct;

    if (buf_len < sizeof(bla_initialize_out_t)) {
        S_ERROR_DEFAULT("Buffer size too small for deserializing parameter");
        ret = S_FAIL;
    } else {
        /* Here safe to do a simple memcpy */
        /* TODO may also want to add a checksum or something */
        memcpy(bla_initialize_out_struct, buf, sizeof(bla_initialize_out_t));
    }
    return ret;
}

/******************************************************************************/
int main(int argc, char *argv[])
{
    char *ion_name;
    fs_peer_t peer;
    na_network_class_t *network_class = NULL;

    fs_id_t bla_initialize_id;
    bla_initialize_in_t  bla_initialize_in_struct;
    bla_initialize_out_t bla_initialize_out_struct;
    fs_request_t bla_initialize_request;

    int bla_initialize_comm = 12345;
    int bla_initialize_ret = 0;

    /* Initialize the interface */
    network_class = shipper_test_client_init(argc, argv);
    ion_name = getenv(ION_ENV);
    if (!ion_name) {
        fprintf(stderr, "getenv(\"%s\") failed.\n", ION_ENV);
    }
    fs_init(network_class);

    /* Look up peer id */
    fs_peer_lookup(ion_name, &peer);

    /* Register function and encoding/decoding functions */
    bla_initialize_id = fs_register("bla_initialize", &bla_initialize_enc, &bla_initialize_dec);

    /* Fill input structure */
    bla_initialize_in_struct.comm = bla_initialize_comm;

    /* Forward call to peer */
    printf("Fowarding bla_initialize, op id: %u...\n", bla_initialize_id);
    fs_forward(peer, bla_initialize_id, &bla_initialize_in_struct,
            &bla_initialize_out_struct, &bla_initialize_request);

    /* Wait for call to be executed and return value to be sent back */
    fs_wait(bla_initialize_request, FS_MAX_IDLE_TIME, FS_STATUS_IGNORE);

    /* Get output parameter */
    bla_initialize_ret = bla_initialize_out_struct.bla_initialize_ret;
    printf("bla_initialize returned: %d\n", bla_initialize_ret);

    /* Free peer id */
    fs_peer_free(peer);

    /* Finalize interface */
    fs_finalize();
    return EXIT_SUCCESS;
}