/* ------------------------------------------------------------------
 * Squashfs Utility - Main Source File
 * ------------------------------------------------------------------ */

#include "squtil.h"

/**
 * Show program usage message
 */
void show_usage ( void )
{
    fprintf ( stderr, "usage: squtil option [endian] file args...\n"
        "\n"
        "options:\n"
        "  -h           show help message\n"
        "  -i           show superblock information\n"
        "\n"
        "endians:\n"
        "  -le          little endian (default)\n" "  -be          big endian\n" "\n" );
}

/**
 * Program entry point 
 */
int main ( int argc, char *argv[] )
{
    int status;
    int fd;
    int arg_off = 0;
    int rw = 0;
    int needsync = 0;
    int endianswap = 0;
    size_t length;
    unsigned char *pmaddr;

    /* Validate agruments count */
    if ( argc < 3 )
    {
        show_usage (  );
        return 1;
    }

    /* Validate arguments count and options count */
    if ( argc < 2 || argv[1][0] != '-' || strlen ( argv[1] ) != 2 )
    {
        show_usage (  );
        return 1;
    }

    /* Enable endian swap is needed */
    if ( argv[2][0] == '-' )
    {
        if ( !strcmp ( argv[2], "-le" ) )
        {
#if __BYTE_ORDER == __BIG_ENDIAN
            endianswap = 1;
#elif __BYTE_ORDER != __LITTLE_ENDIAN
#error "endian not set"
#endif

        } else if ( !strcmp ( argv[2], "-be" ) )
        {
#if __BYTE_ORDER == __LITTLE_ENDIAN
            endianswap = 1;
#elif __BYTE_ORDER != __BIG_ENDIAN
#error "endian not set"
#endif
        } else
        {
            show_usage (  );
            return 1;
        }

        arg_off++;
    }

    /* Validate agruments count again */
    if ( argc < arg_off + 3 )
    {
        show_usage (  );
        return 1;
    }

    /* Select read-write mode if needed */
    if ( argv[1][1] == 'e' || argv[1][1] == 't' )
    {
        rw = 1;
    }

    /* Open squashfs file */
    if ( ( fd = open ( argv[arg_off + 2], rw ? O_RDWR : O_RDONLY ) ) < 0 )
    {
        perror ( argv[arg_off + 2] );
        return 1;
    }

    /* Measure squashfs file length */
    if ( ( length = lseek ( fd, 0, SEEK_END ) ) == ( size_t ) - 1 )
    {
        perror ( "lseek" );
        close ( fd );
        return 1;
    }

    /*
     * Map data from the file into our memory for read & write.
     * Use MAP_SHARED for Persistent Memory so that stores go
     * directly to the PM and are globally visible.
     */
    if ( ( pmaddr = ( unsigned char * )
            mmap ( NULL, length, rw ? ( PROT_READ | PROT_WRITE ) : PROT_READ, MAP_SHARED, fd,
                0 ) ) == MAP_FAILED )
    {
        perror ( "mmap" );
        close ( fd );
        return 1;
    }

    /* don't need the fd anymore, everything done above */
    close ( fd );

    /* Perform requested action */
    switch ( argv[1][1] )
    {
    case 'i':
        status = squashfs_info ( pmaddr, length, endianswap );
        break;
    default:
        show_usage (  );
        status = -1;
    }

    /*
     * The above stores may or may not be sitting in cache at
     * this point, depending on other system activity causing
     * cache pressure.  Force the change to be durable (flushed
     * all the say to the Persistent Memory) using msync().
     */
    if ( needsync && msync ( pmaddr, length, MS_SYNC ) < 0 )
    {
        perror ( "msync" );
        status = -1;
    }

    /* Free mapped memory */
    munmap ( pmaddr, length );

    if ( status )
    {
        printf ( "operation failed.\n" );
    }

    return status ? 1 : 0;
}
