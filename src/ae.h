#include <curses.h>

#define FNLENGTH 128

/* Text Line Data Structures */
typedef struct {
  char  *txt;				/* Editor Text Line */
  size_t len;				/* Length of Text */
  size_t lPtr;				/* Editor Pointers */
  size_t rPtr;
  bool   editP;				/* This Row Edited Predicate */
} row_t;

/* Public Definitions */
WINDOW *getWindowHandle( void ); /* Get Handle for AE Window */
void die( const char * );	 /* Print Die Message on Failure */
void initializeTerminal( void ); /* (re)Initialize Editor Screen */
int getRowOffset( void );
int getColOffset( void );
void setRowOffset( int );
void setColOffset( int );
void setNumRows( int );
int getBufferNumRows( void );

int getEditBufferIndex( void );
char getEditBufferChar(int);
void setEditBufferIndex( int );
void setStatusFlagOriginal( void );

void setDefaultFilename( void );
void setFilename( char * );
char *getBufferFilename( void );
