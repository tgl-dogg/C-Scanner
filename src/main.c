#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <ctype.h>

#define STR_TOKEN_CODE " => tokenCode = "
#define STR_VALUE ", value = "
#define CAN_READ_FILE (x = fgetc(input)) != EOF

#define TOKEN_CODE_INT 40
#define TOKEN_CODE_FLOAT 41
#define TOKEN_CODE_STRING 42
#define TOKEN_CODE_VARIABLE 69

#define TOKEN_MAX_SIZE 65535

#define true 1
#define false 0

int fpeek(); /* Retorna o primeiro char do arquivo de entrada sem removê-lo do buffer. */
int get_token_code(char *key); /* Retorna o tokenCode definido para a palavra reservada ou símbolo, ou devolve -1 para variáveis. */

typedef int bool;

/* Associa uma palavra reservada a um tokenCode. */
struct mapping {
    char *keyword;
    int token_code;
};

/* Cria um vetor de mapeamentos para a definição da linguagem. */
struct mapping mappings[] = {
	// Palavras reservadas
    "and", 1,
    "do", 2,
    "else", 3,
    "if", 4,
    "for", 5,
    "new", 6,
    "nil", 7,
    "not", 8,
    "or", 9,
    "return", 10,
    "void", 11,
    "while", 12,

	// Tipos
	"string", 15,
    "float", 16,
    "int", 17,

    // Símbolos
	"+", 20,
	"-", 21,
	"*", 22,
	"/", 23,
	"=", 24,
	"(", 25,
	")", 26,
	"[", 27,
	"]", 28,
	"{", 29,
	"}", 30,
	"<", 31,
	">", 32,
	"<=", 33,
	">=", 34,
	"!=", 35,
	",", 36,
	";", 37,
	".", 38,

    0, TOKEN_CODE_VARIABLE
};

FILE *input;
FILE *output;

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printf("Poucos argumentos! Insira o nome do arquivo de entrada e do arquivo de saida.\n");
		return 1;
	}

	input = fopen(argv[1], "r");
	output = fopen(argv[2], "w+");

	if (!input) {
		printf("Nao foi possivel abrir o arquivo de entrada!\n");
		return 1;
	}

	if (!output) {
		printf("Nao foi possivel criar o arquivo de saida!\n");
		return 1;
	}

	fclose(input);
	fclose(output);
	return 0;
}

int fpeek() {
    int x = fgetc(input);
    ungetc(x, input);

    return x;
}

int get_token_code(char *key) {
    int i = 0;
    char *name = mappings[i].keyword;

    // Itera sobre a definição de mapeamento criada no vetor.
    while (name) {
    	// Pára quando encontra uma palavra reservada pré-definida.
    	if (strcmp(name, key) == 0) {
    		break;
        }

    	name = mappings[++i].keyword;
    }

    // Se não encontrar nada, assumimos que deve ser uma variável.
    return mappings[i].token_code;
}