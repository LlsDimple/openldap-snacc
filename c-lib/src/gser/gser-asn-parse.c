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


#include "asn-gser.h"

/*
 * Return the next token and its length, skipping sp and msp
 * Token : identifier, value , {, }, 'H, 'B, ', \, \" :
 *
 * Mode : GSER_COPY, GSER_NO_COPY, GSER_PEEK
 * GSER_COPY mode :
 *	identifier and value are copied into
 *	new memory areas and the pointers are returned.
 *	'\0' is attached to the END OF STRING.
 *	applicable token : identifier and value only
 * GSER_NO_COPY :
 *	only the memroy position in buffer are returned
 *	without copying.
 */
int
LocateNextGSERToken PARAMS (( b, pos, mode),
   GenBuf  *b _AND_
   char** pos _AND_
   LOCATE_MODE mode)
{
   unsigned long i, len;
   char *peek_byte;

   while ( 1 ) {
	len = 1;
	peek_byte = BufPeekSeg(b, &len);
	if ( len && (*peek_byte == ' ') )
		BufGetByte(b);
	else
		break;
   }

   len = 1;
   peek_byte = BufPeekSeg(b,&len);

   if ( peek_byte == NULL ){
	*pos = NULL;
	return 0;
   }

   switch ( *peek_byte ) {
	case '{' :
	case '}' :
	case ',' :
	case ':' :
	   if ( mode != GSER_PEEK )
		   BufGetByte(b);
	   *pos = peek_byte;
	   i = 1;
	   break;
	case '\'' :/* 'H/'B/ ' */
	   if ( peek_byte[1] == 'B'){
		len = INDEFINITE_LEN;
		BufPeekSeg(b, &len);
		/*
		 * 'B is a token only if
		 * There is no more OCTS following B or
		 * B is followed by either space or comma
		 * otherwise ' is a token
		 */
		if ( len == 2 || peek_byte[2] == ' ' || peek_byte[2] == ',' ){
	           if( mode != GSER_PEEK )
			   BufGetSeg( b, &len );
		   i = 2;
		}
		else {
	           if( mode != GSER_PEEK )
			   BufGetByte( b );
		   i = 1;
		}
	   }
	   else if ( peek_byte[1] == 'H' ){
		len = 2;
	        if ( mode != GSER_PEEK )
			BufGetSeg( b, &len );
		i = 2;
	   }
	   else {
	        if ( mode != GSER_PEEK )
			BufGetByte( b );
		i = 1;
	   }
	   *pos = peek_byte;
	   break;
	case '\"' : /* All string is placed between double quote */
	   if ( mode != GSER_PEEK )
		BufGetByte ( b );
	   i = 1;
	   *pos = peek_byte;
	   break;
	default : /* identifier and value are parsed */
	   i = 1;
	   len = INDEFINITE_LEN;
	   BufPeekSeg(b,&len);
	   while ( 1 ) {
		if ( (peek_byte[i] == '}') || (peek_byte[i] == ',') ||
			peek_byte[i] == ':' || (peek_byte[i] == ' ')||
			peek_byte[i] == '\'' || (i >= len) ||
			/* double quote is escaped with double quote */
			( ( i <= (len - 1) ) && (peek_byte[i-1] != '\"')
			&& (peek_byte[i] == '\"' ) ) ){
		   switch ( mode ) {
			case GSER_COPY :
				*pos = Asn1Alloc(i+1);
				BufCopy(*pos, b,(unsigned long)i);
				(*pos)[i] = '\0';
				break;
			case GSER_PEEK :
				*pos = peek_byte;
				break;
			case GSER_NO_COPY :
				*pos = peek_byte;
			   	BufGetSeg(b, &i);
				break;
			default :
				return 0;
		   }
		   break;
		}
		i++;
	   }
   }
   while ( 1 ) {
	len = 1;
	peek_byte = BufPeekSeg(b,&len);
	if( len && (*peek_byte == ' ') )
		BufGetByte(b);
	else
		break;
   }

   return i;
}
