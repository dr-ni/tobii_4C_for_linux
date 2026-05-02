
//gcc cap_test.c -o cap_test -I/usr/include -L/usr/lib/tobii -Wl,-rpath,/usr/lib/tobii -ltobii_stream_engine
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tobii/tobii.h>

static void url_receiver( char const* url, void* user_data )
{
    char* buffer = (char*) user_data;

    strncpy( buffer, url, 255 );
    buffer[255] = '\0';
}

static void print_capability(
    tobii_device_t* device,
    tobii_capability_t capability,
    const char* name )
{
    tobii_supported_t supported;
    tobii_error_t error;

    error = tobii_capability_supported(
        device,
        capability,
        &supported );

    if( error != TOBII_ERROR_NO_ERROR )
    {
        printf(
            "%s: query failed (%d)\n",
            name,
            error );

        return;
    }

    printf(
        "%s: %s\n",
        name,
        supported == TOBII_SUPPORTED ? "YES" : "NO" );
}

int main()
{
    tobii_api_t* api = NULL;
    tobii_device_t* device = NULL;
    tobii_error_t error;

    char url[256] = {0};

    error = tobii_api_create(
        &api,
        NULL,
        NULL );

    if( error != TOBII_ERROR_NO_ERROR )
    {
        printf( "API create failed\n" );
        return 1;
    }

    error = tobii_enumerate_local_device_urls(
        api,
        url_receiver,
        url );

    if( strlen( url ) == 0 )
    {
        printf( "No device found\n" );
        return 1;
    }

    printf( "Device URL: %s\n\n", url );

    error = tobii_device_create(
        api,
        url,
        &device );

    if( error != TOBII_ERROR_NO_ERROR )
    {
        printf( "Device create failed\n" );
        return 1;
    }

    print_capability(
        device,
        TOBII_CAPABILITY_DISPLAY_AREA_WRITABLE,
        "DISPLAY_AREA_WRITABLE" );

    print_capability(
        device,
        TOBII_CAPABILITY_CALIBRATION_2D,
        "CALIBRATION_2D" );

    print_capability(
        device,
        TOBII_CAPABILITY_CALIBRATION_3D,
        "CALIBRATION_3D" );

    print_capability(
        device,
        TOBII_CAPABILITY_CALIBRATION_PER_EYE,
        "CALIBRATION_PER_EYE" );

    tobii_device_destroy( device );
    tobii_api_destroy( api );

    return 0;
}
