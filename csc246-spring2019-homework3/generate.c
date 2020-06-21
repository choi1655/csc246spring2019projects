#include <stdio.h>
#include <stdlib.h>

int main() {
	FILE *fp = fopen("input4.txt", "w");
	for (int i = 1; i <= 100; i++) {
		fprintf(fp, "%d\n", i);
	}
	return 0;
}
