#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char ** argv) {
	if (argc != 3)
		fprintf(stderr, "Usage: filegrep PATTERN FILE\n");
	else {
		FILE * f = fopen(argv[2], "r");
		char * line = malloc(255);
		int ln = 1;
		while( fgets(line, 255, f) ) {
			char * ifhas = strstr(line, argv[1]); 
			if ( ifhas != NULL ) {
				printf("%d:%s", ln,line);
			}
			ln++;
		}
		fclose(f);
	}
	return 0;
}
