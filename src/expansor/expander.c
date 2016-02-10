#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "expander.h"

void createExpander(int argc, char *argv[], Expander *expander){
	int i, j;

	// Inicializa vetor para guardar instruções de macro
	for(i=0; i<TABLE_SIZE; i++){
		for(j=0; j<INSTRUCTION_SIZE; j++){
			expander->macroInstructions[i][j] = NULL_CHAR;
		}
	}

	// Inicializa as informações de uma macro
	for(i=0; i<MACRO_SIZE; i++){
		expander->macro[i].begin = NULL_INT;
		expander->macro[i].end = NULL_INT;
		expander->macro[i].macroSize = NULL_INT;

		for(j=0; j<INSTRUCTION_SIZE; j++){
			expander->macro[i].macroName[j] = NULL_CHAR;
			expander->macro[i].parameter[j] = NULL_CHAR;
		}
	}
}

int isLabel(char aux[]){
	int lastCharPosition = strlen(aux) - 1;

	// Procura por ":" no final da string
    if(aux[lastCharPosition] == ':'){
        aux[lastCharPosition] = '\0';
        return TRUE;
    } else {
        return FALSE;
    }
}

int isBeginMacro(char aux[]){
	// Procura por ":" no final da string e BEGINMACRO depois
    if(!strcasecmp(aux, "BEGINMACRO")){
        return TRUE;
    } else {
        return FALSE;
    }
}

int isEndMacro(char aux[]){
	// Verifica se instrução é ENDMACRO
    if(!strcasecmp(aux, "ENDMACRO")){
        return TRUE;
    } else {
        return FALSE;
    }
}

int isBreakLine(char aux[]){
	// Verifica se instrução é ENDMACRO
    if(!strcasecmp(aux, "\n") || !strcasecmp(aux, "")){
        return TRUE;
    } else {
        return FALSE;
    }	
}

void removeComments(char aux[]){
    int i;
    // Remove comentários das linhas
    for (i=0; i<INSTRUCTION_SIZE; i++){
    	if(aux[i] == ';'){
    	    aux[i] = '\n';
    	    aux[i+1] = '\0';
    	    break;
    	}
    }
}

void getMacroInfo(Expander *expander, char input[]){
	FILE *inputProgram;
	char line[INSTRUCTION_SIZE], aux[INSTRUCTION_SIZE], macro[INSTRUCTION_SIZE];
	int isMacro = FALSE;
	int macroId = 0, macroInstructionCounter = 0;

	// Abre o arquivo
	inputProgram = fopen(input, "r");

	// Read file until reach its end
	while(!feof(inputProgram)){
		// Lê a linha
        fgets(line, sizeof(line), inputProgram);

        // Pega instrução/label
    	strcpy(aux, "");
    	sscanf(line,"%s", aux);

    	if(isBreakLine(aux)){
			continue;
		}

		// Se for ENDMACRO
		if(isEndMacro(aux)){
			isMacro = FALSE;
			expander->macro[macroId].end = macroInstructionCounter;
			expander->numberOfMacros = macroId + 1;
			macroId++;
		}
		
		// Se estiver lendo instruções da macro
		if(isMacro){
			// Armazena instrução e atualiza o contador de instruções
			removeComments(line);
			strcpy(expander->macroInstructions[macroInstructionCounter], line);
			macroInstructionCounter++;
		}

		// Se for label
		if(isLabel(aux)){
			// Lê instrução
			sscanf(line, "%*s %s", macro);
			
			// Se for BEGINMACRO
			if(isBeginMacro(macro)){
				isMacro = TRUE;

				// Seta o nome e posição inicial das instruções da macro
				expander->macro[macroId].begin = macroInstructionCounter;
				strcpy(expander->macro[macroId].macroName, aux);

				// Seta o parâmetro da macro
				strcpy(macro, "");
				sscanf(line, "%*s %*s %s", macro);
				strcpy(expander->macro[macroId].parameter, macro);
			}
		}
	}

	// Fecha o programa
	fclose(inputProgram);
}

int isMacroCall(Expander *expander, char aux[]){
	int i;

	// Verifica se é chamada de macro
	for (i = 0; i < expander->numberOfMacros; i++){
		if(!strcasecmp(aux, expander->macro[i].macroName)){
			return i;
		}
	}

	return NULL_INT;
}

int getNumberOfOperands(char operation[]){
	// Instruções de 2 operandos
    if( !strcasecmp(operation, "COPY") || !strcasecmp(operation, "LOAD") ||
        !strcasecmp(operation, "STORE") || !strcasecmp(operation, "ADD") ||
        !strcasecmp(operation, "SUB") || !strcasecmp(operation, "AND") || 
        !strcasecmp(operation, "OR")  || !strcasecmp(operation, "XOR")){
        return 2;
    } else if( 
    	// Instruções de 1 operando
    	!strcasecmp(operation, "READ") || !strcasecmp(operation, "WRITE") ||
    	!strcasecmp(operation, "NOT") || !strcasecmp(operation, "PUSH") ||
        !strcasecmp(operation, "NEG") || !strcasecmp(operation, "JMP") ||
        !strcasecmp(operation, "JZ") || !strcasecmp(operation, "JNZ") ||
        !strcasecmp(operation, "JN") || !strcasecmp(operation, "JNN")  ||
        !strcasecmp(operation, "POP") || !strcasecmp(operation, "CALL") ){
        return 1;
    } else {
    	// HALT ou RET -- Sem operando
        return 0;
    }
}

void replaceParameter(char macro[], char aux[], char toReplace[], char parameter[]){
	// Se encontrar o parâmetro, substitui
	strcat(macro, " ");
	if(!strcasecmp(aux, toReplace)){
		strcat(macro, parameter);
	} else {
		strcat(macro, aux);
	}
}

void expandMacros(Expander *expander, char input[], char output[]){
	FILE *inputProgram, *outputProgram;
	char line[INSTRUCTION_SIZE], aux[INSTRUCTION_SIZE], macro[INSTRUCTION_SIZE], macroParameter[INSTRUCTION_SIZE];
	int macroId, begin, end, isMacroDefinition = FALSE;

	// Abre um arquivo para leitura e um para saída
	inputProgram = fopen(input, "r");
    outputProgram = fopen(output, "w+");    
    
    // Lê até o fim
    while(!feof(inputProgram)){
    	// Remove comentários
    	fgets(line, sizeof(line), inputProgram);
	    removeComments(line);

    	// Pega instrução/label
    	strcpy(aux, "");
    	sscanf(line,"%s", aux);

    	if(isBreakLine(aux)){
			continue;
		}

    	// Se for fim de macro
    	if(isEndMacro(aux)){
    		isMacroDefinition=FALSE;
    		continue;
    	}

    	// Se for instrução dentro de macro, volta
    	if(isMacroDefinition){
    		continue;
    	}

    	// Se for label
    	if(isLabel(aux)){
            sscanf(line, "%*s %s", aux);

            // Se for declaração de macro
            if(isBeginMacro(aux)){
            	isMacroDefinition=TRUE;
            	continue;
            }
    	}

    	// Verifica se é chamada de macro
    	macroId = isMacroCall(expander, aux);
		if(macroId!=NULL_INT){
			// Seta begin e end
			begin = expander->macro[macroId].begin;
			end = expander->macro[macroId].end;

			// Lê instrução e verifica se é label
	    	sscanf(line, "%s", aux);

	    	// Chamada da macro por label externa
	    	if(isLabel(aux)){
				// Concatena label em instrução
				sscanf(line, "%*s %*s %s", macroParameter);
				strcat(aux, ": ");
				strcat(aux, expander->macroInstructions[begin]);
				strcpy(line, aux);

				// Seta macro
				sscanf(line, "%s", aux);
	    		strcpy(macro, aux);
	    		strcat(macro, " ");
	    		sscanf(line, "%*s %s", aux);
	    		strcat(macro, aux);

	    		// Pega instrução
				sscanf(line, "%*s %s", aux);

				// Pelo número de operandos, troca os parâmetros caso haja
		    	if(getNumberOfOperands(aux) == 1){
		    		sscanf(line,"%*s %*s %s", aux);
		    		replaceParameter(macro, aux, expander->macro[macroId].parameter, macroParameter);
		    	} else if(getNumberOfOperands(aux) == 2) {
		    		sscanf(line,"%*s %*s %s", aux);
		    		replaceParameter(macro, aux, expander->macro[macroId].parameter, macroParameter);
		    		sscanf(line,"%*s %*s %*s %s", aux);
		    		replaceParameter(macro, aux, expander->macro[macroId].parameter, macroParameter);
		    	}

		    	// Quebra linha
		    	strcat(macro, "\n");

		    	// Imprime instrução
		    	fprintf(outputProgram, "%s", macro);
		    	strcpy(macro, "");

		    	// Incrementa contador
		    	begin++;
			} else {
				// Lê parâmetro da macro
				sscanf(line, "%*s %s", macroParameter);
			}

			// Escreve instruções armazenadas para a macro
			writeInstructions(expander, outputProgram, macroId, begin, end, macroParameter);

			// Volta no início do loop while
			continue;
		}

		// Imprime instrução
		fprintf(outputProgram, "%s", line);
	}
}

void writeInstructions(Expander *expander, FILE *outputProgram, int macroId, int begin, int end, char macroParameter[]){
	char line[INSTRUCTION_SIZE], aux[INSTRUCTION_SIZE], macro[INSTRUCTION_SIZE], tempMacroParameter[INSTRUCTION_SIZE];
	char token[INSTRUCTION_SIZE];
	int tempMacroId;
	strcpy(macro, "");

	// Enquanto houver instruções
	while(begin<end){
		// Copia a instrução pra line
		strcpy(macro, "");
		strcpy(line, expander->macroInstructions[begin]);
		sscanf(line, "%s", aux);

		// Seta macro com a primeira instrução
		strcat(macro, aux);

		if(isLabel(aux)){
			strcat(macro, " ");
			sscanf(line, "%*s %s", aux);
			strcpy(token, aux);

			// Pelo número de operandos, troca os parâmetros caso haja
			if(getNumberOfOperands(aux) == 1){
	    		sscanf(line,"%*s %*s %s", aux);
	    		strcat(token, " ");
	    		strcat(token, aux);
	    	} else if(getNumberOfOperands(aux) == 2) {
	    		sscanf(line,"%*s %*s %s", aux);
	    		strcat(token, " ");
	    		strcat(token, aux);
	    		sscanf(line,"%*s %*s %*s %s", aux);
	    		strcat(token, " ");
	    		strcat(token, aux);
	    	} else if(isMacroCall(expander, aux)){
	    		sscanf(line,"%*s %*s %s", aux);
	    		strcat(token, " ");
	    		strcat(token, aux);
	    	}

	    	strcpy(line, token);
	    	sscanf(line, "%s", aux);
			strcat(macro, aux);
		}

		// Se for chamada de macro
		tempMacroId = isMacroCall(expander, aux);
		if(tempMacroId!=NULL_INT){
			sscanf(line, "%*s %s", tempMacroParameter);
			// Faz chamada recursiva e expande a macro
			writeInstructions(expander, outputProgram, tempMacroId, expander->macro[tempMacroId].begin, expander->macro[tempMacroId].end, tempMacroParameter);
			begin++;
			continue;
		}

		// Pelo número de operandos, troca os parâmetros caso haja
		if(getNumberOfOperands(aux) == 1){
    		sscanf(line,"%*s %s", aux);
    		replaceParameter(macro, aux, expander->macro[macroId].parameter, macroParameter);
    	} else if(getNumberOfOperands(aux) == 2) {
    		sscanf(line,"%*s %s", aux);
    		replaceParameter(macro, aux, expander->macro[macroId].parameter, macroParameter);
    		sscanf(line,"%*s %*s %s", aux);
    		replaceParameter(macro, aux, expander->macro[macroId].parameter, macroParameter);
    	}

    	// Imprime instrução
    	strcat(macro, "\n");
    	fprintf(outputProgram, "%s", macro);

    	strcpy(macro, "");

		begin++;
	}
}

