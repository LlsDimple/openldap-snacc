
/* Include Files */
#include "asn-incl.h"


/* Function Prototypes */
static int chkNumericString (NumericString *checkBuf);


AsnLen BEncNumericStringContent(GenBuf *b, NumericString *octs)
{
	if (chkNumericString (octs) < 0)
	{
        Asn1Error ("BEncNumericStringContent: ERROR - Format Error");
		GenBufSetWriteError (b, TRUE);
	}

	return (BEncAsnOctsContent (b, octs));
} /* end of BEncNumericStringContent() */


AsnLen BEncNumericString(GenBuf *b, NumericString *v)
{
    AsnLen l;

    l = BEncNumericStringContent (b, v);
    l += BEncDefLen (b, l);
    l += BEncTag1 (b, UNIV, PRIM, NUMERICSTRING_TAG_CODE);
    return l;
} /* end of BEncNumericString() */

#ifdef LDAP_COMPONENT
int
BDecNumericStringContent( void* mem_op, GenBuf *b, AsnTag tagId, AsnLen len,
			  NumericString *result, AsnLen *bytesDecoded )
{
	int rc;
	rc = BDecAsnOctsContent ( mem_op, b, tagId, len, result, bytesDecoded  );
	if (chkNumericString (result) < 0)
	{
		Asn1Error ("BDecNumericStringContent: ERROR - Format Error");
		return -1;
	}
	return rc;
} /* end of BDecNumericStringContent() */
#else
void BDecNumericStringContent(GenBuf *b, AsnTag tagId, AsnLen len,
							  NumericString *result, AsnLen *bytesDecoded, 
							  ENV_TYPE env)
{
	BDecAsnOctsContent (b, tagId, len, result, bytesDecoded,  env);
	if (chkNumericString (result) < 0)
	{
        Asn1Error ("BDecNumericStringContent: ERROR - Format Error");
        longjmp (env, -40);
	}
} /* end of BDecNumericStringContent() */
#endif

#ifdef LDAP_COMPONENT
int
BDecNumericString( void* mem_op, GenBuf *b, NumericString *result, AsnLen *bytesDecoded)
{
    AsnTag tag;
    AsnLen elmtLen1;

    if (((tag = BDecTag (b, bytesDecoded )) != 
		MAKE_TAG_ID (UNIV, PRIM, NUMERICSTRING_TAG_CODE)) &&
		(tag != MAKE_TAG_ID (UNIV, CONS, NUMERICSTRING_TAG_CODE)))
    {
        Asn1Error ("BDecNumericString: ERROR - wrong tag\n");
	return -1;
    }

    elmtLen1 = BDecLen (b, bytesDecoded);
    return BDecNumericStringContent ( mem_op, b, tag, elmtLen1, result, bytesDecoded );

} /* end of BDecNumericString() */
#else
void BDecNumericString(GenBuf *b, NumericString *result, AsnLen *bytesDecoded,
					   ENV_TYPE env)
{
    AsnTag tag;
    AsnLen elmtLen1;

    if (((tag = BDecTag (b, bytesDecoded, env)) != 
		MAKE_TAG_ID (UNIV, PRIM, NUMERICSTRING_TAG_CODE)) &&
		(tag != MAKE_TAG_ID (UNIV, CONS, NUMERICSTRING_TAG_CODE)))
    {
        Asn1Error ("BDecNumericString: ERROR - wrong tag\n");
        longjmp (env, -100);
    }

    elmtLen1 = BDecLen (b, bytesDecoded, env);
    BDecNumericStringContent (b, tag, elmtLen1, result, bytesDecoded, env);

} /* end of BDecNumericString() */
#endif

static int chkNumericString(NumericString *checkBuf)
{
	unsigned int i;
	
	if (checkBuf == NULL)
		return -1;

	for (i = 0; i < checkBuf->octetLen; i++)
	{
		if ((checkBuf->octs[i] != ' ') && 
			((checkBuf->octs[i] < '0') || (checkBuf->octs[i] > '9')))
			return -1;
	}

	return 0;
} /* end of chkNumericString() */

