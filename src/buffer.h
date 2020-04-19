/* Buffer Pointer Type */
typedef row_t** buff_t;

/* Empty Buffer : Default or User Named */
enum _bn { DEFAULT, UNAMED };

/* Buffer Management */
void initializeBuffer( void );
buff_t getBufferHandle( void );
void openEmptyBuffer( enum _bn );
void openBufferFile( char * );
void saveBuffer( void );
void saveBufferNewName( void );
void doubleBufferSize( void );
bool bufferFullP( void );
void closeBuffer( void );

/* Buffer Information */
int getBufferLineLen( int ); 
char getBufferChar( int, int );
bool bufferRowEditedP( int );
