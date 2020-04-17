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
#include <stdbool.h>

//#include <stdio.h>

#include "minibuffer.h"
#include "ae.h"

/* Private State Data */
bool REGIONP   = false;			/* Is Region Active? */

/* Private Module Data */
static int POINT_X    =  0;			/* Point X Position */
static int POINT_Y    =  0;			/* Point Y Position */
static int MARK_X     = -1;			/* Mark X Position */
static int MARK_Y     = -1;			/* Mark Y Position */


/* Gett'rs and Sett'rs for POINT */
int getPointX( void ) {
  return POINT_X;
}
int getPointY( void ) {
  return POINT_Y;
}
void setPointX( int X ) {
  POINT_X = X;
}
void setPointY( int Y ) {
  POINT_Y = Y;
}

/* Gett'rs and Sett'rs for MARK */
int getMarkX( void ) {
  return MARK_X;
}
int getMarkY( void ) {
  return MARK_Y;
}
void setMarkX( int X ) {
  MARK_X = X;
}
void setMarkY( int Y ) {
  MARK_Y = Y;
}


/*** Swap Point and Mark 

     Return TRUE if swapped
***/
bool swapPointAndMark( void ) {

  /* Get Offsets */
  int clOffst = getColOffset();
  int rwOffst = getRowOffset();
  
  /* Calculate row/col offsets for POINT */
  int tmpX = clOffst + POINT_X;
  int tmpY = rwOffst + POINT_Y;

  if( MARK_X != -1      &&	/* Return if Mark Not Set */
      MARK_Y != -1 ) {
    
    POINT_X = MARK_X - clOffst;
    POINT_Y = MARK_Y - rwOffst;
    MARK_X = tmpX;
    MARK_Y = tmpY;

    //miniBufferMessage( "Mark Set" );

    return true;
  }

  return false;
}


/***
			  Region Operations
***/
void setRegionActive( bool activeP ) {

  REGIONP = activeP;
}

bool regionActiveP( void ) {
  
  return REGIONP;
}

/* Is (Row,Col) Within Active Region? */
bool inRegionP( int row, int col ) {

  /* Get Editor Offsets */
  int rowOffset = getRowOffset();
  int colOffset = getColOffset();

  int thisRow   = rowOffset + POINT_Y;
  int thisCol   = colOffset + POINT_X;
  
  /* Region Not Active */
  if( !REGIONP ) return false;        

  /* This Row on MARK Line */
  if( row == MARK_Y ) {

    if( col >= MARK_X ) {

      if( thisRow > row )
        return true;
    
      else if( col < thisCol && thisRow == MARK_Y )
        return true;
    }

    if( col <= MARK_X ) {

      if( thisRow < row )
        return true;

      else if( col > thisCol && thisRow == MARK_Y )
        return true;
    }
  }

  /* POINT_Y > thisRow() > MARK_Y */
  else if( row > MARK_Y ) {

    if( row < thisRow )
      return true;

    else if( row == thisRow && col < thisCol )
      return true;
  }

  /* POINT_Y < thisRow() < MARK_Y */
  else if( row < MARK_Y ) {

    if( row > thisRow )
      return true;

    else if( row == thisRow && col > thisCol )
      return true;
  }
  
  return false;
}

