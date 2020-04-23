/***
==========================================================================================
            _              _         _____    _ _ _
           / \   _ __   __| |_   _  | ____|__| (_) |_
          / _ \ | '_ \ / _` | | | | |  _| / _` | | __|
         / ___ \| | | | (_| | |_| | | |__| (_| | | |_
        /_/   \_\_| |_|\__,_|\__, | |_____\__,_|_|\__|  v0.3
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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "ae.h"
#include "keyPress.h"
#include "minibuffer.h"
#include "pointMarkRegion.h"
#include "render.h"
#include "buffer.h"

/* Macros */
#define CTRL_KEY(k) ((k) & 0x1f)
#define ALT_KEY 27
#define thisRow() (ROWOFFSET + getPointY())
#define thisCol() (COLOFFSET + getPointX())
#define screenRows() (getmaxy( WIN ) - 3)
#define DEFAULTFILENAME "newfile.txt"


/* Buffer Status Flag */
enum _sf { ORIGINAL, MODIFIED, READONLY };
const char _sfname[3][9] = { "ORIGINAL", "MODIFIED", "READONLY" };


/* Global Data */
char FILENAME[FNLENGTH];                /* Buffer Filename */
WINDOW *WIN;				/* Window Handle */
int NUMROWS    =  0;			/* Num Rows in Text Buffer */
int ROWOFFSET  =  0;			/* Buffer Index of Top Row */
int COLOFFSET  =  0;			/* Buffer Index of First Col */
enum _sf STATUSFLAG \
               = ORIGINAL;		/* Is Buffer Modified? */
static char EDITBUFFER[64];		/* Edit Buffer For Text Input */
static int  EBINDEX   =  0;		/* Edit Buffer Index */


/*******************************************************************************
			       INITIALIZE/TERMINATE EDITOR
*******************************************************************************/

/* Restore tty */
void closeEditor() {

  endwin();
}


/* Print Error Message and Exit */
void die( const char *s ) {

  perror(s);
  sleep(3);
  closeEditor();
  
  exit(EXIT_FAILURE);
}


/* Return Window Handle */
WINDOW *getWindowHandle() {

  return WIN;
}


/* Prepare tty for Raw nCurses Input */
void initializeTerminal() {

  if(( WIN = initscr() ) == NULL ) { /* Setup ncurses */
    die( "initializeTerminal: initscr failed" );
  }

  if( cbreak() == ERR ) {        /* Unbuffered Input */
    die( "initializeTerminal: cbreak failed" );
  }

  if( noecho() == ERR ) {        /* Do NOT local echo */
    die( "initializeTerminal: noecho failed" );
  }

  if( nonl() == ERR ) {                /* \n != \r\n */
    die( "initializeTerminal: nonl failed" );
  }

  if( keypad( stdscr, TRUE ) == ERR ) { /* Enable keypad */
    die( "initializeterminal: keypad failed" );
  }

  if( raw() == ERR ) {                /* Set ICANON, ISIG, IXON off */
    die( "initializeterminal: raw" );
  }

  timeout(100);
}


/*******************************************************************************
                          BUFFER MANAGEMENT
*******************************************************************************/

/* Set un-Named File to Default Filename */
void setDefaultFilename( void ) {

  strncpy( FILENAME, DEFAULTFILENAME, FNLENGTH );
  return;
}

void setFilename( char *fn ) {

  if( fn != FILENAME )
    strncpy( FILENAME, fn, FNLENGTH-1 );
}

char *getBufferFilename( void ) {

  return FILENAME;
}

int getEditBufferIndex() {

  return EBINDEX;
}

char getEditBufferChar(int i) {

  return EDITBUFFER[i];
}

void setEditBufferIndex( int x ) {

  EBINDEX = x;
}

void setStatusFlagOriginal( void ) {

  STATUSFLAG = ORIGINAL;
}

/*******************************************************************************
                              NAVIGATION
*******************************************************************************/

/* Move Point to End of Line */
void pointToEndLine() {

  int x = getBufferLineLen( thisRow() ) - 1; /* Text Line Length */
  int y = getmaxx( WIN ) - 1;		     /* Terminal Length */

  if( x > y ) {
    setPointX( y );
    COLOFFSET = x - y;
  }
  else {
    setPointX( x );
    COLOFFSET = 0;
  }
}


/* Move Point to Prior Line */
void priorLine() {

  int PtY      = getPointY();
  
  if( PtY > 0 ) setPointY( --PtY );  
  else if ( ROWOFFSET > 0 ) --ROWOFFSET;
  
  if(( getPointX() + COLOFFSET ) >
     getBufferLineLen( thisRow() ) ) pointToEndLine();
}


/* Move Point to Next Line */
void nextLine() {

  int PtY     = getPointY();
  
  if( PtY + ROWOFFSET < NUMROWS - 1 ) {

    /* Avoid Mode Line */
    if( PtY < screenRows() ) setPointY( ++PtY );
    else ++ROWOFFSET;
    
    if(( getPointX() + COLOFFSET ) >
       getBufferLineLen( thisRow() ) ) pointToEndLine();
  }
}


/* Move Point Forward */
void pointForward() {

  int PtX     = getPointX();
  
  if( thisCol() < getBufferLineLen( thisRow() ) - 1 ) {
    if( PtX < getmaxx( WIN ) - 1 ) setPointX( ++PtX );
    else COLOFFSET++;
  }
}


/* Move Point Backward */
void pointBackward() {

  int PtX = getPointX();
  
  if( thisCol() > 0 ) {
    if( PtX == 0 )
      --COLOFFSET;
    else
      setPointX( --PtX );
  }
}


/* Forward Word */
void forwardWord() {

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
void backwardWord() {

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

  
/* Move Point to End of Buffer */
void pointToEndBuffer() {

  ROWOFFSET = NUMROWS - getmaxy( WIN ) + 2;

  if( ROWOFFSET < 1 ) {                /* Buffer Smaller Than Term */
    ROWOFFSET = 0;
    setPointX( 0 );
    setPointY( NUMROWS - 1 );
  }

  else {                        /* Scroll to End Buffer */
    setPointY( screenRows() );
    setPointX( 0 );    
  }
}


/* Center Line */
void centerLine() {

  int PtY = getPointY();
  int distToCenter = PtY - ( getmaxy( WIN ) / 2 ) + 2;

  /* Point Above Center */
  if( distToCenter < 0 ) {

    if( ROWOFFSET == 0 ) return;
    
    if( ROWOFFSET > abs( distToCenter )) {
      ROWOFFSET += distToCenter;
      setPointY( PtY + abs( distToCenter ));
      setPointX( 0 );
    }

    else {
      setPointY( ROWOFFSET + PtY );
      ROWOFFSET = 0;
    }
  }

  /* Point Below Center */
  else {

    ROWOFFSET += distToCenter;
    setPointY( PtY - distToCenter );
  }
}


/* Jump to <input> Linenumber */
void jumpToLine() {

  int lineNum = miniBufferGetPosInteger( "Line: " );

  if( lineNum < 1 || lineNum > NUMROWS ) return;

  if( NUMROWS < screenRows() ) {
    setPointY( lineNum - 1 );
  }
  else {
    ROWOFFSET = lineNum - 1;
    setPointY( 0 );
    centerLine();
  }
}


/* Page Down */
void pageDown() {

  int PtY = getPointY();
  
  /* Point NOT at bottom of Terminal */
  if( PtY < screenRows() ) {

    /* Last Line of Buffer Already Visible - No scroll */
    if(( NUMROWS - thisRow()) < getmaxy( WIN ) - 2)
      return;

    /* Scroll to Bottom of Terminal */
    setPointY( screenRows() );
    setPointX( 0 );
    return;
  }

  /* Point at Bottom Row - Page Down */
  ROWOFFSET += screenRows();
  setPointY( 0 );
  setPointX( 0 );
}


/* Page Up */
void pageUp() {

  int PtY = getPointY();
  
  /* Point NOT at top of terminal */
  if( PtY > 0 ) {
    setPointY( 0 );
    setPointX( 0 );

    return;
  }

  /* Point at Top - Page Up */
  if( ROWOFFSET > getmaxy( WIN ) - 2 ) {
    ROWOFFSET -= screenRows();
    setPointX( 0 );
    setPointY( screenRows() );
    return;
  }

  /* Scroll to Top of Buffer */
  ROWOFFSET = 0;
  setPointY( 0 );
  setPointX( 0 );
}

/*******************************************************************************
				    UPDATE LINE EDITS
*******************************************************************************/

/* Incorporate Edits Into Row Structure */
void updateLine() {

  int i   = 0;
  int col = 0;

  buff_t buff = getBufferHandle();
  
  int delta = EBINDEX - ( buff[thisRow()]->rPtr -
                          buff[thisRow()]->lPtr );

  int newLen = buff[thisRow()]->len + delta;
    
  char *tmp;                        /* New Text Row */

  if(( tmp = malloc(( sizeof( char ) * newLen ) + 1 )) == NULL )
    die( "updateLine: tmp malloc failed" );

  /* Copy Non-deleted Chars */
  for( i = 0; i<(int)buff[thisRow()]->lPtr; i++ ) {
    tmp[col] = buff[thisRow()]->txt[i];
    col++;
  }

  /* Add Chars from Edit Buffer */
  for( i = 0; i<EBINDEX; i++ ) {
    tmp[col] = EDITBUFFER[i];
    col++;
  }

  /* Add Rest of Chars */
  for( i = buff[thisRow()]->rPtr; i<(int)buff[thisRow()]->len; i++ ) {
    tmp[col] = buff[thisRow()]->txt[i];
    col++;
  }

  tmp[newLen] = '\0';                /* NULL Terminate New String */

  free( buff[thisRow()]->txt );
  buff[thisRow()]->txt  = tmp;
  buff[thisRow()]->len  = newLen;
  buff[thisRow()]->lPtr = 0;
  buff[thisRow()]->rPtr = 0;

  EBINDEX = 0;
}


/*******************************************************************************
                             EDITOR STATE
*******************************************************************************/

/* Cursor Movement Functions */
void updateNavigationState() {

  if( bufferRowEditedP( thisRow() ))
    updateLine();

  setBufferRowEdited( thisRow(), false );
  
  miniBufferClear();
}

/* Edit Line */
void updateEditState() {

  setBufferRowEdited( thisRow(), true );
  STATUSFLAG = MODIFIED;
}

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
  ROWOFFSET = co;
}


/* Update the Number of Lines in Buffer File */
void setNumRows( int x ) {

  NUMROWS = x;
}

int getBufferNumRows( void ) {

  return NUMROWS;
}

/*******************************************************************************
                             INSERT CHARS
*******************************************************************************/

/* Insert User Typed Chars */
void selfInsert( int c ) {

  int PtX     = getPointX();

  /* Save Prior Unsaved Changes to Text Row */ 
  if( bufferRowEditedP( thisRow() ))
    updateNavigationState();

  /* Text Insertion Now Begins at POINT_X */  
  if( !bufferRowEditedP( thisRow() )) {
    setEditBufferPtrs( thisRow(), PtX, PtX );
    updateEditState();
  }

  EDITBUFFER[EBINDEX++] = c;
  setPointX( ++PtX );
}


/*******************************************************************************
                           LINE MANAGEMENT
*******************************************************************************/

/* Kill Line at Point */
void killLine() {

  int PtX = getPointX();
  buff_t buff = getBufferHandle();
  
  /* Line Empty - Delete it */
  if( getBufferLineLen( thisRow() ) == 1 ) {

    if( thisRow() == NUMROWS - 1 ) return; /* Cant Delete if Nothing Follows */

    freeBufferLine( thisRow() );
  }

  /* Delete from POINT to EOL */
  else {                        

    /* Heap Space for Trimmed String */
    char *tmp = malloc( sizeof( char ) * ( PtX + 2 ));
    if( PtX > 0 )
      strncpy( tmp, buff[thisRow()]->txt, PtX );
    free( buff[thisRow()]->txt );

    /* Fix Up row_t For This Row */
    buff[thisRow()]->txt = tmp;
    buff[thisRow()]->len = PtX + 1;
    buff[thisRow()]->txt[buff[thisRow()]->len-1] = '\n';
    buff[thisRow()]->txt[buff[thisRow()]->len] = '\0';

    clrtoeol();
  }
}

/* Indent Newline to Smart Location */
void autoIndent() {

  int len;				/* Len of 'last' row */
  
  int i = 0;				/* Col Index */
  int lastRow = thisRow() - 1;
  buff_t buff = getBufferHandle();

  /* Skip if Middle of Line or Top of Buffer */
  if(( getPointX() != 0 ) || ( thisRow() == 0 ))
    return;

  len = buff[lastRow]->len;

  /* Find Tab Col Prior Row */
  while( buff[lastRow]->txt[i] == ' ' )
    i++;

  /* Skip Blank Lines */
  if( i == len - 1 )
    return;
  
  /* Insert Spaces Until Tab Mark */
  while(( i < len ) && ( i > 0 )) {

    selfInsert( ' ' );
    i--;
  }
}


/* Open a New Line */
void openLine() {

  int PtY = getPointY();
  int PtX = getPointX();

  buff_t buff = getBufferHandle();
  
  /* Double Buffer, If Full */
  if( bufferFullP() ) {
    doubleBufferSize();
    buff = getBufferHandle();		/* New Double-Sized BUFFER */
  }

  /* Move Lines Down */
  int i;

  for( i=NUMROWS; i>thisRow()+1; i-- ) {
    buff[i] = buff[i-1];
  }

  /* Create New Line */
  buff[i] = malloc( sizeof( row_t ));
  buff[i]->len = buff[thisRow()]->len - PtX;
  buff[i]->txt = malloc( sizeof( char ) * ( buff[i]->len + 1 ));
  buff[i]->lPtr   = 0;
  buff[i]->rPtr   = 0;
  buff[i]->editP  = false;


  /* Copy Text Into New Line */
  if( strncpy( buff[i]->txt, 
               buff[thisRow()]->txt + PtX,
               buff[i]->len ) == NULL ) die( "openLine: strncpy failed." );

  buff[i]->txt[buff[i]->len] = '\0';


  /* Heap Space for Trimmed String */
  char *tmp = malloc( sizeof( char ) * ( PtX + 2 ));
  if( PtX > 0 )
    strncpy( tmp, buff[thisRow()]->txt, PtX );
  free( buff[thisRow()]->txt );

  /* Fix Up row_t For This Row */
  buff[thisRow()]->txt = tmp;
  buff[thisRow()]->len = PtX + 1;
  buff[thisRow()]->txt[buff[thisRow()]->len-1] = '\n';
  buff[thisRow()]->txt[buff[thisRow()]->len] = '\0';

  clrtoeol();
  
  /* Move Point */
  setPointX( 0 );
  if( PtY == screenRows() )
    ROWOFFSET++;
  else
    setPointY( ++PtY );

  NUMROWS++;                        /* Increment Num Lines */
}

/*******************************************************************************
                             DELETE CHARS
*******************************************************************************/

void killWord() {

  int currPointX = getPointX();		/* Save Current Point */
  
  forwardWord();			/* Find End Next Word */

  if( getPointX() == currPointX )	/* No Word to Kill */
    return;

  /* Mark Word for Deletion and Restore Point */
  setEditBufferPtrs( thisRow(), currPointX, getPointX() );
  setPointX( currPointX );
  updateEditState();
  updateNavigationState();
}


void combineLineWithPrior() {

  if( thisRow() == 0 )
    return;

  buff_t buff = getBufferHandle();
  
  char *tmp;
        
  /* Allocate Memory for Combined String */
  int preLineLen = buff[thisRow()-1]->len;
  int nxtLineLen = buff[thisRow()]->len;
  int size       = preLineLen + nxtLineLen + 1;

  if(( tmp = malloc( sizeof( char ) * size )) == NULL )
    die( "backspace: tmp malloc failed" );

  /* Copy Old/New Text Into New Line */
  strncpy( tmp, 
           buff[thisRow()-1]->txt,
           preLineLen );
  strncpy( tmp + preLineLen,
           buff[thisRow()]->txt,
           nxtLineLen );
  buff[thisRow()-1]->len = size - 1;
        
  /* Free Old Memory and Set Pointer */
  free( buff[thisRow()-1]->txt );
  buff[thisRow()-1]->txt = tmp;

  /* Destroy Next Line */
  freeBufferLine( thisRow() );

  setPointY( getPointY() - 1 );
  setPointX( preLineLen );
}

void backspace() {

  int PtX     = getPointX();
  buff_t buff = getBufferHandle();
  
  /* Continue Editing This Line? */
  if( buff[thisRow()]->editP ) {

    /* If Deleting Chars After Inserting Chars? */
    /* Then, Save Insertions Befor Making Deletions */ 
    if( buff[thisRow()]->lPtr == buff[thisRow()]->rPtr ) {
      updateLine();
      buff[thisRow()]->lPtr = PtX;
      buff[thisRow()]->rPtr = PtX;
    }

    /* Continue Deleting Chars to BOL */
    if( PtX > 0 ) {
      buff[thisRow()]->lPtr--;
      setPointX( --PtX );
    }

    /* At BOL, Combine With Prior Line */
    else {
      updateNavigationState();
      combineLineWithPrior();
    }
  }

  /* Begin *New* Edits to This Line */
  else {

    if( PtX > 0 ) {
      buff[thisRow()]->lPtr = PtX - 1;
      buff[thisRow()]->rPtr = PtX;
      updateEditState();
      setPointX( --PtX );
    }

    else {
      combineLineWithPrior();
    }
  }
}


/*******************************************************************************
                          REGION OPERATIONS
*******************************************************************************/

static void _removeText( int strt_Col, int strt_Row,
			 int stop_Col, int stop_Row ) {

  buff_t buff = getBufferHandle();


  /* Loop Over Rows in Region */ 
  int row = strt_Row;
  do {

    /* Handle Lines Between Start/End */
    if(( row > strt_Row ) && ( row < stop_Row )) {
      freeBufferLine( row );
      --stop_Row;
    }

    /* Handle First Line */
    else if( row == strt_Row ) {

      /* Delete Entire Line */
      if(( strt_Col == 0 ) && ( stop_Row > strt_Row )) {
	freeBufferLine( row );
	--stop_Row;
	--strt_Row;
      }

      /* Delete Mark to Point OR End of Line */
      else {
	buff[row]->lPtr = strt_Col;
	buff[row]->rPtr = ( stop_Row > strt_Row ) ? 
	  buff[row]->len - 1 :		     /* Delete Rest of Line */
	  (size_t)stop_Col;		     /* Delete Part of Line */

	updateLine();
	++row;
      }
    }

    /* Handle Last Line */
    else {

      /* Delete Entire Last Line */
      if( stop_Col == (int)( buff[row]->len - 1 )) {
	freeBufferLine( row );
	--stop_Row;
	++row;
      }

      /* Ignore if Stop on First Col */
      else if( stop_Col == 0 ) {
	++row;
      }

      /* Delete Part of Last Line */
      else {
	buff[row]->lPtr = 0;
	buff[row]->rPtr = stop_Col;
	updateLine();
	++row;
      }
    }

  } while( row <= stop_Row );
  
  return;
}


/* Kill Text Between Point and Mark */
void killRegion() {

  int temp_X, temp_Y;
  
  int strt_X = getPointX() + getColOffset(); 
  int strt_Y = getPointY() + getRowOffset();
  int stop_X = getMarkX();
  int stop_Y = getMarkY();

  /* Define Start/Stop Deletion Region */
  if((  stop_Y <  strt_Y ) ||
     (( stop_Y == strt_Y ) && ( stop_X < strt_X ))) {

    temp_X = strt_X;
    temp_Y = strt_Y;
    strt_X = stop_X;
    strt_Y = stop_Y;
    stop_X = temp_X;
    stop_Y = temp_Y;
  }

  /* Remove Text/Textlines */
  _removeText( strt_X, strt_Y, stop_X, stop_Y );

  /* Reset Point */
  setPointX( strt_X - getColOffset() );
  setPointY( strt_Y - getRowOffset() );
  setRegionActive( false );

  return;
}


/*******************************************************************************
                         PROCESS KEY PRESSES
*******************************************************************************/

/* Meta Menu */
void metaMenu() {

  int c = readKey();

  switch(c) {

    /* Buffer Navigation */
  case 'f':                        /* Forward Word */
    updateNavigationState();
    forwardWord();
    break;

  case 'b':                        /* Backward Word */
    updateNavigationState();
    backwardWord();
    break;

  case 'd':                        /* Kill Forward Word */
    killWord();
    updateEditState();
    break;

  case 'g':                        /* Alias for c-j */
    jumpToLine();
    break;
    
  case 'v':                        /* Forward Word */
    updateNavigationState();
    pageUp();
    break;

  case '<':                              /* Top of Buffer */
    updateNavigationState();
    setPointY( 0 );
    setPointX( 0 );
    COLOFFSET = 0;
    ROWOFFSET = 0;
    break;

  case '>':                           /* End of Buffer */
    updateNavigationState();
    pointToEndBuffer();
    break;
  }  
}


/* eXtension Menu */
void eXtensionMenu() {

  int c = readKey();

  switch(c) {

  case CTRL_KEY('c'):                 /* Close Editor */
    if( STATUSFLAG == MODIFIED )
      if( miniBufferGetYN( "Buffer Modified. Save? [Y/N] " )) {
	updateNavigationState();
	saveBuffer();
      }
    closeBuffer();
    closeEditor();
    exit(EXIT_SUCCESS);
    break;

  case 'k':                           /* Kill Buffer */
    if( STATUSFLAG == MODIFIED )
      if( miniBufferGetYN( "Buffer Modified. Save? [Y/N] " )) {
	updateNavigationState();
	saveBuffer();
      }
    killBuffer();
    break;
    
  case CTRL_KEY('s'):                /* Save Buffer */
    if( STATUSFLAG == MODIFIED ) {
	updateNavigationState();
	saveBuffer();
      }
    else {
      miniBufferMessage( "Buffer not Modified" );
    }
    break;

  case CTRL_KEY('f'):
  case CTRL_KEY('v'):
    /* Close Old Buffer */
    if( STATUSFLAG == MODIFIED )
      if( miniBufferGetYN( "Buffer Modified. Save? [Y/N] " )) {
	updateNavigationState();
	saveBuffer();
      }
    killBuffer();
    /* Open New Buffer */
    miniBufferGetFilename( FILENAME, FNLENGTH );
    openBufferFile( FILENAME );
    break;
    
  case CTRL_KEY('w'):                /* Save Buffer As */
    if( STATUSFLAG == MODIFIED ) {
	updateNavigationState();
	saveBufferNewName();
      }
    else {
      miniBufferMessage( "Buffer not Modified" );
    }
    break;

  case CTRL_KEY('x'):                /* Swap Point/Mark */
    updateNavigationState();
    swapPointAndMark();
    if(( getPointY() < 0 ) ||
       ( getPointY() > ( ROWOFFSET + screenRows())))
      centerLine();
    break;
  }
}
  

/* Process Keypresses */
void processKeypress() {

  int c       = readKey();
  buff_t buff = getBufferHandle();
  
  switch(c) {
    
    /* Meta Key */
  case ALT_KEY:
    metaMenu();
    break;

    /* CTRL Key */
  case CTRL_KEY('x'):
    eXtensionMenu();
    break;

    /* Keyboard Quit */
  case CTRL_KEY('g'):
    setRegionActive( false );
    setMarkY( -1 );
    setMarkX( -1 );
    break;

    /* Function Keys */
  case KEY_F(1):			/* Help */

    endwin();				/* Dumb Help */
    system( "cat USERGUIDE.md | more" );
    initializeTerminal();
    break;

  case KEY_F(10):			/* Exit */
    closeEditor();
    exit(EXIT_SUCCESS);
    break;

  case '\t':
    miniBufferMessage( "Auto Indent" );
    autoIndent();
    break;
    
    /* Cursor Movement */
  case KEY_HOME:			/* Home */
    updateNavigationState();
    setPointY( 0 );
    setPointX( 0 );
    COLOFFSET = 0;
    ROWOFFSET = 0;
    break;
  case CTRL_KEY('l'):                /* Center Line */
    updateNavigationState();
    centerLine();
    break;
  case CTRL_KEY('b'):                /* Point Back */
  case KEY_LEFT:                
    updateNavigationState();
    pointBackward();
    break;
  case CTRL_KEY('a'):                /* Point BOL */
    updateNavigationState();
    setPointX( 0 );
    COLOFFSET = 0;
    break;
  case CTRL_KEY('f'):                /* Point Forward */
  case KEY_RIGHT:
    updateNavigationState();
    pointForward();
    break;
  case CTRL_KEY('e'):                /* Point EOL */
    updateNavigationState();
    pointToEndLine();
    break;
  case CTRL_KEY('j'):                /* Jump to Linenum */
    updateNavigationState();
    jumpToLine();
    break;
  case CTRL_KEY('p'):                /* Prior Line */
  case KEY_UP:
    updateNavigationState();
    priorLine();
    break;
  case CTRL_KEY('n'):                /* Next Line */
  case KEY_DOWN:
    updateNavigationState();
    nextLine();
    break;
  case KEY_PPAGE:                    /* Page Up */
    updateNavigationState();
    pageUp();
    break;
  case KEY_END:                      /* End of Buffer */
    updateNavigationState();
    pointToEndBuffer();
    break;
  case KEY_NPAGE:                    /* Page Down */
  case CTRL_KEY('v'):
    updateNavigationState();
    pageDown();
    break;

    /* Create a Newline */
  case '\r':                        /* Enter Key */
    updateNavigationState();
    openLine();
    updateEditState();
    break;

    /* Point/Mark */
  case CTRL_KEY(' '):                /* Set Mark */
    setMarkX( thisCol() );
    setMarkY( thisRow() );
    setRegionActive( true );
    miniBufferMessage( "Mark Set" );
    break;

    /* Edit Text */
  case CTRL_KEY('d'):                /* Delete Char */
  case KEY_DC:

    /* Continue Editing This Line? */
    if( buff[thisRow()]->editP ) {

      /* Start Deleting Chars After Inserting Chars? */
      if( buff[thisRow()]->lPtr == buff[thisRow()]->rPtr ) {
        updateLine();
        buff[thisRow()]->lPtr = getPointX();
        buff[thisRow()]->rPtr = getPointX();
      }
        
      /* Continue Deleting Chars Up to EOL  */
      if( buff[thisRow()]->rPtr < buff[thisRow()]->len )
        buff[thisRow()]->rPtr++;
    }
    
    /* *New* Edit To This Line? */
    else {
      int PtX = getPointX();
      if( (size_t)PtX < buff[thisRow()]->len - 1 ) {
        buff[thisRow()]->lPtr = PtX;
        buff[thisRow()]->rPtr = PtX+1;
        updateEditState();
      }
    }
    break;

  case CTRL_KEY('h'):                /* Backspace */
  case KEY_BACKSPACE:
    backspace();
    break;
  case CTRL_KEY('k'):                   /* Kill Line */
    updateEditState();
    killLine();
    updateNavigationState();
    break;

  case CTRL_KEY('w'):                   /* Kill Region  */
    killRegion();
    updateEditState();
    break;
    
    /* Handle Signals */
  case KEY_RESIZE:                /* Window Resized */
    break;

  default:                        /* Self Insert */
    if( isprint( c ))
      selfInsert( c );
    break;
  }
}


/*****************************************************************************************
				       MAIN PROGRAM
*****************************************************************************************/

/* Display Splash Screen */
void displaySplash( void ) {

  int center = (getmaxx(WIN) / 2);
  int third  = (getmaxy(WIN) / 3);
  
  mvaddstr( third + 0, center - 10, "Welcome to Andy Edit!" );
  mvaddstr( third + 1, center - 6, "Version 0.3" );
  mvaddstr( third + 3, center - 9, "(c) Copyright 2020" );

  refresh();
  sleep(2);

  clear();
  renderText( FILENAME, _sfname[ STATUSFLAG ], getmaxy(WIN)-2, getmaxx(WIN), NUMROWS );
}



/* Editor Polling Loop */
int main( int argc, char *argv[] ) {

  /* Initialize */
  initializeTerminal();
  initializeBuffer();

  /* Open File or Display Splash */
  if( argc > 1 ) {
    setFilename( argv[argc-1] ); 
    openBufferFile( FILENAME );
  }
  else {
    openEmptyBuffer( DEFAULT );
    displaySplash();
  }
  
  /* Process Key Presses */
  while( true ) {
    renderText( FILENAME, _sfname[ STATUSFLAG ], getmaxy(WIN)-2, getmaxx(WIN), NUMROWS );
    processKeypress();
  } 

  /* Shutdown */
  closeEditor();
  return EXIT_SUCCESS;
} 


/***
    Local Variables:
    mode: c
    comment-column: 45
    fill-column: 90
    End:

   -----------------------------
  < AndyEdit is Udderly Sweet! >
   -----------------------------
        o   ^__^
         o  (oo)\_______
            (__)\       )\/\
                ||----w |
                ||     ||
END PROGRAM
 ***/
