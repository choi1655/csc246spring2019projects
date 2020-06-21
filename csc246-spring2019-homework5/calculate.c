#include <stdio.h>
#include <stdlib.h>

int main() {
	FILE *fp = fopen("numbers.txt", "r");
	if (!fp) {
		fprintf(stderr, "Cant open numbers.txt\n");
		return EXIT_FAILURE;
	}

	int num1, num2, sum = 0;
	//fscanf(fp, "%d %d", &num1, &num2);
	while (fscanf(fp, "%d %d", &num1, &num2) != EOF) {
		int difference = num1 - num2;
		if (difference < 0) {
			difference *= -1;
		}
		sum += difference;
	}
	fprintf(stdout, "Total distance traveled is %d\n", sum);
	fclose(fp);
	return EXIT_SUCCESS;
}
