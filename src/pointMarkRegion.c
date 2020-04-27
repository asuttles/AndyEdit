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

// TODO
// Remove references to buff_t
// Update gettr and settr functions in buffer.c to elim references to buff_t here.
//

#include <stdbool.h>

#include "minibuffer.h"
#include "ae.h"
#include "buffer.h"

/* Private State Data */
bool REGIONP   = false;			/* Is Region Active? */

/* Private Module Data */
static int POINT_X    =  0;			/* Point X Position */
static int POINT_Y    =  0;			/* Point Y Position */
static int MARK_X     = -1;			/* Mark X Position */
static int MARK_Y     = -1;			/* Mark Y Position */



/*******************************************************************************
			    POINT AND MARK
*******************************************************************************/

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


/* Swap Point and Mark */
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

    return true;
  }

  return false;
}

/*******************************************************************************
			     REGION STATE
*******************************************************************************/
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


/*******************************************************************************
			     KILL REGION
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



/***
    Local Variables:
    mode: c
    comment-column: 45
    fill-column: 90
    End:
 ***/
