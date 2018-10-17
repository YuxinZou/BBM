#include <stdio.h>
#include <malloc.h>
int calc_f(int n)
{
	int i;
	printf("please enter a numer");
    scanf("%d",&n);   
    int* a=(int*)calloc(n,sizeof(int));
	for(int j = 0; j < 1024 +1; j++){
		a[j] = 1;
	}
    // Complete your code here
	if(n <=1024) {
		return 1;
	} else {
		for(i =1024 ; i < n-1 ; i++) {

			 a[0]=a[1]+a[1024];
				for(i=1024-1;i>=0;i--){
		     a[i+1]=a[i];
	   }
	}
	  return a[0];
 
	}

}

// ----------------------------
// DO NOT TOUCH THIS CODE BLOCK
#ifndef __NVIDIA_TEST_
// ----------------------------

int main()
{
    // Write your tests here
    printf("calc_f(0) = %d\n", calc_f(0));
}

// ----------------------------
// DO NOT TOUCH THIS CODE BLOCK
#endif
// ----------------------------
