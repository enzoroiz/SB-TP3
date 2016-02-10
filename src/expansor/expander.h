#define TABLE_SIZE 200
#define INSTRUCTION_SIZE 50
#define MACRO_SIZE 50
#define NULL_INT -1
#define NULL_CHAR '\0'
#define TRUE 1
#define FALSE 0

typedef struct Macro {
	char macroName[INSTRUCTION_SIZE]; // Nome da macro
	char parameter[INSTRUCTION_SIZE]; // Parâmetro da macro
	int begin; // Começo no vetor de instruções de macro
	int end; // Final no vetor de instruções de macro
	int macroSize; // Tamanho total da macro: end - begin
} Macro;

typedef struct Expander {
	char macroInstructions[TABLE_SIZE][INSTRUCTION_SIZE]; // Armazena instruções de uma Macro
	Macro macro[MACRO_SIZE]; // Armazena informações das macros
	int numberOfMacros; // Numero de macros
} Expander;

void createExpander(int argc, char *argv[], Expander *expander); // Inicializa montador com argumentos passados por linha de comando

int isLabel(char aux[]); // Verifica se string é label

int isBeginMacro(char aux[]); // Verifica se é inicio de declaração de macro

int isEndMacro(char aux[]); // Verifica se é fim de declaração de macro

int isBreakLine(char aux[]); // Verifica se é quebra de linha ou linha vazia

void removeComments(char aux[]); // Remove os comentários da linha

void getMacroInfo(Expander *expander, char input[]); // Pega as macros declaradas e armazena

int isMacroCall(Expander *expander, char aux[]); // Verifica se é chamada de macro

int getNumberOfOperands(char operation[]); // Retorna o número de operandos de uma operação

void replaceParameter(char macro[], char aux[], char toReplace[], char parameter[]); // Troca o parâmetro passado pela macro

void expandMacros(Expander *expander, char input[], char output[]); // Expande as macros chamadas

void writeInstructions(Expander *expander, FILE *outputProgram, int macroId, int begin, int end, char macroParameter[]); // Escreve as instruções das macros expandidas