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

#include <math.h>
#include "asn-config.h"
#include "asn-gser.h"

/*
 * GSER encode REAL value
 * RFC 3641
 * RealValue = "0"		;zero REAL Value
 *            / PLUS-INFINITY	;positive infinity
 *            / MINUS-INFINITY	;negative infinity
 *            / realnumber	;positive base 10 REAL value
 *            / "-" realnumber	;negative base 10 REAL value
 *            / SequenceValue	; non-zero REAL value, base 2 or 10
 */

double
pow (double base, double exp ){
	/* check if math library */
	return 0.0;
}

AsnLen
GEncAsnRealContent PARAMS ((b, real),
    GenBuf  *b _AND_
    GAsnReal  *real)
{
	return 0;
}

#ifdef LDAP_COMPONENT
int
GDecAsnRealContent PARAMS ((b, result, bytesDecoded),
    GenBuf *b _AND_
    GAsnReal  *result _AND_
    AsnLen *bytesDecoded )
{
	long strLen, minus=0;
	char* peek_head;
	long exponent;
	double mantissa;
	char data[256];

	*bytesDecoded = 0;
	if( !(strLen = LocateNextGSERToken( b, &peek_head, GSER_COPY )) ){
		Asn1Error("INTEGER : Token Reading ERROR\n");
		return -1;
	}

	if ( peek_head [0] == '0' ) {
		result->value = 0.0;
	}
	else if ( strncmp( peek_head, "PLUS-INFINITY",
			sizeof("PLUS-INFINITY") ) ) {
		result->value = PLUS_INFINITY;
	} else if ( strncmp( peek_head, "MINUS-INFINITY",
			sizeof("MINUS-INFINITY") ) ) {
		result->value = MINUS_INFINITY;
	} else {
		if ( peek_head[0] == '-' ) minus = 1;
		/* parsing mantissa */
		mantissa = atof ( &peek_head[minus+1] );

		/* parsing exponent */
		if( !(strLen = LocateNextGSERToken(b,&peek_head,GSER_NO_COPY )) ){
			Asn1Error("INTEGER : Token Reading ERROR\n");
			return -1;
		}

		if ( peek_head[0] != 'E' ) {
			Asn1Error("INTEGER : Token Reading ERROR\n");
			return -1;
		}

		if ( peek_head[1] == '0' ) {
			exponent = 0;
		}
		else if ( peek_head[1] == '-' ) {
			strncpy ( data, peek_head, strLen-2 );
			exponent = atoi( data );
			exponent = exponent*-1;
		}
		else {
			strncpy ( data, peek_head, strLen-1 );
			exponent = atoi( data );
		}

		result->value = pow ( mantissa , (double)exponent );
		if ( minus )
			result->value = result->value*-1;
	} 

	*bytesDecoded += strLen;

	return 1;
}
#else
void
GDecAsnRealContent PARAMS ((b, result, bytesDecoded, env),
    GenBuf *b _AND_
    GAsnReal  *result _AND_
    AsnLen *bytesDecoded _AND_
    jmp_buf env)
{
	long strLen, minus=0;
	char* peek_head;
	long exponent;
	double mantissa;
	char data[256];

	*bytesDecoded = 0;
	if( !(strLen = LocateNextGSERToken( b, &peek_head, GSER_COPY )) ){
		Asn1Error("INTEGER : Token Reading ERROR\n");
		longjmp( env, -20);
	}

	if ( peek_head [0] == '0' ) {
		result->value = 0.0;
	}
	else if ( strncmp( peek_head, "PLUS-INFINITY",
			sizeof("PLUS-INFINITY") ) ) {
		result->value = PLUS_INFINITY;
	} else if ( strncmp( peek_head, "MINUS-INFINITY",
			sizeof("MINUS-INFINITY") ) ) {
		result->value = MINUS_INFINITY;
	} else {
		if ( peek_head[0] == '-' ) minus = 1;
		/* parsing mantissa */
		mantissa = atof ( &peek_head[minus+1] );

		/* parsing exponent */
		if( !(strLen = LocateNextGSERToken(b,&peek_head,GSER_NO_COPY )) ){
			Asn1Error("INTEGER : Token Reading ERROR\n");
			longjmp( env, -20);
		}

		if ( peek_head[0] != 'E' ) {
			Asn1Error("INTEGER : Token Reading ERROR\n");
			longjmp( env, -20);
		}

		if ( peek_head[1] == '0' ) {
			exponent = 0;
		}
		else if ( peek_head[1] == '-' ) {
			strncpy ( data, peek_head, strLen-2 );
			exponent = atoi( data );
			exponent = exponent*-1;
		}
		else {
			strncpy ( data, peek_head, strLen-1 );
			exponent = atoi( data );
		}

		result->value = pow ( mantissa , (double)exponent );
		if ( minus )
			result->value = result->value*-1;
	} 

	*bytesDecoded += strLen;
}
#endif
