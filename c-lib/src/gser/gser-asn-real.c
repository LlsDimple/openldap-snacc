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

#include "math.h"
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
AsnLen
GEncAsnRealContent PARAMS ((b, real),
    GenBuf  *b _AND_
    GAsnReal  *real)
{
	return 0;
}

void
GDecAsnRealContent PARAMS ((b, result, bytesDecoded, env),
    GenBuf *b _AND_
    GAsnReal  *result _AND_
    AsnLen *bytesDecoded _AND_
    jmp_buf env)
{
}

AsnInt
GMatchingAsnRealContent PARAMS ((a, b),
	GAsnReal *a _AND_
	GAsnReal *b)
{
	assert( a );
	assert( b );
	return ( a->value == b->value );
}
