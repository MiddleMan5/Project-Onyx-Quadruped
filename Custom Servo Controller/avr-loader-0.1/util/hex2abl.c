/* Originally from: */

// hex2abl.cpp
// Copyright 2005 Mike Dvorsky
//
// Convert intel hex files into AVR Boot Loader files.
//
// Input file format
// ":NNAAAATTDDDD...DDDDCC"
// Where
//   NN = byte count of the data
//   AAAA = starting address
//   TT = record type (00=data, 01=termination, 02=segment base address)
//   DDDD...DDDD = data bytes (must match byte count NN)
//   CC = LSB of 2's complement of sum of count, address, type, and data bytes
//
// Output file format
// "eAAAA" = erase page containing address AAAA
// "wAAAA" = write page starting at address AAAA
// "DDDD...DDDD" = data for write command on previous line
// "....... ... ......" = delay of 64 byte times for erase/write to complete
//

/* However, this is really a C program, not C++ */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>

#define MAX_IHEX_STR_LEN 256
#define MAX_PAGE_SIZE 128
#define MIN_PAGE_SIZE  64
static int page_size;
#define MAX_PAGES (65536 / MIN_PAGE_SIZE)
static bool ihex_is_big_endian = false;

#define max_pages() (65536 / page_size)

typedef struct page_data_s
{
    unsigned char data_array[MAX_PAGE_SIZE];
    bool touched ;
    // Tracks whether a page has been referenced by the ihex file
} page_data_t ;

FILE * ihex_input_file ;
FILE * abl_output_file ;
char ihex_string[MAX_IHEX_STR_LEN];
page_data_t page_data_array[MAX_PAGES];
int line_num = 0 ;

unsigned char x2d ( char xdigit )
{
    if ( xdigit >= '0' && xdigit <= '9' )
    {
	return ( xdigit - '0' ) ;
    }
    else
    {
	return ( toupper ( xdigit ) - 'A' + 10 ) ;
    }
}

// Parse the source line in global ihex_string.  Update page_data[] array.
void parse_ihex ( void )
{
    int ch_num = 0 ;
    unsigned char digit3, digit2, digit1, digit0 ;
    unsigned char checksum ;
    unsigned char addressH ;
    unsigned int address ;
    int byte_count ;
    int page_num ;
    int byte_num ;
    unsigned char dataH ;
    unsigned char dataL ;
    unsigned char type ;
	
    // Check for valid line length
    if ( strlen ( ihex_string ) < 11 )
    {
	printf ( "Line %d:  too short for valid Intel hex/n", line_num ) ;
	return ;
    }

    // Check for starting colon
    if ( ihex_string [ ch_num ++ ] != ':' )
    {
	printf ( "Line %d:  first character must be ':'\n", line_num ) ;
	return ;
    }

    // Read byte count
    digit1 = ihex_string [ ch_num ++ ] ;
    digit0 = ihex_string [ ch_num ++ ] ;
    if ( ! isxdigit ( digit1 ) || ! isxdigit ( digit0 ) )
    {
	printf ( "Line %d:  invalid hex digit\n", line_num ) ;
	return ;
    }
    byte_count = x2d ( digit1 ) * 16 + x2d ( digit0 ) ;
    checksum = byte_count ;

    // Ensure byte count is even
    if ( byte_count & 0x01 != 0 )
    {
	printf ( "Line %d:  odd number of data bytes invalid\n", line_num ) ;
	return ;
    }
	
    // Validate string length versus byte count
    if ( strlen ( ihex_string ) < ( 1 + 2 + 4 + 2 + byte_count + 2 ) )
    {
	printf ( "Line %d:  # data bytes < byte count\n", line_num ) ;
	return ;
    }

    // Read address
    digit3 = ihex_string [ ch_num ++ ] ;
    digit2 = ihex_string [ ch_num ++ ] ;
    digit1 = ihex_string [ ch_num ++ ] ;
    digit0 = ihex_string [ ch_num ++ ] ;
    if ( ! isxdigit ( digit3 ) || ! isxdigit ( digit2 )
	 || ( ! isxdigit ( digit1 ) || ! isxdigit ( digit0 ) ) )
    {
	printf ( "Line %d:  invalid hex digit\n", line_num ) ;
	return ;
    }
    addressH = x2d ( digit3 ) * 16 + x2d ( digit2 ) ;
    checksum += addressH ;
    address = ( addressH << 8 ) + x2d ( digit1 ) * 16 + x2d ( digit0 ) ;
    checksum += address ;

    // Verify address is even
    if ( address & 0x01 != 0 )
    {
	printf ( "Line %d:  odd address invalid\n", line_num ) ;
	return ;
    }

    // Read type
    digit1 = ihex_string [ ch_num ++ ] ;
    digit0 = ihex_string [ ch_num ++ ] ;
    if ( ! isxdigit ( digit1 ) || ! isxdigit ( digit0 ) )
    {
	printf ( "Line %d:  invalid hex digit ':'\n", line_num ) ;
	return ;
    }
    type = x2d ( digit1 ) * 16 + x2d ( digit0 ) ;
    checksum += type ;
	
    // Validate type.  Continue if type 0 or 1 (but do not store data for type 1).
    if ( ( type != 0 ) && ( type != 1 ) )
    {
	printf ( "Line %d:  unknown type\n", line_num ) ;
	return ;
    }

    // Start reading data words
    for ( ; byte_count > 0 ; byte_count -= 2, address += 2 )
    {
	digit3 = ihex_string [ ch_num ++ ] ;
	digit2 = ihex_string [ ch_num ++ ] ;
	digit1 = ihex_string [ ch_num ++ ] ;
	digit0 = ihex_string [ ch_num ++ ] ;
	if ( ! isxdigit ( digit3 ) || ! isxdigit ( digit2 )
	     || ( ! isxdigit ( digit1 ) || ! isxdigit ( digit0 ) ) )
	{
	    printf ( "Line %d:  invalid hex digit\n", line_num ) ;
	    return ;
	}
#if ( IHEX_IS_BIG_ENDIAN )
	dataH = x2d ( digit3 ) * 16 + x2d ( digit2 ) ;
	dataL = x2d ( digit1 ) * 16 + x2d ( digit0 ) ;
#else
	dataL = x2d ( digit3 ) * 16 + x2d ( digit2 ) ;
	dataH = x2d ( digit1 ) * 16 + x2d ( digit0 ) ;
#endif
	checksum += dataH ;
	checksum += dataL ;
	page_num = address / page_size;
	byte_num = address % page_size;
	if ( page_num >= max_pages() )
	{
	    printf ( "Line %d:  invalid address %d\n", line_num, address ) ;
	    return ;
	}
	if ( type == 0 )
	{
	    // Write data in little-endian format
	    page_data_array [ page_num ] . data_array [ byte_num ] = dataL ;
	    page_data_array [ page_num ] . data_array [ byte_num + 1 ] = dataH ;
	    page_data_array [ page_num ] . touched = 1 ;
	}
    }        
	
    // Read and verify checksum.  The data is stored regardless of checksum, but an
    // error message will be printed if invalid checksum detected.
    digit1 = ihex_string [ ch_num ++ ] ;
    digit0 = ihex_string [ ch_num ++ ] ;
    if ( ! isxdigit ( digit1 ) || ! isxdigit ( digit0 ) )
    {
	printf ( "Line %d:  invalid hex digit ':'\n", line_num ) ;
	return ;
    }
    checksum = 256 - checksum ;
    if ( checksum != ( x2d ( digit1 ) * 16 + x2d ( digit0 ) ) )
    {
	printf ( "Line %d:  invalid checksum, should be %02X\n", line_num, checksum ) ;
	return ;
    }
}


static void usage(FILE *file, const char *name)
{
    fprintf(file, "Usage: %s [-h] [-p PAGESIZE] [-b]\n"
	    "  [--help] [--page-size=PAGESIZE] [--big-endia-ihex-file]\n"
	    "  INFILE.ihex OUTFILE.abl\n",
	    name);
}

int main(int argc, char* argv[])
{
    static const struct option long_options[] = {
	{ "page-size", required_argument, NULL, 'p' },
	{ "big-endian-ihex-file", no_argument, NULL, 'b' }, 
	{ "help", no_argument, NULL, 'h' },
	{ NULL, 0, NULL, '\0' }
    };
    int option_index = 0, c;
    int page_num;
    int byte_num;
    const char *infile, *outfile;

    page_size = MIN_PAGE_SIZE;

    do {
	c = getopt_long(argc, argv, "p:bh", long_options, &option_index);
	switch (c) {
	case 'h':
	    usage(stdout, argv[0]);
	    return EXIT_SUCCESS;
	case 'p':
	    page_size = atoi(optarg);
	    if (page_size != MIN_PAGE_SIZE && page_size != MAX_PAGE_SIZE) {
		fprintf(stderr, "Page size must be either %d or %d.\n",
			MIN_PAGE_SIZE, MAX_PAGE_SIZE);
		return EXIT_FAILURE;
	    }
	    break;
	case 'b':
	    ihex_is_big_endian = true;
	    break;
		  }
    } while (c != -1);

    if (argc - optind != 2) {
	usage(stderr, argv[0]);
	return EXIT_FAILURE;
    }

    infile = argv[optind++];
    outfile = argv[optind++];

    // Open input and output files
    ihex_input_file = fopen (infile, "r");
    if (ihex_input_file == NULL) {
	printf("\nERROR:  Could not open input file\n");
	return EXIT_FAILURE;
    }
    abl_output_file = fopen(outfile, "w");
    if (abl_output_file == NULL) {
	printf("\nERROR:  Could not open output file\n");
	return EXIT_FAILURE;
    }

    // Initialize page data array to erased, all pages untouched
    for (page_num = 0 ; page_num < max_pages(); ++ page_num) {
	for ( byte_num = 0 ; byte_num < page_size; ++ byte_num) {
	    page_data_array[ page_num].data_array[byte_num] = 0xFF;
	}
	page_data_array[page_num].touched = false;
    }

    // Read lines from the input file and parse, filling page_data_array
    line_num = 1 ;
    while (true) {
	if (fgets(ihex_string, MAX_IHEX_STR_LEN, ihex_input_file) != NULL) {
	    parse_ihex();
	    ++ line_num ;
	} else {
	    break ;
	}
    }

    // Read elements from page_data_array, writing touched pages to the
    // output file.  First erase all touched pages.
    for (page_num = 0; page_num < max_pages(); ++ page_num) {
	if (page_data_array[page_num].touched ) {
	    fprintf(abl_output_file, "e%04X\n", page_num * page_size);
	    for (byte_num = 0; byte_num < page_size; byte_num++) {
		fprintf(abl_output_file, ".");
	    }
	    fprintf(abl_output_file, "\n");
	}
    }
    // Next write all touched pages.  Write in reverse order so that page 0 will
    // be the last written.  This way, if the programming is interrupted, the
    // word at address 0 will probably be erased.
    for (page_num = max_pages() - 1; page_num >= 0; -- page_num) {
	if (page_data_array[page_num].touched) {
	    fprintf(abl_output_file, "w%04X\n", page_num * page_size);
	    for (byte_num = 0; byte_num < page_size; ++ byte_num) {
		fprintf(abl_output_file, "%02X",
			page_data_array[page_num].data_array[byte_num]);
	    }
	    fprintf(abl_output_file, "\n");
	    for (byte_num = 0; byte_num < page_size; byte_num++) {
		fprintf(abl_output_file, ".");
	    }
	    fprintf(abl_output_file, "\n");
	}
    }
	
    fclose(abl_output_file);
    fclose(ihex_input_file);

    return EXIT_SUCCESS;
}

