#include <stdio.h>

//struct regflags {
//    int unsign_overflow;
//    int sign_overflow;
//    int zero;
//    int negative;
//};

//void set_register_flags(int a, int b, int t)
//{
//    rflags.zero = t == 0;
//    rflags.negative = t < 0;
//    rflags.sign_overflow = (a > 0 && b > 0 && t <= 0) || (a < 0 && b < 0 && t >= 0);
//    rflags.unsign_overflow = (unsigned) t < (unsigned) a;
//}

int main(void)
{
    printf("hello, world\n");
    return 0;
}
