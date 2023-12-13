#include <stdio.h>
#include "libraryTest.h"

#define PI 3.1415

//Comentario

int main() {

  int piValue = PI, condition = 1;

/*Test Test 
Test Test Test*/

  if(condition) {
    printf("\nThis is a Test");
  } else if(0) {
    //Do nothing (testing else if spacing)
  } else {
    char *str = "Testem ipsum";
  }

  printf("\nPi's value is: %f", PI);

}