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
    char *buf;
    int i,k,len;

    buf = malloc(o->value.octetLen*2);
 
    BufPutSegRvs (b, "'H", 2 );
    for ( i = 0, k = 0 ; i < o->value.octetLen ; i++, k += 2 ){
	buf[k] = o->value.octs[i] & 0x0F;
	buf[k+1] = (o->value.octs[i] & 0xF0)>>4;

	if ( buf[k] >= 10 )
		buf[k] = buf[k] - 10 + 'A';
	else
		buf[k] += '0';

	if( buf[k+1] >= 10 )
		buf[k+1] = buf[k+1] - 10 + 'A';
	else
		buf[k+1] += '0';
    }
    BufPutSegRvs (b, k, o->value.octetLen*2 );
    BufPutByteRvs (b, '\'' );

   if ( o->identifier != NULL ){
       BufPutByteRvs(b, ' ');
       len = strlen(o->identifier);
       BufPutSegRvs(b, o->identifier, len);
    }

    free ( buf );

    return o->value.octetLen + 3;
}

/*
 * GSER Decodes the content of a GSER OCTET STRING value
 */
#ifdef LDAP_COMPONENT
int
GDecAsnOctsContent PARAMS ((mem_op, b, result, bytesDecoded ),
    void* mem_op _AND_
    GenBuf *b _AND_
    GAsnOcts *result _AND_
    AsnLen *bytesDecoded )
{
	long strLen,i,k;
	char* peek_head;
	unsigned char* data;
	
	*bytesDecoded = 0;
	if ( !(strLen = LocateNextGSERToken( mem_op, b, &peek_head, GSER_NO_COPY )) ){
		Asn1Error("OCTET String : Token Reading ERROR\n");
		return -1;
	}

	*bytesDecoded += strLen;

	if ( *peek_head != '\''){
		Asn1Error("OCTET String :  Should Begin with \'\n");
		return -1;
	}

	if ( !(strLen = LocateNextGSERToken( mem_op, b, &peek_head, GSER_NO_COPY )) ){
		Asn1Error("OCTET String :  String read ERROR\n");
		return -1;
	}

	result->value.octetLen = strLen/2;
	data = CompAlloc( mem_op, sizeof(char)*(strLen>>2)+1 );
	for ( i = 0, k = 0 ; i < strLen ; i += 2, k++ ){
	   if( peek_head[i] >= 'A' )
		peek_head[i] = peek_head[i]-'A'+10;
	   else
		peek_head[i] = peek_head[i]-'0';

	   if( peek_head[i+1] >= 'A' )
		peek_head[i+1] = peek_head[i+1]-'A'+10;
	   else
		peek_head[i+1] = peek_head[i+1]-'0';

	   data[k] = (char)( (peek_head[i]<<4)&0xF0)|(peek_head[i+1]&0x0F );
	}

	data[strLen/2] = '\0';

	result->value.octs = data;
	*bytesDecoded += strLen;

	if ( !(strLen = LocateNextGSERToken( mem_op, b, &peek_head, GSER_NO_COPY )) ){
		Asn1Error("OCTET String :  Token(\"H) read ERROR\n");
		CompFree( mem_op, data );
		return -1;
	}

	*bytesDecoded += strLen;

	if ( peek_head[0] != '\'' || peek_head[1] != 'H' ){
		Asn1Error("OCTET String :  Should End with \"H\n");
		CompFree( mem_op, data );
		return -1;
	}
	return 1;
}
#else
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

	result->value.octetLen = strLen/2;
	data = Asn1Alloc(sizeof(char)*(strLen>>2)+1);
	for ( i = 0, k = 0 ; i < strLen ; i += 2, k++ ){
	   if( peek_head[i] >= 'A' )
		peek_head[i] = peek_head[i]-'A'+10;
	   else
		peek_head[i] = peek_head[i]-'0';

	   if( peek_head[i+1] >= 'A' )
		peek_head[i+1] = peek_head[i+1]-'A'+10;
	   else
		peek_head[i+1] = peek_head[i+1]-'0';

	   data[k] = (char)( (peek_head[i]<<4)&0xF0)|(peek_head[i+1]&0x0F );
	}

	data[strLen/2] = '\0';

	result->value.octs = data;
	*bytesDecoded += strLen;

	if ( !(strLen = LocateNextGSERToken( b, &peek_head, GSER_NO_COPY )) ){
		Asn1Error("OCTET String :  Token(\"H) read ERROR\n");
		Asn1Free(data);
		longjmp( env, -20);
	}

	*bytesDecoded += strLen;

	if ( peek_head[0] != '\'' || peek_head[1] != 'H' ){
		Asn1Error("OCTET String :  Should End with \"H\n");
		Asn1Free(data);
		longjmp( env, -20);
	}
}
#endif
