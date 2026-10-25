#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <unistd.h>
#include <time.h>

#define YELLOW      "\033[93m"
#define LIME        "\033[92m"
#define PINK        "\033[95m"
#define LIGHT_BLUE  "\033[94m"
#define RED         "\033[91m"
#define RESET       "\033[0m"

typedef enum
{
    TECHNIQUE_NONE = 0,
    TECHNIQUE_AES_128_ENCRYPT,
    TECHNIQUE_AES_192_ENCRYPT,
    TECHNIQUE_AES_256_ENCRYPT,
    TECHNIQUE_RC4_ENCRYPT,
    TECHNIQUE_XOR_ENCRYPT,
    TECHNIQUE_IPV4_OBFUSCATE,
    TECHNIQUE_IPV6_OBFUSCATE,
    TECHNIQUE_UUID_OBFUSCATE,
    TECHNIQUE_MAC_OBFUSCATE,
    TECHNIQUE_BASE64_OBFUSCATE
} TechniqueType;

typedef struct
{
    TechniqueType type;
    char *value; /* The arg passed to tech */
} Technique;

typedef struct
{
    char *hexadecimal;
    char *binary;
    Technique technique;
    int help_requested;
    int decrypt;
} Arguments;

typedef struct
{
    unsigned char *data;
    size_t len;
} RawBytes;

Arguments parse_arguments(int argc, char *argv[]);

/* Printers */
void handle_banner(void)
{
    static const char *mascot[14] = {
        "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
        "⠀⠀⠀⠀⠀⣀⣀⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
        "⠀⠀⢀⣴⠿⠛⠛⠛⠛⠿⢶⣦⣤⣄⣀⣤⣤⣶⠟⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
        "⠀⠀⣿⠃⠀⠀⠀⠀⠀⠀⠀⠀⠉⠉⠉⠉⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
        "⠀⠀⣿⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣤⣤⣤⡀⠀⠀⠀",
        "⠀⠀⠙⢷⣤⣀⣀⣀⣤⣶⣾⣿⣿⣿⣿⣿⣿⣿⣶⣦⣤⣾⡿⠋⢙⣿⠿⠷⠀⠀",
        "⠀⠀⠀⠀⠉⢛⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣴⣿⣧⡀⢛⣄⠀",
        "⠀⠀⠀⠀⠀⠘⢿⣿⣿⣿⡿⠿⢿⣿⣿⣿⡿⣿⣿⣿⣿⣿⡿⠿⣿⣿⣿⣿⠏⠀",
        "⠀⠀⠀⠀⠀⣀⣌⢿⣿⣿⣿⠀⠀⠙⠛⢋⣼⣿⣿⣿⣿⠟⣁⡄⠀⠀⠀⠀⠀⠀",
        "⠀⣀⣤⣶⣿⣿⡿⠆⢻⣿⣿⠇⠀⠀⠀⣾⣿⣿⠟⠋⣶⣿⣿⡁⠀⠀⠀⠀⠀⠀",
        "⠀⣿⡟⠋⠉⠀⠀⢠⣿⣿⠋⠀⠀⠀⢸⣿⡟⠁⠀⠀⠈⢿⣿⣧⠀⠀⠀⠀⠀⠀",
        "⠀⣿⡇⠀⠀⠀⢀⣿⡿⠃⠀⠀⠀⣠⣿⡟⠀⠀⠀⠀⠀⠀⢻⣿⣧⠀⠀⠀⠀⠀",
        "⠀⠛⠁⠀⠀⠀⠘⢿⣷⣤⣄⡀⣴⠿⠋⠀⠀⠀⠀⠀⠀⠀⠀⠹⣿⣦⣤⣤⡀⠀",
        "⠀⠀⠀⠀⠀⠀⠀⠀⠈⠉⠉⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠉⠉⠉⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀"
    };
    static const char *logo[6] = {
        "███╗   ███╗ █████╗  ██████╗ █████╗ ███████╗██╗  ██╗███████╗██╗     ██╗     ",
        "████╗ ████║██╔══██╗██╔════╝██╔══██╗██╔════╝██║  ██║██╔════╝██║     ██║     ",
        "██╔████╔██║███████║██║     ███████║███████╗███████║█████╗  ██║     ██║     ",
        "██║╚██╔╝██║██╔══██║██║     ██╔══██║╚════██║██╔══██║██╔══╝  ██║     ██║     ",
        "██║ ╚═╝ ██║██║  ██║╚██████╗██║  ██║███████║██║  ██║███████╗███████╗███████╗",
        "╚═╝     ╚═╝╚═╝  ╚═╝ ╚═════╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝"
    };
    static const char *mascot_colors[5] = { YELLOW, PINK, LIME, LIGHT_BLUE, RED };
    const char *mascot_color;
    const char *p;
    int i;

    if (!isatty(STDOUT_FILENO))
        return;

    srand((unsigned)time(NULL));
    mascot_color = mascot_colors[rand() % 5];

    fputs(mascot_color, stderr);

    for (i = 0; i < 14; ++i)
    {
        fputs(mascot[i], stderr);
        fputc('\n', stderr);
    }

    fputs(RESET, stderr);
    fputs(LIME, stderr);

    for (i = 0; i < 6; ++i)
    {
        p = logo[i];
        while (*p != '\0')
        {
            if (p[1] != '\0' && p[2] != '\0' &&
                (unsigned char)p[0] == 0xE2 &&
                (unsigned char)p[1] == 0x96 &&
                (unsigned char)p[2] == 0x88)
            {
                fputs(YELLOW, stderr);
                fputc((unsigned char)p[0], stderr);
                fputc((unsigned char)p[1], stderr);
                fputc((unsigned char)p[2], stderr);
                fputs(LIME, stderr);
                p += 3;
            }
            else
            {
                fputc((unsigned char)*p, stderr);
                ++p;
            }
        }
        fputc('\n', stderr);
    }

    fputs(RESET, stderr);

    fprintf(stderr,
            "MacaShell - payload encryptor and obfuscator\n"
            "Reads a binary or hex file and prints a 'payload = { ... }' C array.\n"
            "Run 'macashell -h' for usage help.\n");
}

void handle_help(void)
{
    printf("Usage: macashell [options]\n"
           "\n"
           "Input:\n"
           "  -b, --bin <file>       read raw binary file\n"
           "  -0, --hex <file>       read hex text or 'payload = { ... }' file\n");

    printf("\n"
           "Techniques:\n"
           "  -a128, --aes128 <pass> AES-128-CBC encryption (passphrase)\n"
           "  -a192, --aes192 <pass> AES-192-CBC encryption (passphrase)\n"
           "  -a256, --aes256 <pass> AES-256-CBC encryption (passphrase)\n"
           "  -r,  --rc4 <pass>      RC4 stream cipher (passphrase)\n"
           "  -x,  --xor <pass>      repeating-key XOR (passphrase)\n");

    printf("  -4,  --ipv4            IPv4 quad obfuscation\n"
           "  -6,  --ipv6            IPv6 address obfuscation\n"
           "  -u,  --uuid            UUID obfuscation\n"
           "  -m,  --mac             MAC address obfuscation\n"
           "  -b64, --base64         Base64 encoding\n");

    printf("\n"
           "Options:\n"
           "  -d, --decrypt          deobfuscate / decrypt\n"
           "  -h, --help             show this help\n"
           "\n"
           "Examples:\n"
           "  macashell -b payload.bin -a256 hunter2 > enc.txt\n"
           "  macashell -0 enc.txt -a256 hunter2 -d > dec.txt\n"
           "  macashell -b payload.bin -u > uuids.txt\n");
}

/* Global constants */
static const char base64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* Input validation */
int validate_arguments(Arguments args);

/* Decode */
static int hex_val(char c);

/* Read data as bytes */
RawBytes load_bytes_bin(const char *filepath);
RawBytes load_bytes_hex(const char *filepath);

/* Encryption / Decryption */
/* AES */
RawBytes aes_encrypt_cbc(const unsigned char *key, int key_len, RawBytes input);
RawBytes aes_decrypt_cbc(const unsigned char *key, int key_len, RawBytes input);

/* Derive key from passphrase */
int derive_key(const char *passphrase, unsigned char *key, int key_len);

/* Convert to code block */
char *bytes_to_payload(const unsigned char *data, size_t len);

/* XOR Encrypt/Decrypt */
RawBytes xor_encrypt(const unsigned char *key, size_t key_len, RawBytes input);

/* RC4 Encrypt/Decrypt */
RawBytes rc4_encrypt(const unsigned char *key, size_t key_len, RawBytes input);

/* Base64 Encode */
RawBytes b64_obfuscate(RawBytes input);
RawBytes b64_deobfuscate(RawBytes input);

/* IPv4 Obfuscate / Deobfuscate */
RawBytes ipv4_obfuscate(RawBytes input);
RawBytes ipv4_deobfuscate(RawBytes input);

/* IPv6 Obuscate / Deobfuscate */
RawBytes ipv6_obfuscate(RawBytes input);
RawBytes ipv6_deobfuscate(RawBytes input);

/* UUID Obfuscate / Deobfuscate */
RawBytes uuid_obfuscate(RawBytes input);
RawBytes uuid_deobfuscate(RawBytes input);

/* MAC Obfuscate / Deobfuscate */
RawBytes mac_obfuscate(RawBytes input);
RawBytes mac_deobfuscate(RawBytes input);


int main(int argc, char * argv[])
{
    Arguments args;
    RawBytes raw;
    RawBytes transformed;
    char *payload;
    unsigned char key[32];
    int key_len = 0;

    if (argc < 2)
    {
        handle_banner();
        return 1;
    }

    /* Arg parser */
    args = parse_arguments(argc, argv);

    if (!validate_arguments(args))
    {
        handle_error();
        handle_menu();
        return 1;
    }

    if (args.help_requested)
    {
        handle_help();
        return 0;
    }

    raw = args.binary ? load_bytes_bin(args.binary) : load_bytes_hex(args.hexadecimal);
    if (!raw.data)
    {
        fprintf(stderr, "Error: Failed to load data.\n");
        return 1;
    }

    if (args.technique.type == TECHNIQUE_XOR_ENCRYPT)
    {
        transformed = xor_encrypt((const unsigned char *)args.technique.value, 
                                   strlen(args.technique.value), raw);
    } 
    else if (args.technique.type == TECHNIQUE_RC4_ENCRYPT)
    {
        transformed = rc4_encrypt((const unsigned char *)args.technique.value,
                                   strlen(args.technique.value), raw);
    }
    else if (args.technique.type == TECHNIQUE_BASE64_OBFUSCATE)
    {
        if (args.decrypt) transformed = b64_deobfuscate(raw);
        else transformed = b64_obfuscate(raw);
    }
    else if (args.technique.type == TECHNIQUE_IPV4_OBFUSCATE)
    {
        if (args.decrypt) transformed = ipv4_deobfuscate(raw);
        else transformed = ipv4_obfuscate(raw);
    }
    else if (args.technique.type == TECHNIQUE_IPV6_OBFUSCATE)
    {
        if (args.decrypt) transformed = ipv6_deobfuscate(raw);
        else transformed = ipv6_obfuscate(raw);
    }
    else if (args.technique.type == TECHNIQUE_MAC_OBFUSCATE)
    {
        if (args.decrypt) transformed = mac_deobfuscate(raw);
        else transformed = mac_obfuscate(raw);
    }
    else if (args.technique.type == TECHNIQUE_UUID_OBFUSCATE)
    {
        if (args.decrypt) transformed = uuid_deobfuscate(raw);
        else transformed = uuid_obfuscate(raw);
    } else
    { 
        if (args.technique.type == TECHNIQUE_AES_128_ENCRYPT) key_len = 16;
        else if (args.technique.type == TECHNIQUE_AES_192_ENCRYPT) key_len = 24;
        else if (args.technique.type == TECHNIQUE_AES_256_ENCRYPT) key_len = 32;
    
        if (!derive_key(args.technique.value, key, key_len))
        {
            free(raw.data);
            return 1;
        }

        if (args.decrypt)
        {
            transformed = aes_decrypt_cbc(key, key_len, raw);
        } else
        {
            transformed = aes_encrypt_cbc(key, key_len, raw);
        }
    }
    
    free(raw.data);
    if (!transformed.data)
    {
        fprintf(stderr, "Error: Failed to encrypt/decrypt.\n");
        return 1;
    }

    payload = bytes_to_payload(transformed.data, transformed.len);
    if (!payload)
    {
        fprintf(stderr, "Error: Failed to format payload.\n");
        free(transformed.data);
        return 1;
    }

    printf("%s\n", payload);

    free(payload);
    free(transformed.data);

    return 0;
}

/* Helper */
char *require_value(int argc, char*argv[], int *i, const char *flag)
{
    if (*i + 1 >= argc)
    {
        fprintf(stderr, "Error: %s requires a value.\n", flag);
        exit(1);
    }
    return argv[++(*i)];
}

Arguments parse_arguments(int argc, char *argv[])
{
    Arguments args = {0};

    int i = 0;
    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--bin") == 0)
        {
            args.binary = require_value(argc, argv, &i, "-b/--bin");
        }
        else if (strcmp(argv[i], "-0") == 0 || strcmp(argv[i], "--hex" ) == 0)
        {
            args.hexadecimal = require_value(argc, argv, &i, "-0/--hex");
        }
        else if (strcmp(argv[i], "-a128") == 0 || strcmp(argv[i], "--aes128" ) == 0)
        {
            args.technique.type = TECHNIQUE_AES_128_ENCRYPT;
            args.technique.value = require_value(argc, argv, &i, "-a128/--aes128");
        }
        else if (strcmp(argv[i], "-a192") == 0 || strcmp(argv[i], "--aes192" ) == 0)
        {
            args.technique.type = TECHNIQUE_AES_192_ENCRYPT;
            args.technique.value = require_value(argc, argv, &i, "-a192/--aes192");
        }
        else if (strcmp(argv[i], "-a256") == 0 || strcmp(argv[i], "--aes256" ) == 0)
        {
            args.technique.type = TECHNIQUE_AES_256_ENCRYPT;
            args.technique.value = require_value(argc, argv, &i, "-a256/--aes256");
        }
        else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--rc4" ) == 0)
        {
            args.technique.type = TECHNIQUE_RC4_ENCRYPT;
            args.technique.value = require_value(argc, argv, &i, "-r/--rc4");
        }
        else if (strcmp(argv[i], "-x") == 0 || strcmp(argv[i], "--xor" ) == 0)
        {
            args.technique.type = TECHNIQUE_XOR_ENCRYPT;
            args.technique.value = require_value(argc, argv, &i, "-x/--xor");
        }
        else if (strcmp(argv[i], "-4") == 0 || strcmp(argv[i], "--ipv4" ) == 0)
        {
            args.technique.type = TECHNIQUE_IPV4_OBFUSCATE;
        }
        else if (strcmp(argv[i], "-6") == 0 || strcmp(argv[i], "--ipv6" ) == 0)
        {
            args.technique.type = TECHNIQUE_IPV6_OBFUSCATE;
        }
        else if (strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--uuid" ) == 0)
        {
            args.technique.type = TECHNIQUE_UUID_OBFUSCATE;
        }
        else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--mac" ) == 0)
        {
            args.technique.type = TECHNIQUE_MAC_OBFUSCATE;
        }
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help" ) == 0)
        {
            args.help_requested = 1;
        } 
        else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--decrypt") == 0)
        {
            args.decrypt = 1;
        } 
        else if (strcmp(argv[i], "-b64") == 0 || strcmp(argv[i], "--base64") == 0)
        {
            args.technique.type = TECHNIQUE_BASE64_OBFUSCATE;
        }
        else
        {
            fprintf(stderr, "Error: Unknown argument '%s'.\n", argv[i]);
            exit(1);
        }
    }
    return args;
}

int validate_arguments(Arguments args)
{
    /* Techniques encryption/obfuscation */
    int has_input = args.binary != NULL || args.hexadecimal != NULL;
    int has_technique = args.technique.type != TECHNIQUE_NONE;

    if (has_input && !has_technique)
    {
        fprintf(stderr, "Error: Need to specify encryption or obfuscation technique.\n");
        return 0;
    }

    if (!has_input && has_technique)
    {
        fprintf(stderr, "Error: Need to specify input file.\n");
        return 0;
    }

    /* Help, exclusive */
    if (args.help_requested) return 1;

    if (!has_input && !has_technique)
    {
        fprintf(stderr, "Error: Need to specify an input file and a technique.\n");
        return 0;
    }

    return 1;
}

RawBytes load_bytes_bin(const char *filepath)
{
    FILE *file;
    long file_size;
    unsigned char *buffer;
    size_t bytes_read;
    RawBytes result;
    result.data = NULL;
    result.len = 0;

    file = fopen(filepath, "rb");
    if (!file)
    {
        fprintf(stderr, "Error: Can't open binary file %s.\n", filepath);
        return result;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    rewind(file);

    if (file_size <= 0)
    {
        fprintf(stderr, "Error: Binary file is empty.\n");
        fclose(file);
        return result;
    }

    /* Read into buffer */
    buffer = malloc(file_size);
    if (!buffer)
    {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        fclose(file);
        return result;
    }

    bytes_read = fread(buffer, 1, file_size, file);
    fclose(file);

    if (bytes_read != (size_t)file_size)
    {
        fprintf(stderr, "Error: Failed to read entire file.\n");
        free(buffer);
        return result;
    }

    result.data = buffer;
    result.len = (size_t)file_size;
    return result;
}

RawBytes load_bytes_hex(const char *filepath)
{
    FILE *file;
    long file_size;
    unsigned char *buffer;
    size_t bytes_read;
    RawBytes result;
    char *clean_hex;
    int clean_idx;
    size_t i; 
    int hi, lo;
    size_t hex_len;
    result.data = NULL;
    result.len = 0;
    
    file = fopen(filepath, "rb");
    if (!file)
    {
        fprintf(stderr, "Error: Can't open hex file %s.\n", filepath);
        return result;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    rewind(file);

    if (file_size <= 0)
    {
        fprintf(stderr, "Error: Hex file is empty.\n");
        fclose(file);
        return result;
    }

    /* Read into buffer */
    buffer = malloc(file_size);
    if (!buffer)
    {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        fclose(file);
        return result;
    }

    bytes_read = fread(buffer, 1, file_size, file);
    fclose(file);

    if (bytes_read != (size_t)file_size)
    {
        fprintf(stderr, "Error: Failed to read entire file.\n");
        free(buffer);
        return result;
    }

    /* Curlies if there */
    clean_hex = malloc(file_size + 1);
    if (!clean_hex)
    {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        free(buffer);
        return result;
    }

    clean_idx = 0;
    for (i = 0; i < (size_t)file_size; i++)
    {
        char c = buffer[i];
        /* Remove braces, spaces, newlines, tabs */
        /* Be careful with lonely ascii, not handling that */
        if (c == '{' || c == '}' ||
            c == '(' || c == ')' ||
            c == '[' || c == ']' ||
            c == '\n' || c == '\t' ||
            c == ' ' || c == '\r' ||
            c == ',' || c == '='
        ) continue;

        if (i + 7 <= (size_t)file_size &&
            strncmp((const char*)buffer + i, "payload", 7) == 0)
        {
            i += 6;
            continue;
        }

        if (c == '0' && i + 1 < (size_t)file_size && buffer[i + 1] == 'x')
        {
            i++;
            continue;
        }

        clean_hex[clean_idx++] = c;
    }

    clean_hex[clean_idx] = '\0';
    free(buffer);

    /* Validate hex string, no stray characters */
    for (i = 0; i < (size_t)clean_idx; i++)
    {
        if (!isxdigit((unsigned char) clean_hex[i]))
        {
            fprintf(stderr, "Error: Invalid hex character '%c'.\n", clean_hex[i]);
            free(clean_hex);
            return result;
        }
    }

    /* Count if all are pairs */
    if (strlen(clean_hex) % 2 != 0)
    {
        fprintf(stderr, "Error: Hex string has odd length (must be pairs).\n");
        free(clean_hex);
        return result;
    }

    /* Convert to correct hex */
    hex_len = strlen(clean_hex);

    /* In case empty file */
    if (hex_len == 0)
    {
        fprintf(stderr, "Error: No hex data found after cleaning.\n");
        free(clean_hex);
        return result;
    }

    result.data = malloc(hex_len / 2);
    if (!result.data)
    {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        free(clean_hex);
        return result;
    }

    for (i = 0; i < hex_len; i += 2)
    {
        hi = hex_val(clean_hex[i]);
        lo = hex_val(clean_hex[i+1]);
        result.data[i / 2] = (unsigned char)((hi << 4) | lo);
    }
    result.len = hex_len / 2;

    free(clean_hex);
    return result;
}

static int hex_val(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

/* Prep passphrase */
int derive_key(const char *passphrase, unsigned char *key, int key_len)
{
    EVP_MD_CTX *mdctx;
    unsigned char digest[32];
    unsigned int digest_len = 0;

    if (key_len < 1 || key_len > 32)
    {
        fprintf(stderr, "Error: Invalid key length.\n");
        return 0;
    }
    
    mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        fprintf(stderr, "Error: Failed to create digest context.\n");
        return 0;
    }

    if (!EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL))
    {
        fprintf(stderr, "Error: Failed to init digest.\n");
        EVP_MD_CTX_free(mdctx);
        return 0;
    }

    if (!EVP_DigestUpdate(mdctx, passphrase, strlen(passphrase)))
    {
        fprintf(stderr, "Error: Failed to hash passphrase.\n");
        EVP_MD_CTX_free(mdctx);
        return 0;
    }

    if (!EVP_DigestFinal_ex(mdctx, digest, &digest_len))
    {
        fprintf(stderr, "Error: Failed to finalize digest.\n");
        EVP_MD_CTX_free(mdctx);
        return 0;
    }

    EVP_MD_CTX_free(mdctx);
    memcpy(key, digest, (size_t)key_len);
    return 1;
}

/* AES Encryption */
RawBytes aes_encrypt_cbc(const unsigned char *key, int key_len, RawBytes input)
{
    RawBytes result;
    EVP_CIPHER_CTX *ctx;
    const EVP_CIPHER *cipher;
    unsigned char iv[16];
    int len, final_len;

    result.data = NULL;
    result.len = 0;

    if (key_len == 16) cipher = EVP_aes_128_cbc();
    else if (key_len == 24) cipher = EVP_aes_192_cbc();
    else if (key_len == 32) cipher = EVP_aes_256_cbc();
    else return result;

    if (!RAND_bytes(iv, 16))
    {
        fprintf(stderr, "Error: Failed to generate IV.\n");
        return result;
    }
    
    /* 16 for the IV + 16 for PKCS#7 padding */
    result.data = malloc(input.len + 32);
    if (!result.data) 
    {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return result;
    }

    memcpy(result.data, iv, 16);

    ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
    {
        fprintf(stderr, "Error: Failed to create cipher context.\n");
        free(result.data);
        result.data = NULL;
        return result;
    }

    if (!EVP_EncryptInit_ex(ctx, cipher, NULL, key, iv))
    {
        fprintf(stderr, "Error: Failed to init cipher.\n");
        EVP_CIPHER_CTX_free(ctx);
        free(result.data);
        result.data = NULL;
        return result;
    }

    if (!EVP_EncryptUpdate(ctx, result.data + 16, &len, input.data, (int)input.len) ||
        !EVP_EncryptFinal_ex(ctx, result.data + 16 + len, &final_len))
    {
        fprintf(stderr, "Error: Encryption failed.\n");
        EVP_CIPHER_CTX_free(ctx);
        free(result.data);
        result.data = NULL;
        return result;
    }

    EVP_CIPHER_CTX_free(ctx);

    result.len = 16 + (size_t)len + (size_t)final_len;
    return result;
}

/* Convert to hex */
char *bytes_to_payload(const unsigned char *data, size_t len)
{
    char *payload;
    size_t out_size;
    size_t i;
    int offset;

    if (len == 0)
    {
        fprintf(stderr, "Error: No data to format.\n");
        return NULL;
    }

    /* 12 prefix + 6 per byte (last one 4) + 2 suffix + 1 NUL */
    out_size = len * 6 + 13;
    payload = malloc(out_size);
    if (!payload)
    {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return NULL;
    }

    offset = 0;
    offset += sprintf(payload + offset, "payload = { ");
    for (i = 0; i < len; i++)
    {
        if (i < len - 1)
            offset += sprintf(payload + offset, "0x%02x, ", data[i]);
        else
            offset += sprintf(payload + offset, "0x%02x", data[i]);
    }
    offset += sprintf(payload + offset, " }");

    return payload;
}

/* AES Decryptions */
RawBytes aes_decrypt_cbc(const unsigned char *key, int key_len, RawBytes input)
{
    RawBytes result;
    EVP_CIPHER_CTX *ctx;
    const EVP_CIPHER *cipher;
    int len, final_len;

    result.data = NULL;
    result.len = 0;

    if (key_len == 16) cipher = EVP_aes_128_cbc();
    else if (key_len == 24) cipher = EVP_aes_192_cbc();
    else if (key_len == 32) cipher = EVP_aes_256_cbc();
    else return result;

    if (input.len < 32)
    {
        fprintf(stderr, "Error: Ciphertext too short.\n");
        return result;
    }

    result.data = malloc(input.len - 16);
    if (!result.data)
    {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return result;
    }

    ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
    {
        fprintf(stderr, "Error: Failed to create cipher context.\n");
        free(result.data);
        result.data = NULL;
        return result;
    }

    /* First 16 bytes -> IV */
    if (!EVP_DecryptInit_ex(ctx, cipher, NULL, key, input.data))
    {
        fprintf(stderr, "Error: Failed to init cipher.\n");
        EVP_CIPHER_CTX_free(ctx);
        free(result.data);
        result.data = NULL;
        return result;
    }

    if (!EVP_DecryptUpdate(ctx, result.data, &len, input.data + 16, (int)(input.len - 16)) || 
        !EVP_DecryptFinal_ex(ctx, result.data + len, &final_len))
    {
        fprintf(stderr, "Error: Decryption failed (wrong key or corrupted data).\n");
        EVP_CIPHER_CTX_free(ctx);
        free(result.data);
        result.data = NULL;
        return result;
    }

    EVP_CIPHER_CTX_free(ctx);

    result.len = (size_t)len + (size_t)final_len;
    return result;
}

/* XOR Encrypt/Decrypt */
RawBytes xor_encrypt(const unsigned char *key, size_t key_len, RawBytes input)
{
    RawBytes result;
    size_t i;
    
    result.data = NULL;
    result.len = 0;

    if (key_len == 0 || input.len == 0)
    {
        fprintf(stderr, "Error: XOR needs non-empty key and data.\n");
        return result;
    }

    result.data = malloc(input.len);
    if (!result.data) 
    {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return result;
    }

    for (i = 0; i < input.len; i++)
    {
        result.data[i] = input.data[i] ^ key[i % key_len];
    }

    result.len = input.len;
    return result;
}

/* KSA Key-Scheduling Algorithm */
static void rc4_ksa(unsigned char *S, const unsigned char *key, size_t key_len)
{
    size_t i;
    unsigned char j = 0;

    for (i = 0; i < 256; i++)
    {
        S[i] = (unsigned char)i;
    }

    for (i = 0; i < 256; i++)
    {
        unsigned char tmp;

        j = (unsigned char)(j + S[i] + key[i % key_len]);
        tmp = S[i];
        S[i] = S[j];
        S[j] = tmp;
    }
}

/* Pseudo-Random Generation Algorithm */
static void rc4_crypt(const unsigned char *key, size_t key_len,
                      const unsigned char *in, size_t in_len, unsigned char *out)
{
    unsigned char S[256];
    unsigned char i = 0, j = 0;
    size_t n;

    rc4_ksa(S, key, key_len);

    for (n = 0; n < in_len; n++)
    {
        unsigned char tmp;
        i = (unsigned char)(i + 1);
        j = (unsigned char)(j + S[i]);

        tmp = S[i];
        S[i] = S[j];
        S[j] = tmp;

        out[n] = in[n] ^ S[(S[i] + S[j]) & 0xFF];
    }
}

/* RC4 Encrypt/Decrypt */
RawBytes rc4_encrypt(const unsigned char *key, size_t key_len, RawBytes input)
{
    RawBytes result;

    result.data = NULL;
    result.len = 0;

    if (key_len == 0 || key_len > 256 || input.len == 0)
    {
        fprintf(stderr, "Error: RC4 key must be 1-256 bytes, and data non-empty.\n");
        return result;
    }

    result.data = malloc(input.len);
    if (!result.data)
    {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return result;
    }

    rc4_crypt(key, key_len, input.data, input.len, result.data);

    result.len = input.len;
    return result;
}

/* Base64 Encode / Obfuscate */
RawBytes b64_obfuscate(RawBytes input)
{
    RawBytes result;
    size_t i, j;
    unsigned char byte1, byte2, byte3;
    result.data = NULL;
    result.len = 0;

    if (input.len == 0)
    {
        fprintf(stderr, "Error: No data to encode.\n");
        return result;
    }

    result.len = ((input.len + 2) / 3) * 4;
    result.data = malloc(result.len + 1);

    if (!result.data)
    {
        fprintf(stderr, "Error: Failed to allocate memory.\n");
        return result;
    }

    i = 0;
    j = 0;

    while (i < input.len)
    {
        size_t remaining = input.len - i;
        byte1 = input.data[i++];
        byte2 = (remaining > 1) ? input.data[i++] : 0;
        byte3 = (remaining > 2) ? input.data[i++] : 0;

        result.data[j++] = base64_table[(byte1 >> 2) & 0x3F];
        result.data[j++] = base64_table[((byte1 & 0x03) << 4) | ((byte2 >> 4) & 0x0F)];

        if (remaining > 1)
        {
            result.data[j++] = base64_table[((byte2 & 0x0F) << 2) | ((byte3 >> 6) & 0x03)];
        }
        else
        {
            result.data[j++] = '=';
        }

        if (remaining > 2)
        {
            result.data[j++] = base64_table[byte3 & 0x3F];
        }
        else
        {
            result.data[j++] = '=';
        }
    }

    result.data[j] = '\0';
    return result;
}

/* Reverse lookup */
static int b64_val(char c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

/* Base64 Decode / Deobfuscate */
RawBytes b64_deobfuscate(RawBytes input)
{
    RawBytes result;
    size_t i, j;
    int count;
    int v[4];

    result.data = NULL;
    result.len = 0;

    if (input.len == 0)
    {
        fprintf(stderr, "Error: No data to decode.\n");
        return result;
    }

    result.data = malloc((input.len / 4) * 3 + 3);
    if (!result.data)
    {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return result;
    }

    i = 0;
    j = 0;
    while (i < input.len)
    {
        count = 0;
        while (i < input.len && count < 4)
        {
            char c = (char)input.data[i];
            i++;

            if (c == ' ' || c == '\n' || c == '\t' || c == '\r')
                continue;

            if (c == '=')
            {
                v[count++] = -2;
                continue;
            }

            v[count] = b64_val(c);
            if (v[count] < 0)
            {
                free(result.data);
                result.data = NULL;
                fprintf(stderr, "Error: Invalid base64 character '%c'.\n", c);
                return result;
            }
            count++;
        }

        if (count == 0)
            continue;

        if (count < 4 || v[0] < 0 || v[1] < 0)
        {
            free(result.data);
            result.data = NULL;
            fprintf(stderr, "Error: Invalid base64 length.\n");
            return result;
        }

        result.data[j++] = (unsigned char)((v[0] << 2) | (v[1] >> 4));

        if (v[2] == -2)
        {
            if (v[3] != -2)
            {
                free(result.data);
                result.data = NULL;
                fprintf(stderr, "Error: Invalid base64 padding.\n");
                return result;
            }
        }
        else
        {
            result.data[j++] = (unsigned char)((v[1] << 4) | (v[2] >> 2));

            if (v[3] == -2)
            {
                /* Nothing to do */
            }
            else
            {
                result.data[j++] = (unsigned char)((v[2] << 6) | v[3]);
            }
        }
    }

    result.len = j;
    return result;
}

/* IPv4 Obfuscate */
RawBytes ipv4_obfuscate(RawBytes input) 
{
    RawBytes result;
    unsigned char *padded;
    size_t padded_len;
    size_t i;
    size_t off;

    result.data = NULL;
    result.len = 0;

	if (input.len == 0)
    {
        fprintf(stderr, "Error: No data to obfuscate.\n");
		return result;
	}

    padded_len = ((input.len + 7) / 4) * 4;
    padded = malloc(padded_len);
    if (!padded)
    {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return result;
    }

    padded[0] = (unsigned char)(input.len & 0xFF);
    padded[1] = (unsigned char)((input.len >> 8) & 0xFF);
    padded[2] = (unsigned char)((input.len >> 16) & 0xFF);
    padded[3] = (unsigned char)((input.len >> 24) & 0xFF);

    for (i = 0; i < input.len; i++) padded[4 + i] = input.data[i];
    for (i = 4 + input.len; i < padded_len; i++) padded[i] = 0;

    result.len = (padded_len / 4) * 16;
    result.data = malloc(result.len + 1);
    if (!result.data)
    {
        result.len = 0;
        free(padded);
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return result;
    }

    off = 0;
    for (i = 0; i < padded_len; i += 4)
    {
        if (off > 0) result.data[off++] = ' ';

        off += sprintf((char *) result.data + off, "%u.%u.%u.%u", 
                       (unsigned int) padded[i],
                       (unsigned int) padded[i + 1],
                       (unsigned int) padded[i + 2],
                       (unsigned int) padded[i + 3]
                    );
    }
    
    result.data[off] = '\0';
    result.len = off;

    free(padded);
	return result;
}

/* Parse quad number */
static int parse_quad_number(const unsigned char *s, size_t len, size_t *pos)
{
    int value = 0;

    if (*pos >= len || s[*pos] < '0' || s[*pos] > '9')
        return -1;

    while (*pos < len && s[*pos] >= '0' && s[*pos] <= '9')
    {
        value = value * 10 + (int)(s[*pos] - '0');
        if (value > 255)
            return -1;
        (*pos)++;
    }
    return value;
}

/* IPV4 Deobfuscate */
RawBytes ipv4_deobfuscate(RawBytes input)
{
    RawBytes result;
    unsigned char *bytes;
    size_t pos, count, i;
    size_t original_len;
    int n;

    result.data = NULL;
    result.len = 0;

    if (input.len == 0)
    {
        fprintf(stderr, "Error: No data to deobfuscate.\n");
        return result;
    }

    bytes = malloc(input.len);
    if (!bytes)
    {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return result;
    }

    pos = 0;
    count = 0;
    while (pos < input.len)
    {
        if (input.data[pos] == ' ' || input.data[pos] == '\n' ||
            input.data[pos] == '\t' || input.data[pos] == '\r')
        {
            pos++;
            continue;
        }

        n = parse_quad_number(input.data, input.len, &pos);
        if (n < 0)
        {
            free(bytes);
            fprintf(stderr, "Error: Invalid IPv4 format.\n");
            return result;
        }
        bytes[count++] = (unsigned char)n;

        if (pos < input.len && input.data[pos] == '.')
            pos++;
    }

    if (count < 8 || count % 4 != 0)
    {
        free(bytes);
        fprintf(stderr, "Error: Invalid IPv4 payload length.\n");
        return result;
    }

    original_len = (size_t)bytes[0] |
                   ((size_t)bytes[1] << 8) |
                   ((size_t)bytes[2] << 16) |
                   ((size_t)bytes[3] << 24);

    if (original_len == 0 || original_len > count - 4)
    {
        free(bytes);
        fprintf(stderr, "Error: Invalid length header.\n");
        return result;
    }

    result.data = malloc(original_len);
    if (!result.data)
    {
        free(bytes);
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return result;
    }

    for (i = 0; i < original_len; i++)
        result.data[i] = bytes[4 + i];

    result.len = original_len;
    free(bytes);
    return result;
}

/* IPv6 Obfuscate */
RawBytes ipv6_obfuscate(RawBytes input)
{
    RawBytes result;
    unsigned char *padded;
    size_t padded_len;
    size_t i;
    size_t off;
    size_t group;

    result.data = NULL;
    result.len = 0;

    if (input.len == 0)
    {
        fprintf(stderr, "Error: No data to obfuscate.\n");
        return result;
    }

    padded_len = ((input.len + 19) / 16) * 16;
    padded = malloc(padded_len);
    if (!padded)
    {
        fprintf(stderr, "Error: Failed to allocate memory.\n");
        return result;
    }

    padded[0] = (unsigned char)(input.len & 0xFF);
    padded[1] = (unsigned char)((input.len >> 8) & 0xFF);
    padded[2] = (unsigned char)((input.len >> 16) & 0xFF);
    padded[3] = (unsigned char)((input.len >> 24) & 0xFF);
    
    for (i = 0; i < input.len; i++) padded[4 + i] = input.data[i];
    for (i = 4 + input.len; i < padded_len; i++) padded[i] = 0;

    result.len = (padded_len / 16) * 40;
    result.data = malloc(result.len + 1);
    if (!result.data)
    {
        result.len = 0;
        free(padded);
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return result;
    }

    off = 0;
    for (i = 0; i < padded_len; i += 16)
    {
        if (off > 0) result.data[off++] = ' ';
        for (group = 0; group < 8; group++)
        {
            if (group > 0) result.data[off++] = ':';

            off += sprintf((char *)result.data + off, "%02x%02x",
                            padded[i + group * 2],
                            padded[i + group * 2 + 1]);
        }
    }

    result.data[off] = '\0';
    result.len = off;

    free(padded);
    return result;
}

/* IPv6 Deobfuscate */
RawBytes ipv6_deobfuscate(RawBytes input)
{
    RawBytes result;
    unsigned char *bytes;
    size_t pos, count, i;
    size_t original_len;
    int hi, lo;

    result.data = NULL;
    result.len = 0;

    if (input.len == 0)
    {
        fprintf(stderr, "Error: No data to deobfuscate.\n");
        return result;
    }

    bytes = malloc(input.len / 2 + 1);
    if (!bytes)
    {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return result;
    }

    pos = 0;
    count = 0;
    while (pos < input.len)
    {
        if (input.data[pos] == ' ' || input.data[pos] == '\n' ||
            input.data[pos] == '\t' || input.data[pos] == '\r' ||
            input.data[pos] == ':')
        {
            pos++;
            continue;
        }

        if (pos + 1 >= input.len)
        {
            free(bytes);
            fprintf(stderr, "Error: Invalid IPv6 format.\n");
            return result;
        }

        hi = hex_val(input.data[pos]);
        lo = hex_val(input.data[pos + 1]);
        if (hi < 0 || lo < 0)
        {
            free(bytes);
            fprintf(stderr, "Error: Invalid IPv6 format.\n");
            return result;
        }

        bytes[count++] = (unsigned char)((hi << 4) | lo);
        pos += 2;
    }

    if (count < 20 || count % 16 != 0)
    {
        free(bytes);
        fprintf(stderr, "Error: Invalid IPv6 payload length.\n");
        return result;
    }

    original_len = (size_t)bytes[0] | 
                    ((size_t) bytes[1] << 8) |
                    ((size_t) bytes[2] << 16) |
                    ((size_t) bytes[3] << 24);
    
    if (original_len == 0 || original_len > count - 4)
    {
        free(bytes);
        fprintf(stderr, "Error: Invalid length header.\n");
        return result;
    }

    result.data = malloc(original_len);
    if (!result.data)
    {
        free(bytes);
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return result;
    }

    for (i = 0; i < original_len; i++) result.data[i] = bytes[4 + i];
    
    result.len = original_len;
    free(bytes);
    return result;
}

/* UUID Obfuscate */
RawBytes uuid_obfuscate(RawBytes input)
{
    RawBytes result;
    unsigned char *padded;
    size_t padded_len;
    size_t i;
    size_t off;

    result.data = NULL;
    result.len = 0;

    if (input.len == 0)
    {
        fprintf(stderr, "Error: No data to obfuscate.\n");
        return result;
    }

    padded_len = ((input.len + 19) / 16) * 16;
    padded = malloc(padded_len);
    if (!padded)
    {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return result;
    }

    padded[0] = (unsigned char)(input.len & 0xFF);
    padded[1] = (unsigned char)((input.len >> 8) & 0xFF);
    padded[2] = (unsigned char)((input.len >> 16) & 0xFF);
    padded[3] = (unsigned char)((input.len >> 24) & 0xFF);

    for (i = 0; i < input.len; i++) padded[4 + i] = input.data[i];
    for (i = 4 + input.len; i < padded_len; i++) padded[i] = 0;

    result.len = (padded_len / 16) * 37;
    result.data = malloc(result.len + 1);
    if (!result.data)
    {
        result.len = 0;
        free(padded);
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return result;
    }

    off = 0;
    for (i = 0; i < padded_len; i += 16)
    {
        if (off > 0) result.data[off++] = ' ';

        off += sprintf((char *)result.data + off, "%02x%02x%02x%02x",
                       padded[i + 0], padded[i + 1],
                       padded[i + 2], padded[i + 3]);
        off += sprintf((char *)result.data + off, "-%02x%02x",
                       padded[i + 4], padded[i + 5]);
        off += sprintf((char *)result.data + off, "-%02x%02x",
                       padded[i + 6], padded[i + 7]);
        off += sprintf((char *)result.data + off, "-%02x%02x",
                       padded[i + 8], padded[i + 9]);
        off += sprintf((char *)result.data + off, "-%02x%02x%02x%02x%02x%02x",
                       padded[i + 10], padded[i + 11], padded[i + 12],
                       padded[i + 13], padded[i + 14], padded[i + 15]);        
    }

    result.data[off] = '\0';
    result.len = off;

    free(padded);
    return result;
}

/* UUID Deobfuscate */
RawBytes uuid_deobfuscate(RawBytes input)
{
    RawBytes result;
    unsigned char *bytes;
    size_t pos, count, i;
    size_t original_len;
    int hi, lo;

    result.data = NULL;
    result.len = 0;

    if (input.len == 0)
    {
        fprintf(stderr, "Error: No data to deobfuscate.\n");
        return result;
    }

    bytes = malloc(input.len / 2 + 1);
    if (!bytes)
    {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return result;
    }

    pos = 0;
    count = 0;
    while (pos < input.len)
    {
        if (input.data[pos] == ' ' || input.data[pos] == '\n' ||
            input.data[pos] == '\t' || input.data[pos] == '\r' ||
            input.data[pos] == '-')
        {
            pos++;
            continue;
        }

        if (pos + 1 >= input.len)
        {
            free(bytes);
            fprintf(stderr, "Error: Invalid UUID format.\n");
            return result;
        }

        hi = hex_val(input.data[pos]);
        lo = hex_val(input.data[pos + 1]);
        if (hi < 0 || lo < 0)
        {
            free(bytes);
            fprintf(stderr, "Error: Invalid UUID format.\n");
            return result;
        }

        bytes[count++] = (unsigned char)((hi << 4) | lo);
        pos += 2;
    }

    if (count < 20 || count % 16 != 0)
    {
        free(bytes);
        fprintf(stderr, "Error: Invalid UUID payload length.\n");
        return result;
    }

    original_len = (size_t)bytes[0] |
                   ((size_t)bytes[1] << 8) |
                   ((size_t)bytes[2] << 16) |
                   ((size_t)bytes[3] << 24);
    
    if (original_len == 0 || original_len > count - 4)
    {
        free(bytes);
        fprintf(stderr, "Error: Invalid length header.\n");
        return result;
    }

    result.data = malloc(original_len);
    if (!result.data)
    {
        free(bytes);
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return result;
    }

    for (i = 0; i < original_len; i++) result.data[i] = bytes[4 + i];

    result.len = original_len;
    free(bytes);
    return result;
}

/* MAC Obfuscation */
RawBytes mac_obfuscate(RawBytes input)
{
    RawBytes result;
    unsigned char *padded;
    size_t padded_len;
    size_t i;
    size_t off;

    result.data = NULL;
    result.len = 0;

    if (input.len == 0)
    {
        fprintf(stderr, "Error: No data to obfuscate.\n");
        return result;
    }

    padded_len = ((input.len + 9) / 6) * 6;
    padded = malloc(padded_len);
    if (!padded)
    {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return result;
    }

    padded[0] = (unsigned char)(input.len & 0xFF);
    padded[1] = (unsigned char)((input.len >> 8) & 0xFF);
    padded[2] = (unsigned char)((input.len >> 16) & 0xFF);
    padded[3] = (unsigned char)((input.len >> 24) & 0xFF);

    for (i = 0; i < input.len; i++) padded[4 + i] = input.data[i];
    for (i = 4 + input.len; i < padded_len; i++) padded[i] = 0;

    result.len = (padded_len / 6) * 18;
    result.data = malloc(result.len + 1);
    if (!result.data)
    {
        result.len = 0;
        free(padded);
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return result;
    }

    off = 0;
    for (i = 0; i < padded_len; i += 6)
    {
        if (off > 0) result.data[off++] = ' ';
        off += sprintf((char *)result.data + off,
                      "%02x:%02x:%02x:%02x:%02x:%02x",
                      padded[i + 0], padded[i + 1], padded[i + 2],
                      padded[i + 3], padded[i + 4], padded[i + 5]);
    }

    result.data[off] = '\0';
    result.len = off;

    free(padded);
    return result;
}

/* MAC Deobfuscate*/
RawBytes mac_deobfuscate(RawBytes input)
{
    RawBytes result;
    unsigned char *bytes;
    size_t pos, count, i;
    size_t original_len;
    int hi, lo;

    result.data = NULL;
    result.len = 0;

    if (input.len == 0)
    {
        fprintf(stderr, "Error: No data to deobfuscate.\n");
        return result;
    }

    bytes = malloc(input.len / 2 + 1);
    if (!bytes)
    {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return result;
    }

    pos = 0;
    count = 0;
    while (pos < input.len)
    {
        if (input.data[pos] == ' ' || input.data[pos] == '\n' ||
            input.data[pos] == '\t' || input.data[pos] == '\r' ||
            input.data[pos] == ':' || input.data[pos] == '-')
        {
            pos++;
            continue;
        }
        
        if (pos + 1 >= input.len)
        {
            free(bytes);
            fprintf(stderr, "Error: Invalid MAC address.\n");
            return result;
        }

        hi = hex_val(input.data[pos]);
        lo = hex_val(input.data[pos + 1]);
        if (hi < 0 || lo < 0)
        {
            free(bytes);
            fprintf(stderr, "Error: Invalid MAC address.\n");
            return result;
        }

        bytes[count++] = (unsigned char)((hi << 4) | lo);
        pos += 2;
    }

    if (count < 10 || count % 6 != 0)
    {
        free(bytes);
        fprintf(stderr, "Error: Invalid MAC payload length.\n");
        return result;
    }

    original_len = (size_t)bytes[0] |
                   ((size_t)bytes[1] << 8) |
                   ((size_t)bytes[2] << 16) |
                   ((size_t)bytes[3] << 24);
    
    if (original_len == 0 || original_len > count - 4)
    {
        free(bytes);
        fprintf(stderr, "Error: Invalid length header.\n");
        return result;
    }

    result.data = malloc(original_len);
    if (!result.data)
    {
        free(bytes);
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return result;
    }

    for (i = 0; i < original_len; i++) result.data[i] = bytes[4 + i];

    result.len = original_len;
    free(bytes);
    return result;
}
