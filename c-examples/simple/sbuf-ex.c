/*
 * c_examples/simple/sbuf_ex.c - an example of how to call C ASN.1-BER
 *             encoders and decoders generated by snacc
 *             using the SBuf buffer.
 *
 * AUTHOR: Mike Sample
 * DATE:   Mar 92
 *
 * $Header: /baseline/SNACC/c-examples/simple/sbuf-ex.c,v 1.2 2003/12/17 19:05:03 gronej Exp $
 * $Log: sbuf-ex.c,v $
 * Revision 1.2  2003/12/17 19:05:03  gronej
 * SNACC baseline merged with PER v1_7 tag
 *
 * Revision 1.1.2.1  2003/11/05 14:58:57  gronej
 * working PER code merged with esnacc_1_6
 *
 * Revision 1.1.1.1  2000/08/21 20:36:07  leonberp
 * First CVS Version of SNACC.
 *
 * Revision 1.5  1995/07/24  20:47:00  rj
 * changed `_' to `-' in file names.
 *
 * Revision 1.4  1995/02/18  15:12:56  rj
 * cosmetic changes
 *
 * Revision 1.3  1994/09/01  01:02:39  rj
 * more portable .h file inclusion.
 *
 * Revision 1.2  1994/08/31  08:59:37  rj
 * first check-in. for a list of changes to the snacc-1.1 distribution please refer to the ChangeLog.
 *
 */

#include "asn-incl.h"

#include <sys/types.h>
#include <sys/stat.h>
#if HAVE_FCNTL_H 
#include <fcntl.h>
#endif
#include <stdio.h>

#include "p-rec.h"

main PARAMS ((argc, argv),
    int argc _AND_
    char *argv[])
{
    int fd;
    SBuf  buf;
    SBuf  encBuf;
    char *encData;
    AsnLen encodedLen;
    AsnLen decodedLen;
    int     val;
    PersonnelRecord pr;
    int      size;
    char    *origData;
    struct stat sbuf;
    jmp_buf env;
    int  decodeErr;
    AsnTag tag;

    if (argc != 2)
    {
        fprintf (stderr, "Usage: %s <BER data file name>\n", argv[0]);
        fprintf (stderr, "   Decodes the given PersonnelRecord BER data file\n");
        fprintf (stderr, "   and re-encodes it to stdout\n");
        exit (1);
    }

    fd = open (argv[1], O_RDONLY, 0);
    if (fd < 0)
    {
        perror ("main: fopen");
        exit (1);
    }

    if (fstat (fd, &sbuf) < 0)
    {
        perror ("main: fstat");
        exit (1);
    }

    size = sbuf.st_size;
    origData = (char*)malloc (size);
    if (read (fd, origData, size) != size)
    {
        perror ("main: read");
        exit (1);
    }

    close (fd);

    /*
     * puts the given data 'origData' of 'size' bytes
     * into an SBuf and sets the SBuf up for reading
     * origData from the beginning
     */
    SBufInstallData (&buf, origData, size);

    /*
     * the first argument (512) is the number of bytes to
     * initially allocate for the decoder to allocate from.
     * The second argument (512) is the size in bytes to
     * enlarge the nibble memory by when it fills up
     */
    InitNibbleMem (512, 512);


    decodedLen = 0;
    decodeErr = FALSE;
    if ((val = setjmp (env)) == 0)
    {
        BDecPersonnelRecord (&buf, &pr, &decodedLen, env);
    }
    else
    {
        decodeErr = TRUE;
        fprintf (stderr, "ERROR - Decode routines returned %d\n",val);
    }

    if (decodeErr)
        exit (1);

    fprintf (stderr, "decodedValue PersonnelRecord ::= ");
    PrintPersonnelRecord (stderr, &pr, 0);
    fprintf (stderr, "\n\n");

    /*
     * setup a new buffer set up for writing.
     * make sure size is big enough to hold the encoded
     * value (may be larger than decoded value if encoding
     * with indef lengths - so add 512 slush bytes)
     */
    encData = (char*) malloc (size + 512);
    SBufInit (&encBuf, encData, size + 512);
    SBufResetInWriteRvsMode (&encBuf);

    encodedLen =  BEncPersonnelRecord (&encBuf, &pr);

    if (SBufWriteError (&encBuf))
    {
        fprintf (stderr, "ERROR - buffer to hold the encoded value was too small\n");
        exit (1);
    }

    /*
     * free all of the decoded value since
     * it has been encoded into the buffer.
     * This is much more efficient than freeing
     * each compontent of the value individually
     */
    ResetNibbleMem();

    /*
     * write encoded value from encBuf
     * to stdout
     */
    fwrite (SBufDataPtr (&encBuf), SBufDataLen (&encBuf), 1, stdout);

    return 0;
}
