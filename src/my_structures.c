#include "my_structures.h"
#include <stdlib.h>
#include <string.h>

char*   DupStr(char* s)
{
char* sNew;
sNew=(char*)malloc((strlen(s)+1)*sizeof(char));
strcpy(sNew,s);
return sNew;
}
