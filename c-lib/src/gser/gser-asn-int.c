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

#include <ctype.h>
#include <string.h>

#include "asn-config.h"
#include "asn-gser.h"

/*
 * encodes signed long integer's contents
 */
inline unsigned char lastdigittoascii(int integer){
    return (unsigned char)(integer%10 + '0');
}

AsnLen
GEncAsnIntContent PARAMS ((b, data),
    GenBuf *b _AND_
    GAsnInt *data)
{
    int minus=0;
    unsigned char digit;
    AsnLen len;
    int             i;

   if ( data->value == 0 ){
      BufPutByteRvs (b, (unsigned char)'0');
      return 1;
   }

   if ( data->value < 0 ){
      minus = 1;
      data->value = data->value*(-1);
   }

   for ( i = 0 ; data->value ; i++ )
   {
      digit = lastdigittoascii(data->value);
      BufPutByteRvs( b, digit);
      data->value = data->value/10;
   }

   if ( minus ) BufPutByteRvs(b, '-');

   if ( data->identifier != NULL ){
       BufPutByteRvs(b, ' ');
       len = strlen(data->identifier);
       BufPutSegRvs(b, data->identifier, len);
    }

   return i + minus;
}

/*
 * Decodes content of GSER a INTEGER value.
 */
#include <stdlib.h>
void
GDecAsnIntContent PARAMS ((b, result, bytesDecoded, env),
    GenBuf *b _AND_
    GAsnInt    *result _AND_
    AsnLen *bytesDecoded _AND_
    jmp_buf env)
{
	long strLen;
	char* peek_head;

	*bytesDecoded = 0;
	if( !(strLen = LocateNextGSERToken( b, &peek_head, GSER_COPY )) ){
		Asn1Error("INTEGER : Token Reading ERROR\n");
		longjmp( env, -20);
	}
	result->value = atoi(peek_head);
	Asn1Free(peek_head);
	*bytesDecoded += strLen;
}
