#include "asn-incl.h"
AsnLen BEncTeletexStringContent PARAMS ((b, octs),
GenBuf *b _AND_
AsnOcts *octs)
{
	return (BEncAsnOctsContent (b, octs));
}

AsnLen BEncTeletexString PARAMS ((b, v),
GenBuf *b _AND_
TeletexString *v)
{
    AsnLen l;
    l = BEncTeletexStringContent (b, v);
    l += BEncDefLen (b, l);
    l += BEncTag1 (b, UNIV, PRIM, TELETEXSTRING_TAG_CODE);
    return l;
} /* BEncTeletexString */
#ifdef LDAP_COMPONENT
int BDecTeletexStringContent PARAMS (( mem_op, b, tagId, len, result, bytesDecoded ),
 void* mem_op _AND_
 GenBuf *b _AND_
 AsnTag tagId _AND_
 AsnLen len _AND_
 AsnOcts *result _AND_
 AsnLen *bytesDecoded )
{
	return BDecAsnOctsContent ( mem_op, b, tagId, len, result, bytesDecoded );
}
int BDecTeletexString PARAMS (( mem_op, b, result, bytesDecoded ),
void* mem_op _AND_
GenBuf *b _AND_
TeletexString *result _AND_
AsnLen *bytesDecoded )
{
    AsnTag tag;
    AsnLen elmtLen1;

    if (((tag = BDecTag (b, bytesDecoded )) != 
MAKE_TAG_ID (UNIV, PRIM, TELETEXSTRING_TAG_CODE))&&
         (tag != MAKE_TAG_ID (UNIV, CONS, TELETEXSTRING_TAG_CODE)))
    {
        Asn1Error ("BDecTeletexString: ERROR - wrong tag\n");
	return -1;
    }
    elmtLen1 = BDecLen (b, bytesDecoded );
    return BDecTeletexStringContent ( mem_op, b, tag, elmtLen1, result, bytesDecoded );
}  /* BDecTeletexString */
#else
void BDecTeletexStringContent PARAMS ((b, tagId, len, result, bytesDecoded, env),
 GenBuf *b _AND_
 AsnTag tagId _AND_
 AsnLen len _AND_
 AsnOcts *result _AND_
 AsnLen *bytesDecoded _AND_
 ENV_TYPE env)
{
	BDecAsnOctsContent (b, tagId, len, result, bytesDecoded, env);
}
void BDecTeletexString PARAMS ((b, result, bytesDecoded, env),
GenBuf *b _AND_
TeletexString *result _AND_
AsnLen *bytesDecoded _AND_
ENV_TYPE env)
{
    AsnTag tag;
    AsnLen elmtLen1;

    if (((tag = BDecTag (b, bytesDecoded, env)) != 
MAKE_TAG_ID (UNIV, PRIM, TELETEXSTRING_TAG_CODE))&&
         (tag != MAKE_TAG_ID (UNIV, CONS, TELETEXSTRING_TAG_CODE)))
    {
        Asn1Error ("BDecTeletexString: ERROR - wrong tag\n");
        longjmp (env, -102);
    }
    elmtLen1 = BDecLen (b, bytesDecoded, env);
    BDecTeletexStringContent (b, tag, elmtLen1, result, bytesDecoded, env);
}  /* BDecTeletexString */
#endif
