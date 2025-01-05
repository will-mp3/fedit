#include "mu.h"

#define _GNU_SOURCE

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// macro for USAGE output to be used with -h
#define USAGE                                                                                                                                                \
    "Usage: fedit [-h] [-l NROTL] [-r NROTR] [-x NEXPAND] [-c NCONTRACT] [-v CHAR] [-s NSKIP] [-k NKEEP] [-r] PATH\n"                                        \
    "\n"                                                                                                                                                     \
    "fedit: edit a file.\n"                                                                                                                                  \
    "\n"                                                                                                                                                     \
    "optional arguments\n"                                                                                                                                   \
    "   -h, --help\n"                                                                                                                                        \
    "       Show usage statement and exit.\n"                                                                                                                \
    "\n"                                                                                                                                                     \
    "   -l, --rotate-left NROTL\n"                                                                                                                           \
    "       Rotate the file X NROTL bytes left.\n"                                                                                                           \
    "\n"                                                                                                                                                     \
    "   -x, --expand NEXPAND\n"                                                                                                                              \
    "       Expand the file by NEXPAND bytes. The value of these bytes is given by the --value option.\n"                                                    \
    "\n"                                                                                                                                                     \
    "   -c, --contract NCONTRACT\n"                                                                                                                          \
    "       Contract the file by NCONTRACT bytes. If NCONTRACT > FSIZE shrink the file to 0 bytes.\n"                                                        \
    "\n"                                                                                                                                                     \
    "   -k, --keep NKEEP\n"                                                                                                                                  \
    "       Keep NKEEP bytes (starting from the offset provided by --skip) and remove all others. In the case of EOF, fewer than NKEEP bytes may be kept.\n" \
    "\n"                                                                                                                                                     \
    "   -v, --value CHAR\n"                                                                                                                                  \
    "       The character value that is used when expanding the file (default: A).\n"                                                                        \
    "\n"                                                                                                                                                     \
    "   -s, --skip NSKIP\n"                                                                                                                                  \
    "       Optional with --keep, skip NSKIP bytes before keeping (default: 0). Usage without --keep will result in an error\n"                              \
    "\n"

////////////////////////////////////////////////////////////////////////////////////////////

// --help
static void usage(int status)
{
    puts(USAGE);
    exit(status);
}

char *rotate_left(char *buffer, int NROTl, int size)
{
    int nrotl = NROTl;
    int rotations = nrotl % size;

    char *temp = (char *)malloc(size * sizeof(char));
    if (temp == NULL)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    // Rotate the buffer contents to the left by NROTl positions
    for (int i = 0; i < size; i++)
    {
        temp[i] = buffer[(i + rotations) % size];
    }

    // Copy rotated content back to the original buffer
    memcpy(buffer, temp, size);

    free(temp);

    return buffer;
}

char *expand(char *buffer, int NEXPAND, int size, char value)
{
    // Calculate the new size after expansion
    int new_size = size + NEXPAND;

    // Allocate a new buffer with the expanded size
    char *new_buffer = (char *)malloc(new_size * sizeof(char));
    if (new_buffer == NULL)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    memcpy(new_buffer, buffer, size);

    for (int i = size; i < new_size; i++)
    {
        new_buffer[i] = value;
    }

    free(buffer);

    return new_buffer;
}

char *contract(char *buffer, int NCONTRACT, int size)
{
    // Calculate the new size after contraction
    int new_size = size - NCONTRACT;

    // Ensure new_size is not less than 0
    if (new_size < 0)
    {
        new_size = 0;
    }

    char *new_buffer = (char *)malloc(new_size * sizeof(char));
    if (new_buffer == NULL)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    memcpy(new_buffer, buffer, new_size);

    free(buffer);

    return new_buffer;
}

char *keep(char *buffer, int NKEEP, int NSKIP, int size)
{
    int start_pos = NSKIP;

    // Ensure the start position is within the buffer size
    if (start_pos > size)
    {
        start_pos = size;
    }

    int keep_size = NKEEP;
    if (start_pos + keep_size > size)
    {
        keep_size = size - start_pos; // Keep only until the end of the buffer
    }

    char *new_buffer = (char *)malloc(keep_size * sizeof(char));
    if (new_buffer == NULL)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    memcpy(new_buffer, buffer + start_pos, keep_size);

    free(buffer);

    return new_buffer;
}

int main(int argc, char *argv[])
{
    int opt;
    int opt_count = 0;

    // argument variables
    int NROTL = 0;
    int NEXPAND = 0;
    int NCONTRACT = 0;
    int NKEEP = 0;
    int NSKIP = 0;
    char NCHAR = 'A';

    // argument flags
    int bool_rotate_left = 0;
    int bool_expand = 0;
    int bool_contract = 0;
    int bool_keep = 0;

    /*
     * An option that takes a required argument is followed by a ':'.
     * The leading ':' suppresses getopt_long's normal error handling.
     */

    const char *short_opts = ":hl:x:c:k:v:s:";
    struct option long_opts[] = {
        {"help", no_argument, NULL, 'h'},
        {"rotate-left", required_argument, NULL, 'l'},
        {"expand", required_argument, NULL, 'x'},
        {"contract", required_argument, NULL, 'c'},
        {"keep", required_argument, NULL, 'k'},
        {"value", required_argument, NULL, 'v'},
        {"skip", required_argument, NULL, 's'},
        {NULL, 0, NULL, 0}};

    while (1)
    {
        opt = getopt_long(argc, argv, short_opts, long_opts, NULL);
        if (opt == -1)
        {
            /* processed all command-line options */
            break;
        }

        // switch block, call external functions here
        switch (opt)
        {
        case 'h':
        {
            usage(0);
            break;
        }
        case 'l':
        {
            opt_count++;
            if (optarg)
            {
                NROTL = atoi(optarg); // Convert the argument to an integer
                if (NROTL < 0)
                {
                    fprintf(stderr, "NROTL must be a non-negative integer.\n");
                    return 1;
                }
            }
            else
            {
                fprintf(stderr, "Option -l requires a numeric argument.\n");
                return 1;
            }
            bool_rotate_left = 1;
            break;
        }
        case 'x':
        {
            opt_count++;
            if (optarg)
            {
                NEXPAND = atoi(optarg); // Convert the argument to an integer
                if (NEXPAND < 0)
                {
                    fprintf(stderr, "NEXPAND must be a non-negative integer.\n");
                    return 1;
                }
            }
            else
            {
                fprintf(stderr, "Option -x requires a numeric argument.\n");
                return 1;
            }
            bool_expand = 1;
            break;
        }
        case 'c':
        {
            opt_count++;
            if (optarg)
            {
                NCONTRACT = atoi(optarg); // Convert the argument to an integer
                if (NCONTRACT < 0)
                {
                    fprintf(stderr, "NCONTRACT must be a non-negative integer.\n");
                    return 1;
                }
            }
            else
            {
                fprintf(stderr, "Option -c requires a numeric argument.\n");
                return 1;
            }
            bool_contract = 1;
            break;
        }
        case 'k':
        {
            opt_count++;
            if (optarg)
            {
                NKEEP = atoi(optarg); // Convert the argument to an integer
                if (NKEEP < 0)
                {
                    fprintf(stderr, "NKEEP must be a non-negative integer.\n");
                    return 1;
                }
            }
            else
            {
                fprintf(stderr, "Option -k requires a numeric argument.\n");
                return 1;
            }
            bool_keep = 1;
            break;
        }
        case 'v':
        {
            if (optarg)
            {
                NCHAR = *optarg;
            }
            else
            {
                fprintf(stderr, "Option -v requires an alphabetical argument.\n");
                return 1;
            }
            break;
        }
        case 's':
        {
            if (optarg)
            {
                NSKIP = atoi(optarg); // Convert the argument to an integer
                if (NSKIP < 0)
                {
                    fprintf(stderr, "NSKIP must be a non-negative integer.\n");
                    return 1;
                }
            }
            else
            {
                fprintf(stderr, "Option -s requires a numeric argument.\n");
                return 1;
            }
            break;
        }
        case '?':
            mu_die("unknown option '%c' (decimal: %d)", optopt, optopt);
            break;
        case ':':
            mu_die("missing option argument for option %c", optopt);
            break;
        default:
            mu_die("unexpected getopt_long return value: %c\n", (char)opt);
        }
    }

    if (opt_count > 1)
    {
        fprintf(stderr, "Error: Only one option (rotate-left, expand, contract, keep) can be used at a time.\n");
        exit(1);
    }
    // begin file handling and buffer creation

    char *path = argv[optind]; // set path

    // open the file
    FILE *file = fopen(path, "rb+");
    if (file == NULL)
    {
        perror("Error opening file");
        return 1;
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(file_size); // set the buffer size

    fread(buffer, 1, file_size, file); // write the contents fo the file into the buffer

    // add flags to switch statement and begin making separate functions
    // create if statements to evaluate flags with nested if statements to check for value (expand) and skip (keep)

    if (bool_rotate_left)
    {
        char *new_file = rotate_left(buffer, NROTL, file_size);
        fseek(file, 0, SEEK_SET);
        fwrite(new_file, 1, file_size, file);
        free(new_file);
    }
    else if (bool_expand)
    {
        char *new_file = expand(buffer, NEXPAND, file_size, NCHAR);
        int new_size = file_size + NEXPAND;
        fseek(file, 0, SEEK_SET);
        fwrite(new_file, 1, new_size, file);
        free(new_file);
    }
    else if (bool_contract)
    {
        char *new_file = contract(buffer, NCONTRACT, file_size);
        int new_size = file_size - NCONTRACT;
        if (new_size < 0)
            new_size = 0;

        file = freopen(path, "wb", file);
        if (file == NULL)
        {
            perror("Error reopening file for writing");
            free(new_file);
            return 1;
        }
        if (new_size > 0)
        {
            fwrite(new_file, 1, new_size, file);
        }
        free(new_file);
    }
    else if (bool_keep)
    {
        char *new_file = keep(buffer, NKEEP, NSKIP, file_size);
        int new_size;
        if (NSKIP >= file_size)
        {
            new_size = 0; // If skip is greater than or equal to file size, no content should be kept
        }
        else if (NSKIP + NKEEP > file_size)
        {
            new_size = file_size - NSKIP; // Keep until the end of the file
        }
        else
        {
            new_size = NKEEP; // Keep the specified number of bytes
        }
        file = freopen(path, "wb", file);
        if (file == NULL)
        {
            perror("Error reopening file for writing");
            free(new_file);
            return 1;
        }
        if (new_size > 0)
        {
            fwrite(new_file, 1, new_size, file);
        }
        free(new_file);
    }
    fclose(file);
}
