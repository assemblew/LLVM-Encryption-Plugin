#include <stdio.h>
#include <string>
#include <stdint.h>
#include <time.h>

#define COUNT 100
static int arr[2] = {4334234, 23};
static bool bool_ex = 1;
static float float_ex = 52.52;
//static char *char_ex = "wasup";

int aa(int b)
{
  static int int32_ex = 1501;
  printf("%i %.2f %i %i\n", int32_ex, float_ex, arr[0], arr[1]/*, char_ex*/);
  return b;
}

int main()
{
  int a = 0;
  char *name = "gooon !";
  const char *surname = "olegovich";

  if(10 > 5)
  {
    const static int cnst = 69;
    printf("xd %s    %i    %s\n", name, cnst, surname);
  }
  else
  {
    a = 5;
  }

  for(int i = 0; i < 10; i++, a++)
  {
    printf("%i %i\n", COUNT, i);
  }

  void *nul;
  printf("%i %i %p\n", a, aa(52), nul);
  nul = NULL;
  printf("%p\n", nul);

  return 0;
}