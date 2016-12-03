#ifndef COLOR_H
#define COLOR_H

/* Constants */
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

/* Methods */
char* GetColor(int index)
{
  switch(index % 6)
  {
    case 0:
      return KRED;
      break;
    case 1:
      return KGRN;
      break;
    case 2:
      return KYEL;
      break;
    case 3:
      return KBLU;
      break;
    case 4:
      return KMAG;
      break;
    case 5:
      return KCYN;
      break;
  }
}

#endif
