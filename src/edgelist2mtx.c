#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "arg.h"


char *argv0;

char *inname;
char *outname;
char strictcheck = 0;
char forceundir = 0;
char forceweight = 0;


void printusage(void);
char inferformat(FILE *fd, char strict, char *weightedp, unsigned long long *edgecntp, unsigned long long *linecntp, unsigned long long *maxidp);
void readweighted(FILE* fd, unsigned long long *fromp, unsigned long long *top, double *weightp);
void readunweighted(FILE* fd, unsigned long long *fromp, unsigned long long *top);
void wtowmtx(FILE* fd, unsigned long long numlines, unsigned long long *fromp, unsigned long long *top, double *weightp);
void unwtowmtx(FILE* fd, unsigned long long numlines, unsigned long long *fromp, unsigned long long *top);
void unwtounwmtx(FILE* fd, unsigned long long numlines, unsigned long long *fromp, unsigned long long *top);
void writeheader(FILE* fd, char weighted, char undirected);


void printusage(void) {
	printf("Usage: edgelist2mtx [-s] [-uw] INPUT OUTPUT\n");
}

/* Returns 0 if file was parsed successfully, 1 otherwise.
 * *weighted will be equal to 1 if * the inferred format
 * is weighted or 0 otherwise. *edgecnt will store number
 * of non-comment lines, *linecnt will store total number of lines,
 * *maxid will store maximal vertex ID found in file.
 */
char inferformat(FILE* fd, char strict, char *weightedp, unsigned long long *edgecntp, unsigned long long *linecntp, unsigned long long *maxidp) {
	char *line = NULL;
	size_t len = 0;
	unsigned long long u, v, maxlocal = 0;
	double w;
	
	*edgecntp = 0;
	*linecntp = 0;
	*maxidp = 0;

	while(getline(&line, &len, fd) != -1) {
		*linecntp += 1;
		if(line[0] == '%' || line[0] == '#') {
			continue;
		} else {
			break;
		}
	}

	if(sscanf(line, "%llu %llu %lf\n", &u, &v, &w) != 3) {
		if(sscanf(line, "%llu %llu\n", &u, &v) != 2) {
			fprintf(stderr, "Line %llu could not be parsed\n", *linecntp);
			return 1;
		} else {
			printf("Inferring weighted format from line %llu\n", *linecntp);
			*weightedp = 1;
		}
	} else {
		printf("Inferring unweighted format from line %llu\n", *linecntp);
		*weightedp = 0;
	}

	unsigned long long infline = *linecntp;
	while(getline(&line, &len, fd) != -1) {
		*linecntp += 1;
		if(line[0] == '%' || line[0] == '#') {
			if(strict) {
				fprintf(stderr, "Commentary line (%llu) after first non-comentary line (%llu)\n", *linecntp, infline);
				return 1;
			} else {
				continue;
			}
		} else {
			if(*weightedp) {
				if(sscanf(line, "%llu %llu %lf\n", &u, &v, &w) != 3) {
					fprintf(stderr, "Line %llu could not be parsed as weighted (inferred from line %llu)\n", *linecntp, infline);
					return 1;
				} else {
					maxlocal = u > v? u : v;
					if(maxlocal > *maxidp) {
						*maxidp = maxlocal;
					}
				}
			} else {
				if(sscanf(line, "%llu %llu\n", &u, &v) != 2) {
					fprintf(stderr, "Line %llu could not be parsed as unweighted (inferred from line %llu)\n", *linecntp, infline);
					return 1;
				} else {
					maxlocal = u > v ? u : v;
					if(maxlocal > *maxidp) {
						*maxidp = maxlocal;
					}
				}
			}
			*edgecntp += 1;
		}
	}

	return 0;
}

/* Reads fd as weighted edgelist, puts each line
 * in asserted format "FROM TO WEIGHT" to corresponding arrays.
 *
 * Assertions:
 * - arrays are of sufficient size
 * - each non-commentary line can be correctly parsed by sscanf
 */
void readweighted(FILE* fd, unsigned long long *fromp, unsigned long long *top, double *weightp) {
	char* line = NULL;
	size_t len = 0;
	while(getline(&line, &len, fd) != -1) {
		if(line[0] == '#' || line [0] == '%') {
			continue;
		} else {
			/* if this assertion fails, something very wrong happened after format was
			 * inferred and verified */
			assert(sscanf(line, "%llu %llu %lf\n", fromp, top, weightp) == 3);
			++fromp;
			++top;
			++weightp;
		}
	}

	free(line);
}


/* Reads fd as unweighted edgelist, puts each line
 * in asserted format "FROM TO" to corresponding arrays.
 *
 * Assertions:
 * - arrays are of sufficient size
 * - each non-commentary line can be correctly parsed by sscanf
 */
void readunweighted(FILE* fd, unsigned long long *fromp, unsigned long long *top) {
	char* line = NULL;
	size_t len = 0;
	while(getline(&line, &len, fd) != -1) {
		if(line[0] == '#' || line [0] == '%') {
			continue;
		} else {
			/* if this assertion fails, something very wrong happened after format was
			 * inferred and verified */
			assert(sscanf(line, "%llu %llu\n", fromp, top) == 2);
			++fromp;
			++top;
		}
	}

	free(line);
}

void wtowmtx(FILE* fd, unsigned long long numlines, unsigned long long *fromp, unsigned long long *top, double *weightp) {
	for(unsigned long long i = 0; i < numlines; ++i) {
		fprintf(fd, "%llu %llu %lf\n", fromp, top, weightp);
		++fromp;
		++top;
		++weightp;
	}
}

void unwtowmtx(FILE* fd, unsigned long long numlines, unsigned long long *fromp, unsigned long long *top) {
	for(unsigned long long i = 0; i < numlines; ++i) {
		fprintf(fd, "%llu %llu 1.0\n", fromp, top);
		++fromp;
		++top;
	}
}

void unwtounwmtx(FILE* fd, unsigned long long numlines, unsigned long long *fromp, unsigned long long *top) {
	for(unsigned long long i = 0; i < numlines; ++i) {
		fprintf(fd, "%llu %llu\n", fromp, top);
		++fromp;
		++top;
	}
}

void writeheader(FILE* fd, char weighted, char undirected) {
	fprintf(fd, "%%MatrixMarket matrix coordinate");

	if(weighted) {
		fprintf(fd, " real");
	} else {
		fprintf(fd, " pattern");
	}

	if(undirected) {
		fprintf(fd, " symmetric\n");
	} else {
		fprintf(fd, "general\n");
	}
}


int main(int argc, char **argv) {

	ARGBEGIN {
	case 's':
		strictcheck = 1;
		break;
	case 'u':
		forceundir = 0;
		break;
	case 'w':
		forceweight = 1;
		break;
	default:
		fprintf(stderr, "Unrecognized option: \"%c\"\n", ARGC());
		printusage();
		return 1;
	} ARGEND

	if(argc != 2) {
		fprintf(stderr, "Incorrect number of arguments: %d\n", argc);
		printusage();
		return 1;
	}

	inname = argv[0];
	outname = argv[1];

	FILE *fd = fopen(inname, "r");
	if(fd == NULL) {
		fprintf(stderr, "Unable to open input file \"%s\"\n", inname);
		return 1;
	}

	unsigned long long maxid = 0, edgecnt = 0, linecnt = 0;
	char weighted;
	char r = inferformat(fd, strictcheck, &weighted, &edgecnt, &linecnt, &maxid);
	if(r != 0) {
		fprintf(stderr, "Input file parse error, terminating\n");
		return 1;
	}

	printf("Input file parsed\n\tLines: %llu\n\tMaximal vertex id: %llu\n\tEdges: %llu\n", linecnt, maxid, edgecnt);

	unsigned long long *fromp, *top;
	double *weightp = NULL;
	fromp = (unsigned long long *) malloc(linecnt * sizeof(unsigned long long));
	top = (unsigned long long *) malloc(linecnt * sizeof(unsigned long long));

	rewind(fd);
	if(weighted) {
		weightp = (double *) malloc(linecnt * sizeof(double));
		readweighted(fd, fromp, top, weightp);
	} else {
		readunweighted(fd, fromp, top);
	}

	fclose(fd);

	fd = fopen(outname, "w");
	if(fd == NULL) {
		printf("Unable to open output file \"%s\"\n", outname);
		return 1;
	}

	writeheader(fd, weighted || forceweight, forceundir);
	if(weighted) {
		wtowmtx(fd, linecnt, fromp, top, weightp);
	} else if(forceweight) {
		unwtowmtx(fd, linecnt, fromp, top);
	} else {
		unwtounwmtx(fd, linecnt, fromp, top);
	}

	fclose(fd);
	free(fromp);
	free(top);
	if(weightp != NULL) {
		free(weightp);
	}

	return 0;
}

