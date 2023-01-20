%{
	#ifdef __linux__ 
   		#define CUSTOM_EOF 0
	#elif _WIN32
   		#define CUSTOM_EOF -1
	#endif

	#include <string.h>
	#include "parser.h"

	struct alpha_token_t {
		unsigned int numline;
		unsigned int numToken;
		char *content;
		char *type;
	};

	struct alpha_token_t lval;
	struct alpha_token_t *insertToken(int lineNo, char *category, char *content);
	char * stringParse(int lineNo);
	void multilineCommentParse(int yylineno, int isNested);
	void printWarning(char *warningMsg, int warningLine);
	void exitWithError(char *errorMsg, int lineNo);
	void undefInput();
	int tokenNum = 0;
%}

%option yylineno
%option noyywrap

NEW_LINE		"\n"
SPACE			" "
TAB				"\t"

IF				"if"
ELSE			"else"
WHILE			"while"
FOR				"for"
FUNCTION		"function"
RETURN			"return"
BREAK			"break"
CONTINUE		"continue"
AND				"and"
NOT				"not"
OR				"or"
LOCAL			"local"
TRUE			"true"
FALSE			"false"
NIL				"nil"

ASSIGN			"="
PLUS			"+"
MINUS			"-"
ASTERISK		"*"
SLASH			"/"
MODULO			"%"
EQUAL			"=="
UNEQUAL			"!="
PLUS_PLUS		"++"
MINUS_MINUS		"--"
GREATER_THAN	">"
LESSER_THAN		"<"
GREATER_EQUAL	">="
LESSER_EQUAL	"<="

INTCONST		[0-9]+

REALCONST		[0-9]+\.[0-9]+

STRING_START	"\""

CURLY_BR_O		"{"
CURLY_BR_C		"}"
SQUARE_BR_O		"["
SQUARE_BR_C		"]"
ROUND_BR_O		"("
ROUND_BR_C		")"
SEMICOLON		";"
COMMA			","
COLON			":"
DBL_COLON		"::"
PERIOD			"."
DBL_PERIOD		".."

ID				[a-zA-Z][a-zA-Z0-9_]*

COMMENT_LINE	"//".*
COMMENT_MULTI_S "/*"

%%

{NEW_LINE}
{SPACE}
{TAB}

{IF}				yylval.tok = insertToken(yylineno, "KEYWORD IF", yytext); return IF;
{ELSE}				yylval.tok = insertToken(yylineno, "KEYWORD ELSE", yytext); return ELSE;
{WHILE}				yylval.tok = insertToken(yylineno, "KEYWORD WHILE", yytext); return WHILE;
{FOR}				yylval.tok = insertToken(yylineno, "KEYWORD FOR", yytext); return FOR;
{FUNCTION}			yylval.tok = insertToken(yylineno, "KEYWORD FUNCTION", yytext); return FUNCTION;
{RETURN}			yylval.tok = insertToken(yylineno, "KEYWORD RETURN", yytext); return RETURN;
{BREAK}				yylval.tok = insertToken(yylineno, "KEYWORD BREAK", yytext); return BREAK;
{CONTINUE}			yylval.tok = insertToken(yylineno, "KEYWORD CONTINUE", yytext); return CONTINUE;
{AND}				yylval.tok = insertToken(yylineno, "KEYWORD AND", yytext); return AND;
{NOT}				yylval.tok = insertToken(yylineno, "KEYWORD NOT", yytext); return NOT;
{OR}				yylval.tok = insertToken(yylineno, "KEYWORD OR", yytext); return OR;
{LOCAL}				yylval.tok = insertToken(yylineno, "KEYWORD LOCAL", yytext); return LOCAL;
{TRUE}				yylval.tok = insertToken(yylineno, "KEYWORD TRUE", yytext); return TRUE;
{FALSE}				yylval.tok = insertToken(yylineno, "KEYWORD FALSE", yytext); return FALSE;
{NIL}				yylval.tok = insertToken(yylineno, "KEYWORD NIL", yytext); return NIL;

{ASSIGN}			yylval.tok = insertToken(yylineno, "OPERATOR ASSIGN", yytext); return ASSIGN;
{PLUS}				yylval.tok = insertToken(yylineno, "OPERATOR PLUS", yytext); return PLUS;
{MINUS}				yylval.tok = insertToken(yylineno, "OPERATOR MINUS", yytext); return MINUS;
{ASTERISK}			yylval.tok = insertToken(yylineno, "OPERATOR ASTERISK", yytext); return ASTERISK;
{SLASH}				yylval.tok = insertToken(yylineno, "OPERATOR SLASH", yytext); return SLASH;
{MODULO}			yylval.tok = insertToken(yylineno, "OPERATOR MODULO", yytext); return MODULO;
{EQUAL}				yylval.tok = insertToken(yylineno, "OPERATOR EQUAL", yytext); return EQUAL;
{UNEQUAL}			yylval.tok = insertToken(yylineno, "OPERATOR UNEQUAL", yytext); return UNEQUAL;
{PLUS_PLUS}			yylval.tok = insertToken(yylineno, "OPERATOR PLUS_PLUS", yytext); return PLUS_PLUS;
{MINUS_MINUS}		yylval.tok = insertToken(yylineno, "OPERATOR MINUS_MINUS", yytext); return MINUS_MINUS;
{GREATER_THAN}		yylval.tok = insertToken(yylineno, "OPERATOR GREATER_THAN", yytext); return GREATER_THAN;
{LESSER_THAN}		yylval.tok = insertToken(yylineno, "OPERATOR LESSER_THAN", yytext); return LESSER_THAN;
{GREATER_EQUAL}		yylval.tok = insertToken(yylineno, "OPERATOR GREATER_EQUAL", yytext); return GREATER_EQUAL;
{LESSER_EQUAL}		yylval.tok = insertToken(yylineno, "OPERATOR LESSER_EQUAL", yytext); return LESSER_EQUAL;

{INTCONST}			yylval.tok = insertToken(yylineno, "INTCONST", yytext); return INTCONST;

{REALCONST}			yylval.tok = insertToken(yylineno, "REALCONST", yytext); return REALCONST;

{STRING_START}		{	
						int line = yylineno;
						char *string = stringParse(yylineno);
						yylval.tok = insertToken(line, "STRING", string); 
						//free(string);
						return STRING;
					}

{CURLY_BR_O}		yylval.tok = insertToken(yylineno, "PUNCTUATION CURLY_BR_O", yytext); return CURLY_BR_O;
{CURLY_BR_C}		yylval.tok = insertToken(yylineno, "PUNCTUATION CURLY_BR_C", yytext); return CURLY_BR_C;
{SQUARE_BR_O}		yylval.tok = insertToken(yylineno, "PUNCTUATION SQUARE_BR_O", yytext); return SQUARE_BR_O;
{SQUARE_BR_C}		yylval.tok = insertToken(yylineno, "PUNCTUATION SQUARE_BR_C", yytext); return SQUARE_BR_C;
{ROUND_BR_O}		yylval.tok = insertToken(yylineno, "PUNCTUATION ROUND_BR_O", yytext); return ROUND_BR_O;
{ROUND_BR_C}		yylval.tok = insertToken(yylineno, "PUNCTUATION ROUND_BR_C", yytext); return ROUND_BR_C;
{SEMICOLON}			yylval.tok = insertToken(yylineno, "PUNCTUATION SEMICOLON", yytext); return SEMICOLON;
{COMMA}				yylval.tok = insertToken(yylineno, "PUNCTUATION COMMA", yytext); return COMMA;
{COLON}				yylval.tok = insertToken(yylineno, "PUNCTUATION COLON", yytext); return COLON;
{DBL_COLON}			yylval.tok = insertToken(yylineno, "PUNCTUATION DBL_COLON", yytext); return DBL_COLON;
{PERIOD}			yylval.tok = insertToken(yylineno, "PUNCTUATION PERIOD", yytext); return PERIOD;
{DBL_PERIOD}		yylval.tok = insertToken(yylineno, "PUNCTUATION DBL_PERIOD", yytext); return DBL_PERIOD;

{ID}				yylval.tok = insertToken(yylineno, "ID", yytext); return ID;

{COMMENT_LINE}		insertToken(yylineno, "COMMENT LINE_COMMENT", "");

{COMMENT_MULTI_S}  	multilineCommentParse(yylineno, 0);

.					undefInput();
%%

void multilineCommentParse(int lineNo, int isNested){
	int lineStart = lineNo;

	int c = input();
	while(c != CUSTOM_EOF){
		
		if(c == '*'){
			c = input();

			if(c == '/'){
				return;
			}
		}
		else if(c == '/'){
			c = input();

			if(c == '*'){
				multilineCommentParse(yylineno, 1);
			}
		}
		else{
			c = input();
		}
	}
	if(isNested){
		fprintf(stderr, "Line %d: error nested block comment reached EOF\n", lineStart);
		return;
	}
	exitWithError("block comment reached EOF\n", lineStart);
}


// char * stringParse(int lineNo) {
// 	int c;
// 	size_t buffer_size = 10;
// 	char *string; 
// 	int length = 0;

// 	string = malloc(sizeof(char) * buffer_size);

// 	c = input();
// 	while(c != CUSTOM_EOF){
// 		if(length >= buffer_size) 
// 			string = realloc(string, sizeof(char) * (buffer_size*=2));
		
// 		if(c == '\"'){
// 			string[length] = '\0';
// 			return string;
// 		}


// 		if(c == '\\'){
// 			c = input();
// 			if(c == 'n'){
// 				string[length++] = '\n';
// 				c = input();
// 				continue;
// 			}
// 			else if(c == 't'){
// 				string[length++] = '\t';
// 				c = input();
// 				continue;
// 			}
// 			else if(c == '\\'){
// 				string[length++] = '\\';
// 				c = input();
// 				continue;
// 			}
// 			else if(c == '\"'){
// 				string[length++] = '"';
// 				c = input();
// 				continue;
// 			}
// 			else if(c == EOF){
// 				free(string);
// 				exitWithError("string reached EOF.\n", lineNo);
// 			}
// 			else{
// 				string[length++] = '\\';
// 				char warningMessage[60];  
// 				sprintf(warningMessage, "invalid escape sequence \\%c in string.\n", c);
// 				printWarning(warningMessage, yylineno);
// 				if(length >= buffer_size) 
// 					string = realloc(string, sizeof(char) * (buffer_size*=2));
// 			}
// 		}
// 		string[length++] = c;
// 		c = input();
// 	}
// 	exitWithError("string reached EOF.\n", lineNo);
// 	return NULL;
// }

char * stringParse(int lineNo) {
	int c;
	size_t buffer_size = 10;
	char *string; 
	int length = 0;

	string = malloc(sizeof(char) * buffer_size);

	while ((c = input()) != CUSTOM_EOF) {
		/* end of string (exit condition)*/
		if (c == '\"') {
			if(length >= buffer_size) 
			string = realloc(string, sizeof(char) * (buffer_size + 1));
			string[length] = '\0';
			return string;
		}

		if(length >= buffer_size) 
			string = realloc(string, sizeof(char) * (buffer_size*=2));

		/* escape characters handling */
		if (c == '\\') {		
			c = input();
			if(c == 'n')  {
				string[length++] = '\n';
				continue;
			}
			else if(c == 't') {
				string[length++] = '\t';
				continue;
			}
			else if(c == '\\') {
				string[length++] = '\\';
				continue;
			}
			else if(c == 'r') {
				string[length++] = '\r';
				continue;
			}
			else if(c == '\"') {
				string[length++] = '"';
				continue;
			}
			else if(c == EOF) {
				free(string);
				break;
			}
			/* escape character input is wrong.Copy it as it is in string*/
			else {
				string[length++] = '\\';
				
				char warningMessage[60];  
				sprintf(warningMessage, "invalid escape sequence \\%c in string.\n", c);
				printWarning(warningMessage, yylineno);
				if(length >= buffer_size) 
 					string = realloc(string, sizeof(char) * (buffer_size*=2));
			}
		}
		string[length++] = c;
	}

	exitWithError("string reached EOF.\n", lineNo);
	return NULL;
}


struct alpha_token_t *insertToken(int lineNo, char *category, char *content){
	tokenNum++;
	
	lval.numline = lineNo;
	lval.numToken = tokenNum;
	lval.content = content;
	lval.type = category;
	return &lval;
}


void printWarning(char *warningMsg, int warningLine){
	fprintf(stderr, "Line %d: warning %s", warningLine, warningMsg);
}

void exitWithError(char *errorMsg, int errorLine){
	fprintf(stderr, "Line %d: error %s", errorLine, errorMsg);
	exit(-1);
}


/*yytext length <= 1*/
void undefInput(){
	printf("<%s.>\n", yytext);
	char undefCharMsg[30] = "undefined input \'";
	strcat(undefCharMsg, yytext);
	strcat(undefCharMsg, "\'\n");
	exitWithError(undefCharMsg, yylineno);
}