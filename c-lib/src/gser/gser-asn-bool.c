/* Copyright 2004 IBM Corporation
 * All rights reserved.
 * Redisribution and use in source and binary forms, with or without
 * modification, are permitted only as  authorizd by the OpenLADP
 * Public License.
 */
/* ACKNOWLEDGEMENTS
 * This work originally developed by Sang Seok Lim
 * 2004/06/18	03:20:00	slim@OpenLDAP.org
 */

#include <stdlib.h>
#include <string.h>

#include "asn-config.h"
#include "asn-gser.h"

/*
 * GSER-Encodes just the content of the given BOOLEAN value to the given buffer.
 */
AsnLen
GEncAsnBoolContent PARAMS ((b, data),
    GenBuf *b _AND_
    GAsnBool  *data)
{
    int encoded;
    if ( data->value > 0 ){
        BufPutSegRvs(b, " TRUE", 5 );
	encoded = 4;
    }
    else {
        BufPutSegRvs(b, " FALSE", 6 );
	encoded = 5;
    }

    if ( data->identifier )
	BufPutSegRvs(b, data->identifier, strlen(data->identifier));

    return encoded;
}  

/*
 * GSER-Decodes just the content of an ASN.1 BOOLEAN from the given buffer.
 * longjmps if there is a buffer reading problem
 */
void
GDecAsnBoolContent PARAMS ((b, result, bytesDecoded, env),
    GenBuf *b _AND_
    GAsnBool  *result _AND_
    AsnLen  *bytesDecoded _AND_
    jmp_buf env)
{
    char* data;
    unsigned long len;

    len = 4;
    data = BufGetSeg (b, &len);
    *bytesDecoded = 0;

    if ( BufReadError (b) )
    {
         Asn1Error ("GDecAsnBoolContent: ERROR\n");
         longjmp (env, -6);
    }

    if ( data[0] == 'T' ){
       result->value = 1;
       (*bytesDecoded) = 4;
    }
    else if ( data[0] == 'F' ){
       result->value = 0;
       /* To move the pointer after "E" of "FALSE"to the right position */
       BufGetByte (b);
       (*bytesDecoded) = 5;
    }
    else {
	Asn1Error("Invalid BOOLEAN Format\n");
	longjmp(env, -20);
    }
}

/*
 *
 */
AsnInt
GMatchingAsnBoolContent PARAMS ((a, b),
	GAsnBool *a _AND_
	GAsnBool *b)
{
	assert( a );
	assert( b );
	return 1;/* yet to be implemented*/
}
