#include "common.h"

#include "mbedtls/entropy.h"

#include <bl_sec.h>

#if defined(MBEDTLS_ENTROPY_C)

#if defined(MBEDTLS_ENTROPY_HARDWARE_ALT)
int mbedtls_hardware_poll( void *data,
                           unsigned char *output, size_t len, size_t *olen )
{
    if( bl_rand_stream( output, len ) )
        return( MBEDTLS_ERR_ENTROPY_SOURCE_FAILED );
    *olen = len;

    return( 0 );
}
#endif

#endif /* MBEDTLS_ENTROPY_C */
