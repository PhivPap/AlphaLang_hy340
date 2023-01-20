#include "vstack.h"
#include "functions.h"
#include <time.h> // remove
#define MAGIC_NUMBER 340200501

char** string_array;
double* number_array;
userfunc* userfunc_array;
char** libfunc_array;
instruction* instructions;

uint strings_a, numbers_a, userfuncs_a, libfuns_a; // to not be globals
uint globals_a;

bool executionFinished = 0;
uint PC;
uint codeSize;
uint currLine;

static void(*executeFuncs[])(instruction*) = {
	execute_assign,
	execute_add,
	execute_sub,
	execute_mul,
	execute_div,
	execute_mod,
	NULL, //execute_uminus, // ignore
	NULL, //execute_and, // ignore 
	NULL, //execute_or, // ignore
	NULL, //execute_not, // ignore
	execute_jeq,
	execute_jne,
	execute_jle,
	execute_jge,
	execute_jlt,
	execute_jgt,
	execute_call,
	execute_pusharg,
	NULL, //execute_ret, // ignore
	NULL, //execute_getretval, // ignore
	execute_funcenter,
	execute_funcexit,
	execute_newtable,
	execute_tablegetelem,
	execute_tablesetelem,
	execute_jump,
	execute_nop
};

static char** fetch_string_array(FILE *binFile, uint *strings_a){
	assert(binFile);
	fread(strings_a, sizeof(uint), 1, binFile);
	char** string_array = malloc(sizeof(char*) * (*strings_a));
	uint string_size;
	for(uint i = 0; i < *strings_a; i++){
		fread(&string_size, sizeof(uint), 1, binFile);
		string_array[i] = malloc(string_size); // FREE THESE
		fread(string_array[i], string_size, 1, binFile);
	}
	return string_array;
}

static double * fetch_number_array(FILE *binFile, uint *numbers_a){
	assert(binFile);
	fread(numbers_a, sizeof(uint), 1, binFile);
	double *number_array = malloc(sizeof(double) * (*numbers_a));
	fread(number_array, sizeof(double), *numbers_a, binFile);
	return number_array;
}

static userfunc * fetch_userfunc_array(FILE *binFile, uint *userfuncs_a){
	assert(binFile);
	fread(userfuncs_a, sizeof(uint), 1, binFile);
	userfunc *userfunc_array = malloc(sizeof(struct userfunc) * (*userfuncs_a));
	uint stringSize;
	for(uint i = 0; i < *userfuncs_a; i++){
		fread(&userfunc_array[i].address, sizeof(uint), 1, binFile);
		fread(&userfunc_array[i].localSize, sizeof(uint), 1, binFile);
		fread(&stringSize, sizeof(uint), 1, binFile);
		userfunc_array[i].id = malloc(stringSize); // FREE THESE
		fread(userfunc_array[i].id, stringSize, 1, binFile);
	}
	return userfunc_array;
}

static instruction * fetch_instructions(FILE *binFile, uint *instructions_a){
	fread(instructions_a, sizeof(uint), 1, binFile);
	instruction *instructions = malloc(sizeof(struct instruction) * (*instructions_a));
	fread(instructions, sizeof(struct instruction), *instructions_a, binFile);
	return instructions;
}


int binary_deserializer(char *fileName){
	FILE *binFile = fopen(fileName,"rb");
	if(!binFile){
		perror("ERROR");
		exit(1);
	}

	uint file_format;
	fread(&file_format, sizeof(uint), 1, binFile);
	if(file_format != MAGIC_NUMBER){
		printf("Invalid File Format !\nExpected alpha binary. Exiting ..\n");
		exit(1);
	}

	string_array = fetch_string_array(binFile, &strings_a);
	number_array = fetch_number_array(binFile, &numbers_a);
	userfunc_array = fetch_userfunc_array(binFile, &userfuncs_a);
	libfunc_array = fetch_string_array(binFile, &libfuns_a);
	instructions = fetch_instructions(binFile, &codeSize);

	fread(&globals_a, sizeof(uint), 1, binFile);

	fclose(binFile);

	return 0;
}



static inline void execute_cycle(void){
	if(PC == codeSize)
		exit(0);
	
	assert(PC < codeSize);
	instruction* instr = instructions + PC;
	assert(instr->op >= 0 && instr->op <= AVM_MAX_INSTRUCTIONS);
	currLine = instr->srcLine;
	executeFuncs[instr->op](instr);
}

clock_t start_t, end_t;
void end(){
	end_t = clock();
	printf("\nRun time: %lf\n", (double)(end_t - start_t) / CLOCKS_PER_SEC);
	for(int i=0; i<strings_a; i++)
		free(string_array[i]);
	free(string_array);
	free(number_array);
	for (int i=0; i<userfuncs_a; i++)
		free(userfunc_array[i].id);
	free(userfunc_array);
	for(int i=0; i<libfuns_a; i++)
		free(libfunc_array[i]);
	free(libfunc_array);
	free(instructions);
}
 

int main(int argc, char** argv){
	if(argc != 2){
		printf("No input file. Exiting ..\n");
		return 1;
	}
	
	binary_deserializer(argv[1]);
	atexit(end);
	avm_initstack(globals_a);
	start_t = clock(); // remove
	while(1)
		execute_cycle();
	return 0;
}