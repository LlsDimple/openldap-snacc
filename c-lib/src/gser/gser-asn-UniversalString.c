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
 * UniverseString need to be translated into UTF8
 * It can be done just direct mapping
 * RFC 3641
 * StringValue	    = dquote *SafeUTF8Character dquote
 * SafeUTFCharacter = %x00-21 / %x23-7f /
 *                  = dquote dquote /
 *                  = %xc0-DF %x80-BF /
 *                  = %xE0-EF 2(%x80-BF) /
 *                  = %xF0-E7 3(%x80-BF) /
 */
static int UniversalStringtoUTF8( char* octs, int len){
	return 1;
}

static int UTF8toUniversalString( char* octs, int len){
	return 1;
}

AsnLen
GEncUniversalStringContent PARAMS ((b, octs),
    GenBuf *b _AND_
    GUniversalString *octs)
{
	UniversalStringtoUTF8(octs->value.octs,octs->value.octetLen);
	GEncAsnOctsContent(b,(GAsnOcts*)octs);
	return octs->value.octetLen;
}

void
GDecUniversalStringContent PARAMS ((b, result, bytesDecoded, env),
   GenBuf *b _AND_
   GUniversalString *result _AND_
   AsnLen *bytesDecoded _AND_
   ENV_TYPE env)
{
	GDecAsnOctsContent(b, (GAsnOcts*)result, bytesDecoded, env);
	UTF8toUniversalString(result->value.octs, result->value.octetLen);
}
