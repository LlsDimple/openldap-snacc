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
#include "asn-oid.h"
#include <string.h>
#include <stdlib.h>

/*
 * GSER Decodes just the content of the OID.
 * AsnOid is handled the same as a primtive octet string
 * RFC 3641
 * ObjectIdentifierValue = numeric-oid / descr
 * numeric-oid           = oid-component 1*("." oid-component)
 * oid-component         = "0" / positive-number
 */

/*
 * move pointer the next to '.' and return it
 * if there is no more '.' then return NULL
 */

int
IsNumericASCII( char c ) {
	return ((c >= '0') && ( c <= '9' ));
}

char*
LocateNextCompOid ( char* oid ) {
	int i;
	for ( i=0 ; IsNumericASCII( oid[i] ) ; i++ );
	if ( oid[i] == '.' ) return (char*)( oid + i + 1 );
	else return NULL ;
}

#ifdef LDAP_COMPONENT
char*
EncodeComponentOid( void* mem_op, char* gser_oid, int* len ) {
	OID *tmpOid ,*listOid, *currOid;
	char* pos;
	AsnOid result;

	result.octetLen = 0;
	result.octs = NULL;

	tmpOid = NULL;
	listOid = NULL;
	currOid = NULL;
	for ( pos = gser_oid ; pos ; ) {
		if( !tmpOid ) {
			listOid = tmpOid = (OID*)malloc(sizeof(OID));
		}
		else {
			tmpOid->next = (OID*)malloc(sizeof(OID));
			tmpOid = tmpOid->next;
		}
		tmpOid->arcNum = atoi( pos );
		if ( tmpOid->arcNum < 0 ) goto oid_free;
		tmpOid->next = NULL;
		pos = LocateNextCompOid(pos);
	}

	result.octetLen = EncodedOidLen ( listOid );
	if ( result.octetLen <= 0 ) goto oid_free;
	result.octs = (char*)CompAlloc( mem_op, result.octetLen );

	BuildEncodedOid( listOid, &result );

oid_free :
	for ( currOid = listOid ; currOid ; ) {
		tmpOid = currOid;
		currOid = currOid->next;
		free ( tmpOid );
	}

	*len = result.octetLen;
	return result.octs;
}
#else
char*
EncodeComponentOid( char* gser_oid, int* len ) {
	OID *tmpOid ,*listOid, *currOid;
	char* pos;
	AsnOid result;

	result.octetLen = 0;
	result.octs = NULL;

	tmpOid = NULL;
	listOid = NULL;
	currOid = NULL;
	for ( pos = gser_oid ; pos ; ) {
		if( !tmpOid ) {
			listOid = tmpOid = (OID*)malloc(sizeof(OID));
		}
		else {
			tmpOid->next = (OID*)malloc(sizeof(OID));
			tmpOid = tmpOid->next;
		}
		tmpOid->arcNum = atoi( pos );
		if ( tmpOid->arcNum < 0 ) goto oid_free;
		tmpOid->next = NULL;
		pos = LocateNextCompOid(pos);
	}

	result.octetLen = EncodedOidLen ( listOid );
	if ( result.octetLen <= 0 ) goto oid_free;
	result.octs = (char*)malloc( result.octetLen );

	BuildEncodedOid( listOid, &result );

oid_free :
	for ( currOid = listOid ; currOid ; ) {
		tmpOid = currOid;
		currOid = currOid->next;
		free ( tmpOid );
	}

	*len = result.octetLen;
	return result.octs;
}
#endif

#ifdef LDAP_COMPONENT
int
GDecAsnOidContent PARAMS (( mem_op, b, result, bytesDecoded ),
    void* mem_op _AND_
    GenBuf *b _AND_
    GAsnOid *result _AND_
    AsnLen *bytesDecoded )
{
	char* peek_head;
	int strLen = INDEFINITE_LEN;

	strLen = LocateNextGSERToken(mem_op,b,&peek_head, GSER_NO_COPY);

	if ( strLen == INDEFINITE_LEN ){
		Asn1Error("Not in the format of GSER encoded Relative OID\"\n");
		return -1;
	}

	result->value.octs = EncodeComponentOid( mem_op, peek_head, &strLen );
	result->value.octetLen = strLen;

	if ( !result->value.octs ) return (-1);

	*bytesDecoded = strLen;

	return 1;
}
#else
void
GDecAsnOidContent PARAMS ((b, result, bytesDecoded, env),
    GenBuf *b _AND_
    GAsnOid *result _AND_
    AsnLen *bytesDecoded _AND_
    jmp_buf env)
{
	char* peek_head;
	unsigned long strLen = INDEFINITE_LEN;

	strLen = LocateNextGSERToken(b,GSER_PEEK);

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
#endif
