#define _GNU_SOURCE

#include <stdio.h>
#include <inttypes.h>
#include <argp.h>

char* inputFile = "./input/input.txt";
char* outputFile = "./output/output.txt";

static int parse_opt(int key, char *arg, struct argp_state *state);

int main(int argc, char **argv) {

	struct argp_option options[] = {
		{  "input", 'i', "FILE", 0,  "path to input file" },
		{ "output", 'o', "FILE", 0, "path to output file" },
		{ 0 }
	};
	struct argp argp = {options, parse_opt, 0, 0};
	argp_parse(&argp, argc, argv, 0, 0, 0);

	FILE *input = fopen(inputFile, "r");
	if(input == NULL) {
		printf("Unable to open input file \"%s\"\n", inputFile);
		return 1;
	}

	char* line = NULL;
	unsigned long long maxID = 0;
	unsigned long v0 = 0;
	unsigned long v1 = 0;
	unsigned long long edgeCount = 0;
	size_t len = 0;
	while(getline(&line, &len, input) != -1) {
		if(line[0] == '#' || line [0] == '%') {
			printf("Encountered a commented line\n");
		} else {
			if(sscanf(line, "%" SCNu64 " %" SCNu64, &v0, &v1) == 2) {
				edgeCount++;

				if(v0 > v1) {
					if(v0 > maxID) {
						maxID = v0;
					}
				} else {
					if(v1 > maxID) {
						maxID = v1;
					}
				}
			} else {
				printf("Invalid line: \"%s\"\n", line);
			}
		}
	}

	printf("Maximal vertex ID: %llu\n", maxID);
	printf("Edge count: %llu\n", edgeCount);

	rewind(input);

	FILE *output = fopen(outputFile, "w");
	if(output == NULL) {
		printf("Unable to open output file \"%s\"\n", outputFile);
		return 1;
	}

	fprintf(output, "%%MatrixMarket matrix coordinate pattern symmetric\n");
	fprintf(output, "%llu %llu %llu\n", maxID, maxID, edgeCount);
	while(getline(&line, &len, input) != -1) {
		if(line[0] == '#' || line [0] == '%') {
			printf("Encountered a commented line\n");
		} else {
			if(sscanf(line, "%" SCNu64 " %" SCNu64, &v0, &v1) == 2) {
				fprintf(output, "%" SCNu64 " %" SCNu64 "\n", v0, v1);
			} else {
				printf("Invalid line: \"%s\"\n", line);
			}
		}
	}

	if(line) {
		//free(line);
	}
	fclose(input);
	fclose(output);

	return 0;
}

static int parse_opt(int key, char *arg, struct argp_state *state) {
	if(key == 'i') {
		inputFile = arg;
	} else if(key == 'o') {
		outputFile = arg;
	}
	
	return 0;
}

