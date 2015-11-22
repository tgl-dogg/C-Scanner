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
int get_number(char c); /* Remove um número inteiro ou real de até 16 dígitos do arquivo de entrada. */
int get_string(char c); /* Remove uma string do arquivo de entrada que esteja envolta de aspas simples ou duplas. */
int remove_comment(char c); /* Remove comentários do arquivo de entrada que estejam no formato especificado. */
int get_token_code(char *key); /* Retorna o tokenCode definido para a palavra reservada ou símbolo, ou devolve -1 para variáveis. */
int get_word(char c, char str[100]); /* Retorna uma palavra reservada, símbolo ou nome de variável/função do arquivo de entrada. */

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

int remove_comment(char c) {
	int x;

	// Verifica se começa com '/'
	if (c != '/') {
		return 0;
	}

	x = fpeek(input);
	c = (char) x;

	// Verifica se é o fim do arquivo
	if (x == EOF) {
		return 0;
	}

	// Verificamos se realmente começa com "/*"
	if (c == '*') {
		// Consome o caractere '*' do comentario.
		fgetc(input);

		// Começou um comentário, vamos consumí-lo até o fim dele ou do arquivo.
		while (CAN_READ_FILE) {
			c = (char) x;

			// Testa se é o fim do comentário, isto é, se é do formato "*/"
			if (c == '*' && ((char) fpeek(input)) == '/') {
				// Consome o caractere '/' que lemos com fpeek;
				x = fgetc(input);
				return 1;
			}
		}

		// WARNING: comentário aberto porém não fechado.
		fputs(" -- [malformed commentary]", output);
		return -1;
	}

	return 0;	
}

int get_string(char c) {
	int x, i = 0, size = 100;
	char last_char = c;
	bool is_simple = (c == '\''); // Indica se a string começa com aspas simples.
	bool is_double = (c == '\"'); // Indica se a string começa com aspas duplas.
	bool is_scaped = false; // Permite ignorar caracteres escapados, como \" ou \' que não podem terminar a string.
	char *str_buffer = (char *) malloc(size * sizeof(char));

	// Verifica se é uma string mesmo.
	if (!is_simple && !is_double) {
		return 0;
	}

	// Coloca a abertura de aspas no arquivo de saída.
	fputc(c, output);

	while (CAN_READ_FILE) {
		c = (char) x;
		fputc(c, output);
		
		// Realoca o buffer para a string.
		if (i >= (size-1)) {
			size += 100;
			str_buffer = (char *) realloc(str_buffer, size * sizeof(char));
		}		

		// Verifica se o próximo caractere está escapado, como \" ou \'.
		is_scaped = (last_char == '\\');

		// Verifica se a String está acabando exatamente como começou, aspas simples ou duplas.
		if (!is_scaped && ((is_simple && c == '\'') || (is_double && c == '\"'))) {
			str_buffer[i] = '\0';

			// Coloca o tokenCode e value da string no arquivo de saída.
			fprintf(output, STR_TOKEN_CODE);
			fprintf(output, "%d", TOKEN_CODE_STRING);
			fprintf(output, STR_VALUE);
			fputs(str_buffer, output);
			
			// Pula linha no arquivo de saída.
			fputc('\n', output);

			free(str_buffer);
			return 1;
		}

		/* Só coloca a string no buffer após verificar se é o fim dela,
		assim evitar colocar as aspas finais na representação do value. */
		str_buffer[i++] = c;

		// Se o último caractere foi escapado, então ele não é considerado na próxima iteração.
		last_char = is_scaped? 0 : c;
	}

	// WARNING: a string começa mas não termina
	fputs(" -- [malformed string]", output);
	free(str_buffer);
	return -1;
}

int get_number(char c) {
	int x, i = 0;
	char num[TOKEN_MAX_SIZE];
	bool is_float = false; // Indica se é um número real ou inteiro.
	bool already_has_dot = false; // Indica se já foi colocado um ponto decimal no arquivo.
	bool truncate = false; // Indica se o número deve ser truncado por ter mais de 16 dígitos.

	if (!isdigit(c)) {
		return 0;
	}

	// Coloca o dígito no arquivo de saída e no vetor.
	fputc(c, output);
	num[i++] = c;

	while (CAN_READ_FILE) {
		c = (char) x;

		// Valida se o próximo caractere lido é realmente um número, ou se deve ser truncado.
		if (!isdigit(c) || truncate) {
			/* Se não for um dígito, pode ser um '.' indicando um número real, 
			desde que já não exista um ponto neste número. */
			if (c != '.' || already_has_dot || truncate) {
				break;
			} else {
				// Valida se o número já tem um ponto decimal ou não.
				if (!is_float) {
					is_float = true;
				} else {
					already_has_dot = true;
				}
			}
		}

		// Coloca o dígito no arquivo de saída e no vetor.
		fputc(c, output);
		num[i++] = c;

		// Trunca o número aqui, o tamanho máximo é 16 caracteres.
		if (i > 16) {
			truncate = true;
		}
	}

	// Termina a representação do número e devolve o último caractere lido para o buffer do arquivo.
	num[i] = '\0';
	ungetc(c, input);

	// Coloca o tokenCode e o value do número no arquivo de saída.
	fprintf(output, STR_TOKEN_CODE);
	fprintf(output, "%d", is_float ? TOKEN_CODE_FLOAT : TOKEN_CODE_INT);
	fprintf(output, STR_VALUE);
	fputs(num, output);

	// Se o número tinha dois pontos decimais, indica o erro.
	if (already_has_dot) {		
		fputs(" -- [double dot representation]", output);
	}

	// Se o número foi truncado, aponta o erro.
	if (truncate) {
		fputs(" -- [truncated]", output);		
	}

	// Pula linha no arquivo de saída.
	fputc('\n', output);

	return is_float? 2 : 1;
}

int get_word(char c, char str[TOKEN_MAX_SIZE]) {
	int x, i = 0;
	char last_char = c;
	bool is_alpha = isalpha(c); // É alfabético? (alpha != alphanumeric).
	bool is_symbol = ispunct(c); // É um símbolo?
	bool is_double_symbol = false; // Indica se o símbolo é composto (<=, >=, !=).

	// Faz uma cópia do valor no argumento.
	str[i++] = c;

	while (CAN_READ_FILE) {
		c = (char) x;

		// Fim da palavra por caractere de espaço.
		if (isspace(c)) {
			str[i] = '\0';
			return 1;
		}

		// Palavra grande demais.
		if (i >= (TOKEN_MAX_SIZE-1)) {			
			str[i] = '\0';
			return -1;
		}

		// Trata os símbolos.
		if (is_symbol) {
			is_double_symbol = (last_char == '<' || last_char == '>' || last_char == '!') && (c == '=');

			if (!is_double_symbol) {
				ungetc(c, input);
			} else {
				str[i++] = c;
			}

			str[i] = '\0';
			return 1;
		}

		// Trata palavras reservadas e variáveis.
		if (is_alpha) {
			// Variáveis podem ter nome alfanumérico, desde que comecem com letras.
			if (!isalnum(c) && c != '_') {
				str[i] = '\0';
				ungetc(c, input);
				return 1;
			}
		}

		str[i++] = c;
	}

	// Se chegou aqui, era a palavra ou símbolo "final" do arquivo de entrada.
	return 1;
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