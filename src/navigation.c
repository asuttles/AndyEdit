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
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <curses.h>

#include "ae.h"
#include "state.h"
#include "buffer.h"
#include "window.h"
#include "pointMarkRegion.h"
#include "minibuffer.h"
#include "keyPress.h"

#define screenRows() (getWinNumRows() - 3)
#define thisRow() (getRowOffset() + getPointY())
#define thisCol() (getColOffset() + getPointX())

#define _SRCH_STR_LEN 32

/* Private Functions */
static void _goto( int );

/* Search String */
static char _SRCH_STR[_SRCH_STR_LEN];
static bool SEARCHINGP = false;

/***
==========================================================================================
				     POINT NAVIGATION
==========================================================================================
***/

/* Move Point Forward */
void pointForward( void ) {

  int PtX     = getPointX();
  
  if( thisCol() < getBufferLineLen( thisRow() ) - 1 ) {
    if( PtX < getWinNumCols() - 1 )
      setPointX( ++PtX );
    else
      setColOffset( getColOffset() + 1 );
  }
}

/* Move Point Backward */
void pointBackward( void ) {

  int PtX = getPointX();
  
  if( thisCol() > 0 ) {
    if( PtX == 0 )
      setColOffset( getColOffset() - 1 );
    else
      setPointX( --PtX );
  }
}

/* Move Point to End of Line */
void pointToEndLine() {

  int thisRow = thisRow();
  int x = getBufferLineLen( thisRow ) - 1;   /* Text Line Length */
  int y = getWinNumCols() - 1;                     /* Terminal Length */

  if( x > y ) {
    setPointX( y );
    setColOffset( x - y );
  }
  else {
    setPointX( x );
    setColOffset( 0 );
  }
}


/***
==========================================================================================
				     WORD NAVIGATION
==========================================================================================
***/

/* Forward Word */
void forwardWord( void ) {

  char c;
  
  if( thisCol() == getBufferLineLen( thisRow() ) - 1 ) /* At EOL? */
    return;

  pointForward();

 /* Move Past Spaces */
  while(( c = getBufferChar( thisRow(), thisCol() )) == ' ' ) 
    pointForward();

  /* Move to End of Word */
  while( c != '\n' &&
         c != ' '  &&
         c != ')'  &&
         c != ']' ) {

    pointForward();
    c = getBufferChar( thisRow(), thisCol() );
  }

  return;
}


/* Backward Word */
void backwardWord( void ) {

  char c;
  
  if( thisCol() == 0 ) return;        /* At BOL? */

  int old_POINT_X = getPointX();
  pointBackward();
  
 /* Move Past Spaces */
  c = getBufferChar( thisRow(), thisCol() );
  while(( c == ' '   ||
          c == ')'   ||
          c == ';'   ||
          c == ']' ) &&
        thisCol() > 0 ) {
    
    pointBackward();
    c = getBufferChar( thisRow(), thisCol() );
  }

  /* If POINT_X is a Space, No Prior Word this Line */
  if( c == ' ' ) {
    setPointX( old_POINT_X );
    return;
  }
  
 /* Move to Beginning of Word */
  c = getBufferChar( thisRow(), thisCol() );
  while( thisCol() > 0                            &&
         c != ' '  &&
         c != '('  &&
         c != '[' ) {
    pointBackward();
    c = getBufferChar( thisRow(), thisCol() );
  }

 /* Don't Leave POINT on a Space */
  if( c == ' ' )
    pointForward();
}

/* Search FORWARD for a Word */
void wordSearchForward( void ) {

  char *txt, *match;
  
  int row     = getBufferRow();
  int nRow    = getBufferNumRows();
  bool matchP = false;

  /* If not actively searching, get search string */
  if( !SEARCHINGP ) {
    if( miniBufferGetSearchString( _SRCH_STR, _SRCH_STR_LEN ) == false ) {
      miniBufferClear();
      return;
    }
  }

  /* Search for search string in buffer lines */
  for( ; row < nRow; row++ ) {

    int pad = 0;			     /* Padding to Skip Prior  Matches */
    
    /* Search From Current Row Forward */
    txt = getBufferTextLine( row );
    if( row == getBufferRow() ) {	     /* Skip Past Current POINT */
      pad = getPointX() + getColOffset() + 1;
      txt += pad;
    }

    match = strstr( txt, _SRCH_STR );

    /* Match Found */
    if( match ) {			     

      _goto( row+1 );			     /* Set Row */
      setPointX( 0 );			     /* Set/Adjust Column */

      int col = (int)(match - txt) / (int)sizeof( char ) - 1;
      col    += pad;
      
      for( int i = 0; i <= col; i++ )
	pointForward();

      SEARCHINGP = true;
      miniBufferMessage( "Found Match!" );
      matchP = true;
      break;
    }
  }

  if( !matchP ) miniBufferMessage( "No Match Found" );
  
  
  return;
}

/* Find Last Match in Text String */
char *_lastMatch( char *str ) {

  char *lastMatch = NULL;

  while(( str = strstr( str, _SRCH_STR )) != NULL ) {
    lastMatch = str;
    str++;
  }

  return lastMatch;
}

/* Search BACKWARD for a Word */
void wordSearchBackward( void ) {

  char *txt, *match;
  
  int row     = getBufferRow();
  bool matchP = false;

  char *tmp   = NULL;

  /* If not actively searching, get search string */
  if( !SEARCHINGP ) {
    if( miniBufferGetSearchString( _SRCH_STR, _SRCH_STR_LEN ) == false ) {
      miniBufferClear();
      return;
    }
  }

  /* Search for search string in buffer lines */
  for( ; row >= 0; row-- ) {

    /* Search From Current Row Forward */
    txt = getBufferTextLine( row );
    int eol = 0;			     /* Only EOL Cutoff for Active Row  */

    if( row == getBufferRow() ) {	     /* Skip Past Current POINT */

      eol = getBufferCol();
      if(( tmp = malloc( sizeof( char ) * eol )) == NULL ) {
	miniBufferMessage( "Search Failed: Memory Error!" );
	return;
      }

      /* Create a tmp string that does not include last half of line */
      strncpy( tmp, txt, eol-1 );
      tmp[eol] = '\0';
      txt = tmp;
    }

    match = _lastMatch( txt );

    /* Match Found */
    if( match ) {			     

      _goto( row+1 );			     /* Set Row */
      setPointX( 0 );			     /* Set/Adjust Column */

      int col = (int)(match - txt) / (int)sizeof( char ) - 1;
      
      for( int i = 0; i <= col; i++ )
	pointForward();

      SEARCHINGP = true;
      miniBufferMessage( "Found Match!" );
      matchP = true;

      if( tmp ) free( tmp );
      
      break;
    }
  }

  if( !matchP ) miniBufferMessage( "No Match Found" );
  
  return;
}


void clearSearchFlag( void ) {

  SEARCHINGP = false;
}


/***
==========================================================================================
				     LINE NAVIGATION
==========================================================================================
***/

/* Move Point to Next Line */
void nextLine() {

  int PtY  = getPointY();
  int ro   = getRowOffset();
  int co   = getColOffset();
  
  if(( PtY + ro ) < ( getBufferNumRows() - 1 )) {

    /* Avoid Mode Line */
    if( PtY < screenRows() ) setPointY( ++PtY );
    else setRowOffset( ++ro );
    
    if(( getPointX() + co ) >
       getBufferLineLen( PtY + ro )) pointToEndLine();
  }
}

/* Move Point to Prior Line */
void priorLine() {

  int PtY  = getPointY();
  int ro   = getRowOffset(); 
  int co   = getColOffset();
  
  if( PtY > 0 ) setPointY( --PtY );  
  else if ( ro > 0 ) setRowOffset( --ro );
  
  if(( getPointX() + co ) >
     getBufferLineLen( PtY + ro )) pointToEndLine();
}


/* Center Line */
void centerLine( void ) {

  int PtY = getPointY();
  int distToCenter = PtY - ( getWinNumRows() / 2 ) + 2;
  int ro  = getRowOffset();
  
  /* Point Above Center */
  if( distToCenter < 0 ) {

    if( ro == 0 ) return;
    
    if( ro > abs( distToCenter )) {
      setRowOffset( ro + distToCenter );
      setPointY( PtY + abs( distToCenter ));
      setPointX( 0 );
    }

    else {
      setPointY( ro + PtY );
      setRowOffset( 0 );
    }
  }

  /* Point Below Center */
  else {

    setRowOffset( ro + distToCenter );
    setPointY( PtY - distToCenter );
  }
}


/***
==========================================================================================
				    BUFFER NAVIGATION
==========================================================================================
***/

/* Move Point to End of Buffer */
void pointToEndBuffer( void ) {

  int nr = getBufferNumRows();
  
  setRowOffset( nr - getWinNumRows() + 2 );

  if( getRowOffset() < 1 ) {                /* Buffer Smaller Than Term */
    setRowOffset( 0 );
    setPointX( 0 );
    setPointY( nr - 1 );
  }

  else {                                     /* Scroll to End Buffer */
    setPointY( screenRows() );
    setPointX( 0 );    
  }
}

/* Page Down */
void pageDown( void ) {

  int PtY = getPointY();
  int sr  = screenRows();
  
  /* Point NOT at bottom of Terminal */
  if( PtY < sr ) {

    /* Last Line of Buffer Already Visible - No scroll */
    if(( getBufferNumRows() - thisRow()) < getWinNumRows() - 2) {

      pointToEndBuffer();
      return;
    }

    /* Scroll to Bottom of Terminal */
    setPointY( sr );
    setPointX( 0 );
    return;
  }

  /* Point at Bottom Row - Page Down */
  setRowOffset( getRowOffset() + sr );
  setPointY( 0 );
  setPointX( 0 );
}


/* Page Up */
void pageUp( void ) {

  int PtY = getPointY();
  int ro  = getRowOffset();
  
  /* Point NOT at top of terminal */
  if( PtY > 0 ) {
    setPointY( 0 );
    setPointX( 0 );

    return;
  }

  /* Point at Top - Page Up */
  if( ro > getWinNumRows() - 2 ) {
    setRowOffset( ro - screenRows() );
    setPointX( 0 );
    setPointY( screenRows() );
    return;
  }

  /* Scroll to Top of Buffer */
  setRowOffset( 0 );
  setPointY( 0 );
  setPointX( 0 );
}


/* Jump to <input> Linenumber */
static void _goto( int lineNum ) {

  int nr      = getBufferNumRows();

  if( lineNum < 1 || lineNum > nr ) return;

  if( nr < screenRows() ) {
    setPointY( lineNum - 1 );
  }
  else {
    setRowOffset( lineNum - 1 );
    setPointY( 0 );
    centerLine();
  }
}

void jumpToLine( void ) {

  int lineNum = miniBufferGetPosInteger( "Line: " );

  _goto( lineNum );
}

/***
    Local Variables:
    mode: c
    tags-file-name: "~/ae/TAGS"
    comment-column: 45
    fill-column: 90
    End:
 ***/
