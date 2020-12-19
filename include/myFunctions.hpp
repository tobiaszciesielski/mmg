#include <iostream>

namespace my {
  char* strcat( char* dest, char* src ) {
    while (*dest) dest++;
    while (*dest++ = *src++);
    return --dest;
  }

  void reverse(char str[], int length) { 
    int start = 0; 
    int end = length -1; 
    while (start < end) 
    { 
      std::swap(*(str+start), *(str+end)); 
      start++; 
      end--; 
    } 
  } 

  char* itoa(int num, char* str, int base) { 
    int i = 0; 
    bool isNegative = false; 
    if (num == 0) { 
        str[i++] = '0'; 
        str[i] = '\0'; 
        return str; 
    } 
  
    if (num < 0 && base == 10) { 
        isNegative = true; 
        num = -num; 
    } 
  
    while (num != 0) { 
        int rem = num % base; 
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0'; 
        num = num/base; 
    } 
  
    if (isNegative) 
      str[i++] = '-'; 
  
    str[i] = '\0'; 
  
    reverse(str, i); 
  
    return str; 
  } 
}