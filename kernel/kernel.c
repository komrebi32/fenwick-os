void krnl_main() {
    char* video = (char*)0xB8000;
    const char* str = "Hola mundo!";
    int i = 0;
    while (str[i]) {
        video[i*2] = str[i];
        video[i*2+1] = 0x0F;
        i++;
    }
    while(1);
}
