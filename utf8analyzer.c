#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_INPUT_SIZE 256

//function prototypes
int32_t is_ascii(const char str[]);
int32_t capitalize_ascii(char str[]);
int32_t width_from_start_byte(char start_byte);
int32_t utf8_strlen(const char str[]);
int32_t codepoint_index_to_byte_index(const char str[], int32_t cpi);
void utf8_substring(const char str[], int32_t cpi_start, int32_t cpi_end, char result[]);
int32_t codepoint_at(const char str[], int32_t cpi);
bool is_animal_emoji_at(const char str[], int32_t cpi);
void analyze_utf8(const char *input);
void next_utf8_char(const char str[], int32_t cpi, char result[]);

int main(int argc, char *argv[]) {
    char input[MAX_INPUT_SIZE] = {0};

    //check if an argument is provided or read from stdin
    if (argc > 1) {
        FILE *file = fopen(argv[1], "r");
        if (file) {
            fread(input, sizeof(char), MAX_INPUT_SIZE - 1, file);
            input[MAX_INPUT_SIZE - 1] = '\0'; //ensure null termination
            fclose(file);
        } else {
            fprintf(stderr, "error opening file: %s\n", argv[1]);
            return 1; //exit with error
        }
    } else {
        printf("enter a utf-8 encoded string:\n");
        if (fgets(input, sizeof(input), stdin) != NULL) {
            input[strcspn(input, "\n")] = 0; //remove newline
        } else {
            fprintf(stderr, "error reading input.\n");
            return 1; //exit with error
        }
    }

    analyze_utf8(input);
    return 0;
}

void analyze_utf8(const char *input) {
    bool valid_ascii = is_ascii(input);
    char uppercased[MAX_INPUT_SIZE];
    strncpy(uppercased, input, MAX_INPUT_SIZE);
    uppercased[MAX_INPUT_SIZE - 1] = '\0'; //ensure null termination
    capitalize_ascii(uppercased);

    int32_t length = strlen(input);
    int32_t num_code_points = utf8_strlen(input);
    
    printf("valid ascii: %s\n", valid_ascii ? "true" : "false");
    printf("uppercased ascii: \"%s\"\n", uppercased);
    printf("length in bytes: %d\n", length);
    printf("number of code points: %d\n", num_code_points);

    printf("bytes per code point: ");
    for (int32_t i = 0; i < num_code_points; i++) {
        int32_t byte_index = codepoint_index_to_byte_index(input, i);
        if (byte_index >= 0) {
            int width = width_from_start_byte(input[byte_index]);
            if (width > 0) { //only print valid widths
                printf("%d ", width);
            }
        }
    }
    printf("\n");

    char substring[MAX_INPUT_SIZE];
    utf8_substring(input, 0, num_code_points < 6 ? num_code_points : 6, substring);
    printf("substring of the first 6 code points: \"%s\"\n", substring);

    printf("code points as decimal numbers: ");
    for (int32_t i = 0; i < num_code_points; i++) {
        int32_t codepoint = codepoint_at(input, i);
        if (codepoint >= 0) {
            printf("%d", codepoint);
            if (i < num_code_points - 1) {
                printf(" "); //space between code points
            }
        }
    }
    printf("\n");

    //check for specific animal emojis
    bool has_animal_emoji = false;
    for (int32_t i = 0; i < num_code_points; i++) {
        if (is_animal_emoji_at(input, i)) {
            has_animal_emoji = true;
            break;
        }
    }

    printf("animal emojis: %s\n", has_animal_emoji ? "🐩" : "none");

    //call the next_utf8_char function
    char next_char[10];
    next_utf8_char(input, 3, next_char);
    printf("next character of codepoint at index 3: %s\n", next_char);
}

int32_t is_ascii(const char str[]) {
    for (int i = 0; str[i] != '\0'; i++) {
        if ((unsigned char)str[i] > 127) {
            return 0; //not ascii
        }
    }
    return 1; //all ascii
}

int32_t capitalize_ascii(char str[]) {
    int count = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            str[i] -= ('a' - 'A'); //convert to uppercase
            count++;
        }
    }
    return count; //return number of characters updated
}

int32_t width_from_start_byte(char start_byte) {
    unsigned char byte = (unsigned char)start_byte;
    if (byte <= 0x7F) return 1; 
    if ((byte & 0xE0) == 0xC0) return 2; 
    if ((byte & 0xF0) == 0xE0) return 3; 
    if ((byte & 0xF8) == 0xF0) return 4; 
    return -1; //invalid start byte
}

int32_t utf8_strlen(const char str[]) {
    int32_t count = 0;
    for (int i = 0; str[i] != '\0';) {
        int width = width_from_start_byte(str[i]);
        if (width < 0) return -1; //error in utf-8
        i += width; //move to next character
        count++;
    }
    return count; //return number of code points
}

int32_t codepoint_index_to_byte_index(const char str[], int32_t cpi) {
    int32_t current_cpi = 0;
    for (int i = 0; str[i] != '\0';) {
        int width = width_from_start_byte(str[i]);
        if (width < 0) return -1; //error
        if (current_cpi == cpi) return i; //found byte index
        i += width;
        current_cpi++;
    }
    return -1; //code point index out of range
}

void utf8_substring(char str[], int32_t cpi_start, int32_t cpi_end, char result[]) {
    //check for invalid inputs
    if (cpi_start < 0 || cpi_end <= cpi_start) {
        result[0] = '\0'; //set result to empty string
        return;
    }

    //find the byte index for the start and end code points
    int32_t byte_index_start = codepoint_index_to_byte_index(str, cpi_start);
    int32_t byte_index_end = codepoint_index_to_byte_index(str, cpi_end);

    //check for invalid byte indices
    if (byte_index_start < 0 || byte_index_end < 0) {
        result[0] = '\0'; // set result to empty string
        return;
    }

    //ensure the end index does not exceed the string length
    if (byte_index_end > strlen(str)) {
        byte_index_end = strlen(str);
    }

    //copy the substring from the original string
    int j = 0;
    for (int i = byte_index_start; i < byte_index_end; i++) {
        result[j++] = str[i];
    }
    result[j] = '\0'; //null terminate result
}

int32_t codepoint_at(const char str[], int32_t cpi) {
    int32_t byte_index = codepoint_index_to_byte_index(str, cpi);
    if (byte_index < 0) return -1; //error

    unsigned char first_byte = (unsigned char)str[byte_index];
    int32_t codepoint = 0;

    //decode the utf-8 code point
    if (first_byte <= 0x7F) {
        codepoint = first_byte; //1-byte
    } else if ((first_byte & 0xE0) == 0xC0) {
        codepoint = first_byte & 0x1F; //2-byte
        codepoint = (codepoint << 6) | (unsigned char)str[byte_index + 1] & 0x3F;
    } else if ((first_byte & 0xF0) == 0xE0) {
        codepoint = first_byte & 0x0F; //3-byte
        codepoint = (codepoint << 6) | ((unsigned char)str[byte_index + 1] & 0x3F);
        codepoint = (codepoint << 6) | ((unsigned char)str[byte_index + 2] & 0x3F);
    } else if ((first_byte & 0xF8) == 0xF0) {
        codepoint = first_byte & 0x07; //4-byte
        codepoint = (codepoint << 6) | ((unsigned char)str[byte_index + 1] & 0x3F);
        codepoint = (codepoint << 6) | ((unsigned char)str[byte_index + 2] & 0x3F);
        codepoint = (codepoint << 6) | ((unsigned char)str[byte_index + 3] & 0x3F);
    }
    return codepoint; //return the code point value
}

bool is_animal_emoji_at(const char str[], int32_t cpi) {
    int32_t codepoint = codepoint_at(str, cpi);
    return (codepoint == 0x1F429 || codepoint == 0x1F408); //🐩 and 🐈
}

void next_utf8_char(const char str[], int32_t cpi, char result[]) {
    int32_t byte_index = codepoint_index_to_byte_index(str, cpi);
    if (byte_index < 0) {
        result[0] = '\0'; //invalid index
        return;
    }

    int32_t codepoint = codepoint_at(str, cpi);
    if (codepoint < 0) {
        result[0] = '\0'; //error retrieving code point
        return;
    }

    //increment code point
    codepoint++;

    //convert back to utf-8
    if (codepoint <= 0x7F) {
        result[0] = (char)codepoint;
        result[1] = '\0';
    } else if (codepoint <= 0x7FF) {
        result[0] = (char)((codepoint >> 6) | 0xC0);
        result[1] = (char)((codepoint & 0x3F) | 0x80);
        result[2] = '\0';
    } else if (codepoint <= 0xFFFF) {
        result[0] = (char)((codepoint >> 12) | 0xE0);
        result[1] = (char)(((codepoint >> 6) & 0x3F) | 0x80);
        result[2] = (char)((codepoint & 0x3F) | 0x80);
        result[3] = '\0';
    } else if (codepoint <= 0x10FFFF) {
        result[0] = (char)((codepoint >> 18) | 0xF0);
        result[1] = (char)(((codepoint >> 12) & 0x3F) | 0x80);
        result[2] = (char)(((codepoint >> 6) & 0x3F) | 0x80);
        result[3] = (char)((codepoint & 0x3F) | 0x80);
        result[4] = '\0';
    } else {
        result[0] = '\0'; //invalid code point
    }
}
