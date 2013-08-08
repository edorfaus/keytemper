#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <hidapi.h>
#include <stdbool.h>

#define DATA_MAX_LENGTH 256

static unsigned char parse_print_buf(unsigned char* data, int len)
{
	// The TEMPers' keyboard interface uses numbered reports, so the first byte
	// is the report ID, then comes the report data.
	// In the report data, the first byte is the modifier bitmask, the second
	// is reserved (probably all zero), then there's 5 byte of pressed keys.
	// For the TEMPers, I've only ever seen one key at a time being pressed.
	
	if ( len < 2 ) return 0;
	// Ignore key releases.
	bool all_zero = ( data[1] == 0 );
	for ( int i = 3 ; all_zero && i < len ; i++ )
	{
		if ( data[i] != 0 )
		{
			all_zero = false;
			break;
		}
	}
	if ( all_zero )
	{
		return 0;
	}
	// Print all pressed keys
	printf("(");
	// Starting with the modifiers
	if ( data[1] & 0x01 ) printf("C"); if ( data[1] & 0x10 ) printf("c");
	if ( data[1] & 0x02 ) printf("S"); if ( data[1] & 0x20 ) printf("s");
	if ( data[1] & 0x04 ) printf("A"); if ( data[1] & 0x40 ) printf("a");
	if ( data[1] & 0x08 ) printf("W"); if ( data[1] & 0x80 ) printf("w");
	// Then the pressed keys (if any)
	unsigned char last = 0;
	bool first = ( data[1] == 0 );
	for ( int i = 3 ; i < len ; i++ )
	{
		if ( data[i] != 0 )
		{
			last = data[i];
			printf("%s%.2x", first ? "" : " ", data[i]);
			first = false;
		}
	}
	printf(")%s", last == 0x28 ? "\n" : " ");
	fflush(stdout);
	return last;
}

static int start_device(hid_device* dev, unsigned char* last)
{
	unsigned char trigger_data[2] = { 0, 0 };
	unsigned char read_data[DATA_MAX_LENGTH] = { 0 };
	printf("Starting device");
	for ( int write_count = 0; write_count < 100; write_count++ )
	{
		int size = hid_write(dev, trigger_data, sizeof(trigger_data));
		if ( size <= 0 )
		{
			printf("\n");
			fprintf(stderr, "Write to device failed: %ls\n", hid_error(dev));
			return 5;
		}
		printf(".");
		fflush(stdout);
		
		size = hid_read_timeout(dev, read_data, DATA_MAX_LENGTH, 0);
		if ( size < 0 )
		{
			printf("\n");
			fprintf(stderr, "Read from device failed: %ls\n", hid_error(dev));
			return 6;
		}
		else if ( size > 0 )
		{
			// TODO: check if this was a device stop rather than start
			printf(" Device started after %i writes.\n", write_count + 1);
			if ( size == DATA_MAX_LENGTH )
			{
				fprintf(
					stderr,
					"Warning: data buffer full, may have lost some data.\n\n"
				);
			}
			*last = parse_print_buf(read_data, size);
			
			return 0;
		}
		// else, size == 0, which means timeout, which means keep going.
	}
	printf("\n");
	fprintf(stderr, "Device failed to start, giving up.\n");
	return 4;
}

static int read_device(hid_device* dev, long read_count, unsigned char* last)
{
	unsigned char read_data[DATA_MAX_LENGTH] = { 0 };
	int timeout = 10000;
	for ( int cur_count = 0 ; cur_count < read_count ; )
	{
		int size = hid_read_timeout(dev, read_data, DATA_MAX_LENGTH, timeout);
		if ( size < 0 )
		{
			fprintf(stderr, "Read from device failed: %ls\n", hid_error(dev));
			return 6;
		}
		else if ( size == 0 )
		{
			fprintf(stderr, "Read from device timed out.\n");
			return 7;
		}
		else
		{
			if ( size == DATA_MAX_LENGTH )
			{
				fprintf(
					stderr,
					"Warning: data buffer full, may have lost some data.\n\n"
				);
			}
			unsigned char cur = parse_print_buf(read_data, size);
			if ( cur != 0 )
			{
				*last = cur;
				if ( cur == 0x28 )
				{
					// Enter was the last key pressed.
					cur_count++;
				}
			}
		}
	}
	return 0;
}

static int stop_device(hid_device* dev, unsigned char* last)
{
	unsigned char trigger_data[2] = { 0, 0 };
	unsigned char read_data[DATA_MAX_LENGTH] = { 0 };
	printf("Stopping device");
	for ( int write_count = 0; write_count < 100; write_count++ )
	{
		int size = hid_write(dev, trigger_data, sizeof(trigger_data));
		if ( size <= 0 )
		{
			printf("\n");
			fprintf(stderr, "Write to device failed: %ls\n", hid_error(dev));
			return 5;
		}
		printf(".");
		fflush(stdout);
		
		size = hid_read_timeout(dev, read_data, DATA_MAX_LENGTH, 0);
		if ( size < 0 )
		{
			printf("\n");
			fprintf(stderr, "Read from device failed: %ls\n", hid_error(dev));
			return 6;
		}
		else if ( size > 0 )
		{
			unsigned char cur = parse_print_buf(read_data, size);
			if ( cur == 0x28 && *last == 0x28 )
			{
				printf(" Device stopped after %i writes.\n", write_count + 1);
				return 0;
			}
			if ( cur != 0 )
			{
				*last = cur;
			}
			if ( size == DATA_MAX_LENGTH )
			{
				fprintf(
					stderr,
					"Warning: data buffer full, may have lost some data.\n\n"
				);
			}
		}
		// else, size == 0, which means timeout, which means keep going.
	}
	printf("\n");
	fprintf(stderr, "Device failed to acknowledge stop, giving up.\n");
	return 4;
}

static int use_device(char* device_path, long read_count)
{
	hid_device *dev = hid_open_path(device_path);
	if ( !dev )
	{
		fprintf(stderr, "Could not open device: %s\n", device_path);
		return 3;
	}
	
	unsigned char last = 0;
	int ret = start_device(dev, &last);
	
	if ( ret == 0 )
	{
		ret = read_device(dev, read_count, &last);
		printf("\n");
		int ret2 = stop_device(dev, &last);
		if ( ret == 0 )
		{
			ret = ret2;
		}
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
