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

#include <memory.h>
#include <ctype.h>

#include "asn-config.h"
#include "asn-gser.h"

/*
 * GSER encodes just the content of an UTF8String
 * StringValue	= dquote *SafeUTF8Character dquote
 * dquote	= %x22
 */
AsnLen
GEncAsnUTF8StringContent PARAMS ((b, o),
    GenBuf *b _AND_
    GAsnOcts *o)
{
	/* Need to be Implemented */
}

int
UnEscapeDquote PARAMS (( str , strLen ),
	char* str _AND_
	int* strLen )
{
	int i;
	int count = 0;
	for ( i = 0 ; i < *strLen ; i ++ ) {
		if ( str [ i + count ] == '\"' &&
			str [ i + count + 1] != '\"' ) {
			/* unsafe UTF8 Character. Stop processing */
			return 0;
		}

		if ( str [ i + count ] == '\"' &&
			str [ i + count + 1] == '\"' ) {
			count++;
		}
		str [ i ] = str [ i + count ];
	}
	/* Decrease in string length by the number of escapsed characters */
	*strLen -= count;
	return 1;
}

/*
 * GSER Decodes the content of a GSER UTF8String
 */
void
GDecAsnUTF8StringContent PARAMS ((b, result, bytesDecoded, env),
    GenBuf *b _AND_
    GAsnOcts *result _AND_
    AsnLen *bytesDecoded _AND_
    jmp_buf env)
{
	long strLen;
	char* peek_head;
	
	*bytesDecoded = 0;
	if ( !(strLen = LocateNextGSERToken( b, &peek_head, GSER_NO_COPY )) ){
		Asn1Error("UTF8String : Token Reading ERROR\n");
		longjmp( env, -20);
	}

	*bytesDecoded += strLen;

	if ( *peek_head != '\"'){
		Asn1Error("UTFString :  Should Begin with \" \n");
		longjmp( env, -20);
	}
	/* Read StringValue */
	if ( !(strLen = LocateNextGSERToken( b, &peek_head, GSER_NO_COPY )) ){
		Asn1Error("UTF8String : Token Reading ERROR\n");
		longjmp( env, -20);
	}

	*bytesDecoded += strLen;

	result->value.octs = peek_head;
	result->value.octetLen = strLen;
	if( !UnEscapeDquote( peek_head, &strLen ) ) {
		Asn1Error("UTF8String : Unsafe UTF8 Character\n");
		longjmp( env, -20);
	}
	

	if ( !(strLen = LocateNextGSERToken( b, &peek_head, GSER_NO_COPY )) ){
		Asn1Error("UTF8String : Token Reading ERROR\n");
		longjmp( env, -20);
	}
	
	*bytesDecoded += strLen;

	if ( *peek_head != '\"'){
		Asn1Error("UTFString :  Should end with \" \n");
		longjmp( env, -20);
	}
}
