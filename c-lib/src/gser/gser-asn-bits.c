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

#include <string.h>

#include "asn-config.h"
#include "asn-gser.h"

/*
 * GSER Encodes the BIT STRING value (including the unused bits
 * byte) to the given buffer.
 * RFC 3641
 * BitStringValue = bstring / hstring / bit-list
 * bitlist 	= "{" [ sp identifier          
 *		     *( sp identifier) ] sp "}"
 * hstring	= squote "hexdecimal-digit squote %48
 * bstring	= squote *binary-digit squote %42
 * binary-digit	= 0 / 1
 * each bit is encoded to a binary-digit
 */

AsnLen
GEncAsnBitsContent PARAMS ((b, bits),
    GenBuf *b _AND_
    GAsnBits *bits)
{
   int i = 0;
   int merge;
   unsigned char hex;
   if ( bits->identifier ){
   /* This will be supported SOON */
	return 0;
   }
   else if ( !(bits->value.bitLen % 4) )
   {
   /* hstring	= squote "hexdecimal-digit squote %48 */
	BufPutSegRvs(b, "'H", 2);
	for ( i=bits->value.bitLen-4 ; i >= 0  ; i-=4 ){
		merge  = (bits->value.bits[i  ] == '1')*2*2*2;
		merge += (bits->value.bits[i+1] == '1')*2*2;
		merge += (bits->value.bits[i+2] == '1')*2;
		merge += (bits->value.bits[i+3] == '1');
		if ( merge >= 10 ){
			merge = merge - 10 + 'A';
			hex = (unsigned char)merge;
		}
		else
			hex = (unsigned char)(merge + '0');
		BufPutByteRvs(b,hex);
	}
	BufPutByteRvs(b, '\'');
	return bits->value.bitLen/4 + 3;
   }
   else{
   /* bstring	= squote *binary-digit squote %42 */
	BufPutSegRvs(b, "'B", 2);
	BufPutSegRvs (b, bits->value.bits, bits->value.bitLen );
	BufPutByteRvs(b, '\'');
	return bits->value.bitLen + 3;
   }

}

/*
 * GSER Decodes the content of a BIT STRING (including the unused bits octet)
 * Always returns a single contiguous bit string
 */
#define FIRST_BIT	0x01
#define SECOND_BIT	0x02
#define THRID_BIT 	0x04
#define FOURTH_BIT	0x08
#define FIFTH_BIT	0x10
#define SIXTH_BIT	0x20
#define SEVENTH_BIT	0x40
#define EIGITH_BIT	0x80
void
GDecAsnBitsContent PARAMS ((b, result, bytesDecoded, env),
    GenBuf *b _AND_
    GAsnBits *result _AND_
    AsnLen *bytesDecoded _AND_
    jmp_buf env)
{
	long strLen, data_len, bit_pos, num_of_bits;
        char* peek_head, *data;
	unsigned char set;
                                                                          
        *bytesDecoded = 0;
        if ( !(strLen = LocateNextGSERToken( b, &peek_head, GSER_NO_COPY )) ){
		Asn1Error("INTEGER : Token Reading ERROR\n");
		longjmp( env, -20);
        }

	if ( peek_head[0] != '\'' ) {
		Asn1Error("INTEGER : Token Reading ERROR\n");
		longjmp( env, -20);
	}

        if ( !(strLen = LocateNextGSERToken( b, &peek_head, GSER_NO_COPY )) ){
		Asn1Error("INTEGER : Token Reading ERROR\n");
		longjmp( env, -20);
        }

	data = peek_head;
	data_len = strLen;

        if ( !(strLen = LocateNextGSERToken( b, &peek_head, GSER_NO_COPY )) ){
		Asn1Error("INTEGER : Token Reading ERROR\n");
		longjmp( env, -20);
        }

	if ( peek_head[0] != '\'' ||
			!( peek_head[1] == 'H' || peek_head[1] == 'B') ) {
		Asn1Error("INTEGER : Token Reading ERROR\n");
		longjmp( env, -20);
	}

	bit_pos = 0;
	num_of_bits = 8;
	while ( data_len-- ) {
		set = 1;
		set = set << (bit_pos%num_of_bits-1);
		if ( data[bit_pos] == '1' ) {
			data[bit_pos/num_of_bits] = data[bit_pos/num_of_bits] | set;
		}
		else if ( data[bit_pos] == '0' ) {
			data[bit_pos/num_of_bits] = data[bit_pos/num_of_bits] & ~set;
		}
		bit_pos++;
		if ( bit_pos == num_of_bits ) bit_pos = 0;
	}
}
