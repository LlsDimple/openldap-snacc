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


#include "asn-config.h"
#include "asn-gser.h"
#include <string.h>

/*
 * GSER Decodes just the content of the OID.
 * AsnOid is handled the same as a primtive octet string
 * RFC 3641
 * ObjectIdentifierValue = numeric-oid / descr
 * numeric-oid           = oid-component 1*("." oid-component)
 * oid-component         = "0" / positive-number
 * The encoder is the same to the encoder of OCTS String
 */

void
GDecAsnOidContent PARAMS ((b, result, bytesDecoded, env),
    GenBuf *b _AND_
    GAsnOid *result _AND_
    AsnLen *bytesDecoded _AND_
    jmp_buf env)
{
	char* peek_head;
	unsigned long strLen = INDEFINITE_LEN;

	peek_head = BufPeekSeg( b,&strLen );

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
