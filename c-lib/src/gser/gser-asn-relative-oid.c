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

#include <string.h>

#include "asn-config.h"
#include "asn-gser.h"

/*
 * Decodes just the content of the RELATIVE_OID.
 * RFC 3641
 * RelativeOIDValue = oid-component *( "." oid-component )
 * oid-component = "0"/positive-number
 */
void
GDecAsnRelativeOidContent PARAMS ((b, result, bytesDecoded, env),
    GenBuf *b _AND_
    GAsnRelativeOid *result _AND_
    AsnLen *bytesDecoded _AND_
    jmp_buf env)
{
	char* peek_head;
	unsigned long strLen = INDEFINITE_LEN;

	peek_head = BufPeekSeg( b, &strLen );

	if ( strLen == INDEFINITE_LEN ){
		Asn1Error("Not in the format of GSER encoded Relative OID\"\n");
		longjmp( env, -20);
	}
	result->value.octetLen = strLen;
	result->value.octs = Asn1Alloc(strLen+1);
	CheckAsn1Alloc( result->value.octs, env );
	BufCopy( result->value.octs, b, strLen );

	if ( BufReadError(b) )
	{
		Asn1Error("BMP String Read Error\n");
		longjmp( env, -20);
	}

	result->value.octs[strLen] = '\0';
	*bytesDecoded = strLen;

}

/*
 * Matching Rule for Relative OID
 * If and only if the values have the same number of arcs and 
 * corresponding arcs are the same
 */
AsnInt GMatchingAsnRelativeOidContent PARAMS (( a, b ),
	GAsnRelativeOid *a _AND_
	GAsnRelativeOid  *b)
{
	assert( a );
	assert( b );
	assert( a->value.octs );
	assert( b->value.octs );
	return ( strcmp ( a->value.octs, b->value.octs ) == 0 );
}
