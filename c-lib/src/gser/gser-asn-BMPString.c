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

/*
 * BMPString(UCS-2) needs to be translated into UTF8
 * It can be done just direct mapping
 * RFC 3641
 */
static int TranslateUCS2toUTF8( char* octs, int len){
	/* To Be Implemented */
	return 1;
}

static int TranslateUTF8toUCS2( char* octs, int len){
	/* To Be Implemented */
	return 1;
}

AsnLen GEncBMPStringContent(GenBuf *b, GBMPString *result )
{
	int rc;
	rc = TranslateUCS2toUTF8( result->value.octs, result->value.octetLen );
	if ( rc < 0 ) return (-1);
	return GEncUTF8StringContent( b,(GUTF8String*)result );
} 

#ifdef LDAP_COMPONENT
int GDecBMPStringContent(void* mem_op, GenBuf *b, GBMPString *result,
				 AsnLen *bytesDecoded )
{
	int rc;
	/* UTF-8, a Transformation format of ISO RFC 2279 */
	rc = GDecUTF8StringContent( mem_op, b,(GUTF8String*)result,bytesDecoded );
	if ( rc < 0 ) return rc;
	return TranslateUTF8toUCS2( result->value.octs, result->value.octetLen);
}
#else
void GDecBMPStringContent(GenBuf *b, GBMPString *result,
				 AsnLen *bytesDecoded, ENV_TYPE env)
{
	/* UTF-8, a Transformation format of ISO RFC 2279 */
	GDecUTF8StringContent(b,result,bytesDecoded, env);
	TranslateUTF8toUCS2( result->value.octs, result->value.octetLen);
}
#endif
