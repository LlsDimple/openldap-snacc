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
 * GSER encodes universal TAG LENGTH and Contents of and ASN.1 ENUMERATED
 */
AsnLen
GEncAsnEnumContent PARAMS ((b, data),
    GenBuf *b _AND_
    GAsnEnum *data)
{
    AsnLen len;

    len = strlen(data->value_identifier);
    BufPutSegRvs(b, data->value_identifier, len);
    BufPutByteRvs(b,' ');
    if ( data->identifier ){
	len = strlen(data->identifier);
	BufPutSegRvs(b, data->identifier, len);
    }
    return len;
}

/*
 * GSER decodes ENUMERATE
 */
void
GDecAsnEnumContent PARAMS ((b, result, bytesDecoded, env),
    GenBuf *b _AND_
    GAsnEnum    *result _AND_
    AsnLen *bytesDecoded _AND_
    jmp_buf env)
{
	char* peek_head;
	long strLen;

	*bytesDecoded = 0;
	if ( !(strLen = LocateNextGSERToken( b,&peek_head, GSER_NO_COPY )) ){
		Asn1Error("ENUMERATED : Token Reading ERROR\n");
		longjmp( env, -20);
	}

	result->value_identifier = peek_head;
	result->len = strLen;

	*bytesDecoded += strLen;
}
