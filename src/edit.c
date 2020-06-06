/***
==========================================================================================
                 _              _         _____    _ _ _
                / \   _ __   __| |_   _  | ____|__| (_) |_
               / _ \ | '_ \ / _` | | | | |  _| / _` | | __|
              / ___ \| | | | (_| | |_| | | |__| (_| | | |_
             /_/   \_\_| |_|\__,_|\__, | |_____\__,_|_|\__|  v0.5-beta
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
#include <string.h>
#include <ctype.h>
#include <curses.h>

#include "ae.h"
#include "edit.h"
#include "pointMarkRegion.h"
#include "navigation.h"
#include "buffer.h"
#include "minibuffer.h"
#include "state.h"

/* EDIT BUFFER */
#define EBSZ 64
static char EDITBUFFER[EBSZ];		     /* Edit Buffer For Text Input */
static int  EBINDEX   =  0;		     /* Edit Buffer Index */

/* Rectangles */
static int recStrtRow = 0;		     /* Rectangle Coords */
static int recStopRow = 0;
static int recStrtCol = 0;
static int recStopCol = 0;


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

void capitalizeWord( void ) {

  int c;
  
  int thisRow = getBufferRow();
  int thisCol = getBufferCol();
  int nextCol = thisCol;
  
  int lineLen = getBufferLineLen( thisRow );

  while( nextCol < lineLen ) {

    if( isalpha( c = (int)getBufferChar( thisRow, nextCol ))) {

      setBufferChar( thisRow, nextCol, (char)toupper( c ));
      forwardWord();
      return;
    }

    nextCol++;
  }

  return;
}


/* Uppercase Word At Point */
void upcaseWord( void ) {

  int c;
  bool alphaFoundP = false;
  
  int thisRow = getBufferRow();
  int thisCol = getBufferCol();
  int nextCol = thisCol;

  int lineLen = getBufferLineLen( thisRow );
  
  /* Find Next Alpha */
  while( nextCol < lineLen ) {

    c = (int)getBufferChar( thisRow, nextCol );

    /* Char NOT Alphabetic */
    if( !isalpha( c )) {

      if( alphaFoundP ) {
	return;
      }
      else {
	pointForward();
	nextCol++;
      }
    }

    /* Char IS Alphabetic */
    else {
      
      setBufferChar( thisRow, nextCol, (char)toupper( c ));
      pointForward();
      alphaFoundP = true;
      
      nextCol++;
    }
  }

  return;
}


/* Uppercase Word At Point */
void downcaseWord( void ) {

  int c;
  bool alphaFoundP = false;
  
  int thisRow = getBufferRow();
  int thisCol = getBufferCol();
  int nextCol = thisCol;

  int lineLen = getBufferLineLen( thisRow );
  
  /* Find Next Alpha */
  while( nextCol < lineLen ) {

    c = (int)getBufferChar( thisRow, nextCol );

    /* Char NOT Alphabetic */
    if( !isalpha( c )) {

      if( alphaFoundP ) {
	return;
      }
      else {
	pointForward();
	nextCol++;
      }
    }

    /* Char IS Alphabetic */
    else {
      
      setBufferChar( thisRow, nextCol, (char)tolower( c ));
      pointForward();
      alphaFoundP = true;
      
      nextCol++;
    }
  }

  return;
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


/* Kill Line at Point */
void yankLine( void ) {

  int i;
  char *kb = getKillBufferPtr();

  for( i=0; i<getKillBufferLength(); i++ ) {

    selfInsert( kb[i] );
  }

  return;
}


/*****************************************************************************************
					RECTANGLES
*****************************************************************************************/

void _swap( int *x, int *y ) {

  int tmp = *x;
  *x = *y;
  *y = tmp;
}

/* Find Row/Col Bounds */
void _setupRectangle( void ) {

  recStrtRow = getBufferRow();
  recStopRow = getMarkY();

  if( recStrtRow > recStopRow )		     /* Work from Top Down */
    _swap( &recStrtRow, &recStopRow );		     

  recStrtCol = getBufferCol();
  recStopCol = getMarkX();

  if( recStrtCol > recStopCol )		     /* Work Left to Right */
    _swap( &recStrtCol, &recStopCol );		     
}

void killRectangle( void ) {

  int row, endCol, lineLen;

  if( !regionActiveP() ) return;
  
  _setupRectangle();
  
  for( row = recStrtRow; row<=recStopRow; row++ ) {

    endCol = recStopCol;
    
    /* Adjust for short lines */
    lineLen = getBufferLineLen( row );
    if( lineLen < recStrtCol ) continue;
    if( lineLen < recStopCol ) endCol = lineLen;

    /* Update Line */
    setPointX( recStrtCol ); setPointY( row );
    setBufferGapPtrs( row, recStrtCol, endCol );
    updateLine();
  }

  /* Update Editor Status */
  clear();
  setStatusFlagModified();
  setRegionActive( false );
  miniBufferMessage( "Region Killed" );

  return;
}


void rectangleInsert( void ) {

  int row, lineLen;
  
  /* Get User Input */
  if( !regionActiveP() ) return;
  if( !miniBufferGetInput( "Text: " )) return;
  strncpy( EDITBUFFER, miniBufferGetUserText(), EBSZ-1 );
  
  _setupRectangle();

  /* Iter Over Rows.... */
  for( row = recStrtRow; row<=recStopRow; row++ ) {

    /* Update Line */
    setPointX( recStrtCol ); setPointY( row );

    /* Adjust for short lines */
    lineLen = getBufferLineLen( row );
    if( lineLen < recStopCol ) {

      setPointX( lineLen-1 ); 
      
      for( int i = 0; i<(recStopCol-lineLen+1); i++ ) {
	
	selfInsert( 'x' );
      }

      /* EDITBUFFER Got Clobbered by selfInsert */
      strncpy( EDITBUFFER, miniBufferGetUserText(), EBSZ-1 );
      updateNavigationState();
      setPointX( recStrtCol );
    }

    /* Setup Delete Area and Text Insertion Area */
    setEditBufferIndex( strlen( EDITBUFFER ));
    setBufferGapPtrs( row, recStrtCol, recStopCol );
    updateLine();
  }

  /* Update Editor Status */
  clear();
  setStatusFlagModified();
  setRegionActive( false );
  miniBufferMessage( "Rectangle Inserted" );

  return;
}


void rectangleNumberLines( void ) {

  _setupRectangle();

  if( recStrtCol != recStopCol ) {
    miniBufferMessage( "Point and Mark Columns Must Match" );
    return;
  }

  int x = miniBufferGetPosInteger( "Starting Number: " );

  for( int row=recStrtRow; row<=recStopRow; row++ ) {

    setPointX( recStrtCol ); setPointY( row );
    
    // Handle Short Lines...
    
    /* Setup Edit Buffer and Update Line */
    snprintf( EDITBUFFER, EBSZ, "%i", x );
    setEditBufferIndex( strlen( EDITBUFFER ));
    setBufferGapPtrs( row, recStrtCol, recStrtCol );
    updateLine();
    x++;
  }

  /* Update Editor Status */
  clear();
  setStatusFlagModified();
  setRegionActive( false );
  miniBufferMessage( "Rectangle Numbers Inserted" );

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
