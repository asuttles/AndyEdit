#include <curses.h>

/* Text Line Data Structures */
typedef struct {
  char  *txt;				/* Editor Text Line */
  size_t len;				/* Length of Text */
  size_t lPtr;				/* Editor Pointers */
  size_t rPtr;
  bool   editP;				/* This Row Edited Predicate */
} row_t;

typedef row_t** buff_t;

/* Public Definitions */
WINDOW *getWindowHandle( void ); /* Get Handle for AE Window */
void die( const char * );	 /* Print Die Message on Failure */
void initializeTerminal( void ); /* (re)Initialize Editor Screen */
int getRowOffset( void );
int getColOffset( void );
void setRowOffset( int );

buff_t getBuffer( void );
int getEditBufferIndex();
char getEditBufferChar(int);
