/*
 * compiler/back-ends/c-gen/gen-dec.c - routines for printing C decoders from type trees
 *
 *   The type tree has already been run through the c type generator
 *   (type-info.c).  Types that the type generator didn't know how
 *   to handle (or didn't want/need to handle eg macros) get the
 *   C_NO_TYPE label and are ignored for code generation.
 *
 *   NOTE: this is a real rats nest - it sort of evolved.  It was
 *         written assuming SETs/SEQ/CHOICE etc could be nested
 *         hence all the crap about 'levels'.
 *
 * Mike Sample
 * 91/10/23
 * Copyright (C) 1991, 1992 Michael Sample
 *            and the University of British Columbia
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * $Header: /baseline/SNACC/compiler/back-ends/c-gen/gen-dec.c,v 1.15 2004/03/12 18:51:20 gronej Exp $
 *
 */

#include <string.h>
#include "asn-incl.h"
#include "asn1module.h"
#include "rules.h"
#include "snacc-util.h"
#include "util.h"
#include "tag-util.h"
#include "enc-rules.h"


static CRules *genDecCRulesG;
char *valueArgNameG = "v";
static long *longJmpValG;
static char *decodedLenVarNameG = "totalElmtsLen";
static char *itemLenVarNameG = "elmtLen";
/*static char *mecVarNameG = "mandatoryElmtCount";*/
static char *tagIdVarNameG = "tagId";
char *bufTypeNameG  = "GenBuf *";
char *lenTypeNameG = "AsnLen";
char *tagTypeNameG = "AsnTag";
char *envTypeNameG = "ENV_TYPE";



/* non-exported prototypes */

static void PrintCDecoderPrototype PROTO ((FILE *hdr, TypeDef *td));
static void PrintCDecoderDeclaration PROTO ((FILE *src, TypeDef *td));
static void PrintCDecoderDefine PROTO ((FILE *src, TypeDef *td));

static int RecCountVariableLevels PROTO ((Type *t));
static int CountVariableLevels PROTO ((Type *t));
static void PrintCDecoderLocals PROTO ((FILE *src, TypeDef *td));
/*static void PrintCListDecoderLocals PROTO ((FILE *src));*/
static void PrintCSeqGSERDecodeCode PROTO((FILE *src, TypeDef *td, Type *parent, NamedTypeList *elmts, char *varName));
static void PrintCSetDecodeCode PROTO ((FILE *src, TypeDef *td, Type *parent,
									   NamedTypeList *e, int elmtLevel,
									   int totalLevel, int tagLevel,
									   char *varName));
static void PrintCSeqDecodeCode PROTO ((FILE *src, TypeDef *td, Type *parent,
									   NamedTypeList *e, int elmtLevel,
									   int totalLevel, int tagLevel,
									   char *varName));
static void PrintCListDecoderCode PROTO ((FILE *src, TypeDef *td, Type *t,
										 int elmtLevel, int totalLevel,
										 int tagLevel, char *varName));
static void PrintCChoiceDecodeCode PROTO ((FILE *src, TypeDef *td, Type *t,
										  int elmtLevel, int totalLevel,
										  int tagLevel, char *varName));
/*static void PrintCLenDecodingCode PROTO ((FILE *f));
static void PrintCDecoderIncludes PROTO ((FILE *src, Module *m,
										 ModuleList *mods));
*/
static void PrintCElmtDecodeCode PROTO ((FILE *src, TypeDef *td, Type *parent,
										Type *t, int elmtLevel, int totalLevel,
										int tagLevel, char *parnetVarName,
										char *elmtVarName, char* elmtVarName2, int stoleChoiceTags));

static void PrintCElmtMatchingRuleCode PROTO ((FILE *src, TypeDef *td,
			Type *parent, Type *t, char *parnetVarName,
			char *elmtVarName, char* elmtVarName2));
static void PrintCListGSERDecoderCode PROTO ((FILE *src, TypeDef *td,
			Type *list, char *varName));
static void PrintCSetMatchingRuleCode PROTO (( FILE *src, 
   			 TypeDef *td , Type *parent, 
			NamedTypeList *elmts , char *varName));
static void PrintCMatchingCommonHeadCode PROTO (( FILE* src ));
static void PrintCSetGSERDecodeCode PROTO (( FILE *src, TypeDef *td,
			Type *parent, NamedTypeList *elmts,
			char *varName));
static void PrintCElmtExtractorCode PROTO (( FILE *src, TypeDef *td,
			Type *parent, Type *t, char *parentVarName, 
			char *elmtVarName, char *elmtVarName2));
static void PrintCSeqMatchingRuleCode PROTO (( FILE *src, TypeDef *td,
			Type *parent, NamedTypeList *elmts,
			char *varName));
static void PrintCSeqExtractorCode PROTO (( FILE *src, TypeDef *td,
			Type *t, NamedTypeList *elmts, char *varName));
static void
PrintCSetExtractorCode PROTO ((FILE *src, TypeDef* td, Type* t,
			NamedTypeList* elmts, char* varName));
static void
PrintCSeqExtractorCode PROTO ((FILE *src, TypeDef* td, Type* t,
			NamedTypeList* elmts, char* varName));
static void
PrintCSeqMatchingRuleCode PROTO (( FILE *src, TypeDef *td, Type *parent,
			NamedTypeList *elmts, char *varName));
static void
PrintCSetMatchingRuleCode PROTO (( FILE *src, TypeDef *td, Type *parent,
			NamedTypeList *elmts, char *varName));
static void
PrintCListSetOfMatchingRuleCode PROTO (( FILE *src, TypeDef *td, Type *list,
			char *varName));
static void
PrintCListSeqOfMatchingRuleCode PROTO (( FILE *src, TypeDef *td, Type *list,
			char *varName));
static void
PrintCListSetOfMatchingRuleCode PROTO (( FILE *src, TypeDef *td, Type *list,
			char *varName));
static void
PrintCListSeqOfExtractorCode PROTO (( FILE *src, TypeDef *td, Type *list,
			char *varName));

static void
PrintCListSetOfExtractorCode PROTO (( FILE *src, TypeDef *td, Type *list,
			char *varName));

extern EncRulesType GetEncRulesType();

void
PrintCDecoder PARAMS ((src, hdr, r, m,  td, longJmpVal),
    FILE *src _AND_
    FILE *hdr _AND_
    CRules *r _AND_
    Module *m _AND_
    TypeDef *td _AND_
    long *longJmpVal)
{
    int i;
    enum BasicTypeChoiceId typeId;
    int elmtLevel;
    CTDI *ctdi;
    Tag *tag;
    char *classStr;
    char *formStr;
    int stoleChoiceTags;
    TagList *tags;
    EncRulesType *encoding;

    ctdi = td->cTypeDefInfo;
    if (!ctdi->genDecodeRoutine)
        return;

    encoding = GetEncRules();
    while (SetEncRules(*encoding)) {
      encoding++;

      /*
       *  if is type that refs another pdu type or lib type
       *  without generating a new type via tagging or named elmts
       *  print define to the hdr file
       * (a type is a pdu by default if it is ref'd by an ANY)
       */
      if (!IsNewType (td->type)  &&
          (!IsTypeRef (td->type) ||
           (IsTypeRef (td->type) &&
            (td->type->basicType->a.localTypeRef->link->cTypeDefInfo->isPdu ||
             ((td->type->basicType->a.localTypeRef->link->anyRefs != NULL) &&
              !LIST_EMPTY (td->type->basicType->a.localTypeRef->link->anyRefs)))))) {
        fprintf(hdr,"#define %s%s %s%s\n", 
		GetEncRulePrefix(), td->cTypeDefInfo->decodeRoutineName, 
		GetEncRulePrefix(), td->type->cTypeRefInfo->decodeRoutineName);
	/*
         fprintf(hdr,"#define %s%s(b, v, bytesDecoded, env) %s%s(b, v, bytesDecoded, env)\n", 
	 GetEncRulePrefix(), td->cTypeDefInfo->decodeRoutineName, 
	 GetEncRulePrefix() td->type->cTypeRefInfo->decodeRoutineName);
	*/
        return;
      }
      
      

      typeId = GetBuiltinType (td->type);

      /* print proto type to hdr file */
      fprintf (hdr, 
	       "void %s%s PROTO ((%s b, %s *result, %s *bytesDecoded, %s env));\n", 
	       GetEncRulePrefix(), ctdi->decodeRoutineName, bufTypeNameG, 
	       ctdi->cTypeName, lenTypeNameG, envTypeNameG);

      /* print routine in src */
      fprintf (src,"void %s%s PARAMS ((b, result, bytesDecoded, env),\n", 
	       GetEncRulePrefix(), ctdi->decodeRoutineName);
      fprintf (src,"%s b _AND_\n", bufTypeNameG);
      fprintf (src,"%s *result _AND_\n", ctdi->cTypeName);
      fprintf (src,"%s *bytesDecoded _AND_\n", lenTypeNameG);
      fprintf (src,"%s env)\n", envTypeNameG);
      fprintf (src,"{\n");
      fprintf (src,"    %s tag;\n", tagTypeNameG);
      
      /* print extra locals for redundant lengths */
      tags = GetTags (td->type, &stoleChoiceTags);
      for (i = 1; !stoleChoiceTags && (i <= LIST_COUNT (tags)); i++)
        fprintf (src,"    %s elmtLen%d;\n", lenTypeNameG, i);

      /* add extra len for choice */
      if (typeId == BASICTYPE_CHOICE)
        fprintf (src,"    %s elmtLen%d;\n", lenTypeNameG, i);

      fprintf (src,"\n");

      /* decode tag/length pairs */
      elmtLevel = 0;
      if (!stoleChoiceTags) {
	FOR_EACH_LIST_ELMT (tag, tags) {
	  classStr = Class2ClassStr (tag->tclass);
	  if (tag->form == ANY_FORM)
	    formStr = Form2FormStr (PRIM);
	  else
	    formStr = Form2FormStr (tag->form);

	  fprintf (src,
		   "    if (((tag = %sDecTag (b, bytesDecoded, env)) != \n", 
		   GetEncRulePrefix());

	  if (tag->tclass == UNIV) {
	      fprintf (src,"MAKE_TAG_ID (%s, %s, %s))", classStr, formStr, DetermineCode(tag, NULL, 0));//RWC;Code2UnivCodeStr (tag->code));
	      if (tag->form == ANY_FORM)
		fprintf (src,"&&\n         (tag != MAKE_TAG_ID (%s, %s, %s)))\n", classStr, Form2FormStr (CONS), DetermineCode(tag, NULL, 0));//RWC;Code2UnivCodeStr (tag->code));
                else
		  fprintf (src,")\n");
	  } else {
	    fprintf (src,"MAKE_TAG_ID (%s, %s, %s))", classStr, formStr, DetermineCode(tag, NULL, 1));//RWC;tag->code);
	    if (tag->form == ANY_FORM)
	      fprintf (src,"&&\n        (tag != MAKE_TAG_ID (%s, %s, %s)))\n", classStr, Form2FormStr (CONS), DetermineCode(tag, NULL, 1));//RWC;tag->code);
	    else
	      fprintf (src,")\n");

	  }
	  fprintf (src,"    {\n");
	  fprintf (src,"        Asn1Error (\"%s%s: ERROR - wrong tag\\n\");\n", 
		   GetEncRulePrefix(), ctdi->decodeRoutineName);
	  fprintf (src,"        longjmp (env, %d);\n", (int)(*longJmpVal)--);
	  fprintf (src,"    }\n");

	  fprintf (src,"    elmtLen%d = %sDecLen (b, bytesDecoded, env);\n", ++elmtLevel, GetEncRulePrefix());
        }
      }

      /* for choices always decode first tag of the choice's content */
      if (typeId == BASICTYPE_CHOICE) {
	fprintf (src,"    tag = %sDecTag (b, bytesDecoded, env);\n", GetEncRulePrefix());
        fprintf (src,"    elmtLen%d = %sDecLen (b, bytesDecoded, env);\n", ++elmtLevel, GetEncRulePrefix());
      }

      if ((typeId != BASICTYPE_ANY) && (typeId != BASICTYPE_ANYDEFINEDBY))
        fprintf (src,"    %s%sContent (b, tag, elmtLen%d, result, bytesDecoded, env);\n", GetEncRulePrefix(), ctdi->decodeRoutineName, elmtLevel);
      else
        fprintf (src,"    %s%s (b, result, bytesDecoded, env);\n", 
		 GetEncRulePrefix(), ctdi->decodeRoutineName);


      /* grab any EOCs that match redundant, indef lengths */
      for (i = elmtLevel-1; i > 0; i--) {
	fprintf (src,"    if (elmtLen%d == INDEFINITE_LEN)\n", i);
        fprintf (src,"        %sDecEoc (b, bytesDecoded, env);\n", 
		 GetEncRulePrefix());
      }


      fprintf (src,"}  /* %s%s */\n\n", GetEncRulePrefix(),
	       ctdi->decodeRoutineName);

      FreeTags (tags);
    }
    m = m;
    r = r;    /* AVOIDS warning. */
}  /*  PrintCDecoder */

/*
 * GSER Decoder generation routine for CHOICE type
 * Written by Sang Seok Lim (IBM)
 */

static void 
PrintIdentifierParsingCode PARAMS ((src, td, t, varName),
FILE *src _AND_
TypeDef *td _AND_
Type *t _AND_
char *varName)
{ 
    fprintf (src, "\tif ( mode ==2 || mode == 3) {\n");
    fprintf (src, "\tif( !(strLen = LocateNextGSERToken(b,&peek_head,GSER_NO_COPY)) ){\n");
    fprintf (src, "\t\tAsn1Error(\"Error during Reading identifier\");\n");
    fprintf (src, "\t\treturn LDAP_PROTOCOL_ERROR;\n");
    fprintf (src, "\t}\n");
    fprintf (src, "\tif(strncmp( peek_head, \"%s\" ,strLen ) != 0 ){\n",t->cTypeRefInfo->cFieldName);
    fprintf (src, "\t\treturn LDAP_PROTOCOL_ERROR;\n");
    fprintf (src, "\t}\n");
    fprintf (src, "\tk->identifier.bv_val = peek_head;\n");
    fprintf (src, "\tk->identifier.bv_len = strLen;\n");
    fprintf (src, "\t}\n");
    fprintf (src, "\telse if ( mode == 0 ) mode = 2;\n");
    fprintf (src, "\telse if ( mode == 1 ) mode = 3;\n\n\n");
}

static void 
PrintCompDescriptorCode PARAMS ((src, td, t),
FILE *src _AND_
TypeDef *td _AND_
Type *t)
{
    char* name = td->cTypeDefInfo->cTypeName;

    fprintf (src, "\t*v = t = (Component%s*)malloc(sizeof(Component%s));\n",
		name, name);
    fprintf (src, "\t*t = *k;\n");
    fprintf (src, "\tif ( mode == 1 ) {\n");
    fprintf (src, "\t\tt->comp_desc = malloc( sizeof( ComponentDesc ) );\n");
    fprintf (src, "\t\tt->comp_desc->cd_tag = ASN_%s;\n", name);
    fprintf (src, "\t\tt->comp_desc->cd_identifier = NULL;\n");
    fprintf (src, "\t\tt->comp_desc->cd_decoder = GDec%s ;\n", name);
    fprintf (src, "\t\tt->comp_desc->cd_extract_t = NULL;\n");
    fprintf (src, "\t\tt->comp_desc->cd_extract_i = extract_comp_from_id_%s;\n", name);
    fprintf (src, "\t\tt->comp_desc->cd_type = ASN_COMPOSITE;\n");
    fprintf (src, "\t\tt->comp_desc->cd_type_id = COMPOSITE_ASN1_TYPE;\n");
    fprintf (src, "\t\tt->comp_desc->cd_all_match = GMatchingComponent%s;\n", name);
    fprintf (src, "\t}\n");
}

static void
PrintCChoiceGSERDecodeCode PARAMS ((src, td, t, varName),
    FILE *src _AND_
    TypeDef *td _AND_
    Type *t _AND_
    char *varName)
{
    NamedType *e;
    CTRI *ctri;
    enum BasicTypeChoiceId builtinType;
    char  tmpVarName[MAX_VAR_REF];
    char  choiceIdVarName[MAX_VAR_REF];
    CTRI *parentCtri;
    void *tmp;


    parentCtri = t->cTypeRefInfo;

    fprintf (src, "\tk = &c_T1;\n");

    fprintf (src, "\tif( !(strLen = LocateNextGSERToken(b,&peek_head,GSER_NO_COPY)) ){\n");
    fprintf (src, "\t\tAsn1Error(\"Error during Reading identifier\");\n");
    fprintf (src, "\t\treturn LDAP_PROTOCOL_ERROR;\n");
    fprintf (src, "\t}\n");
    fprintf (src, "\tif( !(strLen = LocateNextGSERToken(b,&peek_head2,GSER_NO_COPY)) ){\n");
    fprintf (src, "\t\tAsn1Error(\"Error during Reading identifier\");\n");
    fprintf (src, "\t\treturn LDAP_PROTOCOL_ERROR;\n");
    fprintf (src, "\t}\n");
    fprintf (src, "\tif(*peek_head2 != \':\'){\n");
    fprintf (src, "\t\tAsn1Error(\"Missing : in encoded data\");\n");
    fprintf (src, "\t\treturn LDAP_PROTOCOL_ERROR;\n");
    fprintf (src, "\t}\n");

    FOR_EACH_LIST_ELMT (e,  t->basicType->a.choice)
    {
        tmp = (void*)CURR_LIST_NODE (t->basicType->a.choice);

        ctri =  e->type->cTypeRefInfo;

        builtinType = GetBuiltinType (e->type);

        fprintf(src, "\tif( strcmp(\"%s\",peek_head) == 0){\n",
			ctri->cFieldName);

        MakeChoiceIdValueRef (genDecCRulesG, td, t, e->type, varName,
				choiceIdVarName);
        fprintf (src, "\t\t%s = %s;\n", choiceIdVarName, ctri->choiceIdSymbol);
	if ( ctri->isPtr )
		MakeVarPtrRef (genDecCRulesG,td, t, e->type, "&k", tmpVarName);
	else
		MakeVarPtrRef (genDecCRulesG,td, t, e->type, "k", tmpVarName);

	fprintf (src, "\t\trc = ");
        PrintCElmtDecodeCode (src, td, t, e->type, 0, 0, 0,
			varName, tmpVarName,(char*)NULL,0);
	fprintf (src,"\t\tif ( rc != LDAP_SUCCESS ) return rc;\n" );
	fprintf (src,"\t\t%s->identifier.bv_val = peek_head;\n",tmpVarName);
	fprintf (src,"\t\t%s->identifier.bv_len = strLen;\n",tmpVarName);
        fprintf (src, "\t}\n");
        /* reset curr list node to value remember at beg of loop */
        SET_CURR_LIST_NODE (t->basicType->a.choice, tmp);
    }
    PrintCompDescriptorCode (src, td, t);
}

/*
 * Matching Rule generations routine for CHOICE type
 * Written by Sang Seok Lim(IBM)
 */
static void
PrintCChoiceMatchingRuleCode PARAMS ((src, td, t, varName),
    FILE *src _AND_
    TypeDef *td _AND_
    Type *t _AND_
    char *varName)
{
    NamedType *e;
    CTRI *ctri;
    char  tmpVarName[MAX_VAR_REF] = "(ComponentSyntaxInfo*)";
    char  tmpVarName2[MAX_VAR_REF]= "(ComponentSyntaxInfo*)";
    void *tmp;

    fprintf (src, "\tv1 = (Component%s*)csi_attr;\n",t->cTypeRefInfo->cTypeName);
    fprintf (src, "\tv2 = (Component%s*)csi_assert;\n",t->cTypeRefInfo->cTypeName);

    PrintCMatchingCommonHeadCode ( src );

    fprintf (src, "\tif( (v1->%s != v2->%s ) || ( strncmp( v1->identifier.bv_val, v2->identifier.bv_val, v1->identifier.bv_len) !=0 ) )\n", t->cTypeRefInfo->choiceIdEnumFieldName, t->cTypeRefInfo->choiceIdEnumFieldName);
    fprintf (src, "\treturn LDAP_COMPARE_FALSE;\n");

    fprintf (src, "\tswitch( v1->%s )\n\t{\n",t->cTypeRefInfo->choiceIdEnumFieldName);

    FOR_EACH_LIST_ELMT (e,  t->basicType->a.choice)
    {
        tmp = (void*)CURR_LIST_NODE (t->basicType->a.choice);

        ctri =  e->type->cTypeRefInfo;

	fprintf (src, "\t   case %s :\n",ctri->choiceIdSymbol);

	if (!ctri->isPtr) {
		strcat(tmpVarName,"&");
		strcat(tmpVarName2,"&");
	}
	strcat(tmpVarName,"((Component");
	strcat(tmpVarName,varName);
	strcat(tmpVarName,")csi_attr)->");
	strcat(tmpVarName,e->type->cTypeRefInfo->cFieldName);

	strcat(tmpVarName2,"((Component");
	strcat(tmpVarName2,varName);
	strcat(tmpVarName2,")csi_assert)->");
	strcat(tmpVarName2,e->type->cTypeRefInfo->cFieldName);

	fprintf (src, "\t\trc = ");

        PrintCElmtMatchingRuleCode (src, td, t, e->type, varName,
				tmpVarName, tmpVarName2);
	fprintf (src, "\t\tbreak;\n");

        SET_CURR_LIST_NODE (t->basicType->a.choice, tmp);
    }
    fprintf (src, "\tdefault : \n\t\t return LDAP_PROTOCOL_ERROR;\n");
    fprintf (src, "\t}\n");
    fprintf (src, "\treturn rc;\n");
}

/*
 * Component Extractor generation routine for an CHOICE type
 * Written by Sang Seok Lim( IBM )
 */
static void
PrintCChoiceExtractorCode PARAMS ((src, td, t, varName),
    FILE *src _AND_
    TypeDef *td _AND_
    Type *t _AND_
    char *varName)
{
    NamedType *e;
    CTRI *ctri;
    char  tmpVarName[MAX_VAR_REF];
    char  choiceIdVarName[MAX_VAR_REF];
    void *tmp;

    FOR_EACH_LIST_ELMT (e,  t->basicType->a.choice)
    {
        tmp = (void*)CURR_LIST_NODE (t->basicType->a.choice);

        ctri =  e->type->cTypeRefInfo;

	MakeChoiceIdValueRef (genDecCRulesG,td,t,e->type, "comp",
				choiceIdVarName);

	fprintf (src, "\tif( %s ==  %s &&\n\t    strncmp(\"%s\", cr->cr_curr->ci_val.ci_identifier.bv_val,cr->cr_curr->ci_val.ci_identifier.bv_len) != 0 ) {\n",choiceIdVarName,ctri->choiceIdSymbol, e->type->cTypeRefInfo->cFieldName);

	fprintf (src, "\t\tif ( cr->cr_curr->ci_next == NULL )\n");
	if ( ctri->isPtr ) {
		MakeVarPtrRef (genDecCRulesG, td, t, e->type, "comp", tmpVarName);
		fprintf (src, "\t\t\treturn %s;\n",tmpVarName);
		fprintf (src, "\t\telse\n");
		fprintf (src, "\t\t\treturn ");
		PrintCElmtExtractorCode (src, td, t, e->type,
					varName, tmpVarName, NULL );
	}
	else {
		MakeVarPtrRef (genDecCRulesG, td, t, e->type,"comp", tmpVarName);
		fprintf (src, "\t\t\treturn %s;\n", tmpVarName);
		fprintf (src, "\t\telse\n");
		fprintf (src, "\t\t\treturn NULL;\n");
	}
	fprintf (src, "\t}\n");

        SET_CURR_LIST_NODE (t->basicType->a.choice, tmp);
    }

    fprintf (src, "\tdefault : \n\t\t return LDAP_PROTOCOL_ERROR;\n");
    fprintf (src, "\t}\n");
    fprintf (src, "\treturn NULL;\n");
}

void
PrintCContentDecoder PARAMS ((src, hdr, r, m,  td, longJmpVal),
    FILE *src _AND_
    FILE *hdr _AND_
    CRules *r _AND_
    Module *m _AND_
    TypeDef *td _AND_
    long *longJmpVal)
{
    CTDI *ctdi;
    CTypeId rhsTypeId;  /* cTypeId of the type that defined this typedef */
    EncRulesType* encoding;

    longJmpValG = longJmpVal;

    genDecCRulesG = r;

    ctdi =  td->cTypeDefInfo;
    if ((ctdi == NULL) || (td->type->cTypeRefInfo == NULL))
    {
        fprintf (stderr,"PrintCDecoder: ERROR - no type info\n");
        return;
    }

    if (!ctdi->genDecodeRoutine)
        return;

    rhsTypeId = td->type->cTypeRefInfo->cTypeId;

    encoding = GetEncRules();
    while(SetEncRules(*encoding)) {
      encoding++;
      switch (rhsTypeId) {
        /*
         * type refs or primitive types are
         * defined as calls to the referenced type
         */
      case C_ANY:
	//RWC;fprintf (hdr, "/* ANY - Fix Me! */\n");
      case C_ANYDEFINEDBY:
	fprintf(hdr, "#define %s%s %s%s\n", 
		GetEncRulePrefix(), td->cTypeDefInfo->decodeRoutineName, 
		GetEncRulePrefix(), 
		td->type->cTypeRefInfo->decodeRoutineName);
	
	/*
	  fprintf(hdr, "#define %s%s( b, tagId, elmtLen, v, bytesDecoded, env)  ", GetEncRulePrefix(), td->cTypeDefInfo->decodeRoutineName);
	  fprintf (hdr, "%s%s (b, tagId, elmtLen, v, bytesDecoded, env)", 
	  GetEncRulePrefix(), td->type->cTypeRefInfo->decodeRoutineName);
	*/
	fprintf (hdr,"\n\n");
	break;
	
      case C_LIB:
      case C_TYPEREF:
	        PrintCDecoderDefine (hdr, td);
	        fprintf (hdr,"\n\n");
	        break;

	
      case C_CHOICE:
	PrintCDecoderPrototype (hdr, td);
	fprintf (hdr,"\n\n");
	PrintCDecoderDeclaration (src, td);
	fprintf (src,"{\n");
	PrintCDecoderLocals (src, td);
	fprintf (src,"\n\n");
	if ( GetEncRulesType() == GSER )
		PrintCChoiceGSERDecodeCode (src, td, td->type, valueArgNameG);
	else
		PrintCChoiceDecodeCode (src, td, td->type, FIRST_LEVEL-1,
			FIRST_LEVEL,FIRST_LEVEL-1, valueArgNameG);
	
	if ( GetEncRulesType() != GSER )
		fprintf (src, "    (*bytesDecoded) += totalElmtsLen1;\n");

	fprintf (src,"}  /* %s%sContent */", GetEncRulePrefix(),
		 td->cTypeDefInfo->decodeRoutineName);
	fprintf (src,"\n\n");
	break;
	    
      case C_STRUCT:
	PrintCDecoderPrototype (hdr, td);
	fprintf (hdr,"\n\n");
	PrintCDecoderDeclaration (src, td);
	fprintf (src,"{\n");
	PrintCDecoderLocals (src, td);
	fprintf (src,"\n\n");
	if ( td->type->basicType->choiceId == BASICTYPE_SET ) {
		if ( GetEncRulesType() == GSER ){
			  PrintCSetGSERDecodeCode (src, td, td->type,
				td->type->basicType->a.set,valueArgNameG);
		}
		else {
			PrintCSetDecodeCode (src, td, td->type,
			td->type->basicType->a.set, FIRST_LEVEL-1,
			FIRST_LEVEL, FIRST_LEVEL-1, valueArgNameG);
		}
	} else {
		if ( (GetEncRulesType() == GSER) )
			  PrintCSeqGSERDecodeCode (src, td, td->type,
				td->type->basicType->a.sequence,valueArgNameG);
		else
			  PrintCSeqDecodeCode (src, td, td->type,
				td->type->basicType->a.sequence,
				FIRST_LEVEL-1, FIRST_LEVEL, FIRST_LEVEL-1,
				valueArgNameG);
	}
	if ( (GetEncRulesType() != GSER) )
		fprintf (src, "    (*bytesDecoded) += totalElmtsLen1;\n");

	fprintf (src,"}  /* %s%s*/", GetEncRulePrefix(),
		 td->cTypeDefInfo->decodeRoutineName);
	fprintf (src,"\n\n");
	break;


      case C_LIST:
	PrintCDecoderPrototype (hdr, td);
	fprintf (hdr,"\n\n");

	PrintCDecoderDeclaration (src, td);
	fprintf (src,"{\n");
	PrintCDecoderLocals (src, td);
	fprintf (src,"\n\n");
	if ( (GetEncRulesType() == GSER) )
		PrintCListGSERDecoderCode (src, td, td->type, valueArgNameG);
	else
		PrintCListDecoderCode (src, td, td->type,  FIRST_LEVEL-1,
					FIRST_LEVEL, FIRST_LEVEL-1, valueArgNameG);

	if ( (GetEncRulesType() != GSER) )
		fprintf (src, "    (*bytesDecoded) += totalElmtsLen1;\n");

	fprintf (src,"}  /* %s%sContent */", GetEncRulePrefix(),
		 td->cTypeDefInfo->decodeRoutineName);
	fprintf (src,"\n\n");
	break;

      case C_NO_TYPE:
	/*            fprintf (src,"< sorry, unsupported type >\n\n"); */
	return; /* dont' print newlines */
	break;

      default:
	fprintf (stderr,"PrintCContentDecoder: ERROR - unknown c type id\n");
	return;
	break;
      }
    }

    m = m;   /* AVOIDS warning. */
}  /*  PrintCContentDecoder */



/*
 * Prints prototype for decode routine in hdr file
 */

static void
PrintCDecoderPrototype PARAMS ((hdr, td),
    FILE *hdr _AND_
    TypeDef *td)
{
    CTDI *ctdi;

    ctdi =  td->cTypeDefInfo;
    if ( (GetEncRulesType() == GSER) ){
	fprintf (hdr,"int %s%s PROTO ((%s b, %s **v, %s *bytesDecoded, int mode));\n", GetEncRulePrefix(), ctdi->decodeRoutineName, bufTypeNameG, ctdi->cTypeName, lenTypeNameG );
    }
    else
	fprintf (hdr,"void %s%sContent PROTO ((%s b, %s tagId%d, %s elmtLen%d, %s *v, %s *bytesDecoded, %s env));\n", GetEncRulePrefix(), ctdi->decodeRoutineName, bufTypeNameG, tagTypeNameG, FIRST_LEVEL-1, lenTypeNameG, FIRST_LEVEL-1, ctdi->cTypeName, lenTypeNameG, envTypeNameG);

}  /*  PrintCDecoderPrototype */

static void
PrintCMatchingRulePrototype PARAMS ((hdr, td),
    FILE *hdr _AND_
    TypeDef *td)
{
    CTDI *ctdi;

    ctdi =  td->cTypeDefInfo;

    fprintf (hdr,"int %s%s PROTO ((ComponentSyntaxInfo *, ComponentSyntaxInfo *v2 ));\n", GetEncRulePrefix(), ctdi->matchingRuleName);
} /* PrintCMatchingRulePrototype */

static void
PrintCExtractorPrototype PARAMS ((hdr, td),
    FILE *hdr _AND_
    TypeDef *td)
{
    CTDI *ctdi;

    ctdi =  td->cTypeDefInfo;

    fprintf (hdr,"void* %s PROTO ((ComponentReference *cr, Component%s *comp ));\n", ctdi->compExtractorName, ctdi->cTypeName );
} /* PrintCExtractorPrototype */

/*
 * Prints declarations of decode routine for the given type def
 */
static void
PrintCDecoderDeclaration PARAMS ((src,td),
    FILE *src _AND_
    TypeDef *td)
{
    CTDI *ctdi;

    ctdi =  td->cTypeDefInfo;
    fprintf (src,"void\n");
    if ( (GetEncRulesType() == GSER) ) {
    fprintf (src,"%s%s PARAMS ((b, v, bytesDecoded, mode),\n", 
	     GetEncRulePrefix(), ctdi->decodeRoutineName);
    fprintf (src,"%s b _AND_\n", bufTypeNameG);
    fprintf (src,"void **v _AND_\n");
    fprintf (src,"%s *bytesDecoded _AND_\n", lenTypeNameG);
    fprintf (src,"int mode)\n" );
    }
    else {
    fprintf (src,"%s%sContent PARAMS ((b, tagId%d, elmtLen%d, v, bytesDecoded, env),\n", 
	     GetEncRulePrefix(), ctdi->decodeRoutineName, FIRST_LEVEL -1,
	     FIRST_LEVEL -1);
    fprintf (src,"%s b _AND_\n", bufTypeNameG);
    fprintf (src,"%s tagId%d _AND_\n", tagTypeNameG, FIRST_LEVEL -1);
    fprintf (src,"%s elmtLen%d _AND_\n", lenTypeNameG, FIRST_LEVEL -1);
    fprintf (src,"%s *v _AND_\n", ctdi->cTypeName);
    fprintf (src,"%s *bytesDecoded _AND_\n", lenTypeNameG);
    fprintf (src,"%s env)\n", envTypeNameG);
    }

}  /*  PrintCDecoderDeclaration */

static void
PrintCMatchingRuleDeclaration PARAMS ((src,td),
    FILE *src _AND_
    TypeDef *td)
{
    CTDI *ctdi;

    ctdi =  td->cTypeDefInfo;
    fprintf (src,"int\n");
    fprintf (src,"%s%s ( char* oid, ComponentSyntaxInfo* csi_attr, ComponentSyntaxInfo* csi_assert ) ", GetEncRulePrefix(), ctdi->matchingRuleName);
}  /*  PrintCMatchingRuleDeclaration */

static void
PrintCExtractorDeclaration PARAMS ((src,td),
    FILE *src _AND_
    TypeDef *td)
{
    CTDI *ctdi;

    ctdi =  td->cTypeDefInfo;
    fprintf (src,"void*\n");
    fprintf (src,"%s ( ComponentReference* cr, Component%s *comp )\n", ctdi->compExtractorName, ctdi->cTypeName);
}  /*  PrintCExtractorDeclaration */

/*
 * makes a define for type refs or primitive type renaming
 * EG:
 * TypeX ::= INTEGER --> #define BerDecodeTypeX(b,v) BerDecodeInteger(b,v)
 * TypeX ::= TypeY --> #define BerDecodeTypeX(b,v) BerDecodeTypeY(b,v)
 */
static void
PrintCDecoderDefine PARAMS ((hdr, td),
    FILE *hdr _AND_
    TypeDef *td)
{
    fprintf(hdr, "#define %s%sContent %s%sContent", 
	    GetEncRulePrefix(), td->cTypeDefInfo->decodeRoutineName, 
	    GetEncRulePrefix(), td->type->cTypeRefInfo->decodeRoutineName);

/*
    fprintf(hdr, "#define %sContent( b, tagId, elmtLen, v, bytesDecoded, env)  ", td->cTypeDefInfo->decodeRoutineName);
    fprintf (hdr, "%s%sContent (b, tagId, elmtLen, v, bytesDecoded, env)", td->type->cTypeRefInfo->decodeRoutineName);
*/
}  /*  PrintCDecoderDefine */

static void
PrintCMatchingRuleDefine PARAMS ((hdr, td),
    FILE *hdr _AND_
    TypeDef *td)
{
    fprintf(hdr, "#define %s%s %s%s", 
	    GetEncRulePrefix(), td->cTypeDefInfo->matchingRuleName, 
	    GetEncRulePrefix(), td->type->cTypeRefInfo->matchingRuleName);
}

static void
PrintCExtractorDefine PARAMS ((hdr, td),
    FILE *hdr _AND_
    TypeDef *td)
{
    fprintf(hdr, "#define %s %s", 
	    td->cTypeDefInfo->compExtractorName, 
	    td->type->cTypeRefInfo->compExtractorName);
}

/*
 * used to figure out local variables to declare
 */
static int
RecCountVariableLevels PARAMS ((t),
    Type *t)
{
    CTRI *ctri;
    int maxLevels = 0;
    NamedType *e;
    int tagCount;
    int typeCount;
    void *tmp;
    enum BasicTypeChoiceId typeId;

    ctri = t->cTypeRefInfo;
    typeId = GetBuiltinType (t);

    /* embedded struct/choices aren't really an issue any more */
    if ((ctri->cTypeId == C_STRUCT) ||
        (ctri->cTypeId == C_CHOICE))
    {
        maxLevels = 1;

        tagCount = CountTags (t);

        tmp = (void*)CURR_LIST_NODE (t->basicType->a.set);
        FOR_EACH_LIST_ELMT (e, t->basicType->a.set)
        {
            if ((e->type == NULL) || (e->type->cTypeRefInfo == NULL))
                continue;

            typeCount = RecCountVariableLevels (e->type);

            if (typeCount > maxLevels)
                maxLevels = typeCount;
        }
        SET_CURR_LIST_NODE (t->basicType->a.set, tmp);
        return maxLevels + tagCount;
    }
    else if (ctri->cTypeId == C_LIST)
    {
        return CountTags (t) +RecCountVariableLevels (t->basicType->a.setOf);
    }
    else if (typeId == BASICTYPE_CHOICE)
        return CountTags (t) +1;
    else if ((typeId == BASICTYPE_ANY) || (typeId == BASICTYPE_ANYDEFINEDBY))
        return CountTags (t) +1;
    else
        return CountTags (t);

}  /* RecCountVariableLevels */



/*
 * returns the number of variable contexts needed for
 * decoding the contents of this type.  Does not consider tags on this type.
 */
static int
CountVariableLevels PARAMS ((t),
    Type *t)
{
    CTRI *ctri;
    int maxLevels = 0;
    NamedType *e;
    int typeCount;
    void *tmp;

    ctri =  t->cTypeRefInfo;

    if ((ctri->cTypeId == C_STRUCT) ||
        (ctri->cTypeId == C_CHOICE))
    {
        maxLevels = 1;
        tmp = (void*)CURR_LIST_NODE (t->basicType->a.set);
        FOR_EACH_LIST_ELMT (e, t->basicType->a.set)
        {
            if ((e->type == NULL) || (e->type->cTypeRefInfo == NULL))
                continue;

            typeCount = RecCountVariableLevels (e->type);

            /* add extra level since must decode key tag in choice */
            if (GetBuiltinType (e->type) == BASICTYPE_CHOICE)
                typeCount++;

            if (typeCount > maxLevels)
                maxLevels = typeCount;
        }
        SET_CURR_LIST_NODE (t->basicType->a.set, tmp);
        return maxLevels;
    }
    else if (ctri->cTypeId == C_LIST)
        return RecCountVariableLevels (t->basicType->a.setOf);
    else if ((ctri->cTypeId == C_ANY) ||
             (ctri->cTypeId == C_ANYDEFINEDBY))
        return 1;
    else
        return 0;
}  /* CountVariableLevels */



/*
 * prints local vars for constructed types (set/seq/choice)
 */
static void
PrintCDecoderLocals PARAMS ((src,td),
    FILE *src _AND_
    TypeDef *td)
{
    int levels;
    int i;

    levels = CountVariableLevels (td->type);

    if ( (GetEncRulesType() == GSER) ) {
        fprintf (src, "\tchar* peek_head,*peek_head2;\n");
        fprintf (src, "\tint i, strLen,rc;\n");
	fprintf (src, "\tComponent%s *k,*t, c_T1;\n",td->cTypeDefInfo->cTypeName);
    }
    else {
	fprintf (src, "    int seqDone = FALSE;\n");
	for (i = 0; i < levels; i++)
	{
		fprintf (src, "\t%s totalElmtsLen%d = 0;\n", lenTypeNameG, i + FIRST_LEVEL);
		fprintf (src, "\t%s elmtLen%d;\n",lenTypeNameG,i +FIRST_LEVEL);
		fprintf (src, "\t%s tagId%d;\n", tagTypeNameG, i +FIRST_LEVEL);
		if (i == 0)
			fprintf (src, "\tint mandatoryElmtCount%d = 0;\n", i + FIRST_LEVEL);
	}
    }

}  /*  PrintCDecoderLocals */

static void
PrintCMatchingRuleLocals PARAMS ((src,td),
    FILE *src _AND_
    TypeDef *td)
{
	fprintf (src, "\tint rc;\n");
	fprintf (src, "\tMatchingRule* mr;\n");
	if ( td->type->basicType->choiceId == BASICTYPE_CHOICE ) {
		fprintf (src, "\tComponent%s *v1, *v2;\n",
				td->type->cTypeRefInfo->cTypeName);
	}
	else if ( td->type->basicType->choiceId == BASICTYPE_SETOF ||
		td->type->basicType->choiceId == BASICTYPE_SEQUENCEOF ) {
		fprintf (src, "\tvoid* component1, *component2;\n");
		fprintf (src, "\tAsnList *v1, *v2, t_list;\n");
	}
} /* PrintMatchingRuleLocals */

static void
PrintCExtractorLocals PARAMS ((src,td),
    FILE *src _AND_
    TypeDef *td)
{
	if( td->type->basicType->choiceId == BASICTYPE_SETOF ||
		td->type->basicType->choiceId == BASICTYPE_SEQUENCEOF ) {
		fprintf (src, "\tint count = 0;\n");
		fprintf (src, "\tint total;\n");
		fprintf (src, "\tAsnList *v = &comp->comp_list;\n");
		fprintf (src, "\tComponent%s *component, *k;\n",
				td->cTypeDefInfo->cTypeName);
	}
} /* PrintExtractorRuleLocals */


/*
 * given the Type *(t) of an elmt in a set/seq/choice/list,
 * prints decoding code.
 *   elmtVarName is string ptr ref to field being decoded
 *     eg "(&personnelRecord.name)"
 *   stoleChoiceTags is as returned by GetTags
 *
 * elmtLevel - last elmtLen# var that is valid/used (has a len)
 * totalLevel - totalElmtsLen# to be used for running total of dec bytes
 * tagIdLevel - last tagId# var that is valid/used (contains a tag)
 */
static void
PrintCElmtDecodeCode PARAMS ((src, td, parent, t, elmtLevel, totalLevel, tagLevel, parentVarName, elmtVarName, elmtVarName2, stoleChoiceTags),
    FILE *src _AND_
    TypeDef *td _AND_
    Type *parent _AND_
    Type *t _AND_
    int elmtLevel _AND_
    int totalLevel _AND_
    int tagLevel _AND_
    char *parentVarName _AND_
    char *elmtVarName _AND_
    char *elmtVarName2 _AND_
    int stoleChoiceTags)
{
    CTRI *ctri;
    Type *tmpType;
    char idVarRef[MAX_VAR_REF];
    char tmpVarName[256];
    NamedType *idNamedType;
    enum BasicTypeChoiceId tmpTypeId;

    ctri =  t->cTypeRefInfo;

    /* check if meant to be encoded */
    if (!ctri->isEncDec)
        return;

    tmpType = GetType (t);

   if(!tmpType->extensionAddition)
   {

    if (tmpType->basicType->choiceId == BASICTYPE_ANY)
    {
        fprintf (src,"/* ANY - Fix Me ! */\n");
        fprintf (src,"\tSetAnyTypeUnknown(%s);\n", elmtVarName);
        //RWC;fprintf (src,"    SetAnyTypeBy\?\?\?(%s, \?\?\?);\n", elmtVarName);
        fprintf (src,"    %s%s (b, %s, &%s%d, env);\n", 
		 GetEncRulePrefix(), "DecAsnAny"/*RWC;NOT VALID FOR C_TYPEREF;ctri->decodeRoutineName*/, 
		 elmtVarName, decodedLenVarNameG, totalLevel);
    }
    else if (tmpType->basicType->choiceId == BASICTYPE_ANYDEFINEDBY)
    {
        /* get type of 'defining' field (int/enum/oid)*/
        idNamedType = t->basicType->a.anyDefinedBy->link;
        tmpTypeId = GetBuiltinType (idNamedType->type);

        if (tmpTypeId == BASICTYPE_OID || tmpTypeId == BASICTYPE_RELATIVE_OID)
        {
            MakeVarPtrRef (genDecCRulesG, td, parent, idNamedType->type, parentVarName, idVarRef);
            fprintf (src, "    SetAnyTypeByOid (%s, %s);\n", elmtVarName, idVarRef);
        }
        else
        {
            /* want to ref int by value not ptr */
            MakeVarValueRef (genDecCRulesG, td, parent, idNamedType->type, parentVarName, idVarRef);
            fprintf (src, "    SetAnyTypeByInt (%s, %s);\n", elmtVarName, idVarRef);
        }
        fprintf (src,"    %s%s (b, %s, &%s%d, env);\n", 
		 GetEncRulePrefix(), ctri->decodeRoutineName, 
		 elmtVarName, decodedLenVarNameG, totalLevel);
    }
    else switch (ctri->cTypeId)
    {
        case C_LIB:
        case C_TYPEREF:
            if( (GetEncRulesType() == GSER) ){
		 /*This is not flexible to change in name. Fix it*/
		 if ( strncmp(ctri->cTypeName,"Asn",3) == 0 )
		 	strcpy (tmpVarName,ctri->cTypeName+3);
		 else
			strcpy (tmpVarName,ctri->cTypeName );
                 fprintf (src,"\t%sDecComponent%s (b, %s, &%s, mode);\n", 
		          GetEncRulePrefix(), tmpVarName, 
		          elmtVarName, "bytesDecoded");
	         break;
            }
            /*
             * choices and octet/bit str types need tagId argument
             */
            if ((tmpType->basicType->choiceId == BASICTYPE_CHOICE) &&
                !stoleChoiceTags)
            {
                /*
                 * strip off top tag of choice in not already done
                 * since choice decoders assume you are passing in
                 * their top tag
                 */
                fprintf (src, "    %s%d = %sDecTag (b, &%s%d, env);\n", 
			 tagIdVarNameG, ++tagLevel, 
			 GetEncRulePrefix(), decodedLenVarNameG, totalLevel);
                fprintf (src, "    %s%d = %sDecLen (b, &%s%d, env);\n", 
			 itemLenVarNameG, ++elmtLevel, 
			 GetEncRulePrefix(), decodedLenVarNameG, totalLevel);
            }
            fprintf (src,"    %s%sContent (b, %s%d, %s%d, %s, &%s%d, env);\n", 
		     GetEncRulePrefix(), ctri->decodeRoutineName, 
		     tagIdVarNameG, tagLevel, itemLenVarNameG, elmtLevel, 
		     elmtVarName, decodedLenVarNameG, totalLevel);

	    /* From ftp://ftp.cs.ubc.ca/pub/local/src/snacc/bugs-in-1.1 */
	    if ((tmpType->basicType->choiceId == BASICTYPE_CHOICE)
		    && !stoleChoiceTags)
	    {
		fprintf(src,"    if (elmtLen%d == INDEFINITE_LEN)\n", elmtLevel-1);
		fprintf(src,"        %sDecEoc(b, &totalElmtsLen%d, env);\n", 
			GetEncRulePrefix(), totalLevel);
	    }

        break;


        /*
         * NOTE: the CHOICE, STRUCT and LIST switch clauses won't
         * fire due to the current 'normalization'
         * (see normalize.c)
         */

        case C_CHOICE:
                /*
                 * strip off top tag of choice in not already done
                 * since choice decoders assume you are passing in
                 * their top tag
                 */
            if (!stoleChoiceTags)
            {
                fprintf (src, "    %s%d = %sDecTag (b, &%s%d, env);\n\n", 
			 tagIdVarNameG, ++tagLevel, 
			 GetEncRulePrefix(), decodedLenVarNameG, totalLevel);

                fprintf (src, "    %s%d = %sDecLen (b, &%s%d, env);\n", 
			 itemLenVarNameG, ++elmtLevel, 
			 GetEncRulePrefix(), decodedLenVarNameG, totalLevel);
            }
            PrintCChoiceDecodeCode (src, td, t, elmtLevel, totalLevel+1, tagLevel, elmtVarName);
        break;


        case C_STRUCT:
            if (t->basicType->choiceId == BASICTYPE_SET){
                PrintCSetDecodeCode (src, td, t, t->basicType->a.set,
				     elmtLevel, totalLevel+1, tagLevel,
				     elmtVarName);
	    }
            else
            {
	        if ( (GetEncRulesType() == GSER) ) {
	            PrintCSeqGSERDecodeCode (src, td, td->type,
			   	            td->type->basicType->a.sequence,
					    valueArgNameG);
	        }
	        else {
                    PrintCSeqDecodeCode (src, td, t, t->basicType->a.sequence,
		  	                 elmtLevel,totalLevel+1, tagLevel,
					 elmtVarName);
                    fprintf (src,"    seqDone = FALSE;\n");
	        }
            }
            fprintf (src,"             %s%d += %s%d;\n", decodedLenVarNameG, totalLevel, decodedLenVarNameG, totalLevel+1);
        break;


        case C_LIST:
	    if ( (GetEncRulesType() == GSER) ) {
		PrintCListGSERDecoderCode ( src, td, t, elmtVarName );
	    }
	    else {
		PrintCListDecoderCode (src, td, t,  elmtLevel,
					totalLevel+1, tagLevel, elmtVarName);
	    }
            fprintf (src,"\n\n");
            fprintf (src,"             %s%d += %s%d;\n", decodedLenVarNameG, totalLevel, decodedLenVarNameG, totalLevel+1);
        break;


        case C_NO_TYPE:
            break;

        default:
            fprintf (stderr,"PrintCElmtDecodeCode: ERROR - unknown c type id\n");
        break;
    }
   }

} /* PrintCElmtDecodeCode */

/*
 * Print C element matching rule code
 * Written by Sang Seok Lim(IBM)
 */
static void
PrintCElmtMatchingRuleCode PARAMS ((src, td, parent, t, parentVarName, elmtVarName, elmtVarName2),
    FILE *src _AND_
    TypeDef *td _AND_
    Type *parent _AND_
    Type *t _AND_
    char *parentVarName _AND_
    char *elmtVarName _AND_
    char *elmtVarName2)
    
{
    CTRI *ctri;
    Type *tmpType;
    char idVarRef[MAX_VAR_REF];
    NamedType *idNamedType;
    enum BasicTypeChoiceId tmpTypeId;

    ctri =  t->cTypeRefInfo;

    tmpType = GetType (t);

   if(!tmpType->extensionAddition)
   {
    /* Need to be considered */
    if (tmpType->basicType->choiceId == BASICTYPE_ANY)
    {
        fprintf (src,"/* ANY - Fix Me ! */\n");
        fprintf (src,"\tSetAnyTypeUnknown(%s);\n", elmtVarName);
        fprintf (src,"    %s%s (b, %s, &%s%d, env);\n", 
		 GetEncRulePrefix(), "DecAsnAny", 
		 elmtVarName, decodedLenVarNameG, 0);
    }
    /* Need to be considered */
    else if (tmpType->basicType->choiceId == BASICTYPE_ANYDEFINEDBY)
    {
        idNamedType = t->basicType->a.anyDefinedBy->link;
        tmpTypeId = GetBuiltinType (idNamedType->type);

        if (tmpTypeId == BASICTYPE_OID || tmpTypeId == BASICTYPE_RELATIVE_OID)
        {
            MakeVarPtrRef (genDecCRulesG, td, parent, idNamedType->type, parentVarName, idVarRef);
            fprintf (src, "    SetAnyTypeByOid (%s, %s);\n", elmtVarName, idVarRef);
        }
        else
        {
            MakeVarValueRef (genDecCRulesG, td, parent, idNamedType->type, parentVarName, idVarRef);
            fprintf (src, "    SetAnyTypeByInt (%s, %s);\n", elmtVarName, idVarRef);
        }
        fprintf (src,"    %s%s (b, %s, &%s%d, env);\n", 
		 GetEncRulePrefix(), ctri->decodeRoutineName, 
		 elmtVarName, decodedLenVarNameG, 0);
    }
    else switch (ctri->cTypeId)
    {
        case C_LIB:
        case C_TYPEREF:
            fprintf (src,"\t%s%s ( oid, %s, %s );\n", 
	          GetEncRulePrefix(), ctri->matchingRuleName, 
	          elmtVarName,elmtVarName2);
        break;

        case C_CHOICE:
            PrintCChoiceMatchingRuleCode (src, td, t, elmtVarName);
        break;

        case C_STRUCT:
            if (t->basicType->choiceId == BASICTYPE_SET){
                PrintCSetMatchingRuleCode (src, td, t, t->basicType->a.set, elmtVarName);
	    }
            else
            {
		PrintCSeqMatchingRuleCode (src, td, td->type,
			   	            td->type->basicType->a.sequence,
					    valueArgNameG);
            }
        break;

        case C_LIST:
		if ( td->type->basicType->choiceId == BASICTYPE_SETOF )
			PrintCListSetOfMatchingRuleCode (src, td, t,valueArgNameG);
		else
			PrintCListSeqOfMatchingRuleCode (src, td, t,valueArgNameG);
        break;

        case C_NO_TYPE:
            break;

        default:
            fprintf (stderr,"PrintCElmtMatchingRuleCode: ERROR - unknown c type id\n");
        break;
    }
   }

} /* PrintCElmtMatchingCode */

/*
 * Print C element component extractor code
 * Written by Sang Seok Lim(IBM)
 */
static void
PrintCElmtExtractorCode PARAMS ((src, td, parent, t, parentVarName, elmtVarName, elmtVarName2),
    FILE *src _AND_
    TypeDef *td _AND_
    Type *parent _AND_
    Type *t _AND_
    char *parentVarName _AND_
    char *elmtVarName _AND_
    char *elmtVarName2)
    
{
    CTRI *ctri;
    Type *tmpType;
    char idVarRef[MAX_VAR_REF];
    NamedType *idNamedType;
    enum BasicTypeChoiceId tmpTypeId;

    ctri =  t->cTypeRefInfo;

    tmpType = GetType (t);

   if(!tmpType->extensionAddition)
   {
    /* Need to be considered */
    if (tmpType->basicType->choiceId == BASICTYPE_ANY)
    {
        fprintf (src,"/* ANY - Fix Me ! */\n");
        fprintf (src,"\tSetAnyTypeUnknown(%s);\n", elmtVarName);
        fprintf (src,"    %s%s (b, %s, &%s%d, env);\n", 
		 GetEncRulePrefix(), "ExtractingAsnAny", 
		 elmtVarName, decodedLenVarNameG, 0);
    }
    /* Need to be considered */
    else if (tmpType->basicType->choiceId == BASICTYPE_ANYDEFINEDBY)
    {
        idNamedType = t->basicType->a.anyDefinedBy->link;
        tmpTypeId = GetBuiltinType (idNamedType->type);

        if (tmpTypeId == BASICTYPE_OID || tmpTypeId == BASICTYPE_RELATIVE_OID)
        {
            MakeVarPtrRef (genDecCRulesG, td, parent, idNamedType->type, parentVarName, idVarRef);
            fprintf (src, "    SetAnyTypeByOid (%s, %s);\n", elmtVarName, idVarRef);
        }
        else
        {
            MakeVarValueRef (genDecCRulesG, td, parent, idNamedType->type, parentVarName, idVarRef);
            fprintf (src, "    SetAnyTypeByInt (%s, %s);\n", elmtVarName, idVarRef);
        }
        fprintf (src,"    %s%s (b, %s, &%s%d, env);\n", 
		 GetEncRulePrefix(), ctri->decodeRoutineName, 
		 elmtVarName, decodedLenVarNameG, 0);
    }
    else switch (ctri->cTypeId)
    {
        case C_LIB:
        case C_TYPEREF:
            fprintf (src,"\t%s ( cr, %s );\n", ctri->compExtractorName, elmtVarName);
        break;

        case C_CHOICE:
            PrintCChoiceExtractorCode (src, td, t, elmtVarName);
        break;

        case C_STRUCT:
            if (t->basicType->choiceId == BASICTYPE_SET){
                PrintCSetExtractorCode (src, td, t, t->basicType->a.set, elmtVarName);
	    }
            else
            {
		PrintCSeqExtractorCode (src, td, td->type,
			   	            td->type->basicType->a.sequence,
					    valueArgNameG);
            }
        break;

        case C_LIST:
	if ( td->type->basicType->choiceId == BASICTYPE_SETOF )
		PrintCListSetOfExtractorCode (src, td, t, elmtVarName);
	else
		PrintCListSeqOfExtractorCode (src,td, t, elmtVarName);
        break;

        case C_NO_TYPE:
            break;

        default:
            fprintf (stderr,"PrintCElmtMatchingRuleCode: ERROR - unknown c type id\n");
        break;
    }
   }

} /* PrintCElmtMatchingCode */



/*
 * Prints code for decoding the elmts of SET
 * YET-to-be-Implemented
 */
static void
PrintCSetDecodeCode PARAMS ((src, td, parent, elmts, elmtLevel, totalLevel, tagLevel, varName),
    FILE *src _AND_
    TypeDef *td _AND_
    Type *parent _AND_
    NamedTypeList *elmts _AND_
    int elmtLevel _AND_
    int totalLevel _AND_
    int tagLevel _AND_
    char *varName)
{
    NamedType *e;
    CTRI *ctri;
    TagList *tags;
    Tag *tag;
    enum BasicTypeChoiceId builtinType;
    char *classStr;
    char *formStr;
    char *codeStr;
    int   mandatoryCount = 0;
    char  tmpVarName[MAX_VAR_REF];
    int   stoleChoiceTags;
    char *routineName;
    int initialTagLevel;
    int initialElmtLevel;


    initialTagLevel = tagLevel;
    initialElmtLevel = elmtLevel;


    routineName = td->cTypeDefInfo->decodeRoutineName;

    if ((elmts == NULL) || LIST_EMPTY (elmts)) /* empty set */
    {
        fprintf (src,"    if (elmtLen%d == INDEFINITE_LEN)\n", elmtLevel);
        fprintf (src,"    {\n");
        fprintf (src,"        %sDecEoc (b, &totalElmtsLen%d, env);\n", 
		 GetEncRulePrefix(), totalLevel);
        fprintf (src,"    }\n");
        fprintf (src,"    else if (elmtLen%d != 0)\n", elmtLevel);
        fprintf (src,"    {\n");
        fprintf (src,"         Asn1Error (\"Expected an empty SET\\n\");\n");
        fprintf (src,"         longjmp (env, %d);\n", (int)(*longJmpValG)--);

        fprintf (src,"    }\n");

/* forget about possible extension types for now
        fprintf (src,"    if (elmtLen%d == INDEFINITE_LEN)\n", elmtLevel);
        fprintf (src,"    {\n");
        fprintf (src,"        tagId%d = %sDecTag (b, &totalElmtsLen%d, env);\n\n", 
	++tagLevel, GetEncRulePrefix(),  totalLevel);

        fprintf (src,"        if (tagId%d == EOC_TAG_ID)\n", tagLevel);
        fprintf (src,"            %sDEC_2ND_EOC_OCTET (b, &totalElmtsLen%d, env)\n", 
	GetEncRulePrefix(), totalLevel);
        fprintf (src,"        else\n");
        fprintf (src,"            BerDiscardElmt (b, &totalElmtsLen%d, env);\n\n",totalLevel);
        fprintf (src,"    }\n");
        fprintf (src,"    else\n");
        fprintf (src,"    {\n");
        fprintf (src,"        BufSkip (b, elmtLen%d);\n", elmtLevel);
        fprintf (src,"        totalElmtsLen%d += elmtLen%d;\n", totalLevel, elmtLevel);
        fprintf (src,"    }\n");
*/
        return;
    }


    fprintf (src, "for ( ; (totalElmtsLen%d < elmtLen%d) || (elmtLen%d == INDEFINITE_LEN);)\n", totalLevel, elmtLevel, elmtLevel);
    fprintf (src, "{\n");
    fprintf (src, "    tagId%d = %sDecTag (b, &totalElmtsLen%d, env);\n\n", 
	     ++tagLevel, GetEncRulePrefix(), totalLevel);
    fprintf (src, "    if ((tagId%d == EOC_TAG_ID) && (elmtLen%d == INDEFINITE_LEN))\n", tagLevel, elmtLevel);
    fprintf (src, "    {\n");
    fprintf (src, "        %sDEC_2ND_EOC_OCTET (b, &totalElmtsLen%d, env)\n", 
	     GetEncRulePrefix(), totalLevel);
    fprintf (src, "        break; /* got EOC so can exit this SET's for loop*/\n");
    fprintf (src, "    }\n");

    fprintf (src, "    elmtLen%d = %sDecLen (b, &totalElmtsLen%d, env);\n", 
	     ++elmtLevel, GetEncRulePrefix(), totalLevel);

    fprintf (src, "    switch (tagId%d)\n", tagLevel);
    fprintf (src, "    {\n");

    FOR_EACH_LIST_ELMT (e, elmts)
    {
      if(!e->type->extensionAddition)
      {

        elmtLevel = initialElmtLevel+1;
        tagLevel = initialTagLevel+1;
        if ((e->type == NULL) || (e->type->cTypeRefInfo == NULL))
        {
            fprintf (src, "< ERROR - no c type information - prob unsuported type>\n");
            continue;
        }

        ctri = e->type->cTypeRefInfo;

        /* check if meant to be encoded */
        if (!ctri->isEncDec)
            continue;

        tags  = GetTags (e->type, &stoleChoiceTags);
        builtinType = GetBuiltinType (e->type);

        if ((tags == NULL) || LIST_EMPTY (tags))
        {
            if ((builtinType != BASICTYPE_ANY) &&
                (builtinType != BASICTYPE_ANYDEFINEDBY))
            {
                if(e->type->extensionAddition)
                {
                    fprintf (src, "<Extensibility not supported in c-library>\n");
                    fprintf (src, "<--Suggest removing extension marker and making all respective extension additions optional>\n");

                }
                else
                {   
                    fprintf (src, "<What? no tag on a SetElmt?>\n");
                }
            }
            else
            {
                fprintf (src,"       /* ANY - Fix Me ! */\n");
                fprintf (src,"       case MAKE_TAG_ID (?,?,?):\n");
            }
        }
        else
        {
            tag = (Tag*)FIRST_LIST_ELMT (tags);
            classStr = Class2ClassStr (tag->tclass);
            codeStr = DetermineCode(tag, NULL, 0);//RWC;Code2UnivCodeStr (tag->code);
            formStr = Form2FormStr (tag->form);

            if (tag->tclass == UNIV)
            {
                if (tag->form == ANY_FORM)
                {
                    fprintf (src,"       case MAKE_TAG_ID (%s, %s, %s):\n", classStr, Form2FormStr (CONS), codeStr);
                    fprintf (src,"       case MAKE_TAG_ID (%s, %s, %s):\n", classStr, Form2FormStr (PRIM), codeStr);
                }
                else
                    fprintf (src,"       case MAKE_TAG_ID (%s, %s, %s):\n", classStr, formStr, codeStr);
            }
            else
            {
                if (tag->form == ANY_FORM)
                {
                    fprintf (src,"       case MAKE_TAG_ID (%s, %s, %s):\n", classStr, Form2FormStr (CONS), DetermineCode(tag, NULL, 1));//RWC;tag->code);

                    fprintf (src,"       case MAKE_TAG_ID (%s, %s, %s):\n", classStr, Form2FormStr (PRIM), DetermineCode(tag, NULL, 1));//RWC;tag->code);
                }
                else
                    fprintf (src,"       case MAKE_TAG_ID (%s, %s, %s):\n", classStr, formStr, DetermineCode(tag, NULL, 1));//RWC;tag->code);
            }

            AsnListFirst (tags);
            AsnListNext (tags); /* set curr to 2nd tag */
            FOR_REST_LIST_ELMT (tag, tags)
            {

                codeStr = DetermineCode(tag, NULL, 0);//RWC;Code2UnivCodeStr (tag->code);
                classStr = Class2ClassStr (tag->tclass);
                formStr = Form2FormStr (tag->form);

                if (stoleChoiceTags)
                {
                    if (tag->tclass == UNIV)
                    {
                        if (tag->form == ANY_FORM)
                        {
                            fprintf (src,"       case MAKE_TAG_ID (%s, %s, %s):\n", classStr, Form2FormStr (CONS), DetermineCode(tag, NULL, 1));//RWC;tag->code);

                            fprintf (src,"       case MAKE_TAG_ID (%s, %s, %s):\n", classStr, Form2FormStr (PRIM), DetermineCode(tag, NULL, 1));//RWC;tag->code);
                        }
                        fprintf (src,"       case MAKE_TAG_ID (%s, %s, %s):\n", classStr, formStr, codeStr);
                    }
                    else
                    {
                        if (tag->form == ANY_FORM)
                        {
                            fprintf (src,"       case MAKE_TAG_ID (%s, %s, %s):\n", classStr, Form2FormStr (CONS), DetermineCode(tag, NULL, 1));//RWC;tag->code);

                            fprintf (src,"       case MAKE_TAG_ID (%s, %s, %s):\n", classStr, Form2FormStr (PRIM), DetermineCode(tag, NULL, 1));//RWC;tag->code);
                        }
                        fprintf (src,"       case MAKE_TAG_ID (%s, %s, %s):\n", classStr, formStr, DetermineCode(tag, NULL, 1));//RWC;tag->code);
                    }
                }
                else
                {
                    tagLevel = initialTagLevel+2;
                    if (tag->form == ANY_FORM)
                    {
                        fprintf (src,"    tagId%d = %sDecTag (b, &totalElmtsLen%d, env);\n", 
				 tagLevel, GetEncRulePrefix(), totalLevel);
                        if (tag->tclass == UNIV)
                        {
                            fprintf (src,"if ((tagId%d != MAKE_TAG_ID (%s, %s, %s)) &&\n", tagLevel, classStr, Form2FormStr (PRIM), codeStr);
                            fprintf (src,"   (tagId%d != MAKE_TAG_ID (%s, %s, %s)))\n", tagLevel, classStr, Form2FormStr (CONS), codeStr);
                        }
                        else
                        {
                            fprintf (src,"if ((tagId%d != MAKE_TAG_ID (%s, %s, %s)) &&\n", tagLevel, classStr, Form2FormStr (PRIM), DetermineCode(tag, NULL, 1));//RWC;tag->code);
                            fprintf (src,"   (tagId%d != MAKE_TAG_ID (%s, %s, %s)))\n", tagLevel, classStr, Form2FormStr (CONS), DetermineCode(tag, NULL, 1));//RWC;tag->code);
                        }

                    }
                    else
                    {
                        if (tag->tclass == UNIV)
                            fprintf (src,"if (%sDecTag (b, &totalElmtsLen%d, env) != MAKE_TAG_ID (%s, %s, %s))\n", GetEncRulePrefix(),  totalLevel, classStr, formStr, codeStr);
                        else
                            fprintf (src,"if (%sDecTag (b, &totalElmtsLen%d, env) != MAKE_TAG_ID (%s, %s, %s))\n", GetEncRulePrefix(),  totalLevel, classStr, formStr, DetermineCode(tag, NULL, 1));//RWC;tag->code);
                    }

                    fprintf (src,"    {\n");
                    fprintf (src,"         Asn1Error (\"Unexpected Tag\\n\");\n");
                    fprintf (src,"         longjmp (env, %d);\n", (int)(*longJmpValG)--);
                    fprintf (src,"    }\n\n");
                    fprintf (src,"elmtLen%d = %sDecLen (b, &totalElmtsLen%d, env);\n", ++elmtLevel, GetEncRulePrefix(),  totalLevel);
                }
            }
          
        }

        MakeVarPtrRef (genDecCRulesG, td, parent, e->type, varName, tmpVarName);

        /*
         * allocate mem for decoding result
         */
        PrintElmtAllocCode (src, e->type, tmpVarName);

        PrintCElmtDecodeCode (src, td, parent, e->type, elmtLevel, totalLevel, tagLevel, varName, tmpVarName, NULL, stoleChoiceTags);

        /*
         * must check for another EOC for ANYs
         * Since the any decode routines
         * decode their own first tag/len pair
         */
        if ((builtinType == BASICTYPE_ANY) ||
            (builtinType == BASICTYPE_ANYDEFINEDBY))
            PrintEocDecoders (src, elmtLevel, initialElmtLevel, itemLenVarNameG, totalLevel, decodedLenVarNameG);
        /*
         * must check for another EOC for tagged CHOICEs
         * since the choice decoder routines do not check
         * for an EOC on the choice's overall length -
         * they are only passed the tag/len of the choice's
         * component.
         */
        else if ((builtinType == BASICTYPE_CHOICE) && !(stoleChoiceTags) &&
                ((tags != NULL) && !LIST_EMPTY (tags)))
            PrintEocDecoders (src, elmtLevel, initialElmtLevel, itemLenVarNameG, totalLevel, decodedLenVarNameG);

        else
            PrintEocDecoders (src, elmtLevel-1, initialElmtLevel, itemLenVarNameG, totalLevel, decodedLenVarNameG);

        if ((!e->type->optional) && (e->type->defaultVal == NULL))
        {
            mandatoryCount++;
            fprintf (src, "    mandatoryElmtCount%d++;\n", totalLevel);
        }

        FreeTags (tags);
      } 
        fprintf (src,"    break;\n\n");
    }  /* end for */

    fprintf (src, "    default:\n");
    fprintf (src, "        Asn1Error (\"%s%sContent: ERROR - Unexpected tag in SET\\n\");\n", GetEncRulePrefix(), routineName);
    fprintf (src, "        longjmp (env, %d);\n", (int)(*longJmpValG)--);
    fprintf (src, "        break;\n");

/*
    fprintf (src, "        Asn1Warning (\"%s%sContent: Warning - unexpected tag in SET, discarding elmt\\n\");\n", GetEncRulePrefix(), routineName);
    fprintf (src, "        BerDiscardElmt (b, &totalElmtsLen%d, env);\n\n", totalLevel);
*/

    fprintf (src, "        } /* end switch */\n");
    fprintf (src, "    } /* end for */\n");

    fprintf (src, "    if (mandatoryElmtCount%d != %d)\n", totalLevel, mandatoryCount);

    fprintf (src, "    {\n");
    fprintf (src, "        Asn1Error (\"%s%sContent: ERROR - non-optional elmt missing from SET\\n\");\n", GetEncRulePrefix(), routineName);
    fprintf (src, "        longjmp (env, %d);\n", (int)(*longJmpValG)--);
    fprintf (src, "    }\n");

}  /*  PrintCSetDecodeCode */


/*
 * Prints code for decoding the elmts of a SEQUENCE in GSER
 * This Routine is orginally developed by sang seok lim
 */
static void
PrintCSeqGSERDecodeCode PARAMS ((src, td, parent, elmts, varName),
    FILE *src _AND_
    TypeDef *td _AND_
    Type *parent _AND_
    NamedTypeList *elmts _AND_
    char *varName)
{
    CTRI *ctri;
    NamedType *e;
    NamedType *last;
    enum BasicTypeChoiceId tmpTypeId;
    char  tmpVarName[MAX_VAR_REF];
    char *routineName;
    int   inTailOptElmts = FALSE;
    int   count=0;

    routineName = td->cTypeDefInfo->decodeRoutineName;

    AsnListFirst (elmts);
    inTailOptElmts = IsTailOptional (elmts);
    e = (NamedType*)FIRST_LIST_ELMT (elmts);
    tmpTypeId = GetBuiltinType (e->type);
    fprintf (src, "\tk = &c_T1;\n");
/*
 * Print codes for reading '{' in GSER encoded data stream,
 */
    fprintf (src,"\t*bytesDecoded = 0;\n");
    fprintf (src,"\tif( !(strLen = LocateNextGSERToken(b,&peek_head,GSER_NO_COPY)) ){\n");
    fprintf (src,"\t\tAsn1Error(\"Error during Reading{ in encoded data\");\n");
    fprintf (src,"\t\treturn LDAP_PROTOCOL_ERROR;\n");
    fprintf (src,"\t}\n");
    fprintf (src,"\tif(*peek_head != \'{\'){\n");
    fprintf (src,"\t\tAsn1Error(\"Missing { in encoded data\");\n");
    fprintf (src,"\t\treturn LDAP_PROTOCOL_ERROR;\n");
    fprintf (src,"\t}\n");

    last = (NamedType*)LAST_LIST_ELMT (elmts);

    FOR_EACH_LIST_ELMT (e, elmts)
    {
	ctri = e->type->cTypeRefInfo;

/*
 * Print code for reading comma separator between Named Value
 */
	if ( count != 0 ) {
	fprintf (src,"\tif( !(strLen = LocateNextGSERToken(b,&peek_head,GSER_NO_COPY)) ){\n");
	fprintf (src,"\t\tAsn1Error(\"Error during Reading { \");\n");
	fprintf (src,"\t\treturn LDAP_PROTOCOL_ERROR;\n");
	fprintf (src,"\t}\n");
	fprintf (src,"\tif(*peek_head != \',\'){\n");
	fprintf (src,"\t\tAsn1Error(\"Missing , in encoding\");\n");
	fprintf (src,"\t\treturn LDAP_PROTOCOL_ERROR;\n");
	fprintf (src,"\t}\n");
	}
	else {
		count =1;
	}

	/*
	 * Print codes for reading identifier of basic types in GSER encodings
	 * identifier of composite types will be read in their decoers
	 */
	if ( !ctri->isPtr )
		MakeVarPtrRef (genDecCRulesG,td, parent, e->type, "k" ,tmpVarName);
	else
		MakeVarPtrRef (genDecCRulesG,td, parent, e->type, "&k" ,tmpVarName);
	fprintf (src,"\tif( !(strLen = LocateNextGSERToken(b,&peek_head,GSER_COPY)) ){\n");
	fprintf (src,"\t  Asn1Error(\"Error during Reading identifier\");\n");
	fprintf (src,"\t\treturn LDAP_PROTOCOL_ERROR;\n");
	fprintf (src,"\t}\n");
	PrintCElmtDecodeCode (src, td, parent, e->type, 0, 0, 0, varName,
				tmpVarName,(char*)NULL, 0);
	if( ctri->isPtr ) tmpVarName[1] = ' ';
	fprintf (src,"\t%s->identifier.bv_val = peek_head;\n",tmpVarName);
	fprintf (src,"\t%s->identifier.bv_len = strLen;\n",tmpVarName);

    } /* End of For */

    /*
     * Print codes for reading '}' in GSER encoded data stream,
     */
    fprintf (src,"\tif( !(strLen = LocateNextGSERToken(b,&peek_head,GSER_NO_COPY)) ) {\n");
    fprintf (src,"\t\tAsn1Error(\"Error during Reading } in encoding\");\n");
    fprintf (src,"\t\treturn LDAP_PROTOCOL_ERROR;\n");
    fprintf (src,"\t}\n");
    fprintf (src,"\tif(*peek_head != \'}\'){\n");
    fprintf (src,"\t\tAsn1Error(\"Missing } in encoding\");\n");
    fprintf (src,"\t\treturn LDAP_PROTOCOL_ERROR;\n");
    fprintf (src,"\t}\n");

    PrintCompDescriptorCode (src, td, td->type);
}

/*
 * Prints code for decoding the elmts of a SET in GSER
 * This Routine is orginally developed by sang seok lim
 */
static void
PrintCSetGSERDecodeCode PARAMS ((src, td, parent, elmts, varName),
    FILE *src _AND_
    TypeDef *td _AND_
    Type *parent _AND_
    NamedTypeList *elmts _AND_
    char *varName)
{
    CTRI *ctri;
    NamedType *e;
    NamedType *last;
    enum BasicTypeChoiceId tmpTypeId;
    char  tmpVarName[MAX_VAR_REF];
    char *routineName;
    int   inTailOptElmts = FALSE;
    int   count=0;

    routineName = td->cTypeDefInfo->decodeRoutineName;

    AsnListFirst (elmts);
    inTailOptElmts = IsTailOptional (elmts);
    e = (NamedType*)FIRST_LIST_ELMT (elmts);
    tmpTypeId = GetBuiltinType (e->type);
    fprintf (src, "\tk = &c_T1;\n");
/*
 * Print codes for reading '{' in GSER encoded data stream,
 */
    fprintf (src,"\t*bytesDecoded = 0;\n");
    fprintf (src,"\tif( !(strLen = LocateNextGSERToken(b,&peek_head,GSER_NO_COPY)) ){\n");
    fprintf (src,"\t\tAsn1Error(\"Error during Reading{ in encoded data\");\n");
    fprintf (src,"\t\treturn LDAP_PROTOCOL_ERROR;\n");
    fprintf (src,"\t}\n");
    fprintf (src,"\tif(*peek_head != \'{\'){\n");
    fprintf (src,"\t\tAsn1Error(\"Missing { in encoded data\");\n");
    fprintf (src,"\t\treturn LDAP_PROTOCOL_ERROR;\n");
    fprintf (src,"\t}\n");

    last = (NamedType*)LAST_LIST_ELMT (elmts);

    FOR_EACH_LIST_ELMT (e, elmts)
    {
	ctri = e->type->cTypeRefInfo;

/*
 * Print code for reading comma separator between Named Value
 */
	if ( count != 0 ) {
	fprintf (src,"\tif( !(strLen = LocateNextGSERToken(b,&peek_head,GSER_NO_COPY)) ){\n");
	fprintf (src,"\t\tAsn1Error(\"Error during Reading { \");\n");
	fprintf (src,"\t\treturn LDAP_PROTOCOL_ERROR;\n");
	fprintf (src,"\t}\n");
	fprintf (src,"\tif(*peek_head != \',\'){\n");
	fprintf (src,"\t\tAsn1Error(\"Missing , in encoding\");\n");
	fprintf (src,"\t\treturn LDAP_PROTOCOL_ERROR;\n");
	fprintf (src,"\t}\n");
	}
	else {
		count =1;
	}

	/*
	 * Print codes for reading identifier of basic types in GSER encodings
	 * identifier of composite types will be read in their decoers
	 */
	if ( !ctri->isPtr )
		MakeVarPtrRef (genDecCRulesG,td, parent, e->type, "k" ,tmpVarName);
	else
		MakeVarPtrRef (genDecCRulesG,td, parent, e->type, "&k" ,tmpVarName);
	fprintf (src,"\tif( !(strLen = LocateNextGSERToken(b,&peek_head,GSER_COPY)) ){\n");
	fprintf (src,"\t  Asn1Error(\"Error during Reading identifier\");\n");
	fprintf (src,"\t\treturn LDAP_PROTOCOL_ERROR;\n");
	fprintf (src,"\t}\n");
	PrintCElmtDecodeCode (src, td, parent, e->type, 0, 0, 0, varName,
				tmpVarName,(char*)NULL, 0);
	if( ctri->isPtr ) tmpVarName[1] = ' ';
	fprintf (src,"\t%s->identifier.bv_val = peek_head;\n",tmpVarName);
	fprintf (src,"\t%s->identifier.bv_len = strLen;\n",tmpVarName);

    } /* End of For */

    /*
     * Print codes for reading '}' in GSER encoded data stream,
     */
    fprintf (src,"\tif( !(strLen = LocateNextGSERToken(b,&peek_head,GSER_NO_COPY)) ) {\n");
    fprintf (src,"\t\tAsn1Error(\"Error during Reading } in encoding\");\n");
    fprintf (src,"\t\treturn LDAP_PROTOCOL_ERROR;\n");
    fprintf (src,"\t}\n");
    fprintf (src,"\tif(*peek_head != \'}\'){\n");
    fprintf (src,"\t\tAsn1Error(\"Missing } in encoding\");\n");
    fprintf (src,"\t\treturn LDAP_PROTOCOL_ERROR;\n");
    fprintf (src,"\t}\n");

    PrintCompDescriptorCode (src, td, td->type);
}

static void
PrintCMatchingCommonHeadCode PARAMS ((src),
FILE *src)
{
	fprintf ( src, "\tif ( oid ) {\n");
	fprintf ( src, "\t\tmr = retrieve_matching_rule( oid, csi->attr->csi_comp_desc->cd_type_id);\n");
	fprintf ( src, "\t\tif ( mr ) ");
	fprintf ( src, "return component_value_match( mr, csi_attr, csi_assert )\n");
	fprintf ( src, "\t}\n\n");
}

/*
 * Prints C code for Matching Rules the elmts of a SEQUENCE
 * This Routine is orginally developed by sang seok lim
 */
static void
PrintCSeqMatchingRuleCode PARAMS ((src, td, parent, elmts, varName),
    FILE *src _AND_
    TypeDef *td _AND_
    Type *parent _AND_
    NamedTypeList *elmts _AND_
    char *varName)
{
	NamedType *e;

	AsnListFirst (elmts);
	e = (NamedType*)FIRST_LIST_ELMT (elmts);

	PrintCMatchingCommonHeadCode ( src );

	fprintf (src, "\trc = 1;\n");
	FOR_EACH_LIST_ELMT (e, elmts)
	{
		char  tmpVarName[MAX_VAR_REF]="(ComponentSyntaxInfo*)";
		char  tmpVarName2[MAX_VAR_REF]="(ComponentSyntaxInfo*)";
		if (!e->type->cTypeRefInfo->isPtr) {
			strcat(tmpVarName,"&");
			strcat(tmpVarName2,"&");
		}
		strcat(tmpVarName,"((Component");
		strcat(tmpVarName,parent->cTypeRefInfo->cTypeName);
		strcat(tmpVarName,")csi_attr)->");
		strcat(tmpVarName,e->type->cTypeRefInfo->cFieldName);

		strcat(tmpVarName2,"((Component");
		strcat(tmpVarName2,varName);
		strcat(tmpVarName2,")csi_assert)->");
		strcat(tmpVarName2,e->type->cTypeRefInfo->cFieldName);

		fprintf (src, "\tif (");
		PrintCElmtMatchingRuleCode (src, td, parent, e->type,
					varName, tmpVarName, tmpVarName2);
		fprintf (src, " == LDAP_COMPARE_FALSE )\n");
		fprintf (src, "\t\treturn LDAP_COMPARE_FALSE;\n");
	}

	fprintf (src, "\treturn LDAP_COMPARE_TRUE;\n");
}

/*
 * Prints C code for component extractor the elmts of a SEQUENCE
 * This Routine is orginally developed by sang seok lim
 */
static void
PrintCSeqExtractorCode PARAMS ((src, td, t, elmts, varName),
    FILE *src _AND_
    TypeDef *td _AND_
    Type *t _AND_
    NamedTypeList *elmts _AND_
    char *varName)
{
	NamedType *e;
	CTRI *ctri;
	void *tmp;

	FOR_EACH_LIST_ELMT (e,  t->basicType->a.choice)
	{
		char  tmpVarName[MAX_VAR_REF] = "comp";
		char  tmpVarName2[MAX_VAR_REF] = "&comp";
		tmp = (void*)CURR_LIST_NODE (t->basicType->a.choice);

		ctri =  e->type->cTypeRefInfo;

		fprintf (src, "\tif ( strncmp(\"%s\", cr->cr_curr->ci_val.ci_identifier.bv_val,cr->cr_curr->ci_val.ci_identifier.bv_len) == 0 ) {\n",e->type->cTypeRefInfo->cFieldName);

		fprintf (src, "\t\tif ( cr->cr_curr->ci_next == NULL )\n");

		if ( ctri->isPtr ) {
			fprintf (src, "\t\t\treturn %s->%s;\n",tmpVarName,
				e->type->cTypeRefInfo->cFieldName);
			fprintf (src, "\t\telse\n");
			strcat(tmpVarName,"->");
			strcat(tmpVarName,e->type->cTypeRefInfo->cFieldName);
			fprintf (src, "\t\t\treturn ");
			PrintCElmtExtractorCode (src, td, t, e->type,
					varName, tmpVarName, NULL );
		}
		else {
			fprintf (src, "\t\treturn %s->%s;\n",tmpVarName2,
				e->type->cTypeRefInfo->cFieldName);
			fprintf (src, "\telse\n");
			fprintf (src, "\t\treturn NULL;\n");
		}
		fprintf (src ,"\t};\n");
		SET_CURR_LIST_NODE (t->basicType->a.choice, tmp);
	}
	fprintf (src, "\treturn NULL;\n");
}

/*
 * Prints C code for Matching Rules the elmts of a SET
 * This Routine is orginally developed by sang seok lim
 */
static void
PrintCSetMatchingRuleCode PARAMS ((src, td, parent, elmts, varName),
    FILE *src _AND_
    TypeDef *td _AND_
    Type *parent _AND_
    NamedTypeList *elmts _AND_
    char *varName)
{
	NamedType *e;
	char  tmpVarName[MAX_VAR_REF]="(ComponentSyntaxInfo*)";
	char  tmpVarName2[MAX_VAR_REF]="(ComponentSyntaxInfo*)";

	AsnListFirst (elmts);
	e = (NamedType*)FIRST_LIST_ELMT (elmts);

	PrintCMatchingCommonHeadCode ( src );

	fprintf (src, "\trc = 1;\n");
	FOR_EACH_LIST_ELMT (e, elmts)
	{
		if (!e->type->cTypeRefInfo->isPtr) {
			strcat(tmpVarName,"&");
			strcat(tmpVarName2,"&");
		}
		strcat(tmpVarName,"((Component");
		strcat(tmpVarName,varName);
		strcat(tmpVarName,")csi_attr)->");
		strcat(tmpVarName,e->type->cTypeRefInfo->cFieldName);

		strcat(tmpVarName2,"((Component");
		strcat(tmpVarName2,varName);
		strcat(tmpVarName2,")csi_assert)->");
		strcat(tmpVarName2,e->type->cTypeRefInfo->cFieldName);

		fprintf (src, "\tif (");
		PrintCElmtMatchingRuleCode (src, td, parent, e->type,
					varName, tmpVarName, tmpVarName2);
		fprintf (src, " == LDAP_COMPARE_FALSE )\n");
		fprintf (src, "\t\treturn LDAP_COMPARE_FALSE;\n");
	}

	fprintf (src, "\treturn LDAP_COMPARE_TRUE;\n");
}

/*
 * Prints C code for component extracting the elmts of a SET
 * This Routine is orginally developed by sang seok lim ( IBM )
 */
static void
PrintCSetExtractorCode PARAMS ((src, td, t, elmts, varName),
    FILE *src _AND_
    TypeDef *td _AND_
    Type *t _AND_
    NamedTypeList *elmts _AND_
    char *varName)
{
    NamedType *e;
    CTRI *ctri;
    char  tmpVarName[MAX_VAR_REF] = "comp";
    char  tmpVarName2[MAX_VAR_REF] = "&comp";
    void *tmp;

    FOR_EACH_LIST_ELMT (e,  t->basicType->a.choice)
    {
        tmp = (void*)CURR_LIST_NODE (t->basicType->a.choice);

        ctri =  e->type->cTypeRefInfo;

	fprintf (src, "\tif ( strncmp(\"%s\", cr->cr_curr->ci_val.ci_identifier.bv_val,cr->cr_curr->ci_val.ci_identifier.bv_len) == 0 ) {",e->type->cTypeRefInfo->cFieldName);

	fprintf (src, "\tif ( cr->cr_curr->ci_next == NULL )\n");

	if ( ctri->isPtr ) {
		fprintf (src, "\t\treturn %s->%s;\n",tmpVarName,
			e->type->cTypeRefInfo->cFieldName);
		fprintf (src, "\telse\n");
		strcat(tmpVarName,e->type->cTypeRefInfo->cFieldName);
		fprintf (src, "\t\treturn ");
		PrintCElmtExtractorCode (src, td, t, e->type,
					varName, tmpVarName, NULL );
	}
	else {
		fprintf (src, "\t\treturn %s->%s;\n",tmpVarName2,
				e->type->cTypeRefInfo->cFieldName);
		fprintf (src, "\telse\n");
		fprintf (src, "\t\treturn NULL;\n");
	}
	fprintf (src ,"\t};\n");

        SET_CURR_LIST_NODE (t->basicType->a.choice, tmp);
    }
    fprintf (src, "\treturn NULL;\n");

}

/*
 * Prints code for decoding the elmts of a SEQUENCE
 */
static void
PrintCSeqDecodeCode PARAMS ((src, td, parent, elmts, elmtLevel, totalLevel, tagLevel, varName),
    FILE *src _AND_
    TypeDef *td _AND_
    Type *parent _AND_
    NamedTypeList *elmts _AND_
    int elmtLevel _AND_
    int totalLevel _AND_
    int tagLevel _AND_
    char *varName)
{
    CTRI *ctri;
    NamedType *e;
    NamedType *tmpElmt;
    NamedType *last;
    TagList *tags;
    Tag *tag;
    enum BasicTypeChoiceId builtinType;
    enum BasicTypeChoiceId tmpTypeId;
    char *classStr;
    char *formStr;
    char *codeStr;
    char  tmpVarName[MAX_VAR_REF];
    int   stoleChoiceTags;
    char *routineName;
    int   inTailOptElmts = FALSE;
    int   initialElmtLevel;
    int   initialTagLevel;


    initialTagLevel = tagLevel;
    initialElmtLevel = elmtLevel;


    routineName = td->cTypeDefInfo->decodeRoutineName;

    if ((elmts == NULL) || LIST_EMPTY (elmts)) /* empty seq */
    {
        fprintf (src,"    if (elmtLen%d == INDEFINITE_LEN)\n", elmtLevel);
        fprintf (src,"    {\n");
        fprintf (src,"        %sDecEoc (b, &totalElmtsLen%d, env);\n", 
		 GetEncRulePrefix(), totalLevel);
        fprintf (src,"    }\n");
        fprintf (src,"    else if (elmtLen%d != 0)\n", elmtLevel);
        fprintf (src,"    {\n");
        fprintf (src,"         Asn1Error (\"Expected an empty SEQUENCE\\n\");\n");
        fprintf (src,"         longjmp (env, %d);\n", (int)(*longJmpValG)--);

        fprintf (src,"    }\n");

/*
        forget about extended types for now
        fprintf (src,"        tagId%d = %sDecTag (b, &totalElmtsLen%d, env);\n\n", tagLevel+1, GetEncRulePrefix(), totalLevel);
        fprintf (src,"    {\n");
        fprintf (src,"        if (tagId%d == EOC_TAG_ID)\n", tagLevel+1);
        fprintf (src,"            %sDEC_2ND_EOC_OCTET (b, &totalElmtsLen%d, env)\n", GetEncRulePrefix(), totalLevel);
        fprintf (src,"        else\n");
        fprintf (src,"            BerDiscardElmt (b, &totalElmtsLen%d, env);\n\n",totalLevel);
        fprintf (src,"    }\n");
        fprintf (src,"    else \n");
        fprintf (src,"    {\n");
        fprintf (src,"        BufSkip (b, elmtLen%d);\n", elmtLevel);
        fprintf (src,"        totalElmtsLen%d += elmtLen%d\n", totalLevel, elmtLevel);
        fprintf (src,"    }\n");
*/
        return;
    }

    /*
     * must set list curr since IsTailOptional checks from curr pt
     * onward
     */
    AsnListFirst (elmts);
    inTailOptElmts = IsTailOptional (elmts);
    e = (NamedType*)FIRST_LIST_ELMT (elmts);
    tmpTypeId = GetBuiltinType (e->type);

    /*
     * print code to decode the first tag
     */
    tagLevel++;
    if (!inTailOptElmts)
    {
        if (((tmpTypeId == BASICTYPE_ANY) ||
             (tmpTypeId == BASICTYPE_ANYDEFINEDBY)) &&
            (CountTags (e->type) == 0))
        {
            if ((e->type->optional) && (e != (NamedType*)LAST_LIST_ELMT (elmts)))
            {
                /* let this cause a compile error in the generated code */
                fprintf (src,"<untagged optional ANY - you must fix this>\n");
            }
        }
        else
            fprintf (src, "    tagId%d = %sDecTag (b, &totalElmtsLen%d, env);\n\n",  tagLevel, GetEncRulePrefix(), totalLevel);
    }
    else
    {
        fprintf (src, "    if ((elmtLen%d != INDEFINITE_LEN) && (totalElmtsLen%d == elmtLen%d))\n", elmtLevel, totalLevel, elmtLevel);
        fprintf (src, "        seqDone = TRUE;\n");
        fprintf (src, "    else\n");
        fprintf (src, "    {\n");

        if (((tmpTypeId == BASICTYPE_ANY) ||
             (tmpTypeId == BASICTYPE_ANYDEFINEDBY)) &&
            (CountTags (e->type) == 0))
        {
            if ((e->type->optional) && (e != (NamedType*)LAST_LIST_ELMT (elmts)))
            {
                /* let this cause a compile error in the generated code */
                fprintf (src,"<untagged optional ANY - you must fix this>\n");
            }
        }
        else
            fprintf (src, "        tagId%d = %sDecTag (b, &totalElmtsLen%d, env);\n\n", tagLevel, GetEncRulePrefix(), totalLevel);
        fprintf (src,"         if ((elmtLen%d == INDEFINITE_LEN) && (tagId%d == EOC_TAG_ID))\n", elmtLevel, tagLevel);
        fprintf (src, "        {\n");
        fprintf (src, "            %sDEC_2ND_EOC_OCTET (b, &totalElmtsLen%d, env)\n", GetEncRulePrefix(), totalLevel);
        fprintf (src, "            seqDone = TRUE;\n");
        fprintf (src, "        }\n");
        fprintf (src, "    }\n\n");
    }

    last = (NamedType*)LAST_LIST_ELMT (elmts);
    FOR_EACH_LIST_ELMT (e, elmts)
    {
        elmtLevel = initialElmtLevel;
        tagLevel = initialTagLevel+1;

        if ((e->type == NULL) || (e->type->cTypeRefInfo == NULL))
        {
            fprintf (src, "< ERROR - no c type information - prob unsuported type>\n");
            continue;
        }

        ctri = e->type->cTypeRefInfo;

        /* check if meant to be encoded */
        if (!ctri->isEncDec)
            continue;

        tags  = GetTags (e->type, &stoleChoiceTags);
        builtinType = GetBuiltinType (e->type);


        if ((tags == NULL) || LIST_EMPTY (tags))
        {
            if ((builtinType != BASICTYPE_ANY) &&
                (builtinType != BASICTYPE_ANYDEFINEDBY))
            {
                if(e->type->extensionAddition)
                {
                    fprintf (src, "<Extensibility not supported in c-library>\n");
                    fprintf (src, "<--Suggest removing extension marker and making all respective extension additions optional>\n");
                }
                else
                {   
                    fprintf (src, "<What? no tag on a SetElmt?>\n");
                }
            }

            if (inTailOptElmts)
            {
                fprintf (src,"    if (!seqDone)");
            }
            /* always enclose elmt decoder in block */
            fprintf (src,"    {\n");

/*
             else
            {
                fprintf (src,"    if (tagId%d == MAKE_TAG_ID (?, ?, ?))\n", tagLevel);
                fprintf (src,"    {\n");
            }
*/
        }
        else  /* has tags */
        {
            tag = (Tag*)FIRST_LIST_ELMT (tags);

            classStr = Class2ClassStr (tag->tclass);
            codeStr = DetermineCode(tag, NULL, 0);//RWC;Code2UnivCodeStr (tag->code);
            formStr = Form2FormStr (tag->form);



            if (inTailOptElmts)
                fprintf (src,"    if ((!seqDone) && (");
            else
                fprintf (src,"    if ((");

            if (tag->tclass == UNIV)
            {
                if (tag->form == ANY_FORM)
                {
                    fprintf (src,"(tagId%d == MAKE_TAG_ID (%s, %s, %s)) ||\n", tagLevel, classStr, Form2FormStr (PRIM), codeStr);
                fprintf (src,"(tagId%d == MAKE_TAG_ID (%s, %s, %s))", tagLevel, classStr, Form2FormStr (CONS), codeStr);
                }
                else
                    fprintf (src,"(tagId%d == MAKE_TAG_ID (%s, %s, %s))", tagLevel, classStr, formStr, codeStr);
            }
            else
            {
                if (tag->form == ANY_FORM)
                {
                    fprintf (src,"(tagId%d == MAKE_TAG_ID (%s, %s, %s)) ||\n", tagLevel, classStr, Form2FormStr (PRIM), DetermineCode(tag, NULL, 1));//RWC;tag->code);
                fprintf (src,"(tagId%d == MAKE_TAG_ID (%s, %s, %s))", tagLevel, classStr, Form2FormStr (CONS), DetermineCode(tag, NULL, 1));//RWC;tag->code);
                }
                else
                    fprintf (src,"(tagId%d == MAKE_TAG_ID (%s, %s, %s))", tagLevel, classStr, formStr, DetermineCode(tag, NULL, 1));//RWC;tag->code);
            }

            if (!stoleChoiceTags)
            {
                fprintf (src,"))\n");
                fprintf (src, "    {\n");
                fprintf (src,"    elmtLen%d = %sDecLen (b, &totalElmtsLen%d, env);\n", ++elmtLevel, GetEncRulePrefix(), totalLevel);
            }

            AsnListFirst (tags);
            AsnListNext (tags);

            FOR_REST_LIST_ELMT (tag, tags)
            {
                classStr = Class2ClassStr (tag->tclass);
                codeStr = DetermineCode(tag, NULL, 0);//RWC;Code2UnivCodeStr (tag->code);
                formStr = Form2FormStr (tag->form);


                if (stoleChoiceTags)
                {
                    fprintf (src," ||\n");
                    if (tag->tclass == UNIV)
                    {
                        if (tag->form == ANY_FORM)
                        {
                            fprintf (src,"     (tagId%d ==MAKE_TAG_ID (%s, %s, %s))", tagLevel, classStr, Form2FormStr (PRIM), codeStr);
                            fprintf (src,"||\n    (tagId%d == MAKE_TAG_ID (%s, %s, %s))", tagLevel, classStr, Form2FormStr (CONS), codeStr);
                        }
                        else
                            fprintf (src,"     (tagId%d ==MAKE_TAG_ID (%s, %s, %s))", tagLevel, classStr, formStr, codeStr);
                    }
                    else
                    {
                        if (tag->form == ANY_FORM)
                        {
                            fprintf (src,"    (tagId%d == MAKE_TAG_ID (%s, %s, %s))", tagLevel, classStr, Form2FormStr (PRIM), DetermineCode(tag, NULL, 1));//RWC;tag->code);
                            fprintf (src,"||\n    (tagId%d == MAKE_TAG_ID (%s, %s, %s))", tagLevel, classStr, Form2FormStr (CONS), DetermineCode(tag, NULL, 1));//RWC;tag->code);
                        }
                        else
                            fprintf (src,"    (tagId%d == MAKE_TAG_ID (%s, %s, %s))", tagLevel, classStr, formStr, DetermineCode(tag, NULL, 1));//RWC;tag->code);
                    }
                }
                else
                {

                    tagLevel = initialTagLevel + 2;
                    fprintf (src, "        tagId%d = %sDecTag (b, &totalElmtsLen%d, env);\n\n", tagLevel, GetEncRulePrefix(), totalLevel);
                    if (tag->tclass == UNIV)
                    {
                        if (tag->form == ANY_FORM)
                        {
                            fprintf (src,"    if ((tagId%d != MAKE_TAG_ID (%s, %s, %s)) &&\n", tagLevel, classStr, Form2FormStr (PRIM), codeStr);
                            fprintf (src,"       (tagId%d != MAKE_TAG_ID (%s, %s, %s)))\n", tagLevel, classStr, Form2FormStr (CONS), codeStr);
                        }
                        else
                            fprintf (src,"    if (tagId%d != MAKE_TAG_ID (%s, %s, %s))\n", tagLevel, classStr, formStr, codeStr);
                    }
                    else
                    {
                        if (tag->form == ANY_FORM)
                        {
                            fprintf (src,"    if ((tagId%d != MAKE_TAG_ID (%s, %s, %s)) &&\n", tagLevel, classStr, Form2FormStr (PRIM), DetermineCode(tag, NULL, 1));//RWC;tag->code);
                            fprintf (src,"        (tagId%d != MAKE_TAG_ID (%s, %s, %s)))\n", tagLevel, classStr, Form2FormStr (CONS), DetermineCode(tag, NULL, 1));//RWC;tag->code);
                        }
                        else
                            fprintf (src,"    if (tagId%d != MAKE_TAG_ID (%s, %s, %s))\n", tagLevel, classStr, formStr, DetermineCode(tag, NULL, 1));//RWC;tag->code);
                    }


                    fprintf (src,"    {\n");
                    fprintf (src,"         Asn1Error (\"Unexpected Tag\\n\");\n");
                    fprintf (src,"         longjmp (env, %d);\n",(int)(*longJmpValG)--);
                    fprintf (src,"    }\n\n");
                    fprintf (src,"    elmtLen%d = %sDecLen (b, &totalElmtsLen%d, env);\n", ++elmtLevel, GetEncRulePrefix(), totalLevel);
                }
            } /* end tag list for */

            if (stoleChoiceTags)
            {
                fprintf (src,"))\n");
                fprintf (src, "    {\n");
                fprintf (src, "        elmtLen%d = %sDecLen (b, &totalElmtsLen%d, env);\n", ++elmtLevel, GetEncRulePrefix(), totalLevel);
            }
        }


        MakeVarPtrRef (genDecCRulesG, td, parent, e->type, varName, tmpVarName);

        /*
         * allocate mem for decoding result
         */
        PrintElmtAllocCode (src, e->type, tmpVarName);

        PrintCElmtDecodeCode (src, td, parent, e->type, elmtLevel, totalLevel, tagLevel, varName, tmpVarName, NULL, stoleChoiceTags);

        /*
         * must check for another EOC for ANYs
         * Since the any decode routines
         * decode their own first tag/len pair
         */
        if ((builtinType == BASICTYPE_ANY) ||
            (builtinType == BASICTYPE_ANYDEFINEDBY))
            PrintEocDecoders (src, elmtLevel, initialElmtLevel, itemLenVarNameG, totalLevel, decodedLenVarNameG);
        /*
         * must check for another EOC for tagged CHOICEs
         * since the choice decoder routines do not check
         * for an EOC on the choice's overall length -
         * they are only passed the tag/len of the choice's
         * component.
         */
        else if ((builtinType == BASICTYPE_CHOICE) && (!stoleChoiceTags) &&
                ((tags != NULL) && !LIST_EMPTY (tags)))
            PrintEocDecoders (src, elmtLevel, initialElmtLevel, itemLenVarNameG, totalLevel, decodedLenVarNameG);

        else
            PrintEocDecoders (src, elmtLevel-1, initialElmtLevel, itemLenVarNameG, totalLevel, decodedLenVarNameG);


        /*  could check cons len vs decode len here */

        if (!inTailOptElmts)
        {
            /*
             * determine whether next elmt in Seq is start
             * of tailing optionals
             */
            AsnListNext (elmts);
            inTailOptElmts = IsTailOptional (elmts);
            AsnListPrev (elmts);
        }

        /*
         * print code for getting the next tag
         */
        tmpTypeId = GetBuiltinType (e->type);

        if  (e != last)
        {
            tmpElmt = (NamedType*)NEXT_LIST_ELMT (elmts);
            tmpTypeId = GetBuiltinType (tmpElmt->type);
            if (!inTailOptElmts)
            {
                if (((tmpTypeId == BASICTYPE_ANY) ||
                     (tmpTypeId == BASICTYPE_ANYDEFINEDBY)) &&
                    (CountTags (tmpElmt->type) == 0))
                {
                    if ((e->type->optional) ||
                        ((tmpElmt->type->optional) && (tmpElmt != last)))
                    {
                        /* let this cause a compile error in the gen'd code */
                        fprintf (src,"  <problems with untagged ANY that is optional or follows an optional sequence element - you must fix this>\n");
                    }
                    /* don't get a tag since ANY's decode their own */
                }
                else
                    fprintf (src, "    tagId%d = %sDecTag (b, &totalElmtsLen%d, env);\n", initialTagLevel+1, GetEncRulePrefix(), totalLevel);
            }
            else
            {
                fprintf (src, "    if ((elmtLen%d != INDEFINITE_LEN) && (totalElmtsLen%d == elmtLen%d))\n", initialElmtLevel, totalLevel, initialElmtLevel);
                fprintf (src, "        seqDone = TRUE;\n");
                fprintf (src, "    else\n");
                fprintf (src, "    {\n");
                if (((tmpTypeId == BASICTYPE_ANY) ||
                     (tmpTypeId == BASICTYPE_ANYDEFINEDBY)) &&
                    (CountTags (tmpElmt->type) == 0))
                {
                    if ((e->type->optional) ||
                        ((tmpElmt->type->optional) && (tmpElmt != last)))
                    {
                        /* let this cause a compile error in the gen'd code */
                        fprintf (src,"  <problems with untagged ANY that is optional or follows an optional sequence element - you must fix this>\n");

                    }

                    /* peek ahead for first octet of eoc */
                    fprintf (src,"         tagId%d = BufPeekByte (b);\n", initialTagLevel+1);
                    fprintf (src,"         if ((elmtLen%d == INDEFINITE_LEN) && (tagId%d == EOC_TAG_ID))\n", initialElmtLevel, initialTagLevel+1);
                    fprintf (src, "        {\n");
                    fprintf (src, "            %sDecEoc (b, &totalElmtsLen%d, env);\n", GetEncRulePrefix(), totalLevel);
                    fprintf (src, "            seqDone = TRUE;\n");
                    fprintf (src, "        }\n");
                }
                else
                {
                    fprintf (src, "        tagId%d = %sDecTag (b, &totalElmtsLen%d, env);\n\n", initialTagLevel+1, GetEncRulePrefix(), totalLevel);
                    fprintf (src,"         if ((elmtLen%d == INDEFINITE_LEN) && (tagId%d == EOC_TAG_ID))\n", initialElmtLevel, initialTagLevel+1);
                    fprintf (src, "        {\n");
                    fprintf (src, "            %sDEC_2ND_EOC_OCTET (b, &totalElmtsLen%d, env)\n", GetEncRulePrefix(), totalLevel);
                    fprintf (src, "            seqDone = TRUE;\n");
                    fprintf (src, "        }\n");
                }
                fprintf (src, "    }\n");
            }
        }
        else /* for last elmt only */
        {
            fprintf (src,"        seqDone = TRUE;\n");
            fprintf (src,"        if (elmtLen%d == INDEFINITE_LEN)\n", initialElmtLevel);
            fprintf (src,"            %sDecEoc (b, &totalElmtsLen%d, env);\n", GetEncRulePrefix(), totalLevel);
            fprintf (src,"        else if (totalElmtsLen%d != elmtLen%d)\n", totalLevel, initialElmtLevel);
            fprintf (src,"            longjmp (env, %d);\n",(int)(*longJmpValG)--);
        }

        /*
         * close (tag check/seqDone test) if block and
         * print else clause to handle missing non-optional elmt
         * errors
         */
        tmpTypeId = GetBuiltinType (e->type);
        if (((tmpTypeId == BASICTYPE_ANYDEFINEDBY) ||
             (tmpTypeId == BASICTYPE_ANY)) &&
            (CountTags (e->type) == 0))
        {
            /* close if stmt block */
            fprintf (src,"    }\n");
        }
        else if (!e->type->optional && (e->type->defaultVal == NULL))
        {

            fprintf (src, "    }\n"); /* end of tag check if */
            fprintf (src, "    else\n");
            fprintf (src, "        longjmp (env, %d);\n", (int)(*longJmpValG)--);
        }
        else
        {
            fprintf (src, "    }\n"); /* end of tag check if */
        }

        fprintf (src,"\n\n");
        FreeTags (tags);
    }


    /*
     * print code to make sure that truly finished with sequence
     */

    fprintf (src,"    if (!seqDone)\n");
    fprintf (src, "        longjmp (env, %d);\n\n", (int)(*longJmpValG)--);

}  /*  PrintCSeqDecodeCode */


/*
 * Generates code for internally defined lists
 * eg:
 * TypeX = SET { foo INTEGER, bar SEQUENCE OF INTEGER } -->
 * BerDecodeTypeX (b, len, v, bytesDecoded, env)
 * {
 *    ...
 *         listLen1 = BerDecodeLen (b, &totalElmtsLen, env);
 *         retVal->bar = NewList();
 *         for ( ; totalElmtsLen1 < listLen1 || listLen1== INDEFINITE_LEN;)
 *         {
 *              tagId1 = BerDecodeTag (b, &totalElmtsLen1, env);
 *               check for EOC
 *              elmtLen1 = BerDecodeLen (b, &totalElmtsLen1, env)
 *              tmpInt = Asn1Alloc (sizeof (int));
 *              BerDecodeInteger (b, elmtLen1, tmpInt, &totalElmtsLen1, env);
 *              AppendList (retVal->bar, tmpInt);
 *         }
 *         totalElmtsLen += totalElmtsLen1;
 *    ...
 * }
 */
static void
PrintCListDecoderCode PARAMS ((src, td, list, elmtLevel, totalLevel, tagLevel, varName),
    FILE *src _AND_
    TypeDef *td _AND_
    Type *list _AND_
    int elmtLevel _AND_
    int totalLevel _AND_
    int tagLevel _AND_
 char *varName)
{
    CTRI *ctri;
    TagList *tags;
    Tag  *tag;
    enum BasicTypeChoiceId builtinType;
    char *classStr;
    char *formStr;
    char *codeStr;
    char tmpVarName[MAX_VAR_REF];
    int  stoleChoiceTags;
    char *routineName;
    int initialTagLevel;
    int initialElmtLevel;
    int taglessAny;

    initialTagLevel = tagLevel;
    initialElmtLevel = elmtLevel;


    routineName = td->cTypeDefInfo->decodeRoutineName;
    ctri = list->basicType->a.setOf->cTypeRefInfo;
    tags  = GetTags (list->basicType->a.setOf, &stoleChoiceTags);
    builtinType = GetBuiltinType (list->basicType->a.setOf);

    taglessAny = (((tags == NULL) || LIST_EMPTY (tags)) &&
                  ((builtinType == BASICTYPE_ANY) ||
                   (builtinType == BASICTYPE_ANYDEFINEDBY)));

    fprintf (src, "    for (totalElmtsLen%d = 0; (totalElmtsLen%d < elmtLen%d) || (elmtLen%d == INDEFINITE_LEN);)\n", totalLevel, totalLevel, elmtLevel, elmtLevel);
    fprintf (src, "    {\n");
    fprintf (src,"        %s **tmpVar;\n", ctri->cTypeName);

    if (taglessAny)
    {
        fprintf (src, "    tagId%d = BufPeekByte (b);\n\n", ++tagLevel);
        fprintf (src, "    if ((tagId%d == EOC_TAG_ID) && (elmtLen%d == INDEFINITE_LEN))\n", tagLevel, elmtLevel);
        fprintf (src, "    {\n");
        fprintf (src, "        %sDecEoc (b, &totalElmtsLen%d, env);\n", 
		 GetEncRulePrefix(), totalLevel);
        fprintf (src, "        break; /* got EOC so can exit this SET OF/SEQ OF's for loop*/\n");
        fprintf (src, "    }\n");
    }
    else
    {
        fprintf (src, "    tagId%d = %sDecTag (b, &totalElmtsLen%d, env);\n\n", ++tagLevel, GetEncRulePrefix(), totalLevel);
        fprintf (src, "    if ((tagId%d == EOC_TAG_ID) && (elmtLen%d == INDEFINITE_LEN))\n", tagLevel, elmtLevel);
        fprintf (src, "    {\n");
        fprintf (src, "        %sDEC_2ND_EOC_OCTET (b, &totalElmtsLen%d, env)\n", GetEncRulePrefix(), totalLevel);
        fprintf (src, "        break; /* got EOC so can exit this SET OF/SEQ OF's for loop*/\n");
        fprintf (src, "    }\n");
    }


    if ((tags == NULL) || LIST_EMPTY (tags))
    {
        if (!taglessAny)
            fprintf (src, "<What? no tag on a SET OF/SEQ OF Elmt?>\n");
/*
        else
        {
            fprintf (src,"    if (tagId%d == MAKE_TAG_ID (?, ?, ?))",tagLevel);
            fprintf (src,"    {\n");
        }
*/

    }
    else if (!stoleChoiceTags) /* choice decoder will check tag */
    {
        tag = (Tag*)FIRST_LIST_ELMT (tags);
        classStr = Class2ClassStr (tag->tclass);
        codeStr = DetermineCode(tag, NULL, 0);//RWC;Code2UnivCodeStr (tag->code);
        formStr = Form2FormStr (tag->form);

        if (tag->tclass == UNIV)
        {
            if (tag->form == ANY_FORM)
            {
                fprintf (src,"    if ((tagId%d == MAKE_TAG_ID (%s, %s, %s)) ||", tagLevel, classStr, Form2FormStr (PRIM), codeStr);

                fprintf (src,"         (tagId%d == MAKE_TAG_ID (%s, %s, %s))", tagLevel, classStr, Form2FormStr (CONS), codeStr);
            }
            else
                fprintf (src,"    if ((tagId%d == MAKE_TAG_ID (%s, %s, %s))", tagLevel, classStr, formStr, codeStr);
        }
        else
        {
            if (tag->form == ANY_FORM)
            {
                fprintf (src,"    if ((tagId%d == MAKE_TAG_ID (%s, %s, %s)) ||\n", tagLevel, classStr, Form2FormStr (PRIM), DetermineCode(tag, NULL, 1));//RWC;tag->code);
                fprintf (src,"       (tagId%d == MAKE_TAG_ID (%s, %s, %s))", tagLevel, classStr, Form2FormStr (CONS), DetermineCode(tag, NULL, 1));//RWC;tag->code);
            }
            else
                fprintf (src,"    if ((tagId%d == MAKE_TAG_ID (%s, %s, %s))", tagLevel, classStr, formStr, DetermineCode(tag, NULL, 1));//RWC;tag->code);
        }

        fprintf (src,")\n");
        fprintf (src, "    {\n");
        fprintf (src, "        elmtLen%d = %sDecLen (b, &totalElmtsLen%d, env);\n", ++elmtLevel, GetEncRulePrefix(), totalLevel);

        AsnListFirst (tags);
        AsnListNext (tags);
        FOR_REST_LIST_ELMT (tag, tags)
        {
            tagLevel = initialTagLevel+2;
            fprintf (src, "        tagId%d = %sDecTag (b, &totalElmtsLen%d, env);\n\n", tagLevel, GetEncRulePrefix(), totalLevel);
            classStr = Class2ClassStr (tag->tclass);
            codeStr = DetermineCode(tag, NULL, 0);//RWC;Code2UnivCodeStr (tag->code);
            formStr = Form2FormStr (tag->form);

            if (tag->tclass == UNIV)
            {
                if (tag->form == ANY_FORM)
                {
                    fprintf (src,"    if ((tagId%d != MAKE_TAG_ID (%s, %s, %s)) ||\n", tagLevel, classStr, Form2FormStr (PRIM), codeStr);
                    fprintf (src,"        (tagId%d != MAKE_TAG_ID (%s, %s, %s)))\n", tagLevel, classStr, Form2FormStr (CONS), codeStr);
                }
                else
                    fprintf (src,"    if (tagId%d != MAKE_TAG_ID (%s, %s, %s))\n", tagLevel, classStr, formStr, codeStr);
            }
            else
            {
                if (tag->form == ANY_FORM)
                {
                    fprintf (src,"    if ((tagId%d != MAKE_TAG_ID (%s, %s, %s)) ||\n", tagLevel, classStr, Form2FormStr (PRIM), DetermineCode(tag, NULL, 1));//RWC;tag->code);
                    fprintf (src,"        (tagId%d != MAKE_TAG_ID (%s, %s, %s)))\n", tagLevel, classStr, Form2FormStr (CONS), DetermineCode(tag, NULL, 1));//RWC;tag->code);

                }
                else
                    fprintf (src,"    if (tagId%d != MAKE_TAG_ID (%s, %s, %s))\n", tagLevel, classStr, formStr, DetermineCode(tag, NULL, 1));//RWC;tag->code);
            }


            fprintf (src,"    {\n");
            fprintf (src,"         Asn1Error (\"Unexpected Tag\\n\");\n");
            fprintf (src,"         longjmp (env, %d);\n", (int)(*longJmpValG)--);
            fprintf (src,"    }\n\n");
            fprintf (src,"    elmtLen%d = %sDecLen (b, &totalElmtsLen%d, env);\n", ++elmtLevel, GetEncRulePrefix(), totalLevel);
        }
    }
    if (stoleChoiceTags)
    {
        fprintf (src, "        elmtLen%d = %sDecLen (b, &totalElmtsLen%d, env);\n", ++elmtLevel, GetEncRulePrefix(), totalLevel);
    }



    strcpy (tmpVarName, "(*tmpVar)");
    fprintf (src,"    tmpVar = (%s**) AsnListAppend (%s);\n", ctri->cTypeName, varName);
    fprintf (src, "    %s = (%s*) Asn1Alloc (sizeof (%s));\n", tmpVarName, ctri->cTypeName, ctri->cTypeName);

    fprintf (src,"    CheckAsn1Alloc (%s, env);\n", tmpVarName);
    PrintCElmtDecodeCode (src, td, list, list->basicType->a.setOf, elmtLevel, totalLevel, tagLevel, varName, tmpVarName, NULL, stoleChoiceTags);

    /*
     * must check for another EOC for ANYs
     * Since the any decode routines
     * decode their own first tag/len pair
     */
    if ((builtinType == BASICTYPE_ANY) ||
        (builtinType == BASICTYPE_ANYDEFINEDBY))
        PrintEocDecoders (src, elmtLevel, initialElmtLevel, itemLenVarNameG, totalLevel, decodedLenVarNameG);
        /*
         * must check for another EOC for tagged CHOICEs
         * since the choice decoder routines do not check
         * for an EOC on the choice's overall length -
         * they are only passed the tag/len of the choice's
         * component.
         */
    else if ((builtinType == BASICTYPE_CHOICE) && (!stoleChoiceTags) &&
             ((tags != NULL) && !LIST_EMPTY (tags)))
        PrintEocDecoders (src, elmtLevel, initialElmtLevel, itemLenVarNameG, totalLevel, decodedLenVarNameG);

    else
        PrintEocDecoders (src, elmtLevel-1, initialElmtLevel, itemLenVarNameG, totalLevel, decodedLenVarNameG);


    if ((!stoleChoiceTags) && (!taglessAny))
    {
        fprintf (src, "    }  /* end of tag check if */\n");
        fprintf (src, "    else  /* wrong tag */\n");
        fprintf (src,"    {\n");
        fprintf (src,"         Asn1Error (\"Unexpected Tag\\n\");\n");
        fprintf (src,"         longjmp (env, %d);\n", (int)(*longJmpValG)--);
        fprintf (src,"    }\n");
    }
    fprintf (src, "    } /* end of for */\n\n");

    FreeTags (tags);

}  /*  PrintCListDecodeCode */

/*
 * GSER C_LIST Decoder Generation Routine Written by Sang Seok Lim (IBM)
 */
static void
PrintCListGSERDecoderCode PARAMS ((src, td, list, varName),
    FILE *src _AND_
    TypeDef *td _AND_
    Type *list _AND_
    char *varName)
{
    CTRI *ctri;
    enum BasicTypeChoiceId builtinType;
    char *routineName;
    CTypeId rhsTypeId; 

    rhsTypeId = td->type->cTypeRefInfo->cTypeId;

    routineName = td->cTypeDefInfo->decodeRoutineName;
    ctri = list->basicType->a.setOf->cTypeRefInfo;
    builtinType = GetBuiltinType (list->basicType->a.setOf);
    
    fprintf (src, "\tAsnLen elmtLen1;\n");
    fprintf (src, "\tAsnListInit(&k->comp_list, sizeof( Component%s ) );\n", td->cTypeDefInfo->cTypeName);
    fprintf (src, "\tbytesDecoded = 0;\n");

    fprintf (src, "\tk = &c_T1;\n");

    fprintf (src, "\tif( !(strLen = LocateNextGSERToken(b, &peek_head, GSER_NO_COPY)) ){\n");
    fprintf (src, "\t\tAsn1Error(\"Error during Reading { in encoding\");\n");
    fprintf (src, "\t\treturn LDAP_PROTOCOL_ERROR;\n");
    fprintf (src, "\t}\n");
    fprintf (src,"\tif(*peek_head != \'{\'){\n");
    fprintf (src,"\t\tAsn1Error(\"Missing { in encoded data\");\n");
    fprintf (src,"\t\treturn LDAP_PROTOCOL_ERROR;\n");
    fprintf (src,"\t}\n");


    fprintf (src, "\tfor (ElmtsLen1 = 0; ElmtsLen1 >= INDEFINITE_LEN; ElmtsLen1++)\n");
    fprintf (src, "\t{\n");

    fprintf (src,"\t\tComponent%s **tmpVar;\n", ctri->cTypeName+3);

    fprintf (src, "\t\tif( !(strLen = LocateNextGSERToken(b, &peek_head, GSER_NO_COPY)) ){\n");
    fprintf (src, "\t\t\tAsn1Error(\"Error during Reading{ in encoding\");\n");
    fprintf (src, "\t\t\treturn LDAP_PROTOCOL_ERROR;\n");
    fprintf (src, "\t\t}\n");
    fprintf (src, "\t\tif(*peek_head == \'}\') break;\n");

    fprintf (src, "\t\tif( !(*peek_head == \'{\' || *peek_head =\',\') ) { ;\n");
    fprintf (src, "\t\t\treturn LDAP_PROTOCOL_ERROR;\n" );
    fprintf (src, "\t\t}\n");

    fprintf (src, "\t\ttmpVar = (Component%s**) AsnListAppend (k->comp_list);\n",ctri->cTypeName+3);
    fprintf (src, "\t\t*tmpVar = (tmpVar+1);\n");
    fprintf (src, "\t\tif ( tmpVar == NULL )\n");
    fprintf (src, "\t\t\tAsn1Error(\"Error during Reading{ in encoding\");\n");
    fprintf (src, "\t\t\treturn LDAP_PROTOCOL_ERROR;\n");
    fprintf (src, "\t\t}");

    PrintCElmtDecodeCode (src, td, list, list->basicType->a.setOf, 0, 0, 0,
				varName, "*tmpVar", (char*)NULL, NULL);

    fprintf (src, "\t} /* end of for */\n\n");

    PrintCompDescriptorCode (src, td, td->type);
}

/*
 * Matching Rule Generation Routine for SEQUENCE OF
 * Written by Sang Seok Lim (IBM)
 */
static void
PrintCListSeqOfMatchingRuleCode PARAMS ((src, td, list, varName),
    FILE *src _AND_
    TypeDef *td _AND_
    Type *list _AND_
    char *varName)
{
    CTRI *ctri;
    char tmpVarName[] = "(ComponentSyntaxInfo*)component1";
    char tmpVarName2[] = "(ComponentSyntaxInfo*)component2";

    ctri = list->basicType->a.sequenceOf->cTypeRefInfo;

    PrintCMatchingCommonHeadCode ( src );
    fprintf (src, "\tv1 = &((Component%s*)csi_attr)->comp_list;\n", td->cTypeDefInfo->cTypeName);
    fprintf (src, "\tv2 = &((Component%s*)csi_assert)->comp_list;\n", td->cTypeDefInfo->cTypeName);


    fprintf (src, "\tFOR_EACH_LIST_PAIR_ELMT");
    fprintf (src, "(component1, component2, v1, v2)\n");
    fprintf (src, "\t{\n");

    /* By print the code instead of calling PrintCElmtMatchingRuleCode */
    /* Is it Right? */
    fprintf (src, "\t\tif( %s%s(%s,%s) == LDAP_COMPARE_FALSE) {\n", 
	     GetEncRulePrefix(), ctri->matchingRuleName, tmpVarName, tmpVarName2);
    fprintf (src, "\t\t\treturn LDAP_COMPARE_FALSE;\n");
    fprintf (src, "\t\t}\n");
    fprintf (src, "\t} /* end of for */\n\n");

    fprintf (src,"\tif( (!component1 && component2) || (component1 && !component2))\n");
    fprintf (src,"\t\treturn LDAP_COMPARE_FALSE;\n");
    fprintf (src,"\telse\n\t\treturn LDAP_COMPARE_TRUE;\n");
}

/*
 * component extractor Generation Routine for SEQUENCE OF
 * Written by Sang Seok Lim(IBM)
 */
static void
PrintCListSeqOfExtractorCode PARAMS ((src, td, list,varName),
    FILE *src _AND_
    TypeDef *td _AND_
    Type *list _AND_
    char *varName)
{
	fprintf ( src, "\tswtich ( cr->cr_curr->ci_type ) {\n");
	fprintf ( src, "\tcase LDAP_COMREF_FROM_BEGINNING :\n");
	fprintf ( src, "\t\tcount = cr->cr_curr->ci_val.ci_from_beginning;\n");
	fprintf ( src, "\t\tFOR_EACH_LIST_ELMT( component , v ) {\n");
	fprintf ( src, "\t\t\tif( --count == 0 ) return component;\n");
	fprintf ( src, "\t\t}\n");
	fprintf ( src, "\t\tbreak;\n");
	fprintf ( src, "\tcase LDAP_COMPREF_FROM_END :\n");
	fprintf ( src, "\t\ttotal = AsnListCount ( v );\n");
	fprintf ( src, "\t\tcount = cr_cr_curr->ci_val.ci_from_end;\n");
	fprintf ( src, "\t\tcount = total + count +1;\n");
	fprintf ( src, "\t\tFOR_EACH_LIST_ELMT ( component, v ) {\n");
	fprintf ( src, "\t\t\tif( --count == 0 ) return component;\n");
	fprintf ( src, "\t\t}\n");
	fprintf ( src, "\t\tbreak;\n");
	fprintf ( src, "\tcase LDAP_COMPREF_ALL :\n");
	fprintf ( src, "\t\treturn comp;\n");
	fprintf ( src, "\tcase LDAP_COMPREF_COUNT :\n");
	PrintCompDescriptorCode (src, td, NULL);
	fprintf ( src, "\t\treturn k;\n");
	fprintf ( src, "\tdefault :\n");
	fprintf ( src, "\t\treturn NULL;\n");
	fprintf ( src, "\t}\n");
}

/*
 * Matching Rule Generation Routine for SET OF
 * Written by Sang Seok Lim (IBM)
 */
static void
PrintCListSetOfMatchingRuleCode PARAMS ((src, td, list, varName),
    FILE *src _AND_
    TypeDef *td _AND_
    Type *list _AND_
    char *varName)
{
    CTRI *ctri;
    char tmpVarName[] = "(ComponentSyntaxInfo*)component1";
    char tmpVarName2[] = "(ComponentSyntaxInfo*)component2";

    ctri = list->basicType->a.setOf->cTypeRefInfo;

    PrintCMatchingCommonHeadCode ( src );

    fprintf (src, "\tv1 = &((Component%s*)csi_attr)->comp_list;\n", td->cTypeDefInfo->cTypeName);
    fprintf (src, "\tv2 = &((Component%s*)csi_assert)->comp_list;\n", td->cTypeDefInfo->cTypeName);

    fprintf (src, "\tAsnListInit( &t_list, 0 );\n");

    fprintf (src, "\tif( AsnListCount( v1 ) != AsnListCount( v2 ) )\n");
    fprintf (src, "\t\treturn LDAP_COMPARE_FALSE;\n");

    fprintf (src, "\tFOR_EACH_LIST_ELMT (component1, v1)\n");
    fprintf (src, "\t{\n");
    fprintf (src, "\t\tFOR_EACH_LIST_ELMT(component2, v2)");
    fprintf (src, "\n\t\t{\n");

    /* By print the code instead of calling PrintCElmtMatchingRuleCode */
    /* Is it Right? */
    fprintf (src, "\t\t\tif( %s%s(oid, %s,%s) == LDAP_COMPARE_TRUE ) {\n", 
	     GetEncRulePrefix(), ctri->matchingRuleName, tmpVarName, tmpVarName2);

    fprintf (src, "\t\t\tAsnElmtMove( v2, &t_list );}\n\t\t\t   break;\n");
    fprintf (src, "\t\t}\n");

    fprintf (src, "\t\t} /* end of inner for */\n");
    fprintf (src, "\t} /* end of outer for */\n\n");

    fprintf (src, "\tif( AsnListCount( v2 ) == 0 )\n");
    fprintf (src, "\t\t rc = LDAP_COMPARE_TRUE;\n");
    fprintf (src, "\telse\n");
    fprintf (src, "\t\t rc = LDAP_COMPARE_FALSE;\n");

    fprintf (src, "\tAsnListMove( &t_list, v2 );\n");
    fprintf (src, "\treturn rc;\n");
}

/*
 * component extractor Generation Routine for SET OF
 * Written by Sang Seok Lim (IBM)
 */
static void
PrintCListSetOfExtractorCode PARAMS ((src, td, list,varName),
    FILE *src _AND_
    TypeDef *td _AND_
    Type *list _AND_
    char *varName)
{
	fprintf ( src, "\tswtich ( cr->cr_curr->ci_type ) {\n");
	fprintf ( src, "\tcase LDAP_COMREF_FROM_BEGINNING :\n");
	fprintf ( src, "\t\tcount = cr->cr_curr->ci_val.ci_from_beginning;\n");
	fprintf ( src, "\t\tFOR_EACH_LIST_ELMT( component , v ) {\n");
	fprintf ( src, "\t\t\tif( --count == 0 ) return component;\n");
	fprintf ( src, "\t\t}\n");
	fprintf ( src, "\t\tbreak;\n");
	fprintf ( src, "\tcase LDAP_COMPREF_FROM_END :\n");
	fprintf ( src, "\t\ttotal = AsnListCount ( v );\n");
	fprintf ( src, "\t\tcount = cr_cr_curr->ci_val.ci_from_end;\n");
	fprintf ( src, "\t\tcount = total + count +1;\n");
	fprintf ( src, "\t\tFOR_EACH_LIST_ELMT ( component, v ) {\n");
	fprintf ( src, "\t\t\tif( --count == 0 ) return component;\n");
	fprintf ( src, "\t\t}\n");
	fprintf ( src, "\t\tbreak;\n");
	fprintf ( src, "\tcase LDAP_COMPREF_ALL :\n");
	fprintf ( src, "\t\treturn comp;\n");
	fprintf ( src, "\tcase LDAP_COMPREF_COUNT :\n");
	PrintCompDescriptorCode (src, td, NULL);
	fprintf ( src, "\t\treturn k;\n");
	fprintf ( src, "\tdefault :\n");
	fprintf ( src, "\t\treturn NULL;\n");
	fprintf ( src, "\t}\n");
}

/*
 * t is the choice type pointer
 */
static void
PrintCChoiceDecodeCode PARAMS ((src, td, t, elmtLevel, totalLevel, tagLevel, varName),
    FILE *src _AND_
    TypeDef *td _AND_
    Type *t _AND_
    int elmtLevel _AND_
    int totalLevel _AND_
    int tagLevel _AND_
    char *varName)
{
    NamedType *e;
    CTRI *ctri;
    TagList *tags;
    Tag *tag;
    enum BasicTypeChoiceId builtinType;
    char *classStr;
    char *formStr;
    char *codeStr;
    char  tmpVarName[MAX_VAR_REF];
    char  choiceIdVarName[MAX_VAR_REF];
    CTRI *parentCtri;
    int   stoleChoiceTags;
    void *tmp;
    int initialTagLevel;
    int initialElmtLevel;

    initialTagLevel = tagLevel;
    initialElmtLevel = elmtLevel;

    parentCtri = t->cTypeRefInfo;


    fprintf (src, "    switch (tagId%d)\n", tagLevel);
    fprintf (src, "    {\n");


    FOR_EACH_LIST_ELMT (e,  t->basicType->a.choice)
    {
        /* hack ! remember curr loc cause called routine hacks it */
        tmp = (void*)CURR_LIST_NODE (t->basicType->a.choice);

        tagLevel = initialTagLevel;
        elmtLevel = initialElmtLevel;

        if ((e->type == NULL) || (e->type->cTypeRefInfo == NULL))
        {
            fprintf (src, "< ERROR - no c type information - prob unsuported type>\n");
            continue;
        }

        if(e->type->extensionAddition)
        {
            fprintf(src, "< ERROR - extensibility unsupported in c-library>\n");
            continue;
        }

        ctri =  e->type->cTypeRefInfo;

        tags  = GetTags (e->type, &stoleChoiceTags);
        builtinType = GetBuiltinType (e->type);

        if ((tags == NULL) || LIST_EMPTY (tags))
        {
            if ((builtinType != BASICTYPE_ANY) &&
                (builtinType != BASICTYPE_ANYDEFINEDBY))
            {
                if(e->type->extensionAddition)
                {
                    fprintf (src, "<Extensibility not supported in c-library>\n");
                    fprintf (src, "<--Suggest removing extension marker and making all respective extension additions optional>\n");
                }
                else
                {   
                    fprintf (src, "<What? no tag on a SetElmt?>\n");
                }
            }
            else
            {
                fprintf (src, "    /* You must hand code ANY type refs */\n");
                fprintf (src,"       case MAKE_TAG_ID (?, ?, ?):\n");
            }

        }
        else
        {
            tag = (Tag*)FIRST_LIST_ELMT (tags);
            classStr = Class2ClassStr (tag->tclass);
            codeStr = DetermineCode(tag, NULL, 0);//RWC;Code2UnivCodeStr (tag->code);
            formStr = Form2FormStr (tag->form);

            if (tag->tclass == UNIV)
            {
                if (tag->form == ANY_FORM)
                {
                    fprintf (src,"       case MAKE_TAG_ID (%s, %s, %s):\n", classStr, Form2FormStr (PRIM), codeStr);
                    fprintf (src,"       case MAKE_TAG_ID (%s, %s, %s):\n", classStr, Form2FormStr (CONS), codeStr);
                }
                else
                    fprintf (src,"       case MAKE_TAG_ID (%s, %s, %s):\n", classStr, formStr, codeStr);
            }
            else
            {
                if (tag->form == ANY_FORM)
                {
                    fprintf (src,"       case MAKE_TAG_ID (%s, %s, %s):\n", classStr, Form2FormStr (PRIM), DetermineCode(tag, NULL, 1));//RWC;tag->code);
                    fprintf (src,"       case MAKE_TAG_ID (%s, %s, %s):\n", classStr, Form2FormStr (CONS), DetermineCode(tag, NULL, 1));//RWC;tag->code);
                }
                else
                    fprintf (src,"       case MAKE_TAG_ID (%s, %s, %s):\n", classStr, formStr, DetermineCode(tag, NULL, 1));//RWC;tag->code);
            }


	    AsnListFirst (tags);
	    AsnListNext (tags); /* set curr ptr to 2nd elmt */
            FOR_REST_LIST_ELMT (tag, tags)
            {
                classStr = Class2ClassStr (tag->tclass);
                codeStr = DetermineCode(tag, NULL, 0);//RWC;Code2UnivCodeStr (tag->code);
                formStr = Form2FormStr (tag->form);


                if (stoleChoiceTags)
                {
                    if (tag->tclass == UNIV)
                    {
                        if (tag->form == ANY_FORM)
                        {
                            fprintf (src,"       case MAKE_TAG_ID (%s, %s, %s):\n", classStr, Form2FormStr (PRIM), codeStr);
                            fprintf (src,"       case MAKE_TAG_ID (%s, %s, %s):\n", classStr, Form2FormStr (CONS), codeStr);
                        }
                        else
                            fprintf (src,"       case MAKE_TAG_ID (%s, %s, %s):\n", classStr, formStr, codeStr);
                    }
                    else
                    {
                        codeStr = DetermineCode(tag, NULL, 1);
                        if (tag->form == ANY_FORM)
                        {
                            fprintf (src,"       case MAKE_TAG_ID (%s, %s, %s):\n", classStr, Form2FormStr (PRIM), codeStr);//RWC;tag->code);
                            fprintf (src,"       case MAKE_TAG_ID (%s, %s, %s):\n", classStr, Form2FormStr (CONS), codeStr);//RWC;tag->code);
                        }
                        else
                            fprintf (src,"       case MAKE_TAG_ID (%s, %s, %s):\n", classStr, formStr, codeStr);//RWC;tag->code);
                    }
                }
                else
                {
                    tagLevel = initialTagLevel +1;
                    if (tag->form == ANY_FORM)
                    {
                        fprintf (src,"    tagId%d = %sDecTag (b, &totalElmtsLen%d, env);\n", tagLevel, GetEncRulePrefix(), totalLevel);
                        if (tag->tclass == UNIV)
                        {
                            fprintf (src,"if ((tagId%d != MAKE_TAG_ID (%s, %s, %s)) &&\n", tagLevel, classStr, Form2FormStr (PRIM), codeStr);
                            fprintf (src,"   (tagId%d != MAKE_TAG_ID (%s, %s, %s)))\n", tagLevel, classStr, Form2FormStr (CONS), codeStr);
                        }
                        else
                        {
                            codeStr = DetermineCode(tag, NULL, 1);
                            fprintf (src,"if ((tagId%d != MAKE_TAG_ID (%s, %s, %s)) &&\n", tagLevel, classStr, Form2FormStr (PRIM), codeStr);//RWC;tag->code);
                            fprintf (src,"   (tagId%d != MAKE_TAG_ID (%s, %s, %s)))\n", tagLevel, classStr, Form2FormStr (CONS), codeStr);//RWC;tag->code);
                        }

                    }
                    else
                    {
                        if (tag->tclass == UNIV)
                            fprintf (src,"if (%sDecTag (b, &totalElmtsLen%d, env) != MAKE_TAG_ID (%s, %s, %s))\n", GetEncRulePrefix(), totalLevel, classStr, formStr, codeStr);
                        else
                        {
                            codeStr = DetermineCode(tag, NULL, 1);
                            fprintf (src,"if (%sDecTag (b, &totalElmtsLen%d, env) != MAKE_TAG_ID (%s, %s, %s))\n", GetEncRulePrefix(), totalLevel, classStr, formStr, codeStr);//RWC;tag->code);
                        }
                    }

                    fprintf (src,"    {\n");
                    fprintf (src,"         Asn1Error (\"Unexpected Tag\\n\");\n");
                    fprintf (src,"         longjmp (env, %d);\n", (int)(*longJmpValG)--);
                    fprintf (src,"    }\n\n");
                    fprintf (src,"    elmtLen%d = %sDecLen (b, &totalElmtsLen%d, env);\n", ++elmtLevel, GetEncRulePrefix(), totalLevel);
                }
            }
        }


        MakeChoiceIdValueRef (genDecCRulesG, td, t, e->type, varName, choiceIdVarName);
        fprintf (src, "        %s = %s;\n", choiceIdVarName, ctri->choiceIdSymbol);

        MakeVarPtrRef (genDecCRulesG, td, t, e->type, varName, tmpVarName);

        PrintElmtAllocCode (src, e->type, tmpVarName);

        PrintCElmtDecodeCode (src, td, t, e->type, elmtLevel, totalLevel, tagLevel, varName, tmpVarName, NULL, stoleChoiceTags);

        /*
         * this is slightly diff from set/seq since
         * no loop checking for eoc (set) and no next elmt (seq)
         * so should check elmtLen0 for EOC if nec
         * (therefore (initialElmtLevel-1) instead of initialElmtLevel)
         *
         * must check for another EOC for ANYs
         * Since the any decode routines
         * decode their own first tag/len pair
         */
        if ((builtinType == BASICTYPE_ANY) ||
            (builtinType == BASICTYPE_ANYDEFINEDBY))
            PrintEocDecoders (src, elmtLevel, initialElmtLevel-1, itemLenVarNameG, totalLevel, decodedLenVarNameG);
        /*
         * must check for another EOC for tagged CHOICEs
         * since the choice decoder routines do not check
         * for an EOC on the choice's overall length -
         * they are only passed the tag/len of the choice's
         * component.
         */
        else if ((builtinType == BASICTYPE_CHOICE) && (!stoleChoiceTags) &&
                ((tags != NULL) && !LIST_EMPTY (tags)))
            PrintEocDecoders (src, elmtLevel, initialElmtLevel-1, itemLenVarNameG, totalLevel, decodedLenVarNameG);

        else
            PrintEocDecoders (src, elmtLevel-1, initialElmtLevel-1, itemLenVarNameG, totalLevel, decodedLenVarNameG);


        FreeTags (tags);

        fprintf (src,"    break;\n\n");

        /* reset curr list node to value remember at beg of loop */
        SET_CURR_LIST_NODE (t->basicType->a.choice, tmp);
    }  /* end for */

    fprintf (src,"    default:\n");
    fprintf (src,"        Asn1Error (\"ERROR - unexpected tag in CHOICE\\n\");\n");
    fprintf (src,"        longjmp (env, %d);\n",(int)(*longJmpValG)--);
    fprintf (src,"        break;\n");

    fprintf (src, "    } /* end switch */\n");

}  /* PrintCChoiceDecodeCode */

void
PrintSyntaxLoader PARAMS (( src, hdr, r, m ,td ),
    FILE *src _AND_
    FILE *hdr _AND_
    CRules *r _AND_
    Module *m _AND_
    TypeDef *td )
{
	CTDI *ctdi;
	CTypeId rhsTypeId;
	ctdi =  td->cTypeDefInfo;
	rhsTypeId = td->type->cTypeRefInfo->cTypeId;

	fprintf ( src, "void*\n");
	fprintf ( src, "asn_attr_to_comp_%s ( struct berval *bv )\n",
		ctdi->cTypeName);
	fprintf ( src, "{\n");
	fprintf ( src, "\tchar* peek_head;\n");
	fprintf ( src, "\tint i, strLen;\n");
	fprintf ( src, "\tGenBuf* b;\n");
	fprintf ( src, "\tExpBuf* buf;\n");
	fprintf ( src, "\tint bytesDecoded;\n");
	fprintf ( src, "\tComponent%s *c_T1;\n",ctdi->cTypeName );
	fprintf ( src, "\tint mode;\n");
	fprintf ( src, "\tExpBufInit ( 1024 );\n");
	fprintf ( src, "\tbuf = ExpBufAllocBufAndData ();\n");
	fprintf ( src, "\tExpBufResetInWriteRvsMode ( buf )\n");
	fprintf ( src, "\tExpBuftoGenBuf( buf, &n );\n");
	fprintf ( src, "\tBufPutSegRvs( b, bv->bv_val, bv->bv_len );\n");
	fprintf ( src, "\tBufResetInReadMode ( b );\n");
	fprintf ( src, "\tmode = 1;\n");
	fprintf ( src, "\tif ( GDec%s ( b, &c_T1, &bytesDecoded, mode ) == LDAP_PROTOCOL_ERROR)\n",ctdi->cTypeName );
	fprintf ( src, "\t\treturn NULL;\n");
	fprintf ( src, "\telse\n");
	fprintf ( src, "\t\tc_T1;\n");
	fprintf ( src, "}\n");
}

/*
 * Print Component Extractor codes
 * Written by Sang Seok Lim (IBM)
 */
void
PrintComponentExtractor PARAMS (( src, hdr, r, m ,td ),
    FILE *src _AND_
    FILE *hdr _AND_
    CRules *r _AND_
    Module *m _AND_
    TypeDef *td )
{
    CTDI *ctdi;
    CTypeId rhsTypeId;

    genDecCRulesG = r;

    ctdi =  td->cTypeDefInfo;
    if ((ctdi == NULL) || (td->type->cTypeRefInfo == NULL))
    {
        fprintf (stderr,"PrintCDecoder: ERROR - no type info\n");
        return;
    }

    rhsTypeId = td->type->cTypeRefInfo->cTypeId;

    switch (rhsTypeId) {
      case C_ANY:
      case C_ANYDEFINEDBY:
	fprintf(hdr, "#define %s %s\n", 
		td->cTypeDefInfo->compExtractorName, 
		td->type->cTypeRefInfo->compExtractorName);
	fprintf (hdr,"\n\n");
	break;
	
      case C_LIB:
      case C_TYPEREF:
		PrintCExtractorDefine ( hdr, td );
	        fprintf (hdr,"\n\n");
	        break;
	
      case C_CHOICE:
	PrintCExtractorPrototype (hdr, td);
	fprintf (hdr,"\n\n");
	PrintCExtractorDeclaration (src, td);
	fprintf (src,"{\n");
	PrintCExtractorLocals (src, td);
	fprintf (src,"\n\n");
	PrintCChoiceExtractorCode (src, td, td->type, valueArgNameG);
	fprintf (src,"}  /* %sContent */",
		 td->cTypeDefInfo->compExtractorName);
	fprintf (src,"\n\n");
	break;
	    
      case C_STRUCT:
	PrintCExtractorPrototype (hdr, td);
	fprintf (hdr,"\n\n");
	PrintCExtractorDeclaration (src, td);
	fprintf (src,"{\n");
	PrintCExtractorLocals (src, td);
	fprintf (src,"\n");
	if ( td->type->basicType->choiceId == BASICTYPE_SET )
		/* need to be reconsidered */
		PrintCSetExtractorCode ( src, td, td->type,
					td->type->basicType->a.set,
					valueArgNameG );
	else
		PrintCSeqExtractorCode ( src, td, td->type,
					td->type->basicType->a.sequence,
					valueArgNameG );
	fprintf (src,"}  /* %s */",
		 td->cTypeDefInfo->compExtractorName);
	fprintf (src,"\n\n");
	break;


      case C_LIST:
	PrintCExtractorPrototype (hdr, td);
	fprintf (hdr,"\n\n");
	PrintCExtractorDeclaration (src, td);
	fprintf (src,"{\n");
	PrintCExtractorLocals (src, td);
	fprintf (src,"\n\n");

	if ( td->type->basicType->choiceId == BASICTYPE_SETOF )
		PrintCListSetOfExtractorCode (src, td, td->type, NULL);
	else
		PrintCListSeqOfExtractorCode (src,td, td->type, NULL);

	fprintf (src,"}  /* %s */", td->cTypeDefInfo->compExtractorName);
	fprintf (src,"\n\n");
	break;

      case C_NO_TYPE:
	return;
	break;

      default:
	fprintf (stderr,"PrintMatchingRule : ERROR - unknown c type id\n");
	return;
	break;
      }

    m = m;   /* AVOIDS warning. */

}

/*
 * Print Matching Rule Functions for ASN.1 Types
 * Written by Sang Seok Lim (IBM)
 */
void
PrintMatchingRule PARAMS ((src, hdr, r, m,  td ),
    FILE *src _AND_
    FILE *hdr _AND_
    CRules *r _AND_
    Module *m _AND_
    TypeDef *td )
{
    CTDI *ctdi;
    CTypeId rhsTypeId;

    genDecCRulesG = r;

    ctdi =  td->cTypeDefInfo;
    if ((ctdi == NULL) || (td->type->cTypeRefInfo == NULL))
    {
        fprintf (stderr,"PrintCDecoder: ERROR - no type info\n");
        return;
    }

    rhsTypeId = td->type->cTypeRefInfo->cTypeId;

    switch (rhsTypeId) {
      case C_ANY:
      case C_ANYDEFINEDBY:
	fprintf(hdr, "#define %s%s %s%s\n", 
		GetEncRulePrefix(), td->cTypeDefInfo->matchingRuleName, 
		GetEncRulePrefix(), 
		td->type->cTypeRefInfo->matchingRuleName);
	
	fprintf (hdr,"\n\n");
	break;
	
      case C_LIB:
      case C_TYPEREF:
		PrintCMatchingRuleDefine ( hdr, td );
	        fprintf (hdr,"\n\n");
	        break;
	
      case C_CHOICE:
	PrintCMatchingRulePrototype (hdr, td);
	fprintf (hdr,"\n\n");
	PrintCMatchingRuleDeclaration (src, td);
	fprintf (src,"{\n");
	PrintCMatchingRuleLocals (src, td);
	fprintf (src,"\n\n");
	/*Check following functio's parameters*/
	PrintCChoiceMatchingRuleCode (src, td, td->type, valueArgNameG);
	fprintf (src,"}  /* %s%sContent */", GetEncRulePrefix(),
		 td->cTypeDefInfo->matchingRuleName);
	fprintf (src,"\n\n");
	break;
	    
      case C_STRUCT:
	PrintCMatchingRulePrototype (hdr, td);
	fprintf (hdr,"\n\n");
	PrintCMatchingRuleDeclaration (src, td);
	fprintf (src,"{\n");
	PrintCMatchingRuleLocals (src, td);
	fprintf (src,"\n");
	if ( td->type->basicType->choiceId == BASICTYPE_SET )
		/* need to be reconsidered */
		/*following function's parameter's td->type be checked*/
		PrintCSetMatchingRuleCode ( src, td, td->type,
					td->type->basicType->a.set,
					valueArgNameG );
	else
		PrintCSeqMatchingRuleCode ( src, td, td->type,
					td->type->basicType->a.sequence,
					valueArgNameG );
	fprintf (src,"}  /* %s%s */", GetEncRulePrefix(),
		 td->cTypeDefInfo->matchingRuleName);
	fprintf (src,"\n\n");
	break;


      case C_LIST:
	PrintCMatchingRulePrototype (hdr, td);
	fprintf (hdr,"\n\n");
	PrintCMatchingRuleDeclaration (src, td);
	fprintf (src,"{\n");
	PrintCMatchingRuleLocals (src, td);
	fprintf (src,"\n\n");

	if ( td->type->basicType->choiceId == BASICTYPE_SETOF )
	PrintCListSetOfMatchingRuleCode (src, td, td->type, valueArgNameG);
	else
	PrintCListSeqOfMatchingRuleCode (src, td, td->type, valueArgNameG);

	fprintf (src,"}  /* %s%sContent */", GetEncRulePrefix(),
		 td->cTypeDefInfo->matchingRuleName);
	fprintf (src,"\n\n");
	break;

      case C_NO_TYPE:
	return;
	break;

      default:
	fprintf (stderr,"PrintMatchingRule : ERROR - unknown c type id\n");
	return;
	break;
      }

    m = m;   /* AVOIDS warning. */
}
