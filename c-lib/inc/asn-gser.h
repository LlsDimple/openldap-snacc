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

#ifndef _H_ASN_GSER
#define _H_ASN_GSER

#include "asn-incl.h"

/*
 * BIT STRING
 */

typedef struct GAsnBits
{
	char* identifier;
	AsnBits value;
} GAsnBits;
#define GASNBITS_PRESENT(abits) ((abits)->value.bits != NULL)
AsnLen GEncAsnBitsContent PROTO ((GenBuf *b, GAsnBits *bits));
void GDecAsnBitsContent PROTO ((GenBuf *b, GAsnBits *result,
				AsnLen *bytesDecoded, ENV_TYPE env));

/*
 * BMP String
 */
typedef struct GBMPSting
{
	char* identifier;
	BMPString value;
} GBMPString;

AsnLen GEncBMPStringContent PROTO ((GenBuf *b, GBMPString *octs));
void GDecBMPStringContent PROTO ((GenBuf *b, GBMPString *result,
				 AsnLen *bytesDecoded, ENV_TYPE env));

/*
 * BOOLEAN
 */
typedef struct GAsnBool{
	char* identifier;
	AsnBool value;
} GAsnBool;

AsnLen GEncAsnBoolContent PROTO ((GenBuf *b, GAsnBool *data));
void GDecAsnBoolContent PROTO ((GenBuf *b, GAsnBool *result,
				AsnLen *bytesDecoded, ENV_TYPE env));

/*
 * ENUMERTED
 */
typedef struct GAsnEnum{
	char* identifier;
	AsnEnum value;
	char* value_identifier;
} GAsnEnum;

AsnLen GEncAsnEnumContent PROTO ((GenBuf *a, GAsnEnum* data));
void GDecAsnEnumContent PROTO ((GenBuf *a, GAsnEnum *result,
				AsnLen *bytesDecoded,ENV_TYPE env));

/*
 * IA5 String
 */
typedef struct GIA5String{
	char* identifier;
	IA5String value;
} GIA5String;

AsnLen GEncIA5StringContent PROTO ((GenBuf *b, GIA5String *octs));
void GDecIA5StringContent PROTO ((GenBuf *b, GIA5String *result,
				 AsnLen *bytesDecoded, ENV_TYPE env));


/*
 * INTEGER
 */
typedef struct GAsnInt{
	char*	identifier;/*It follows 11.3 cluase in X.680*/
	AsnInt	value;
} GAsnInt;

#define GNOT_NULL(ptr) ((ptr) != NULL)

AsnLen GEncAsnIntContent PROTO ((GenBuf *b, GAsnInt *data));
void GDecAsnIntContent PROTO ((GenBuf *b, GAsnInt *result,
				AsnLen *bytesDecoded, ENV_TYPE env));

/*
 * LIST Data Structure for C_LIST
 */
typedef AsnList GAsnList;

/*
 * NULL
 */
typedef struct GAsnNull{
	char* identifier;
	AsnNull value;
} GAsnNull;

AsnLen GEncAsnNullContent PROTO ((GenBuf *b, GAsnNull *data));
void GDecAsnNullContent PROTO ((GenBuf *b, GAsnNull *result,
								AsnLen *bytesDecoded, ENV_TYPE env));

/*
 * Numeric String
 */
typedef struct GNumericString{
	char* identifier;
	NumericString value;
} GNumericString;

AsnLen GEncNumericStringContent PROTO ((GenBuf *b, GNumericString *octs));
void GDecNumericStringContent PROTO ((GenBuf *b, GNumericString *result,
									AsnLen *bytesDecoded, ENV_TYPE env));

/*
 * OCTETS STRING
 */
typedef struct GAsnOcts{
	char* identifier;
	AsnOcts value;
} GAsnOcts;

#define GASNOCTS_PRESENT(aocts) ((aocts)->value.octs != NULL)

AsnLen GEncAsnOctsContent PROTO ((GenBuf *b, GAsnOcts *octs));
void GDecAsnOctsContent PROTO ((GenBuf *b, GAsnOcts *result,
								AsnLen *bytesDecoded, ENV_TYPE env));

/*
 * OID
 */
typedef struct GAsnOid{
	char* identifier;
	AsnOid value;
} GAsnOid;

#define GASNOID_PRESENT(aoid) ASNOCTS_PRESENT(aoid)

#define GEncAsnOidContent( b, oid)   GEncAsnOctsContent (b, oid)
void GDecAsnOidContent PROTO ((GenBuf *b, GAsnOid  *result,
								AsnLen *bytesDecoded, ENV_TYPE env));

/*
 * Printable String
 */
typedef struct GPrintableString{
	char* identifier;
	PrintableString value;
} GPrintableString;

AsnLen GEncPrintableStringContent PROTO ((GenBuf *b, GPrintableString *octs));
void GDecPrintableStringContent PROTO ((GenBuf *b, GPrintableString *result,
										AsnLen *bytesDecoded, ENV_TYPE env));

/*
 * REAL
 */
typedef struct GAsnReal{
	char* identifier;
	AsnReal value;
} GAsnReal;

AsnLen GEncAsnRealContent PROTO ((GenBuf *b, GAsnReal *data));
void GDecAsnRealContent PROTO ((GenBuf *b, GAsnReal *result,
								AsnLen *bytesDecoded, ENV_TYPE env));

/*
 * Relative OID
 */

typedef struct GAsnRelativeOid{
	char* identifier;
	AsnRelativeOid value;
} GAsnRelativeOid;

#define GEncAsnRelativeOidContent( b, oid)   GEncAsnOctsContent (b, oid)
void GDecAsnRelativeOidContent PROTO ((GenBuf *b, GAsnRelativeOid  *result,
									  AsnLen *bytesDecoded, ENV_TYPE env));

/*
 * Teletex String
 */
typedef struct GTeletexString
{
	char* identifier;
	TeletexString value;
} GTeletexString;

AsnLen GEncTeletexStringContent PROTO ((GenBuf *b, GAsnOcts *octs));
void GDecTeletexStringContent PROTO ((GenBuf *b, GAsnOcts *result,
									  AsnLen *bytesDecoded, ENV_TYPE env));


/*
 * Universal String
 */
typedef struct GUniversalString{
	char* identifier;
	UniversalString value;
} GUniversalString;

AsnLen GEncUniversalStringContent PROTO ((GenBuf *b, GUniversalString *octs));
void GDecUniversalStringContent PROTO ((GenBuf *b, GUniversalString *result,
										AsnLen *bytesDecoded, ENV_TYPE env));

/*
 * UTF8 String
 */
typedef struct GUTF8String{
	char* identifier;
	UTF8String value;
} GUTF8String;

AsnLen GEncUTF8StringContent PROTO ((GenBuf *b, GUTF8String *octs));
void GDecUTF8StringContent PROTO ((GenBuf *b, GUTF8String *result,
									AsnLen *bytesDecoded, ENV_TYPE env));

/*
 * Visible String
 */
typedef struct GVisibleString{
	char* identifier;
	VisibleString value;
} GVisibleString;

AsnLen GEncVisibleStringContent PROTO ((GenBuf *b, GVisibleString *octs));
void GDecVisibleStringContent PROTO ((GenBuf *b, GVisibleString *result,
									  AsnLen *bytesDecoded, ENV_TYPE env));

/*
 * Utilitiies for GSER DEcoding
 */
typedef enum LOCATE_MODE{//default is to move curr pointer
	GSER_COPY=1,
	GSER_NO_COPY=2,
	GSER_PEEK=3//(it implies NO_COPY, just peek a position)
} LOCATE_MODE;

extern int LocateNextGSERToken(GenBuf*, char**, LOCATE_MODE);

#endif
