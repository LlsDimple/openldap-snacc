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
 * GSER Decoder of a DirectoryString ASN.1 type
 */
void
GDecAsnDirectoryStringContent PARAMS ((b, result, bytesDecoded, env),
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

	if ( strncmp ( peek_head, "teletexString", sizeof ("teletexString") ) == 0 ){
		if ( !(strLen = LocateNextGSERToken( b, &peek_head, GSER_NO_COPY )) ){
			Asn1Error("DirectoryString : Token Reading ERROR\n");
			longjmp( env, -20);
		}
		GDecTeletexStringContent( b, (GTeletexString*)result, bytesDecoded, env );
	} 
	else if ( strncmp ( peek_head, "printableString", sizeof ("printableString") ) == 0 ){
		if ( !(strLen = LocateNextGSERToken( b, &peek_head, GSER_NO_COPY )) ){
			Asn1Error("DirectoryString : Token Reading ERROR\n");
			longjmp( env, -20);
		}
		GDecPrintableStringContent( b, (GPrintableString*)result, bytesDecoded, env );
	} 
	else if ( strncmp ( peek_head, "bmpString", sizeof ("bmpString") ) == 0 ){
		if ( !(strLen = LocateNextGSERToken( b, &peek_head, GSER_NO_COPY )) ){
			Asn1Error("DirectoryString : Token Reading ERROR\n");
			longjmp( env, -20);
		}
		GDecBMPStringContent( b, (GBMPString*)result, bytesDecoded, env );
	} 
	else if ( strncmp ( peek_head, "universalString", sizeof ("universalString") ) == 0 ){
		if ( !(strLen = LocateNextGSERToken( b, &peek_head, GSER_NO_COPY )) ){
			Asn1Error("DirectoryString : Token Reading ERROR\n");
			longjmp( env, -20);
		}
		GDecUniversalStringContent( b, (GUniversalString*)result, bytesDecoded, env );
	} 
	else if ( strncmp ( peek_head, "uTF8String", sizeof ("uTF8String") ) == 0 ){
		if ( !(strLen = LocateNextGSERToken( b, &peek_head, GSER_NO_COPY )) ){
			Asn1Error("DirectoryString : Token Reading ERROR\n");
			longjmp( env, -20);
		}
		GDecUTF8StringContent( b, (GUTF8String*)result, bytesDecoded, env );
	}
	else {
		if ( *peek_head != '\"'){
			Asn1Error("StringValue : Should Begin with \" \n");
			longjmp( env, -20);
		}
		/* Read StringValue */
		if ( !(strLen = LocateNextGSERToken( b, &peek_head, GSER_NO_COPY )) ){
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

		if ( !(strLen = LocateNextGSERToken( b, &peek_head, GSER_NO_COPY )) ){
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
