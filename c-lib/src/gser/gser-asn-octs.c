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
 * GSER encodes just the content of an OCTET STRING.
 * OctetStringValue = hstring
 * hstring = squote *hexadecimal-digit squote %x48
 * EX '012ABCD'H
 */
AsnLen
GEncAsnOctsContent PARAMS ((b, o),
    GenBuf *b _AND_
    GAsnOcts *o)
{
    char data[2];
    int i,len;

    BufPutSegRvs (b, "'H", 2 );
    for ( i = 0 ; i < o->value.octetLen ; i++ ){
	data[0] = o->value.octs[i] & 0x0F;
	data[1] = (o->value.octs[i] & 0xF0)>>4;

	if ( data[1] >= 10 )
		data[1] = data[1] - 10 + 'A';
	else
		data[1] += '0';

	if( data[0] >= 10 )
		data[0] = data[0] - 10 + 'A';
	else
		data[0] += '0';
	BufPutSegRvs (b, data, 2 );
    }
    BufPutByteRvs (b, '\'' );

   if ( o->identifier != NULL ){
       BufPutByteRvs(b, ' ');
       len = strlen(o->identifier);
       BufPutSegRvs(b, o->identifier, len);
    }

    return o->value.octetLen + 3;
}

/*
 * GSER Decodes the content of a GSER OCTET STRING value
 */
void
GDecAsnOctsContent PARAMS ((b, result, bytesDecoded, env),
    GenBuf *b _AND_
    GAsnOcts *result _AND_
    AsnLen *bytesDecoded _AND_
    jmp_buf env)
{
	long strLen,i,k;
	char* peek_head;
	unsigned char* data;
	
	*bytesDecoded = 0;
	if ( !(strLen = LocateNextGSERToken( b, &peek_head, GSER_NO_COPY )) ){
		Asn1Error("OCTET String : Token Reading ERROR\n");
		longjmp( env, -20);
	}


	*bytesDecoded += strLen;

	if ( *peek_head != '\''){
		Asn1Error("OCTET String :  Should Begin with \'\n");
		longjmp( env, -20);
	}

	if ( !(strLen = LocateNextGSERToken( b, &peek_head, GSER_NO_COPY )) ){
		Asn1Error("OCTET String :  String read ERROR\n");
		longjmp( env, -20);
	}

	result->value.octetLen = strLen;
	data = Asn1Alloc(sizeof(char)*(strLen>>2)+1);
	k = strLen/2-1;
	for ( i = 0 ; i < strLen ; i += 2, k-- ){
	   if( peek_head[i] >= 'A' )
		peek_head[i] = peek_head[i]-'A'+10;
	   else
		peek_head[i] = peek_head[i]-'0';

	   if( peek_head[i+1] >= 'A' )
		peek_head[i+1] = peek_head[i+1]-'A'+10;
	   else
		peek_head[i+1] = peek_head[i+1]-'0';

	   data[k] = (char)( (peek_head[i+1]<<4)&0xF0)|(peek_head[i]&0x0F );
	}

	data[strLen] = '\0';

	result->value.octs = data;
	*bytesDecoded += strLen;

	if ( !(strLen = LocateNextGSERToken( b, &peek_head, GSER_NO_COPY )) ){
		Asn1Error("OCTET String :  Token(\"H) read ERROR\n");
		Asn1Free(peek_head);
		longjmp( env, -20);
	}

	*bytesDecoded += strLen;

	if ( peek_head[0] != '\'' || peek_head[1] != 'H' ){
		Asn1Error("OCTET String :  Should End with \"H\n");
		Asn1Free(peek_head);
		longjmp( env, -20);
	}
}
