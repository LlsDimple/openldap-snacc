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

AsnLen
GEncAsnNullContent PARAMS ((b, data),
    GenBuf *b _AND_
    GAsnNull *data)
{
    BufPutSegRvs(b, "NULL", 4);

    if ( data->identifier ){
	BufPutByteRvs(b,' ');
	BufPutSegRvs(b, data->identifier, strlen(data->identifier));
    }
    return 4;
}

#ifdef LDAP_COMPONENT
int
GDecAsnNullContent PARAMS (( mem_op, b, result, bytesDecoded ),
    void* mem_op _AND_
    GenBuf *b _AND_
    GAsnNull *result _AND_
    AsnLen *bytesDecoded )
{
	char* data;
	unsigned long int len=4;

	*bytesDecoded = 0;
	data = BufGetSeg(b,&len);
	if ( BufReadError(b) ) {
		Asn1Error("NULL Read Error\n");
		return -1;
	}

	if ( strncmp(data,"NULL", 4) != 0 ) {
		Asn1Error("Not in the format of NULL\n");
		return -1;
	}

	result->value = (int) NULL;
	*bytesDecoded = 4;

	return 1;
}
#else
void
GDecAsnNullContent PARAMS ((b, result, bytesDecoded, env),
    GenBuf *b _AND_
    GAsnNull *result _AND_
    AsnLen *bytesDecoded _AND_
    jmp_buf env)
{
	char* data;
	unsigned long int len=4;

	*bytesDecoded = 0;
	data = BufGetSeg(b,&len);
	if ( BufReadError(b) )
	{
		Asn1Error("NULL Read Error\n");
		longjmp( env, -20);
	}

	if ( strncmp(data,"NULL", 4) != 0 ){
		Asn1Error("Not in the format of NULL\n");
		longjmp( env, -20);
	}

	result->value = (int) NULL;
	*bytesDecoded = 4;
}
#endif
