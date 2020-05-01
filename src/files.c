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
#include <menu.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "keyPress.h"
#include "minibuffer.h"
#include "ae.h"
#include "files.h"

#define DEFAULTFILENAME "newfile.txt"


static char FILENAME[FNLENGTH];		     /* Buffer Filename */


/*****************************************************************************************
				 GET/SET BUFFER FILENAME
*****************************************************************************************/

/* Set un-Named File to Default Filename */
void setDefaultFilename( void ) {

  strncpy( FILENAME, DEFAULTFILENAME, FNLENGTH );
  return;
}

void setFilename( const char *fn ) {

  if( fn != FILENAME )
    strncpy( FILENAME, fn, FNLENGTH-1 );
}

char *getBufferFilename( void ) {

  return FILENAME;
}


/*****************************************************************************************
				    READ FILENAME MENU
*****************************************************************************************/

int _getMenuChoice( MENU *menu ) {

  miniBufferMessage( "Choose a filename or c-g to escape" );
  
  while(1) {

    switch(readKey()) {

    case KEY_DOWN:
      menu_driver( menu, REQ_DOWN_ITEM );
      break;

    case KEY_UP:
      menu_driver( menu, REQ_UP_ITEM );
      break;

    case 27:				     /* Escape */
    case ( (int)'g' & 0x1f ):		     /* Ctrl-g */
      return -1;
      break;

    case (int)'\r':                        /* Enter Key */      
      return item_index( current_item( menu ));
      break;
    }
  }

  return -1;
}


static void _checkFileOrDirectory( void ) {

  struct stat sb;

  /* Check To See if Selected FILENAME is Actually a DIR */
  if( stat( FILENAME, &sb ) == -1 ) {
    miniBufferMessage( "WARNING: Cannot stat selected filename" );
  }

  /* If FN is a DIR, Chose File in DIR */
  if( S_ISDIR( sb.st_mode )) {

    if( chdir( FILENAME ) == ERR ) {
      die( "Cannot Chdir" );
    }
      
    openFile();
  }

  return;
}


void openFile( void ) {

  int i, choice, countFiles;
  ITEM **items;
  MENU *menu;
  
  struct dirent *entry;
  DIR *dp;

  if(( dp = opendir( "." )) == NULL )
    die( "Cannot open current directory" );

  /* Count Entries in Dir */
  countFiles = 0;
  while(( entry = readdir(dp) )) {
    countFiles++;
  }
  rewinddir( dp );

  /* Allocate NULL-initialized Heap Space for Item Array */
  items = (ITEM **)calloc( countFiles+1, sizeof( ITEM * ));

  // Create NEW File Option with line break...
  
  /* Save Each Menu Item Into Menu Structure */
  for( i=0; i<countFiles; i++ ) {
    entry = readdir( dp );
    items[i] = new_item( entry->d_name, NULL ); // entry->d_name );
  }
  items[countFiles] = (ITEM *)NULL;

  /* Create the Menu */
  menu = new_menu( (ITEM **)items );
  set_menu_mark( menu, "->" );

  // Decoreate Menu
  // http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/menus.html
  
  /* Display it */
  post_menu( menu );
  refresh();

  /* Read Menu Inputs */
  choice = _getMenuChoice( menu );

  setFilename( item_name( items[choice] ));
  
  /* Free Menu Memory */
  for( i=0; i<=countFiles; i++ ) {
    free_item( items[i] );
  }
  free_menu( menu );

  closedir( dp );

  clear();
  
  _checkFileOrDirectory();
}


/***
    Local Variables:
    mode: c
    comment-column: 45
    fill-column: 90
    End:
 ***/
