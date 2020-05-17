/***
==========================================================================================
                 _              _         _____    _ _ _
                / \   _ __   __| |_   _  | ____|__| (_) |_
               / _ \ | '_ \ / _` | | | | |  _| / _` | | __|
              / ___ \| | | | (_| | |_| | | |__| (_| | | |_
             /_/   \_\_| |_|\__,_|\__, | |_____\__,_|_|\__|  v0.4-beta [Draft]
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
#include <stdbool.h>

#include "ae.h"
#include "edit.h"
#include "pointMarkRegion.h"
#include "navigation.h"
#include "buffer.h"
#include "minibuffer.h"
#include "state.h"

/* EDIT BUFFER */
static char EDITBUFFER[64];		/* Edit Buffer For Text Input */
static int  EBINDEX   =  0;		/* Edit Buffer Index */


/*****************************************************************************************
				    EDIT BUFFER STATUS
*****************************************************************************************/

int getEditBufferIndex( void ) {

  return EBINDEX;
}

char getEditBufferChar(int i) {

  return EDITBUFFER[i];
}

void setEditBufferIndex( int x ) {

  EBINDEX = x;
}

/*****************************************************************************************
					  SPACES
*****************************************************************************************/

/* Indent Newline to Smart Location */
void autoIndent( void ) {

  int i = 0;				/* Col Index */

  /* Get This and Prior Row Indices */
  int thisRow  = getRowOffset() + getPointY();
  int priorRow = thisRow - 1;
  
  /* Skip if Middle of Line or Top of Buffer */
  if(( getPointX() != 0 ) || ( thisRow == 0 ))
    return;

  /* Get Length of Prior Line */
  int priorRowLen = getBufferLineLen( priorRow );
  
  /* Find Tab Col Prior Row */
  while( getBufferChar( priorRow, i ) == ' ' )
    i++;

  /* Skip Blank Lines */
  if( i == priorRowLen - 1 )
    return;
  
  /* Insert Spaces Until Tab Mark */
  while(( i < priorRowLen ) && ( i > 0 )) {

    selfInsert( ' ' );
    i--;
  }

  if( getPointX() > 0 )
    miniBufferMessage( "Auto Indent" );

  return;
}

/*****************************************************************************************
					CHARACTERS
*****************************************************************************************/

/* Insert User Typed Chars */
void selfInsert( int c ) {

  int PtX     = getPointX();
  int thisRow = getRowOffset() + getPointY();
  
  /* Save Prior Unsaved Changes to Text Row */ 
  if( bufferRowEditedP( thisRow ))
    updateNavigationState();

  /* Text Insertion Now Begins at POINT_X */  
  if( !bufferRowEditedP( thisRow )) {
    setBufferGapPtrs( thisRow, PtX, PtX );
    updateEditState();
  }

  EDITBUFFER[EBINDEX++] = c;
  setPointX( ++PtX );
}


/* Delete Forward One Char */
void deleteChar( void ) {

  int thisRow = getBufferRow();
  
  /* Continue Editing This Line? */
  if( bufferRowEditedP( thisRow )) {

    /* If Deleting Chars After Inserting Chars? */
    /* Then, Save Insertions Befor Making Deletions */ 
      if( getBufferGapSize( thisRow ) == 0 ) {
	updateLine();
	setBufferGapPtrs( thisRow, getPointX(), getPointX() );
      }

      /* Continue Deleting Chars Up to EOL  */
      increaseBufferGap( thisRow );
  }

  /* *New* Edit To This Line? */
  else {
    setBufferGapPtrs( thisRow, getBufferCol(), getBufferCol() );
    increaseBufferGap( thisRow );
    updateEditState();
  }
}


/* Delete Back One Char */
void backspace( void ) {

  int PtX     = getPointX();
  int thisRow = getRowOffset() + getPointY();
  
  /* Continue Editing This Line? */
  if( bufferRowEditedP( thisRow )) {

    /* If Deleting Chars After Inserting Chars? */
    /* Then, Save Insertions Befor Making Deletions */ 
    if( getBufferGapSize( thisRow ) == 0 ) {
      updateLine();
      setBufferGapPtrs( thisRow, PtX, PtX );
    }

    /* Continue Deleting Chars to BOL */
    if( PtX > 0 ) {
      setBufferGapPtrs( thisRow, PtX-1,
			getBufferGapRightIndex(thisRow) );
      setPointX( --PtX );
    }

    /* At BOL, Combine With Prior Line */
    else {
      updateNavigationState();
      combineLineWithPrior();
    }
  }

  /* Begin *NEW* Edits to This Line */
  else {

    if( PtX > 0 ) {
      setBufferGapPtrs( thisRow, PtX-1, PtX );
      updateEditState();
      setPointX( --PtX );
    }

    else {
      combineLineWithPrior();
    }
  }
}

/*****************************************************************************************
					  WORDS
*****************************************************************************************/

/* Kill the Next Space-Delimited Word or Expression */
void killWord( void ) {

  int currPointX = getPointX();		/* Save Current Point */
  int thisRow    = getRowOffset() + getPointY();
  
  forwardWord();			/* Find End Next Word */

  if( getPointX() == currPointX )	/* No Word to Kill */
    return;

  /* Mark Word for Deletion and Restore Point */
  setBufferGapPtrs( thisRow, currPointX, getPointX() );
  setPointX( currPointX );
  updateEditState();
  updateNavigationState();
}

/*****************************************************************************************
					  LINES
*****************************************************************************************/

/* Kill Line at Point */
void killLine( void ) {

  int thisRow = getRowOffset() + getPointY();
  int thisCol = getColOffset() + getPointX();
  
  /* Line Empty - Delete it */
  if( getBufferLineLen( thisRow ) == 1 ) {

    if( thisRow == getBufferNumRows() - 1 ) return; /* Cant Delete if Nothing Follows */

    freeBufferLine( thisRow );
  }

  /* Delete from POINT to EOL */
  else
    freeBufferPointToEOL( thisRow, thisCol );

  return;
}



/***
    Local Variables:
    mode: c
    tags-file-name: "~/ae/TAGS"
    comment-column: 45
    fill-column: 90
    End:
 ***/
