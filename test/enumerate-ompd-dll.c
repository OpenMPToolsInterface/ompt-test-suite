#include <stdio.h>
#include <ompt.h>

main()
{
  int i;
  printf("enumerate ompd dll locations:\n"
         "-----------------------------\n");
  if (ompd_dll_locations != NULL) { 
    for (i = 0; ompd_dll_locations[i]; i++) {
      printf("dll name = %s\n", ompd_dll_locations[i]);
    }
  } else {
      printf("WARNING: ompd_dll_locations not properly initialized\n");
  }
}
