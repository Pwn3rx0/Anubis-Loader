import argparse
import sys

def obfuscate_shellcode(shellcode_bytes):
    base_unicode = 0x13000
    
    obfuscated = ""
    for byte in shellcode_bytes:
        unicode_char = chr(base_unicode + byte)
        obfuscated += unicode_char
    
    return obfuscated

def main():
    parser = argparse.ArgumentParser(description='Obfuscate shellcode using Unicode characters')
    parser.add_argument('input_file', help='Input shellcode binary file')
    parser.add_argument('-o', '--output', required=True, help='Output text file for obfuscated shellcode')
    
    args = parser.parse_args()
    
    try:
        with open(args.input_file, 'rb') as f:
            shellcode = f.read()
    except Exception as e:
        print(f"Error reading input file: {e}")
        sys.exit(1)
    
    obfuscated = obfuscate_shellcode(shellcode)
    
    try:
        with open(args.output, 'w', encoding='utf-8') as f:
            f.write(obfuscated)
        print(f"Shellcode obfuscated and saved to {args.output}")
    except Exception as e:
        print(f"Error writing output file: {e}")
        sys.exit(1)


main()