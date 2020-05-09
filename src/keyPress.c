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

#include <stdio.h>

#include "ae.h"
#include "window.h"

/*******************************************************************************
			    READ KEY INPUT
*******************************************************************************/

/* Read Keypresses */
int readKey() {

  int c;                        /* 'Char' or Flags */

  /* wgetch handles SIGWINCH */
  while(( c = wgetch( getWindowHandle() )) == ERR ) {

    /* Handle Timeouts */
    refresh();
  }
  
  return c;
}


/***
    Local Variables:
    mode: c
    tags-file-name: "~/ae/TAGS"
    comment-column: 45
    fill-column: 90
    End:
 ***/
