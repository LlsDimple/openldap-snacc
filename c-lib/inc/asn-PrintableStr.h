#ifndef _asn_PrintableString_h_
#define _asn_PrintableString_h_

#ifdef __cplusplus
extern "C" {
#endif


typedef AsnOcts PrintableString; /* [UNIVERSAL 19] IMPLICIT OCTET STRING */

AsnLen BEncPrintableString PROTO ((GenBuf *b, PrintableString *v));
AsnLen BEncPrintableStringContent PROTO ((GenBuf *b, PrintableString *octs));

#ifdef LDAP_COMPONENT
int BDecPrintableString PROTO (( void* mem_op, GenBuf *b, PrintableString *result, AsnLen *bytesDecoded ));
int BDecPrintableStringContent PROTO (( void* mem_op, GenBuf *b, AsnTag tagId, AsnLen len, PrintableString *result, AsnLen *bytesDecoded ));
#else
void BDecPrintableString PROTO ((GenBuf *b, PrintableString *result, AsnLen *bytesDecoded, ENV_TYPE env));
void BDecPrintableStringContent PROTO ((GenBuf *b, AsnTag tagId, AsnLen len, PrintableString *result, AsnLen *bytesDecoded, ENV_TYPE env));
#endif

#define PrintPrintableString PrintAsnOcts

#define FreePrintableString FreeAsnOcts 

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
