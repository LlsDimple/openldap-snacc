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

extern int UnEscapeDquote ( char* str, int* strLen );

AsnLen
GEncDirectoryStringContent PARAMS ((b, o),
    GenBuf *b _AND_
    GDirectoryString *o)
{
	GTeletexString tel_str;
	GPrintableString print_str;
	GBMPString bmp_str;
	GUniversalString uni_str;
	GUTF8String utf_str;

	int rc;
	switch ( o->strType )
	{
		case teletexStr :
			tel_str.value = o->value;
			rc = GEncTeletexStringContent( b, &tel_str);
		case printableStr:
			print_str.value = o->value;
			rc = GEncPrintableStringContent( b, &print_str);
		case bmpStr:
			bmp_str.value = o->value;
			rc = GEncBMPStringContent( b, &bmp_str);
		case universalStr:
			uni_str.value = o->value;
			rc = GEncUniversalStringContent( b, &uni_str);
		case utf8Str:
			utf_str.value = o->value;
			rc = GEncUTF8StringContent( b, &utf_str);
		default :
			utf_str.value = o->value;
			rc = GEncUTF8StringContent( b, &utf_str);
	}
	return rc;
}

/*
 * GSER Decodes the content of DirectoryString
 */
#ifdef LDAP_COMPONENT
int
GDecDirectoryStringContent PARAMS ((mem_op, b, result, bytesDecoded ),
    void* mem_op _AND_
    GenBuf *b _AND_
    GDirectoryString *result _AND_
    AsnLen *bytesDecoded )
{
	int strLen;
	char* peek_head, *separator;
	
	*bytesDecoded = 0;
	if ( !(strLen = LocateNextGSERToken( mem_op, b, &peek_head, GSER_NO_COPY )) ){
		Asn1Error("UTF8String : Token Reading ERROR\n");
		return (-1);
	}
	*bytesDecoded += strLen;
	if ( !(strLen = LocateNextGSERToken( mem_op, b, &separator, GSER_NO_COPY )) ){
		Asn1Error("DirectoryString : Token Reading ERROR\n");
		return (-1);
	}
	if ( separator[0] != ':' ) {
		Asn1Error("DirectoryString : Token Reading ERROR\n");
		return (-1);
	}
	*bytesDecoded += strLen;

	if ( strncmp ( peek_head, "teletexString", sizeof ("teletexString") ) == 0 ){
		GDecTeletexStringContent( mem_op, b, (GTeletexString*)result, bytesDecoded );
	} 
	else if ( strncmp ( peek_head, "printableString", sizeof ("printableString") ) == 0 ){
		GDecPrintableStringContent( mem_op,  b, (GPrintableString*)result, bytesDecoded );
	} 
	else if ( strncmp ( peek_head, "bmpString", sizeof ("bmpString") ) == 0 ){
		GDecBMPStringContent( mem_op, b, (GBMPString*)result, bytesDecoded);
	} 
	else if ( strncmp ( peek_head, "universalString", sizeof ("universalString") ) == 0 ){
		GDecUniversalStringContent( mem_op, b, (GUniversalString*)result, bytesDecoded );
	} 
	else if ( strncmp ( peek_head, "uTF8String", sizeof ("uTF8String") ) == 0 ){
		GDecUTF8StringContent( mem_op, b, (GUTF8String*)result, bytesDecoded );
	}
	else {
		if ( *peek_head != '\"'){
			Asn1Error("StringValue : Should Begin with \" \n");
			return -1;
		}
		/* Read StringValue */
		if ( !(strLen = LocateNextGSERToken( mem_op, b, &peek_head, GSER_NO_COPY )) ){
			Asn1Error("StringValue : Token Reading ERROR\n");
			return -1;
		}

		*bytesDecoded += strLen;

		result->value.octs = peek_head;
		result->value.octetLen = strLen;
		if( !UnEscapeDquote( peek_head, &strLen ) ) {
			Asn1Error("StringValue : Unsafe UTF8 Character\n");
			return -1;
		}

		if ( !(strLen = LocateNextGSERToken( mem_op, b, &peek_head, GSER_NO_COPY )) ){
			Asn1Error("StringValue : Token Reading ERROR\n");
			return -1;
		}
	
		*bytesDecoded += strLen;

		if ( *peek_head != '\"'){
			Asn1Error("StringValue :  Should end with \" \n");
			return -1;
		}
	}
	return 1;
}
#else
int
GDecDirectoryStringContent PARAMS ((b, result, bytesDecoded, env),
    GenBuf *b _AND_
    GAsnOcts *result _AND_
    AsnLen *bytesDecoded _AND_
    jmp_buf env)
{
	int strLen;
	char* peek_head, *separator;
	
	*bytesDecoded = 0;
	if ( !(strLen = LocateNextGSERToken( mem_op, b, &peek_head, GSER_NO_COPY )) ){
		Asn1Error("UTF8String : Token Reading ERROR\n");
		longjmp( env, -20);
	}
	*bytesDecoded += strLen;
	if ( !(strLen = LocateNextGSERToken( mem_op, b, &separator, GSER_NO_COPY )) ){
		Asn1Error("DirectoryString : Token Reading ERROR\n");
		longjmp( env, -20);
	}
	if ( separator[0] != ':' ) {
		Asn1Error("DirectoryString : Token Reading ERROR\n");
		longjmp( env, -20);
	}
	*bytesDecoded += strLen;

	if ( strncmp ( peek_head, "teletexString", sizeof ("teletexString") ) == 0 ){
		GDecTeletexStringContent( mem_op, b, (GTeletexString*)result, bytesDecoded );
	} 
	else if ( strncmp ( peek_head, "printableString", sizeof ("printableString") ) == 0 ){
		GDecPrintableStringContent( mem_op,  b, (GPrintableString*)result, bytesDecoded );
	} 
	else if ( strncmp ( peek_head, "bmpString", sizeof ("bmpString") ) == 0 ){
		GDecBMPStringContent( mem_op, b, (GBMPString*)result, bytesDecoded);
	} 
	else if ( strncmp ( peek_head, "universalString", sizeof ("universalString") ) == 0 ){
		GDecUniversalStringContent( mem_op, b, (GUniversalString*)result, bytesDecoded );
	} 
	else if ( strncmp ( peek_head, "uTF8String", sizeof ("uTF8String") ) == 0 ){
		GDecUTF8StringContent( mem_op, b, (GUTF8String*)result, bytesDecoded );
	}
	else {
		if ( *peek_head != '\"'){
			Asn1Error("StringValue : Should Begin with \" \n");
			longjmp( env, -20);
		}
		/* Read StringValue */
		if ( !(strLen = LocateNextGSERToken( mem_op, b, &peek_head, GSER_NO_COPY )) ){
			Asn1Error("StringValue : Token Reading ERROR\n");
			longjmp( env, -20);
		}

		*bytesDecoded += strLen;

		result->value.octs = peek_head;
		result->value.octetLen = strLen;
		if( !UnEscapeDquote( peek_head, &strLen ) ) {
			Asn1Error("StringValue : Unsafe UTF8 Character\n");
			longjmp( env, -20);
		}

		if ( !(strLen = LocateNextGSERToken( mem_op, b, &peek_head, GSER_NO_COPY )) ){
			Asn1Error("StringValue : Token Reading ERROR\n");
			longjmp( env, -20);
		}
	
		*bytesDecoded += strLen;

		if ( *peek_head != '\"'){
			Asn1Error("StringValue :  Should end with \" \n");
			longjmp( env, -20);
		}
	}
}
#endif
