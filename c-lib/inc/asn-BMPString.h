#ifndef _asn_BMPString_h_
#define _asn_BMPString_h_

#ifdef __cplusplus
extern "C" {
#endif


typedef AsnOcts BMPString; /* [UNIVERSAL 30] IMPLICIT OCTET STRING */

AsnLen BEncBMPString PROTO ((GenBuf *b, BMPString *v));
AsnLen BEncBMPStringContent PROTO ((GenBuf *b, BMPString *octs));

#ifdef LDAP_COMPONENT
int BDecBMPString PROTO (( void* mem_op, GenBuf *b, BMPString *result, AsnLen *bytesDecoded ));
int BDecBMPStringContent PROTO (( void* mem_op, GenBuf *b, AsnTag tagId, AsnLen len, BMPString *result, AsnLen *bytesDecoded ));
#else
void BDecBMPString PROTO ((GenBuf *b, BMPString *result, AsnLen *bytesDecoded, ENV_TYPE env));
void BDecBMPStringContent PROTO ((GenBuf *b, AsnTag tagId, AsnLen len, BMPString *result, AsnLen *bytesDecoded, ENV_TYPE env));
#endif

#define PrintBMPString PrintAsnOcts

#define FreeBMPString FreeAsnOcts 

int CvtBMPString2wchar(BMPString *inOcts, wchar_t **outStr);

#ifdef __cplusplus
}
#endif /* extern 'C' */

#endif

