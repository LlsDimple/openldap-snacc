#ifndef _asn_TeletexString_h_
#define _asn_TeletexString_h_

#ifdef __cplusplus
extern "C" {
#endif 

typedef AsnOcts TeletexString; /* [UNIVERSAL 20] IMPLICIT OCTET STRING */

AsnLen BEncTeletexString PROTO ((GenBuf *b, TeletexString *v));
AsnLen BEncTeletexStringContent PROTO ((GenBuf *b, AsnOcts *octs));

#ifdef LDAP_COMPONENT
int BDecTeletexString PROTO ((GenBuf *b, TeletexString *result, AsnLen *bytesDecoded));
int BDecTeletexStringContent PROTO ((GenBuf *b, AsnTag tagId, AsnLen len, AsnOcts *result, AsnLen *bytesDecoded ));
#else
void BDecTeletexString PROTO ((GenBuf *b, TeletexString *result, AsnLen *bytesDecoded, ENV_TYPE env));
void BDecTeletexStringContent PROTO ((GenBuf *b, AsnTag tagId, AsnLen len, AsnOcts *result, AsnLen *bytesDecoded, ENV_TYPE env));
#endif

#define PrintTeletexString PrintAsnOcts

#define FreeTeletexString FreeAsnOcts 

#ifdef __cplusplus 
}
#endif

#endif
