#ifndef _asn_NumericString_h_
#define _asn_NumericString_h_

#ifdef __cplusplus
extern "C" {
#endif

typedef AsnOcts NumericString; /* [UNIVERSAL 18] IMPLICIT OCTET STRING */

AsnLen BEncNumericString PROTO ((GenBuf *b, NumericString *v));
AsnLen BEncNumericStringContent PROTO ((GenBuf *b, NumericString *octs));

#ifdef LDAP_COMPONENT
int BDecNumericString PROTO ((GenBuf *b, NumericString *result, AsnLen *bytesDecoded));
int BDecNumericStringContent PROTO ((GenBuf *b, AsnTag tagId, AsnLen len, NumericString *result, AsnLen *bytesDecoded ));
#else
void BDecNumericString PROTO ((GenBuf *b, NumericString *result, AsnLen *bytesDecoded, ENV_TYPE env));
void BDecNumericStringContent PROTO ((GenBuf *b, AsnTag tagId, AsnLen len, NumericString *result, AsnLen *bytesDecoded, ENV_TYPE env));
#endif

#define PrintNumericString PrintAsnOcts

#define FreeNumericString FreeAsnOcts

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

