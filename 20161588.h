#define MAX_BUFF 1048575
typedef struct cnode{
    int commandCnt;
    char commandName[100];
    struct cnode *next;
}historyNode;
typedef struct bitstruct{
	int n;
	int i;
	int b;
	int x;
	int p;
	int e;
}bits;
typedef struct hnode{
	char mnemonic[10];
	char form[10];
	int opcode;
	struct hnode *next;
}hashNode;
void file_remove(char* filename,int flag);
void int_swap(int *a, int *b);
void str_swap(char **a, char **b);
int find_symbol(char *symbol);
void bit_init(bits *bit);
int assembler(char *filename);
int pass1(FILE *filename);
int pass2(char *filename);
void hash_print();
int hash_search(char mnemonic[10],int print_flag);
int hash_function(char mnemonic[10]);
void hash_push(int opcode, char mnemonic[10],char form[10]);
void print_dump(int start, int end);
int hex_check(char* token1,int *errorFlag);
char* nomalization(char command[100]);
void node_init(historyNode *node);
historyNode* command_push(char command[100], historyNode *tail);
void print_history(historyNode *node);
void print_help();
void print_dir();
