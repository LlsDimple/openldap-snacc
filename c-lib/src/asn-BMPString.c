
/* Include Files */
#include "asn-incl.h"
#include "gen-buf.h"

AsnLen BEncBMPStringContent(GenBuf *b, BMPString *octs)
{
	if ((octs->octetLen % 2) != 0)
	{
		BufSetWriteError (b, TRUE);
	}
	return BEncAsnOctsContent(b, octs);
} /* end of BEncBMPStringContent() */


AsnLen BEncBMPString(GenBuf *b, BMPString *v)
{
    AsnLen l;

    l = BEncBMPStringContent (b, v);
    l += BEncDefLen (b, l);
    l += BEncTag1 (b, UNIV, PRIM, BMPSTRING_TAG_CODE);
    return l;
} /* end of BEncBMPString() */


#ifdef LDAP_COMPONENT
int BDecBMPStringContent(GenBuf *b, AsnTag tagId, AsnLen len,
				 BMPString *result, AsnLen *bytesDecoded )
{
	int rc;
	rc = BDecAsnOctsContent(b, tagId, len, result, bytesDecoded );
	if ((result->octetLen % 2) != 0)
	{
		Asn1Error ("BDecBMPStringContent: ERROR - Invalid BMPString Format");
		return -1;
	}
	return rc;
}
#else
void BDecBMPStringContent(GenBuf *b, AsnTag tagId, AsnLen len,
						  BMPString *result, AsnLen *bytesDecoded,
						  ENV_TYPE env)
{
	BDecAsnOctsContent(b, tagId, len, result, bytesDecoded, env);
	if ((result->octetLen % 2) != 0)
	{
		Asn1Error ("BDecBMPStringContent: ERROR - Invalid BMPString Format");
		longjmp (env, -40);
	}
}
#endif

#ifdef LDAP_COMPONENT
int BDecBMPString(GenBuf *b, BMPString *result, AsnLen *bytesDecoded)
{
	AsnTag tag;
	AsnLen elmtLen1;
	if (((tag = BDecTag (b, bytesDecoded )) != 
		MAKE_TAG_ID (UNIV, PRIM, BMPSTRING_TAG_CODE)) &&
		(tag != MAKE_TAG_ID (UNIV, CONS, BMPSTRING_TAG_CODE)))
	{
		Asn1Error ("BDecBMPString: ERROR - wrong tag\n");
		return -1;
	}
	elmtLen1 = BDecLen (b, bytesDecoded );
	BDecBMPStringContent (b, tag, elmtLen1, result, bytesDecoded );
	return 1;

}  /* BDecBMPString */
#else
void BDecBMPString(GenBuf *b, BMPString *result, AsnLen *bytesDecoded,
				   ENV_TYPE env)
{
	AsnTag tag;
	AsnLen elmtLen1;
	if (((tag = BDecTag (b, bytesDecoded, env)) != 
		MAKE_TAG_ID (UNIV, PRIM, BMPSTRING_TAG_CODE)) &&
		(tag != MAKE_TAG_ID (UNIV, CONS, BMPSTRING_TAG_CODE)))
	{
		Asn1Error ("BDecBMPString: ERROR - wrong tag\n");
		longjmp (env, -113);
	}
	elmtLen1 = BDecLen (b, bytesDecoded, env);
	BDecBMPStringContent (b, tag, elmtLen1, result, bytesDecoded, env);

}  /* BDecBMPString */
#endif

/* Convert a BMPString to a wide character string */
int CvtBMPString2wchar(BMPString *inOcts, wchar_t **outStr)
{
	unsigned int i, j;

	if ((inOcts == NULL) || (outStr == NULL))
		return -1;
	if ((inOcts->octetLen % 2) != 0)
		return -2;

	*outStr = (wchar_t*)calloc(inOcts->octetLen / 2 + 1, sizeof(wchar_t));
	if (*outStr == NULL)
		return -1;

  	/* Transform the BMPString into wchar_t array */
	for (i = 0, j = 0; i < (inOcts->octetLen / 2); i++, j += 2)
		(*outStr)[i] = (wchar_t)((inOcts->octs[j] << 8) | inOcts->octs[j + 1]);

	return 0;

} /* end of CvtBMPAsnOcts2wchar() */

