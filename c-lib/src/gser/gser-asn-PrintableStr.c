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
/*
 * PrintableString is not necessary to be translated to UTF8
 * RFC 3641
 * StringValue	    = dquote *SafeUTF8Character dquote
 * SafeUTFCharacter = %x00-21 / %x23-7f /
 *                  = dquote dquote /
 *                  = %xc0-DF %x80-BF /
 *                  = %xE0-EF 2(%x80-BF) /
 *                  = %xF0-E7 3(%x80-BF) /
 */
#include "asn-PrintableStr.h"

extern int CheckPrintableString (PrintableString*);

AsnLen
GEncPrintableStringContent PARAMS ((b, o),
    GenBuf *b _AND_
    GPrintableString *o)
{
	int rc;
	rc = CheckPrintableString ( &o->value );
	if ( rc < 0 ) return rc;
	rc = GEncUTF8StringContent ( b , (GUTF8String*)o);
	return rc;
}

/*
 * GSER Decodes the content of a GSER Printable String
 */
#ifdef LDAP_COMPONENT
int
GDecPrintableStringContent PARAMS (( mem_op, b, result, bytesDecoded ),
    void* mem_op _AND_
    GenBuf *b _AND_
    GPrintableString *result _AND_
    AsnLen *bytesDecoded )
{
	long strLen;
	char* peek_head;
	
	*bytesDecoded = 0;
	if ( !(strLen = LocateNextGSERToken( mem_op, b, &peek_head, GSER_NO_COPY )) ){
		Asn1Error("PrintableString : Token Reading ERROR\n");
		return -1;
	}

	*bytesDecoded += strLen;

	if ( *peek_head != '\"'){
		Asn1Error("UTFString :  Should Begin with \" \n");
		return -1;
	}
	/* Read StringValue */
	if ( !(strLen = LocateNextGSERToken( mem_op, b, &peek_head, GSER_COPY )) ){
		Asn1Error("PrintableString : Token Reading ERROR\n");
		return -1;
	}

	*bytesDecoded += strLen;

	result->value.octs = peek_head;
	result->value.octetLen = strLen;

	if ( CheckPrintableString ( &result->value ) < 0 ) {
		Asn1Error("PrintableString : Invalid Printable Format\n");
		return (-1);
	}

	if ( !(strLen = LocateNextGSERToken( mem_op, b, &peek_head, GSER_NO_COPY )) ){
		Asn1Error("PrintableString : Token Reading ERROR\n");
		return (-1);
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
GDecPrintableStringContent PARAMS ((b, result, bytesDecoded, env),
    GenBuf *b _AND_
    GPrintableString *result _AND_
    AsnLen *bytesDecoded _AND_
    jmp_buf env)
{
	long strLen;
	char* peek_head;
	
	*bytesDecoded = 0;
	if ( !(strLen = LocateNextGSERToken( b, &peek_head, GSER_NO_COPY )) ){
		Asn1Error("PrintableString : Token Reading ERROR\n");
		longjmp( env, -20);
	}

	*bytesDecoded += strLen;

	if ( *peek_head != '\"'){
		Asn1Error("UTFString :  Should Begin with \" \n");
		longjmp( env, -20);
	}
	/* Read StringValue */
	if ( !(strLen = LocateNextGSERToken( b, &peek_head, GSER_NO_COPY )) ){
		Asn1Error("PrintableString : Token Reading ERROR\n");
		longjmp( env, -20);
	}

	*bytesDecoded += strLen;

	result->value.octs = peek_head;
	result->value.octetLen = strLen;

	if ( CheckPrintableString ( &result->value ) < 0 ) {
		Asn1Error("PrintableString : Invalid Printable Format\n");
		longjmp( env, -20);
	}

	if ( !(strLen = LocateNextGSERToken( b, &peek_head, GSER_NO_COPY )) ){
		Asn1Error("PrintableString : Token Reading ERROR\n");
		longjmp( env, -20);
	}
	
	*bytesDecoded += strLen;

	if ( *peek_head != '\"'){
		Asn1Error("UTFString :  Should end with \" \n");
		longjmp( env, -20);
	}
}
#endif
