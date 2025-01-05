# fedit
## Basic byte manipulation engine made in C, a simple version of a file editor
### NOTE: fedit implementation makes use of the Mu.c file for many of its functions. This helper file was not created by myself, all credit to original authors.

fedit includes one text file for use, though any text file may be used alongside the code.

## Functionality includes:

### -h --help option

Print a usage statement to stdout and exit with status 0.

### -l, --rotate-left NROTL

Rotate the file NROTL bytes left (with wrap around).

### -x, --expand NEXPAND

Expand the file by NEXPAND. The value of these bytes is given by the --value option.

### -c, --contract NCONTRACT

Contract (shrink) the file by NCONTRACT bytes. If NCONTRACT > FSIZE, shrink the file to 0 bytes.

### -v, --value CHAR

The character value that is used when expanding the file.
Default: A.

### -k, --keep NKEEP

Keep this many bytes (starting from the offset provided by --skip) (and remove all others). In the case of EOF, fewer than NKEEP bytes may be kept.

### -s, --skip NSKIP
Optionally used with --keep. Skip NKSIP bytes before keeping.
Default: 0
It is an error to use this option if the user does not also specify --keep.
