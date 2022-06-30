int cnt=20000000;
int main(){
    while(cnt--);
    __asm( "movl $1, %eax\n"
           "movl $23, %ebx\n"
           "int $0x80\n"); /*右括号前不能有空格!!!!*/
    return 0;
}
