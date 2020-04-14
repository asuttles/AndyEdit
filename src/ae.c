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

#define _POSIX_C_SOURCE 200809L		/* getline() is POSIX */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curses.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include <readline/readline.h>

#include "keyPress.h"
#include "minibuffer.h"

/* Macros */
#define CTRL_KEY(k) ((k) & 0x1f)
#define ALT_KEY 27
#define MXRWS 512
#define MSGBUFFSIZE 64
#define FNLENGTH 128
#define DEFAULTFILENAME "newfile.txt"
#define thisRow() (ROWOFFSET + POINT_Y)
#define thisCol() (COLOFFSET + POINT_X)
#define screenRows() (getmaxy( WIN ) - 3)

/* Buffer Status Flag */
enum _sf { ORIGINAL, MODIFIED, READONLY };
const char _sfname[3][9] = { "ORIGINAL", "MODIFIED", "READONLY" };

/* Empty Buffer : Default or User Named */
enum _bn { DEFAULT, UNAMED };

/* Text Line Data Structures */
typedef struct {
  char  *txt;				/* Editor Text Line */
  size_t len;				/* Length of Text */
  size_t lPtr;				/* Editor Pointers */
  size_t rPtr;
  bool   editP;				/* This Row Edited Predicate */
} row_t;

/* Global Data */
char FILENAME[FNLENGTH];                /* Buffer Filename */
WINDOW *WIN;				/* Window Handle */
int POINT_X    =  0;			/* Point X Position */
int POINT_Y    =  0;			/* Point Y Position */
int MARK_X     = -1;			/* Mark X Position */
int MARK_Y     = -1;			/* Mark Y Position */
int NUMROWS    =  0;			/* Num Rows in Text Buffer */
int ROWOFFSET  =  0;			/* Buffer Index of Top Row */
int COLOFFSET  =  0;			/* Buffer Index of First Col */
int MAXROWS    = MXRWS;			/* MAX Number of Buffer Lines */
enum _sf STATUSFLAG \
               = ORIGINAL;		/* Is Buffer Modified? */
row_t **BUFFER;				/* File Buffer */
char EDITBUFFER[64];			/* Edit Buffer For Text Input */
int  EBINDEX   =  0;			/* Edit Buffer Index */
bool REGIONP   = false;			/* Is Region Active? */


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
                                 TABS
*******************************************************************************/

/* Convert Tabs to Spaces */
char *removeTabs( char *line ) {

  size_t i;

  size_t countTabs = 0;
  size_t len = strlen( line );
  
  /* Count the Number of Tabs */
  for( i = 0; i<len; i++ ) {
    if( line[i] == '\t' ) {
      countTabs++;
    }
  }

  /* Return if No Tabs Found */
  if( countTabs == 0 ) return line;

  /* Create Space for 'Newline' Without Tabs */
  char *newline = malloc(( sizeof( char ) *
                           ( len + ( countTabs * 7 ))) + 1 );
  size_t j;
  size_t index = 0;

  /* Copy Line to Newline With Spaces */
  for( i = 0; i<len; i++ ) {

    if( line[i] == '\t' ) {        /* Tabs Found */
      for( j = 0; j<8; j++ ) {
        newline[index] = ' ';
        index++;
      }
    }
    else {                        /* Not a Tab */
      newline[index] = line[i];
      index++;
    }
  }

  newline[index] = '\0';
  free( line );                        /* Delete Old Line */
  return newline;                /* Save New Line */
}

/*******************************************************************************
                          BUFFER MANAGEMENT
*******************************************************************************/

/* Setup Editor Data Structures */
void initializeData() {

  /* Reserve Heap Space for Buffer */
  if(( BUFFER = malloc( sizeof( row_t * ) * MXRWS )) == NULL )
    die( "initializeData: BUFFER malloc failed" );
}


/* Open AE on an Empty Buffer */
void emptyBuffer(enum _bn bn) {

  BUFFER[0] = malloc( sizeof( row_t ));
  BUFFER[0]->txt = malloc( sizeof( char ) * 2 );
  BUFFER[0]->txt[0] = '\n';
  BUFFER[0]->txt[1] = '\0';
  BUFFER[0]->len    = 1;
  BUFFER[0]->lPtr   = 0;
  BUFFER[0]->rPtr   = 0;
  BUFFER[0]->editP  = false;

  if( bn == DEFAULT ) {
    strncpy( FILENAME, DEFAULTFILENAME, FNLENGTH-1 );
  }
  
  NUMROWS = 1;
}

/* Close Text Buffer */
void closeBuffer() {

  int i;

  for( i=0; i<NUMROWS; i++ ) {
    free( BUFFER[i]->txt );
    free( BUFFER[i] );
  }

  free( BUFFER );
  BUFFER = (row_t **)NULL;

  MAXROWS    = MXRWS;
  NUMROWS    = 0;
  POINT_X    = 0;
  POINT_Y    = 0;
  MARK_X     = -1;
  MARK_Y     = -1;
  ROWOFFSET  = 0;
  COLOFFSET  = 0;
  EBINDEX   =  0;
  REGIONP   = false;

  STATUSFLAG = ORIGINAL;

  initializeData();
  emptyBuffer( DEFAULT );
  
  clear();
}


/* Double Buffer Size */
void doubleBufferSize() {
  
  row_t **ptr = BUFFER;
  row_t **newPtr = NULL;

  int newMaxRows = MAXROWS * 2;        /* Double Buffer Size */
    
  newPtr = realloc( ptr, newMaxRows * sizeof( row_t *));
  MAXROWS = newMaxRows;
  
  if( newPtr == NULL ) die( "doubleBufferSize: realloc failed" );
  if( newPtr != ptr  ) {

    BUFFER = newPtr;
    free( ptr );
  }
}


/* Read A Text File from Disk */
void openBuffer( char * fn ) {

  int i = 0;
  FILE *fp = NULL;
  bool newFile = FALSE;

  /* Check to See if File Exists and Readable */
  if( access( fn, R_OK | F_OK ) == -1 ) {

    miniBufferMessage( "Filename doesn't exist. Creating buffer for new file." );
    emptyBuffer( UNAMED );
    newFile = TRUE;
  }

  /* Save fn as Global */
  if( fn != FILENAME )
    strncpy( FILENAME, fn, FNLENGTH-1 );

  /* If newfile, Skip Opening/Reading from Disk */
  if( newFile ) return;

  
  /* Open File for Editing */
  if(( fp = fopen( fn, "r" )) == NULL ) {
    die( "openBuffer: fopen failed." );
  }


  /* Read File Rows into BUFFER */
  while( true ) {

    /* Check Buffer Size */
    if( i == MAXROWS ) {        
      doubleBufferSize();
    }

    /* Reserve Heap Space for Text Row  */
    BUFFER[i] = malloc( sizeof( row_t ));
    BUFFER[i]->len   = 0;
    BUFFER[i]->lPtr  = 0;
    BUFFER[i]->rPtr  = 0;
    BUFFER[i]->editP = false;
    BUFFER[i]->txt   = NULL;

    /* Read Next Text Row */
    if( getline( &BUFFER[i]->txt, &BUFFER[i]->len, fp ) == ERR ) break;

    /* Clean-up Tabs and Set Length */
    BUFFER[i]->txt = removeTabs( BUFFER[i]->txt );
    BUFFER[i]->len = strlen( BUFFER[i]->txt );

    i++;                        /* Line Counter */
  }
  
  NUMROWS = i;

  fclose(fp);
}


/* Save Buffer Lines */
void saveBuffer() {

  int row;
  char buffer[ 20 + FNLENGTH ];
  FILE *fp = NULL;

  /* Open File for Editing */
  if(( fp = fopen( FILENAME, "w" )) == NULL ) {
    die( "saveBuffer: fopen failed." );
  }

  for( row = 0; row<NUMROWS; row++ ) {
    fprintf( fp, "%s", BUFFER[row]->txt );
  }

  fclose( fp );
  STATUSFLAG = ORIGINAL;

  snprintf( buffer, 20 + FNLENGTH, "Wrote %d lines to %s", row, FILENAME );
  miniBufferMessage( buffer );
}


/* Save Buffer Lines */
void saveBufferNewName() {

  int row;
  FILE *fp = NULL;
  char *fn = FILENAME;

  /* Get Filename to Write */
  miniBufferGetFilename( fn, FNLENGTH );

  /* Open File for Editing */
  if(( fp = fopen( fn, "w" )) == NULL ) {
    die( "saveBufferNewName: fopen failed." );
  }

  for( row = 0; row<NUMROWS; row++ ) {
    fprintf( fp, "%s", BUFFER[row]->txt );
  }

  fclose( fp );
  STATUSFLAG = ORIGINAL;
  miniBufferMessage( "Wrote Text File." );
}

/*******************************************************************************
                              STATUS BAR
*******************************************************************************/

/* Draw Status Line */
void drawStatusLine() {

  int  curCol, maxCol;
  char status[256];

  int  curRow = getmaxy( WIN ) - 2;
  
  
  attron( A_REVERSE );                /* Reverse Video */

  snprintf( status, (getmaxx( WIN ) > 256 ? 256 : getmaxx( WIN )) - 1,
            "--[ %s ]-------(%s)------- Row %d of %d ------- Col %d of %d ------- F1 for Help --- F10 to Quit",
            FILENAME,
            _sfname[ STATUSFLAG ],
            thisRow() + 1, NUMROWS,
            thisCol() + 1, (int)BUFFER[thisRow()]->len );
            
    
    
  mvaddstr( curRow, 0, status );

  curCol = getcurx( WIN );
  maxCol = getmaxx( WIN ); 
  
  while( curCol < maxCol ) {
    mvaddch( curRow, curCol++, '-' );
  }
  
  attroff( A_REVERSE );
}


/*******************************************************************************
                             RENDER TEXT
*******************************************************************************/

/* Is (Row,Col) Within Active Region? */
bool inRegion( int row, int col ) {

  /* Region Not Active */
  if( !REGIONP ) return false;        


  /* This Row on MARK Line */
  if( row == MARK_Y ) {

    if( col >= MARK_X ) {

      if( thisRow() > row )
        return true;
    
      else if( col < thisCol() && thisRow() == MARK_Y )
        return true;
    }

    if( col <= MARK_X ) {

      if( thisRow() < row )
        return true;

      else if( col > thisCol() && thisRow() == MARK_Y )
        return true;
    }
  }

  /* POINT_Y > thisRow() > MARK_Y */
  else if( row > MARK_Y ) {

    if( row < thisRow() )
      return true;

    else if( row == thisRow() && col < thisCol() )
      return true;
  }

  /* POINT_Y < thisRow() < MARK_Y */
  else if( row < MARK_Y ) {

    if( row > thisRow() )
      return true;

    else if( row == thisRow() && col > thisCol() )
      return true;
  }
  
  return false;
}


/* Draw and Color the Rows of Text */
void renderText() {

  int i, j, row, col, colMax, txtLen, nextRow;
  int maxRows = getmaxy( WIN ) - 2;
  int maxCols = getmaxx( WIN );

  for( row = 0; row < maxRows; row++ ) {

    nextRow = row+ROWOFFSET;

    
    if( nextRow < NUMROWS ) {        /* Write Text */

      /* Calculate what subset of the Row Fits in the Term */
      txtLen = (int)BUFFER[nextRow]->len - COLOFFSET;
      colMax = maxCols > txtLen ? txtLen : maxCols;

      /* Write Letter at a Time */
      col = 0;
      for( i=0; i < colMax; i++ ) {

        /* Insert Edit Buffer Chars */
        if( nextRow == thisRow()    &&
            BUFFER[nextRow]->editP  &&
            ( BUFFER[nextRow]->lPtr == BUFFER[nextRow]->rPtr ) &&
            ( i == (int)BUFFER[nextRow]->lPtr )) {

          for( j = 0; j<EBINDEX; j++ ) {
            mvaddch( row, col, EDITBUFFER[j] );
            col++;
          }

          if( inRegion( nextRow, i+COLOFFSET ))
            attron( A_STANDOUT );
          mvaddch( row, col, BUFFER[nextRow]->txt[i+COLOFFSET] );
          col++;
          attroff( A_STANDOUT );
        }

        /* Ignore Chars In Line Buffer Gap */
        else if( nextRow == thisRow()     &&
                 BUFFER[thisRow()]->editP &&
                 i >= (int)BUFFER[nextRow]->lPtr && 
                 i <  (int)BUFFER[nextRow]->rPtr ) {
          continue;
        }

        /* Insert Line Buffer Chars */
        else {
          if( inRegion( nextRow, i+COLOFFSET ))
            attron( A_STANDOUT );
          mvaddch( row, col, BUFFER[nextRow]->txt[i+COLOFFSET] );
          col++;
          attroff( A_STANDOUT );
        }
      }
    } 

    
    else {                        /* vi style EOF Markers */      
      mvaddch( row, 0, '~' );
      clrtoeol();
    }    
  }

  drawStatusLine();

  move( POINT_Y, POINT_X );        /* Set POINT */
  refresh();
}

/*******************************************************************************
                              NAVIGATION
*******************************************************************************/

/* Move Point to End of Line */
void pointToEndLine() {

  int x = BUFFER[thisRow()]->len - 1; /* Text Line Length */
  int y = getmaxx( WIN ) - 1;              /* Terminal Length */

  if( x > y ) {
    POINT_X = y;
    COLOFFSET = x - y;
  }
  else {
    POINT_X = x;
    COLOFFSET = 0;
  }
}


/* Move Point to Prior Line */
void priorLine() {

  if( POINT_Y > 0 ) --POINT_Y;  
  else if ( ROWOFFSET > 0 ) --ROWOFFSET;
  
  if(( POINT_X + COLOFFSET ) > 
     ((int)BUFFER[thisRow()]->len - 1 )) pointToEndLine();
}


/* Move Point to Next Line */
void nextLine() {
  
  if( POINT_Y + ROWOFFSET < NUMROWS - 1 ) {

    /* Avoid Mode Line */
    if( POINT_Y < screenRows() ) ++POINT_Y;
    else ++ROWOFFSET;
    
    if(( POINT_X + COLOFFSET ) > 
       ((int)BUFFER[thisRow()]->len - 1 )) pointToEndLine();
  }
}


/* Move Point Forward */
void pointForward() {
  
  if( thisCol() < (int)BUFFER[thisRow()]->len - 1 ) {
    if( POINT_X < getmaxx( WIN ) - 1 ) ++POINT_X;
    else COLOFFSET++;
  }
}


/* Move Point Backward */
void pointBackward() {
  
  if( thisCol() > 0 ) {
    if( POINT_X == 0 )
      --COLOFFSET;
    else
      --POINT_X;
  }
}


/* Forward Word */
void forwardWord() {
  
  if( thisCol() == (int)BUFFER[thisRow()]->len-1 ) /* At EOL? */
    return;

  pointForward();

 /* Move Past Spaces */
  while( BUFFER[thisRow()]->txt[ thisCol() ] == ' ' ) 
    pointForward();

 /* Move to End of Word */
 while( BUFFER[thisRow()]->txt[ thisCol() ] != '\n' &&
        BUFFER[thisRow()]->txt[ thisCol() ] != ' '  &&
        BUFFER[thisRow()]->txt[ thisCol() ] != ')'  &&
        BUFFER[thisRow()]->txt[ thisCol() ] != ']' )

   pointForward();
}


/* Backward Word */
void backwardWord() {

  if( thisCol() == 0 ) return;        /* At BOL? */

  int old_POINT_X = POINT_X;
  pointBackward();
  
 /* Move Past Spaces */
  while(( BUFFER[thisRow()]->txt[ thisCol() ] == ' '   ||
          BUFFER[thisRow()]->txt[ thisCol() ] == ')'   ||
          BUFFER[thisRow()]->txt[ thisCol() ] == ';'   ||
          BUFFER[thisRow()]->txt[ thisCol() ] == ']' ) &&
        thisCol() > 0 )
    pointBackward();

  /* If POINT_X is a Space, No Prior Word this Line */
  if( BUFFER[thisRow()]->txt[ thisCol() ] == ' ' ) {
    POINT_X = old_POINT_X;
    return;
  }
  
 /* Move to Beginning of Word */
  while( thisCol() > 0                              &&
        BUFFER[thisRow()]->txt[ thisCol() ] != ' '  &&
        BUFFER[thisRow()]->txt[ thisCol() ] != '('  &&
        BUFFER[thisRow()]->txt[ thisCol() ] != '[' )
    pointBackward();

 /* Don't Leave POINT on a Space */
  if( BUFFER[thisRow()]->txt[ thisCol() ] == ' ' )
    pointForward();
}

  
/* Move Point to End of Buffer */
void pointToEndBuffer() {

  ROWOFFSET = NUMROWS - getmaxy( WIN ) + 2;

  if( ROWOFFSET < 1 ) {                /* Buffer Smaller Than Term */
    ROWOFFSET = 0;
    POINT_X = 0;
    POINT_Y = NUMROWS - 1;
  }

  else {                        /* Scroll to End Buffer */
    POINT_Y = screenRows();
    POINT_X = 0;
  }
}


/* Center Line */
void centerLine() {

  int distToCenter = POINT_Y - ( getmaxy( WIN ) / 2 ) + 2;

  /* Point Above Center */
  if( distToCenter < 0 ) {

    if( ROWOFFSET > abs( distToCenter )) {
      ROWOFFSET += distToCenter;
      POINT_Y += abs( distToCenter );
      POINT_X = 0;
    }

    else {
      POINT_Y += ROWOFFSET;
      ROWOFFSET = 0;
    }
  }

  /* Point Below Center */
  else {

    ROWOFFSET += distToCenter;
    POINT_Y -= distToCenter;
  }
}


/* Jump to <input> Linenumber */
void jumpToLine() {

  int lineNum = miniBufferGetPosInteger( "Line: " );

  if( lineNum < 1 || lineNum > NUMROWS ) return;

  if( NUMROWS < screenRows() ) {
    POINT_Y = lineNum - 1;
  }
  else {
    ROWOFFSET = lineNum - 1;
    POINT_Y = 0;
    centerLine();
  }
}


/* Page Down */
void pageDown() {

  /* Point NOT at bottom of Terminal */
  if( POINT_Y < screenRows() ) {

    /* Last Line of Buffer Already Visible - No scroll */
    if(( NUMROWS - thisRow()) < getmaxy( WIN ) - 2)
      return;

    /* Scroll to Bottom of Terminal */
    POINT_Y = screenRows();
    POINT_X = 0;
    return;
  }

  /* Point at Bottom Row - Page Down */
  ROWOFFSET += screenRows();
  POINT_Y = 0;
  POINT_X = 0;
}


/* Page Up */
void pageUp() {

  /* Point NOT at top of terminal */
  if( POINT_Y > 0 ) {
    POINT_X = 0;
    POINT_Y = 0;

    return;
  }

  /* Point at Top - Page Up */
  if( ROWOFFSET > getmaxy( WIN ) - 2 ) {
    ROWOFFSET -= screenRows();
    POINT_X = 0;
    POINT_Y = screenRows();
    return;
  }

  /* Scroll to Top of Buffer */
  ROWOFFSET = 0;
  POINT_X = 0;
  POINT_Y = 0;
}

/*******************************************************************************
				    UPDATE LINE EDITS
*******************************************************************************/

/* Incorporate Edits Into Row Structure */
void updateLine() {

  int i   = 0;
  int col = 0;
  
  int delta = EBINDEX - ( BUFFER[thisRow()]->rPtr -
                          BUFFER[thisRow()]->lPtr );

  int newLen = BUFFER[thisRow()]->len + delta;
    
  char *tmp;                        /* New Text Row */

  if(( tmp = malloc(( sizeof( char ) * newLen ) + 1 )) == NULL )
    die( "updateLine: tmp malloc failed" );

  /* Copy Non-deleted Chars */
  for( i = 0; i<(int)BUFFER[thisRow()]->lPtr; i++ ) {
    tmp[col] = BUFFER[thisRow()]->txt[i];
    col++;
  }

  /* Add Chars from Edit Buffer */
  for( i = 0; i<EBINDEX; i++ ) {
    tmp[col] = EDITBUFFER[i];
    col++;
  }

  /* Add Rest of Chars */
  for( i = BUFFER[thisRow()]->rPtr; i<(int)BUFFER[thisRow()]->len; i++ ) {
    tmp[col] = BUFFER[thisRow()]->txt[i];
    col++;
  }

  tmp[newLen] = '\0';                /* NULL Terminate New String */

  free( BUFFER[thisRow()]->txt );
  BUFFER[thisRow()]->txt  = tmp;
  BUFFER[thisRow()]->len  = newLen;
  BUFFER[thisRow()]->lPtr = 0;
  BUFFER[thisRow()]->rPtr = 0;

  EBINDEX = 0;
}


/*******************************************************************************
                             EDITOR STATE
*******************************************************************************/

/* Cursor Movement Functions */
void updateNavigationState() {

  if( BUFFER[thisRow()]->editP )
    updateLine();
  
  BUFFER[thisRow()]->editP = false;
  
  miniBufferClear();
}


/* Edit Line */
void updateEditState() {

  BUFFER[thisRow()]->editP = true;
  STATUSFLAG = MODIFIED;
}

/*******************************************************************************
                             INSERT CHARS
*******************************************************************************/

/* Insert User Typed Chars */
void selfInsert( int c ) {

  if( BUFFER[thisRow()]->lPtr != BUFFER[thisRow()]->rPtr )
    updateNavigationState();
  
  if( !BUFFER[thisRow()]->editP ) {
    BUFFER[thisRow()]->lPtr = POINT_X;
    BUFFER[thisRow()]->rPtr = POINT_X;
    updateEditState();
  }

  EDITBUFFER[EBINDEX++] = c;
  POINT_X++;
}


/*******************************************************************************
                           LINE MANAGEMENT
*******************************************************************************/

/* Free row_t */
void freeLine() {

  free( BUFFER[thisRow()]->txt );
  free( BUFFER[thisRow()] );
    
  for( int i=thisRow(); i<NUMROWS-1; i++ ) {

    BUFFER[i] = BUFFER[i+1];
  }

  --NUMROWS;
}

/* Kill Line at Point */
void killLine() {

  /* Line Empty - Delete it */
  if( BUFFER[thisRow()]->len == 1 ) {

    if( thisRow() == NUMROWS - 1 ) return; /* Cant Delete if Nothing Follows */

    freeLine();
  }

  /* Delete from POINT to EOL */
  else {                        

    /* Heap Space for Trimmed String */
    char *tmp = malloc( sizeof( char ) * ( POINT_X + 2 ));
    if( POINT_X > 0 )
      strncpy( tmp, BUFFER[thisRow()]->txt, POINT_X );
    free( BUFFER[thisRow()]->txt );

    /* Fix Up row_t For This Row */
    BUFFER[thisRow()]->txt = tmp;
    BUFFER[thisRow()]->len = POINT_X + 1;
    BUFFER[thisRow()]->txt[BUFFER[thisRow()]->len-1] = '\n';
    BUFFER[thisRow()]->txt[BUFFER[thisRow()]->len] = '\0';

    clrtoeol();
  }
}

/* Indent Newline to Smart Location */
void autoIndent() {

  int len;				/* Len of 'last' row */
  
  int i = 0;				/* Col Index */
  int lastRow = thisRow() - 1;

  /* Skip if Middle of Line or Top of Buffer */
  if(( POINT_X != 0 ) || ( thisRow() == 0 ))
    return;

  len = BUFFER[lastRow]->len;

  /* Find Tab Col Prior Row */
  while( BUFFER[lastRow]->txt[i] == ' ' )
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

  /* Check Buffer Size */
  if( NUMROWS == MAXROWS ) {        
    doubleBufferSize();
  }

  /* Move Lines Down */
  int i;
  
  for( i=NUMROWS; i>thisRow()+1; i-- ) {
    BUFFER[i] = BUFFER[i-1];
  }

  /* Create New Line */
  BUFFER[i] = malloc( sizeof( row_t ));
  BUFFER[i]->len = BUFFER[thisRow()]->len - POINT_X;
  BUFFER[i]->txt = malloc( sizeof( char ) * ( BUFFER[i]->len + 1 ));

  /* Copy Text Into New Line */
  if( strncpy( BUFFER[i]->txt, 
               BUFFER[thisRow()]->txt + POINT_X,
               BUFFER[i]->len ) == NULL ) die( "openLine: strncpy failed." );

  BUFFER[i]->txt[BUFFER[i]->len] = '\0';

  /* Heap Space for Trimmed String */
  char *tmp = malloc( sizeof( char ) * ( POINT_X + 2 ));
  if( POINT_X > 0 )
    strncpy( tmp, BUFFER[thisRow()]->txt, POINT_X );
  free( BUFFER[thisRow()]->txt );

  /* Fix Up row_t For This Row */
  BUFFER[thisRow()]->txt = tmp;
  BUFFER[thisRow()]->len = POINT_X + 1;
  BUFFER[thisRow()]->txt[BUFFER[thisRow()]->len-1] = '\n';
  BUFFER[thisRow()]->txt[BUFFER[thisRow()]->len] = '\0';

  clrtoeol();
  
  /* Move Point */
  POINT_X = 0;        
  if( POINT_Y == screenRows() )
    ROWOFFSET++;
  else
    POINT_Y++;
  NUMROWS++;                        /* Increment Num Lines */
}

/*******************************************************************************
                             DELETE CHARS
*******************************************************************************/

void killWord() {

  int thisPoint = POINT_X;        /* Save Current Point */

  forwardWord();                /* Find End Next Word */

  if( POINT_X == thisPoint )        /* No Word to Kill */
    return;

  /* Mark Word for Deletion and Restore Point */
  BUFFER[thisRow()]->lPtr = thisPoint;
  BUFFER[thisRow()]->rPtr = POINT_X;
  POINT_X = thisPoint;
  updateEditState();
  updateNavigationState();
}


void combineLineWithPrior() {

  if( thisRow() == 0 )
    return;

  char *tmp;
        
  /* Allocate Memory for Combined String */
  int preLineLen = BUFFER[thisRow()-1]->len;
  int nxtLineLen = BUFFER[thisRow()]->len;
  int size       = preLineLen + nxtLineLen + 1;

  if(( tmp = malloc( sizeof( char ) * size )) == NULL )
    die( "backspace: tmp malloc failed" );

  /* Copy Old/New Text Into New Line */
  strncpy( tmp, 
           BUFFER[thisRow()-1]->txt,
           preLineLen );
  strncpy( tmp + preLineLen,
           BUFFER[thisRow()]->txt,
           nxtLineLen );
  BUFFER[thisRow()-1]->len = size - 1;
        
  /* Free Old Memory and Set Pointer */
  free( BUFFER[thisRow()-1]->txt );
  BUFFER[thisRow()-1]->txt = tmp;

  /* Destroy Next Line */
  freeLine();

  POINT_Y--;
  POINT_X = preLineLen;
}

void backspace() {

  /* Continue Editing This Line? */
  if( BUFFER[thisRow()]->editP ) {

    /* If Deleting Chars After Inserting Chars? */
    /* Then, Save Insertions Befor Making Deletions */ 
    if( BUFFER[thisRow()]->lPtr == BUFFER[thisRow()]->rPtr ) {
      updateLine();
      BUFFER[thisRow()]->lPtr = POINT_X;
      BUFFER[thisRow()]->rPtr = POINT_X;
    }

    /* Continue Deleting Chars to BOL */
    if( POINT_X > 0 ) {
      BUFFER[thisRow()]->lPtr--;
      POINT_X--;
    }

    /* At BOL, Combine With Prior Line */
    else {
      updateNavigationState();
      combineLineWithPrior();
    }
  }

  /* Begin *New* Edits to This Line */
  else {

    if( POINT_X > 0 ) {
      BUFFER[thisRow()]->lPtr = POINT_X - 1;
      BUFFER[thisRow()]->rPtr = POINT_X;
      updateEditState();
      POINT_X--;
    }

    else {
      combineLineWithPrior();
    }
  }
}


/*******************************************************************************
                           POINT, MARK, and
                          REGION OPERATIONS
*******************************************************************************/

/* Swap Point and Mark */
void swapPointAndMark() {

  int tmpX, tmpY;
  
  if( MARK_X != -1      &&          /* Mark Not Set */
      MARK_Y != -1 ) {
    
    tmpX    = thisCol();            /* Swap Point/Mark */
    tmpY    = thisRow();
    POINT_X = MARK_X;
    POINT_Y = MARK_Y;
    MARK_X  = tmpX;
    MARK_Y  = tmpY;

    if( NUMROWS >= screenRows() ) { /* Scroll to Point Location */

      ROWOFFSET = POINT_Y;
      POINT_Y = 0;
      centerLine();
    }
    miniBufferMessage( "Mark Set" );
  }
}


/* Kill Text Between Point and Mark */
void killRegion() {

  /* Swap Point/Mark */
  if( MARK_Y < POINT_Y )
    swapPointAndMark();

  /* Kill From POINT to EOL */
  if( POINT_Y < MARK_Y && POINT_X > 0 ) {
    
    updateEditState();
    killLine();
    updateNavigationState();
    nextLine();

    POINT_X = 0;
  }

  /* Kill Lines Between POINT/MARK */
  while( POINT_Y < MARK_Y ) {

    freeLine();
    MARK_Y--;
  }
  
  /* Kill Last Line Up to MARK */
  if( POINT_Y == MARK_Y ) {

    if( POINT_X < MARK_X ) {
      swapPointAndMark();
    }
    
    BUFFER[thisRow()]->lPtr = MARK_X;
    BUFFER[thisRow()]->rPtr = POINT_X;
    updateLine();
  }
  
  POINT_X = 0;
  backspace();

  MARK_X  = -1;
  MARK_Y  = -1;
  REGIONP = false;
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
    POINT_X = 0;
    POINT_Y = 0;
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
    closeBuffer();
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
    closeBuffer();
    /* Open New Buffer */
    miniBufferGetFilename( FILENAME, FNLENGTH );
    openBuffer( FILENAME );
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

  case CTRL_KEY('x'):                /* Forward Word */
    updateNavigationState();
    swapPointAndMark();
    break;

  }  
}
  

/* Process Keypresses */
void processKeypress() {

  int c = readKey();
  
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
    REGIONP = false;
    MARK_X  = -1;
    MARK_Y  = -1;
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
    POINT_X = 0;
    POINT_Y = 0;
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
    POINT_X = 0;
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
    MARK_X = thisCol();
    MARK_Y = thisRow();
    REGIONP = true;
    miniBufferMessage( "Mark Set" );
    break;

    /* Edit Text */
  case CTRL_KEY('d'):                /* Delete Char */
  case KEY_DC:

    /* Continue Editing This Line? */
    if( BUFFER[thisRow()]->editP ) {

      /* Start Deleting Chars After Inserting Chars? */
      if( BUFFER[thisRow()]->lPtr == BUFFER[thisRow()]->rPtr ) {
        updateLine();
        BUFFER[thisRow()]->lPtr = POINT_X;
        BUFFER[thisRow()]->rPtr = POINT_X;
      }
        
      /* Continue Deleting Chars Up to EOL  */
      if( BUFFER[thisRow()]->rPtr < BUFFER[thisRow()]->len )
        BUFFER[thisRow()]->rPtr++;
    }
    
    /* *New* Edit To This Line? */
    else {
      if( (size_t)POINT_X < BUFFER[thisRow()]->len - 1 ) {
        BUFFER[thisRow()]->lPtr = POINT_X;
        BUFFER[thisRow()]->rPtr = POINT_X+1;
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
  renderText();
}



/* Editor Polling Loop */
int main( int argc, char *argv[] ) {

  /* Initialize */
  initializeTerminal();
  initializeData();
  
  /* Open File or Display Splash */
  if( argc > 1 ) {
    openBuffer( argv[argc-1] );
  }
  else {
    emptyBuffer( DEFAULT );
    displaySplash();
  }
  
  /* Process Key Presses */
  while( true ) {
    renderText();
    processKeypress();
  } 

  /* Shutdown */
  closeEditor();
  return EXIT_SUCCESS;
} 


/***
    Local Variables:
    mode: c
    comment-column: 40
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
