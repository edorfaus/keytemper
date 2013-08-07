#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <hidapi.h>

#define DATA_MAX_LENGTH 256

static int start_device(
	hid_device* dev, unsigned char* read_data, int read_data_len
)
{
	unsigned char trigger_data[2] = { 0, 0 };
	printf("Starting device");
	for ( long write_count = 0; write_count < 50; write_count++ )
	{
		int size = hid_write(dev, trigger_data, sizeof(trigger_data));
		if ( size <= 0 )
		{
			printf("\n");
			fprintf(stderr, "Write to device failed: %ls\n", hid_error(dev));
			return -5;
		}
		printf(".");
		fflush(stdout);
		
		size = hid_read_timeout(dev, read_data, read_data_len, 100);
		if ( size < 0 )
		{
			printf("\n");
			fprintf(stderr, "Read from device failed: %ls\n", hid_error(dev));
			return -6;
		}
		else if ( size > 0 )
		{
			// TODO: check if this was a device stop rather than start
			printf(" Device started.\n");
			if ( size == read_data_len )
			{
				fprintf(
					stderr,
					"Warning: data buffer full, may have lost some data.\n\n"
				);
			}
			return size;
		}
		// else, size == 0, which means timeout, which means keep going.
	}
	printf("\n");
	fprintf(stderr, "Device still not started after 5 seconds, giving up.\n");
	return -4;
}

static int use_device(char* device_path, long read_count)
{
	hid_device *dev = hid_open_path(device_path);
	if ( !dev )
	{
		fprintf(stderr, "Could not open device: %s\n", device_path);
		return 3;
	}
	
	unsigned char read_data[DATA_MAX_LENGTH] = { 0 };
	int ret = start_device(dev, read_data, DATA_MAX_LENGTH);
	
	if ( ret >= 0 )
	{
		// ret is size of read data
	}
	else
	{
		ret = -ret;
	}
	
	hid_close(dev);
	
	return ret;
}

static int show_help(char* error_message)
{
	if ( error_message != NULL )
	{
		fprintf(stderr, "%s\n\n", error_message);
	}
	fprintf(stderr,
"Usage: keytemper --help|-h\n"
" or  : keytemper <device> [read-count]\n"
"\n"
"The first form prints this help message to stderr.\n"
"The second form attempts to use the given device as a HID TEMPer device, and\n"
"tries to trigger its direct-to-Excel feature and then parsing the response.\n"
"If the read-count is given, that many measurements will be read from the\n"
"device before we turn it back off and exit, if not given it defaults to 5.\n"
	);
	return 1;
}

int main(int argc, char** argv)
{
	int i;
	if ( argc < 2 )
	{
		return show_help("Too few parameters.");
	}
	for ( i = 1; i < argc; i++ )
	{
		if ( strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0 )
		{
			return show_help(NULL);
		}
	}
	if ( argc > 3 )
	{
		return show_help("Too many parameters.");
	}
	char *device = argv[1];
	long read_count = 5;
	if ( argc > 2 )
	{
		char *endptr;
		read_count = strtol(argv[2], &endptr, 0);
		if ( *endptr != '\0' || read_count < 1 )
		{
			return show_help("Invalid read count specified.");
		}
	}
	
	if ( hid_init() != 0 )
	{
		fprintf(stderr, "Could not initialize the HID API.\n");
		return 2;
	}
	
	int ret = use_device(device, read_count);
	
	if ( hid_exit() != 0 )
	{
		fprintf(stderr, "Error shutting down the HID API.\n");
		if ( ret == 0 )
		{
			ret = 2;
		}
	}
	
	return ret;
}
