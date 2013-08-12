#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <hidapi.h>
#include <stdbool.h>
#include "keymap.h"

#define DATA_MAX_LENGTH 16

#define USE_NUMBERED_REPORTS

typedef struct keytemper_dev keytemper_dev;

struct keytemper_dev
{
	hid_device *hid_dev;
	char *error;
	const wchar_t *hid_error;
	unsigned char read_data[DATA_MAX_LENGTH];
	int read_data_len;
	unsigned char last_key, last_key_modifiers;
	unsigned char last_key_char;
};

/** Return true if write succeeded, false if it failed. */
static bool kt_write(keytemper_dev *dev, int count)
{
	unsigned char data[2] = { 0, 0 };
	for ( int i = 0 ; i < count ; i++ )
	{
		int size = hid_write(dev->hid_dev, data, sizeof(data));
		if ( size <= 0 )
		{
			dev->error = "Write to device failed";
			dev->hid_error = hid_error(dev->hid_dev);
			return false;
		}
	}
	return true;
}

/** Return true if read succeeded, false if it failed or timed out. */
static bool kt_read(keytemper_dev *dev, int timeout)
{
	int size = hid_read_timeout(
		dev->hid_dev, dev->read_data, DATA_MAX_LENGTH, timeout
	);
	if ( size < 0 )
	{
		dev->error = "Read from device failed";
		dev->hid_error = hid_error(dev->hid_dev);
		return false;
	}
	if ( size == 0 )
	{
		dev->error = NULL;
		return false;
	}
	if ( size >= DATA_MAX_LENGTH )
	{
		fprintf(
			stderr,
			"Warning: read data buffer full, may have lost some data.\n\n"
		);
		size = DATA_MAX_LENGTH;
	}
	dev->read_data_len = size;
	return true;
}

/** Return true if the last data included a pressed key, false otherwise. */
static bool kt_parse_key(keytemper_dev *dev)
{
	// The TEMPer's keyboard interface uses numbered reports, so the first byte
	// is the report ID, then comes the report data.

	#ifdef USE_NUMBERED_REPORTS
	const int ofs = 1;
	#else
	const int ofs = 0;
	#endif

	// In the report data, the first byte is the modifier bitmask, the second
	// is reserved (probably all zero), then there's 5 bytes of pressed keys.
	// For the TEMPers, I've only ever seen one key at a time being pressed,
	// and it doesn't rely on key repeat, so I simplify a lot by completely
	// ignoring releases and only handling one key being pressed at a time.

	if ( dev->read_data_len <= ofs )
	{
		// We don't have enough data to parse anything from it.
		// I haven't seen this happen yet, but better safe than sorry.
		return false;
	}
	unsigned char modifiers = dev->read_data[ofs], key = 0;
	bool warned = false;
	for ( int i = 2 + ofs ; i < dev->read_data_len ; i++ )
	{
		if ( dev->read_data[i] != 0 )
		{
			if ( key != 0 && !warned )
			{
				fprintf(
					stderr,
					"Warning: multiple keys pressed in one event,"
					" dropping all but the last."
				);
				warned = true;
			}
			key = dev->read_data[i];
			dev->last_key_char = get_char_for_key(modifiers, key);
		}
	}
	dev->last_key_modifiers = modifiers;
	dev->last_key = key;
	return ( key != 0 );
}

/** Return true if it managed to start the device (or it was already running). */
static bool kt_start(keytemper_dev *dev)
{
	printf("Starting the device...");
	fflush(stdout);
	for ( int i = 0; i < 100; i++ )
	{
		int timeout = 10;
		while ( kt_read(dev, timeout) )
		{
			if ( kt_parse_key(dev) )
			{
				// Got a key *press* from the device
				if ( dev->last_key_char == '\n' )
				{
					// This may have been a stop message, or may have been the
					// end of the previous line of an already running TEMPer.
					// We can't know which, so we should just pretend it's a
					// stop message, meaning we ignore it and keep reading
					// until we get something that isn't a newline.
					// At worst, this means we spend some time first stopping
					// the device and then starting it again, which is just the
					// price you have to pay for having left it running.
					timeout = 100;
				}
				else
				{
					// We got something that isn't a stop message, which means
					// the device is now running.
					// Technically it could be some leftover data that needs to
					// be read before the stop message gets through, but that
					// needs to be handled by the other code anyway.
					printf(" done.\n");
					fflush(stdout);
					return true;
				}
			}
		}
		if ( dev->error != NULL )
		{
			break;
		}
		
		if ( !kt_write(dev, 10) )
		{
			break;
		}
		printf(".");
		fflush(stdout);
	}
	printf(" failed.\n");
	fflush(stdout);
	if ( dev->error == NULL )
	{
		dev->error = "Device did not respond to start, giving up.";
		dev->hid_error = NULL;
	}
	return false;
}

/** Return true if it managed to stop the device. */
static bool kt_stop(keytemper_dev *dev)
{
	printf("Stopping the device...");
	fflush(stdout);
	for ( int i = 0; i < 100; i++ )
	{
		char last_key = dev->last_key_char;
		while ( kt_read(dev, 10) )
		{
			if ( kt_parse_key(dev) )
			{
				// Got a key *press* from the device
				if ( last_key == '\n' && dev->last_key_char == '\n' )
				{
					// Got a second newline in a row, which is a stop message.
					printf(" done.\n");
					fflush(stdout);
					// Flush the key release from the device's output buffer.
					kt_read(dev, 10);
					return true;
				}
				last_key = dev->last_key_char;
			}
		}
		if ( dev->error != NULL )
		{
			break;
		}
		
		if ( !kt_write(dev, 10) )
		{
			break;
		}
		printf(".");
		fflush(stdout);
	}
	printf(" failed.\n");
	fflush(stdout);
	if ( dev->error == NULL )
	{
		dev->error = "Device did not respond to stop, giving up.";
		dev->hid_error = NULL;
	}
	return false;
}

/** Print an error message for the given device. */
static void kt_print_error(keytemper_dev *dev)
{
	if ( dev->error != NULL )
	{
		if ( dev->hid_error != NULL )
		{
			fprintf(stderr, "%s: %ls\n", dev->error, dev->hid_error);
		}
		else
		{
			fprintf(stderr, "%s\n", dev->error);
		}
	}
	else if ( dev->hid_error != NULL )
	{
		fprintf(stderr, "Unknown HID-only error: %ls\n", dev->hid_error);
	}
	else
	{
		fprintf(stderr, "Unknown error with no message\n");
	}
}

static bool read_device(keytemper_dev *dev, long read_count)
{
	int timeout = 10000;
	char prev_key = '\0';
	for ( int cur_count = 0 ; cur_count < read_count ; )
	{
		if ( kt_parse_key(dev) )
		{
			if ( dev->last_key_char != '\0' )
			{
				printf("%c", dev->last_key_char);
				fflush(stdout);
				if ( dev->last_key_char == '\n' )
				{
					cur_count++;
					if ( prev_key == '\n' )
					{
						// TODO: handle stop
						printf("\n");
						fflush(stdout);
						fprintf(stderr, "Received stop message prematurely.\n");
					}
				}
			}
			prev_key = dev->last_key_char;
		}
		// This will read once more after reading the wanted number of lines,
		// but that's a good thing, as that will be the release of the key.
		if ( !kt_read(dev, timeout) )
		{
			if ( dev->error == NULL )
			{
				dev->error = "Read from device time out.";
			}
			printf("\n");
			fflush(stdout);
			return false;
		}
	}
	return true;
}

static int use_device(char* device_path, long read_count)
{
	keytemper_dev device = {0}, *dev = &device;
	dev->hid_dev = hid_open_path(device_path);
	if ( !dev->hid_dev )
	{
		fprintf(stderr, "Could not open device: %s\n", device_path);
		return 3;
	}
	
	if ( !kt_start(dev) )
	{
		kt_print_error(dev);
		hid_close(dev->hid_dev);
		return 4;
	}
	
	int ret = 0;
	if ( !read_device(dev, read_count) )
	{
		kt_print_error(dev);
		ret = 5;
	}
	
	if ( !kt_stop(dev) )
	{
		kt_print_error(dev);
		if ( ret == 0 ) ret = 6;
	}
	
	hid_close(dev->hid_dev);
	
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
"If the read-count is given, that many lines will be read from the device\n"
"before we turn it back off and exit; if not given, it defaults to 5.\n"
"Note that this count includes the two header lines that are printed on start.\n"
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
