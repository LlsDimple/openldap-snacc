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
#include <assert.h>

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
#ifdef LDAP_COMPONENT
int GDecAsnBitsContent PROTO ((void* mem_op, GenBuf *b, GAsnBits *result,
				AsnLen *bytesDecoded ));
#else
void GDecAsnBitsContent PROTO ((GenBuf *b, GAsnBits *result,
				AsnLen *bytesDecoded, ENV_TYPE env));
#endif

#define GFreeAsnBits(v);

/*
 * BMP String
 */
typedef struct GBMPSting
{
	char* identifier;
	BMPString value;
} GBMPString;

AsnLen GEncBMPStringContent PROTO ((GenBuf *b, GBMPString *octs));
#ifdef LDAP_COMPONENT
int GDecBMPStringContent PROTO ((void* mem_op, GenBuf *b, GBMPString *result,
				 AsnLen *bytesDecoded ));
#else
void GDecBMPStringContent PROTO ((GenBuf *b, GBMPString *result,
				 AsnLen *bytesDecoded, ENV_TYPE env));
#endif

/*
 * BOOLEAN
 */
typedef struct GAsnBool{
	char* identifier;
	AsnBool value;
} GAsnBool;

AsnLen GEncAsnBoolContent PROTO ((GenBuf *b, GAsnBool *data));
#ifdef LDAP_COMPONENT
int GDecAsnBoolContent PROTO ((void* mem_op, GenBuf *b, GAsnBool *result,
				AsnLen *bytesDecoded ));
#else
void GDecAsnBoolContent PROTO ((GenBuf *b, GAsnBool *result,
				AsnLen *bytesDecoded, ENV_TYPE env));
#endif
/*
 * ENUMERTED
 */
typedef struct GAsnEnum{
	char* identifier;
	AsnEnum value;
	char* value_identifier;
	int len;
} GAsnEnum;

AsnLen GEncAsnEnumContent PROTO ((GenBuf *a, GAsnEnum* data));
#ifdef LDAP_COMPONENT
int GDecAsnEnumContent PROTO ((void* mem_op, GenBuf *a, GAsnEnum *result,
				AsnLen *bytesDecoded));
#else
void GDecAsnEnumContent PROTO ((GenBuf *a, GAsnEnum *result,
				AsnLen *bytesDecoded,ENV_TYPE env));
#endif
/*
 * IA5 String
 */
typedef struct GIA5String{
	char* identifier;
	IA5String value;
} GIA5String;

AsnLen GEncIA5StringContent PROTO ((GenBuf *b, GIA5String *octs));
#ifdef LDAP_COMPONENT
int GDecIA5StringContent PROTO ((void* mem_op, GenBuf *b, GIA5String *result,
				 AsnLen *bytesDecoded ));
#else
void GDecIA5StringContent PROTO ((GenBuf *b, GIA5String *result,
				 AsnLen *bytesDecoded, ENV_TYPE env));
#endif
/*
 * INTEGER
 */
typedef struct GAsnInt{
	char*	identifier;/*It follows 11.3 cluase in X.680*/
	AsnInt	value;
} GAsnInt;

#define GNOT_NULL(ptr) ((ptr) != NULL)

AsnLen GEncAsnIntContent PROTO ((GenBuf *b, GAsnInt *data));
#ifdef LDAP_COMPONENT
int GDecAsnIntContent PROTO ((void* mem_op, GenBuf *b, GAsnInt *result,
				AsnLen *bytesDecoded ));
#else
void GDecAsnIntContent PROTO ((GenBuf *b, GAsnInt *result,
				AsnLen *bytesDecoded, ENV_TYPE env));
#endif
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
#ifdef LDAP_COMPONENT
int GDecAsnNullContent PROTO ((void* mem_op, GenBuf *b, GAsnNull *result,
				AsnLen *bytesDecoded ));
#else
void GDecAsnNullContent PROTO ((GenBuf *b, GAsnNull *result,
				AsnLen *bytesDecoded, ENV_TYPE env));
#endif

/*
 * Numeric String
 */
typedef struct GNumericString{
	char* identifier;
	NumericString value;
} GNumericString;

AsnLen GEncNumericStringContent PROTO ((GenBuf *b, GNumericString *octs));
#ifdef LDAP_COMPONENT
int GDecNumericStringContent PROTO ((void* mem_op, GenBuf *b, GNumericString *result,
					AsnLen *bytesDecoded ));
#else
void GDecNumericStringContent PROTO ((GenBuf *b, GNumericString *result,
					AsnLen *bytesDecoded, ENV_TYPE env));
#endif

/*
 * OCTETS STRING
 */
typedef struct GAsnOcts{
	char* identifier;
	AsnOcts value;
} GAsnOcts;

#define GASNOCTS_PRESENT(aocts) ((aocts)->value.octs != NULL)

AsnLen GEncAsnOctsContent PROTO ((GenBuf *b, GAsnOcts *octs));
#ifdef LDAP_COMPONENT
int GDecAsnOctsContent PROTO ((void* mem_op, GenBuf *b, GAsnOcts *result,
				AsnLen *bytesDecoded ));
#else
void GDecAsnOctsContent PROTO ((GenBuf *b, GAsnOcts *result,
				AsnLen *bytesDecoded, ENV_TYPE env));
#endif

/*
 * OID
 */
typedef struct GAsnOid{
	char* identifier;
	AsnOid value;
} GAsnOid;

#define GASNOID_PRESENT(aoid) ASNOCTS_PRESENT(aoid)

#define GEncAsnOidContent( b, oid)   GEncAsnOctsContent (b, oid)
#ifdef LDAP_COMPONENT
int GDecAsnOidContent PROTO ((void* mem_op, GenBuf *b, GAsnOid  *result,
				AsnLen *bytesDecoded ));
#else
void GDecAsnOidContent PROTO ((GenBuf *b, GAsnOid  *result,
				AsnLen *bytesDecoded, ENV_TYPE env));
#endif

/*
 * Printable String
 */
typedef struct GPrintableString{
	char* identifier;
	PrintableString value;
} GPrintableString;

AsnLen GEncPrintableStringContent PROTO ((GenBuf *b, GPrintableString *octs));
#ifdef LDAP_COMPONENT
int GDecPrintableStringContent PROTO ((void* mem_op, GenBuf *b, GPrintableString *result,
					AsnLen *bytesDecoded ));
#else
void GDecPrintableStringContent PROTO ((GenBuf *b, GPrintableString *result,
					AsnLen *bytesDecoded, ENV_TYPE env));
#endif

/*
 * REAL
 */
typedef struct GAsnReal{
	char* identifier;
	AsnReal value;
} GAsnReal;

AsnLen GEncAsnRealContent PROTO ((GenBuf *b, GAsnReal *data));
#ifdef LDAP_COMPONENT
int GDecAsnRealContent PROTO ((void* mem_op,GenBuf *b, GAsnReal *result,
				AsnLen *bytesDecoded ));
#else
void GDecAsnRealContent PROTO ((GenBuf *b, GAsnReal *result,
				AsnLen *bytesDecoded, ENV_TYPE env));
#endif

/*
 * Relative OID
 */

typedef struct GAsnRelativeOid{
	char* identifier;
	AsnRelativeOid value;
} GAsnRelativeOid;

#define GEncAsnRelativeOidContent( b, oid)   GEncAsnOctsContent (b, oid)
#ifdef LDAP_COMPONENT
int GDecAsnRelativeOidContent PROTO ((void* mem_op, GenBuf *b, GAsnRelativeOid  *result,
					  AsnLen *bytesDecoded ));
#else
void GDecAsnRelativeOidContent PROTO ((GenBuf *b, GAsnRelativeOid  *result,
					  AsnLen *bytesDecoded, ENV_TYPE env));
#endif

/*
 * Teletex String
 */
typedef struct GTeletexString
{
	char* identifier;
	TeletexString value;
} GTeletexString;

AsnLen GEncTeletexStringContent PROTO ((GenBuf *b, GTeletexString *octs));
#ifdef LDAP_COMPONENT
int GDecTeletexStringContent PROTO ((void* mem_op,GenBuf *b, GTeletexString *result,
				  AsnLen *bytesDecoded ));
#else
void GDecTeletexStringContent PROTO ((GenBuf *b, GAsnOcts *result,
				  AsnLen *bytesDecoded, ENV_TYPE env));
#endif

/*
 * Universal String
 */
typedef struct GUniversalString{
	char* identifier;
	UniversalString value;
} GUniversalString;

AsnLen GEncUniversalStringContent PROTO ((GenBuf *b, GUniversalString *octs));
#ifdef LDAP_COMPONENT
int GDecUniversalStringContent PROTO ((void* mem_op, GenBuf *b, GUniversalString *result,
					AsnLen *bytesDecoded ));
#else
void GDecUniversalStringContent PROTO ((GenBuf *b, GUniversalString *result,
					AsnLen *bytesDecoded, ENV_TYPE env));
#endif

/*
 * UTF8 String
 */
typedef struct GUTF8String{
	char* identifier;
	UTF8String value;
} GUTF8String;

AsnLen GEncUTF8StringContent PROTO ((GenBuf *b, GUTF8String *octs));
#ifdef LDAP_COMPONENT
int GDecUTF8StringContent PROTO ((void* mem_op,GenBuf *b, GUTF8String *result,
				AsnLen *bytesDecoded ));
#else
void GDecUTF8StringContent PROTO ((GenBuf *b, GUTF8String *result,
				AsnLen *bytesDecoded, ENV_TYPE env));
#endif

/*
 * Visible String
 */
typedef struct GVisibleString{
	char* identifier;
	VisibleString value;
} GVisibleString;

AsnLen GEncVisibleStringContent PROTO ((GenBuf *b, GVisibleString *octs));
#ifdef LDAP_COMPONENT
int GDecVisibleStringContent PROTO ((void* mem_op, GenBuf *b, GVisibleString *result,
					  AsnLen *bytesDecoded ));
#else
void GDecVisibleStringContent PROTO ((GenBuf *b, GVisibleString *result,
					  AsnLen *bytesDecoded, ENV_TYPE env));
#endif

#define GDecISO646StringContent GDecUTF8StringContent
#define GDecGeneralizedTimeContent GDecUTF8StringContent
#define GDecUTCTimeContent GDecUTF8StringContent

/*
 * Directory String
 */
typedef enum stringtype {
	teletexStr = 1,
	printableStr,
	bmpStr,
	universalStr,
	utf8Str
} string_type;

typedef struct GDirectoryString {
	char* identifier;
	string_type strType;
	AsnOcts value;
} GDirectoryString;

AsnLen GEncDirectoryStringContent PROTO (( GenBuf *b, GDirectoryString *octs ));
#ifdef LDAP_COMPONENT
int GDecDirectoryStringContent PROTO ((void* mem_op, GenBuf *b,
			GDirectoryString *result, AsnLen *bytesDecoded ));
#else
void GDecDirectoryStringContent PROTO ((GenBuf *b, GDirectoryString *result,
				AsnLen *bytesDecoded, ENV_TYPE env));
#endif

/*
 * Utilitiies for GSER DEcoding
 */
typedef enum LOCATE_MODE{//default is to move curr pointer
	GSER_COPY=1,
	GSER_NO_COPY=2,
	GSER_PEEK=3//(it implies NO_COPY, just peek a position)
} LOCATE_MODE;
#ifdef LDAP_COMPONENT
extern int LocateNextGSERToken( void* mem_op, GenBuf*, char**, LOCATE_MODE );
#else
extern int LocateNextGSERToken(GenBuf*, char**, LOCATE_MODE);
#endif

#endif
