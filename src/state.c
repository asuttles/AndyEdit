/***
==========================================================================================
                      _              _         _____    _ _ _
                     / \   _ __   __| |_   _  | ____|__| (_) |_
                    / _ \ | '_ \ / _` | | | | |  _| / _` | | __|
                   / ___ \| | | | (_| | |_| | | |__| (_| | | |_
                  /_/   \_\_| |_|\__,_|\__, | |_____\__,_|_|\__|  v0.4-beta
                                       |___/

        Copyright 2020 (andrew.suttles@gmail.com)
        MIT LICENSE

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
 INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE 
 LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT 
 OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 DEALINGS IN THE SOFTWARE.

 AndyEDIT is a simple, line-oriented, terminal-based text editor with emacs-like keybindings.

 For more information about AndyEdit, see README.md.

==========================================================================================
 ***/
#include <stdlib.h>
#include <stdbool.h>

#include "ae.h"
#include "pointMarkRegion.h"
#include "buffer.h"
#include "minibuffer.h"

/* Useful Macros */
#define thisRow() (getRowOffset() + getPointY())
#define thisCol() (getColOffset() + getPointX())

/* Global Data */
static int ROWOFFSET  =  0;		     /* Buffer Index of Top Row */
static int COLOFFSET  =  0;		     /* Buffer Index of First Col */

/* Buffer Status Flag */
enum _sf { ORIGINAL, MODIFIED, READONLY };
static const char _sfname[3][9] = { "ORIGINAL", "MODIFIED", "READONLY" };

/* Is Buffer Modified? */
static enum _sf STATUSFLAG = ORIGINAL;		

/*****************************************************************************************
				  GET/SET BUFFER STATUS
*****************************************************************************************/

void setStatusFlagOriginal( void ) {

  STATUSFLAG = ORIGINAL;
}

void setStatusFlagModified( void ) {
  STATUSFLAG = MODIFIED;
}

bool statusFlagModifiedP( void ) {

  return ( STATUSFLAG == MODIFIED );
}

char *getStatusFlagName( void ) {

  return (char *)_sfname[ STATUSFLAG ];
}

/*****************************************************************************************
				 GET/SET ROW/COL OFFSETS
*****************************************************************************************/

/* Get/Set Row/Col Offset for Nav Functions */
int getRowOffset( void ) {
  return ROWOFFSET;
}
int getColOffset( void ) {
  return COLOFFSET;
}
void setRowOffset( int ro ) {
  ROWOFFSET = ro;
}
void setColOffset( int co ) {
  COLOFFSET = co;
}

/*****************************************************************************************
				    UPDATE LINE EDITS
*****************************************************************************************/

/* Incorporate Edits Into Row Structure */
void updateLine( void ) {

  char *tmp;                        /* New Text Row */

  int i   = 0;
  int thisCol = 0;
  int thisRow = thisRow();
  
  int ebIndex    = getEditBufferIndex();
  int oldLineLen = getBufferLineLen( thisRow );

  /* Calculate Size of New Line */
  int delta = ebIndex - getBufferGapSize( thisRow );

  int newLineLen = oldLineLen + delta; 
    
  /* Allocate Heap Space for New Line */
  if(( tmp = malloc(( sizeof( char ) * newLineLen ) + 1 )) == NULL )
    die( "updateLine: tmp malloc failed" );

  /* Copy Non-deleted Chars */
  for( i = 0; i<getBufferGapLeftIndex( thisRow ); i++ ) {
    tmp[thisCol] = getBufferChar( thisRow, i );
    thisCol++;
  }

  /* Add Chars from Edit Buffer */
  for( i = 0; i<ebIndex; i++ ) {
    tmp[thisCol] = getEditBufferChar(i);
    thisCol++;
  }

  /* Add Rest of Chars */
  for( i = getBufferGapRightIndex( thisRow ); i<oldLineLen; i++ ) {
    tmp[thisCol] = getBufferChar( thisRow, i );
    thisCol++;
  }

  tmp[newLineLen] = '\0';		     /* NULL Terminate New String */

  /* Update Text in Buffer Line */
  replaceBufferLineText( thisRow, newLineLen, tmp );

  /* Clear Edit Buffer */
  setEditBufferIndex( 0 );
}


/* Update Edit Line State */
void updateEditState( void ) {

  setBufferRowEdited( thisRow(), true );
  setStatusFlagModified();
}


/*****************************************************************************************
			    UPDATE LINE EDITS AFTER NAVIGATION
*****************************************************************************************/

/* Cursor Movement Functions */
void updateNavigationState( void ) {

  if( bufferRowEditedP( thisRow() ))
    updateLine();

  setBufferRowEdited( thisRow(), false );
  
  miniBufferClear();
}


/***
    Local Variables:
    mode: c
    tags-file-name: "~/ae/TAGS"
    comment-column: 45
    fill-column: 90
    End:
 ***/
