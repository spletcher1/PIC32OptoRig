#include "GlobalIncludes.h"

unsigned char dfmID;
unsigned char isInDarkMode;
unsigned char ledStatusBits;
unsigned char usingNewPortOnly;

void InitializeBoard(){
    // Set All ID Selectors    
    HEARTBEAT_TRIS = 0;
    HEARTBEAT_PORT = 0; 
    SIGNALLED_TRIS=0;
    SIGNALLED_PORT=0;
    USERBUTTON1_PORT=0;
    USERBUTTON2_PORT=0;
    USERBUTTON1_TRIS=1;
    USERBUTTON2_TRIS=1;    
    TRIGGER_TRIS = 1;
    SUPRESSOR_TRIS=1;      
    INDICATOR_TRIS=0;     
}

