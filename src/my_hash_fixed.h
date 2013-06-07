// *** The following defines the data structure for the values stored in the hash

typedef void * valStruct; // Use a void pointer when the values in the hash are complex types 

//	typedef unsigned long valStruct; // Use something like this for storing simple types 

// ********************

typedef struct EntryType *Entry;
struct EntryType 
	{
	char * key;
	Entry  next;
	valStruct val;
	}; 

typedef struct HashType *Hash;
struct HashType
	{
	Entry *b; // points to first element of array b, which is an array of entry pointers themselves	
	unsigned long bSize; // size of the hash table 
	unsigned long numElements; 
	};
//char*   DupStr(char* s);
valStruct insert(Hash h, char *key, valStruct val);
void printHash(Hash h);
void  printEntry(Entry e);
Entry newEntry(char *key, valStruct val);
Hash newHash(unsigned long size);
Entry exists(Hash h, char *Key);
unsigned long hash(char *key);


