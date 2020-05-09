#include <curses.h>
#include <stdbool.h>

#define NORMAL_BACKGROUND 0
#define HIGHLT_BACKGROUND 1



/* Text Line Data Structures */
typedef struct {
  char  *txt;				/* Editor Text Line */
  size_t len;				/* Length of Text */
  size_t lPtr;				/* Editor Pointers */
  size_t rPtr;
  bool   editP;				/* This Row Edited Predicate */
} row_t;

/* Public Definitions */
void die( const char * );	 /* Print Die Message on Failure */

int getEditBufferIndex( void );
char getEditBufferChar(int);
void setEditBufferIndex( int );

//
void combineLineWithPrior( void );
