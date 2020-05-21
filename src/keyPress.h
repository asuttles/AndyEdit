/* Public Macros */
#define CTRL_KEY(k) ((k) & 0x1f)
#define ALT_KEY 27


/* Public Declarations */
int readKey( void );		/* Read Key Press */
void metaMenu( void );		/* Handle Meta Menu Inputs */
void eXtensionMenu( void );	/* Handle Extension Menu Inputs */
void processKeypress( void );	/* Handle Keyboard Input */
