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
#include "asn-gser.h"
#include "gen-buf.h"
AsnLen
GEncTeletexStringContent PARAMS ((b, o),
    GenBuf *b _AND_
    GTeletexString *o)
{
	/* YET-To-Be-Implemented */
	return 0;
}

/*
 * GSER Decodes the content of a GSER Teletex String
 */
#ifdef LDAP_COMPONENT
int
GDecTeletexStringContent PARAMS ((b, result, bytesDecoded ),
    GenBuf *b _AND_
    GTeletexString *result _AND_
    AsnLen *bytesDecoded )
{
	long strLen;
	char* peek_head;
	
	*bytesDecoded = 0;
	if ( !(strLen = LocateNextGSERToken( b, &peek_head, GSER_NO_COPY )) ){
		Asn1Error("TeletexString : Token Reading ERROR\n");
		return -1;
	}

	*bytesDecoded += strLen;

	if ( *peek_head != '\"'){
		Asn1Error("UTFString :  Should Begin with \" \n");
		return -1;
	}
	/* Read StringValue */
	if ( !(strLen = LocateNextGSERToken( b, &peek_head, GSER_COPY )) ){
		Asn1Error("TeletexString : Token Reading ERROR\n");
		return -1;
	}

	*bytesDecoded += strLen;

	result->value.octs = peek_head;
	result->value.octetLen = strLen;

	if ( !(strLen = LocateNextGSERToken( b, &peek_head, GSER_NO_COPY )) ){
		Asn1Error("TeletexString : Token Reading ERROR\n");
		return -1;
	}
	
	*bytesDecoded += strLen;

	if ( *peek_head != '\"'){
		Asn1Error("UTFString :  Should end with \" \n");
		return -1;
	}
	return 1;
}
#else
void
GDecTeletexStringContent PARAMS ((b, result, bytesDecoded, env),
    GenBuf *b _AND_
    GTeletexString *result _AND_
    AsnLen *bytesDecoded _AND_
    jmp_buf env)
{
	long strLen;
	char* peek_head;
	
	*bytesDecoded = 0;
	if ( !(strLen = LocateNextGSERToken( b, &peek_head, GSER_NO_COPY )) ){
		Asn1Error("TeletexString : Token Reading ERROR\n");
		longjmp( env, -20);
	}

	*bytesDecoded += strLen;

	if ( *peek_head != '\"'){
		Asn1Error("UTFString :  Should Begin with \" \n");
		longjmp( env, -20);
	}
	/* Read StringValue */
	if ( !(strLen = LocateNextGSERToken( b, &peek_head, GSER_NO_COPY )) ){
		Asn1Error("TeletexString : Token Reading ERROR\n");
		longjmp( env, -20);
	}

	*bytesDecoded += strLen;

	result->value.octs = peek_head;
	result->value.octetLen = strLen;

	if ( !(strLen = LocateNextGSERToken( b, &peek_head, GSER_NO_COPY )) ){
		Asn1Error("TeletexString : Token Reading ERROR\n");
		longjmp( env, -20);
	}
	
	*bytesDecoded += strLen;

	if ( *peek_head != '\"'){
		Asn1Error("UTFString :  Should end with \" \n");
		longjmp( env, -20);
	}
}
#endif
