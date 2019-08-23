#include <stdio.h>		// Required for standard input/output		
#include <stdlib.h>		// Required for malloc, free, ....

void main() {
	// Declarations and definitions
	int *array = malloc(sizeof(int) * 10);
	int n, i, j, swap;

	printf( "Enter number of elements : \t");	scanf( "%d", &n );
	printf( "Entered integer is %d. \n", n);

	for ( i = 0; i < n; i++ ) { scanf( "%d", array + i ); }

	/* 예를 들어 array에 1, 6, 8, 2, 3의 숫자들이 저장되어 있다 하자. 그러면 오름차순 버블정렬은 아래와 같다.
		1. [1 6] 8 2 3 ···>> 1 [6 8] 2 3 ···>> 1 6 [8 2] 3 ···>> 1 6 2 [8 3] ∴ 1 6 2 3 "8" (비교 n - 1 번 시행)
		2. [1 6] 2 3 8 ···>> 1 [6 2] 3 8 ···>> 1 2 [6 3] 8 ∴ 1 2 3 "6" 8 (비교 3번 시행)
		3. [1 2] 3 6 8 ···>> 1 [2 3] 6 8 ∴ 1 2 "3" 6 8 (비교 2번 시행)
		4. [1 2] 3 6 8 ∴ 1 "2" 3 6 8 (비교 1번 시행) */

	for ( i = n - 1; i > 0; i-- ) {
		for ( j = 0; j < i; j++ ) {
			if ( *(array + j) > *(array + j + 1) ) { // Ascending order is used
				swap					= *(array + j);
				*(array + j) 		= *(array + j + 1);
				*(array + j + 1)	= swap;
			}
		}
	}

	printf("Sorted list in ascending order : \t");
	for (i = 0; i < n; i++) { printf( "%d", *(array + i) ); }

	free(array);	// Deallocate the memory.
}
