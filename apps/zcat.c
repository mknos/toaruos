#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char * argv[]) {
	char ** args = calloc(argc + 2, sizeof(char*));
	if (args == NULL) {
		perror("zcat: calloc");
		return 1;
	}
	args[0] = "gunzip";
	args[1] = "-c";
	for (int i = 1; i < argc; ++i)
		args[i+1] = argv[i];
	args[argc+1] = NULL;
	int r = execvp("gunzip", args);
	if (r == -1)
		perror("zcat: execvp");
	return r;
}
