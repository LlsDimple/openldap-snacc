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
	GEncAsnUTF8StringContent(b,(GAsnOcts*)octs);
	return octs->value.octetLen;
}

void
GDecUniversalStringContent PARAMS ((b, result, bytesDecoded, env),
   GenBuf *b _AND_
   GUniversalString *result _AND_
   AsnLen *bytesDecoded _AND_
   ENV_TYPE env)
{
	GDecAsnUTF8StringContent(b, (GAsnOcts*)result, bytesDecoded, env);
	UTF8toUniversalString(result->value.octs, result->value.octetLen);
}
