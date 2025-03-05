#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>  // Consider adding string.h for strcmp
#include <ctype.h>

typedef enum {
    TOKEN_IDENT, TOKEN_CONST, TOKEN_ASSIGN_OP, TOKEN_ADD_OP, TOKEN_MIN_OP, TOKEN_MULT_OP, TOKEN_DIVI_OP,
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN, TOKEN_SEMI_COLON, TOKEN_END
} TokenType;

char token_string[100];
int next_token;

// Global variables to keep track of identifiers and constants
int ident_count = 0, const_count = 0, op_count = 0;
int num_idents = 0;

// �־����ڵ�
typedef struct {
    char name[50];
    char value[100];
}Ident;

char line[100];
FILE* file;
Ident idArray[50];

void printResultByLine(char* line, int ID, int CON, int OP);
void printIdent(int num_ident);
void parse();
void parse_V();
void printOPWarning(int code);
void printIDError(char* name);
void printOK();
void printToken(char*);
void lexical();
int parse_term();
int parse_factor();
int parse_expression();
void parse_statement();
void printOPError();
int find_ident(char*);
int find_or_add_ident(char*);

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [-v] <filepath>\n", argv[0]);
        return 1;
    }

    int verbose = 0;
    char* filepath = NULL;

    if (strcmp("-v", argv[1]) == 0) {
        verbose = 1;
        if (argc < 3) {
            fprintf(stderr, "Error: No file specified.\n");
            return 1;
        }
        filepath = argv[2];
    }
    else {
        filepath = argv[1];
    }

    file = fopen(filepath, "r");

    if (file == NULL) {
        fprintf(stderr, "Error: Could not open file %s\n", filepath);
        return 1;
    }

    // Gets input line for the first time
    fgets(line, sizeof(line), file);

    // Depending on the verbose flag, call the appropriate function
    if (verbose) {
        parse_V();
    }
    else {
        parse();
    }

    fclose(file);
    return 0;
}
// �־��� �ڵ�

char* line_ptr = line;
int error; //���������� üũ
int isdiviedby0 = 0; //0���� ���������� üũ
int iderror = 0; // ident ���������� üũ
int isunknown = 0;
char errorID[100];
int duplicateop = 0;
int assignop = 0;

void lexical() {
    // Skip white spaces
    while (*line_ptr && isspace(*line_ptr)) {
        line_ptr++;
    }

    if (isspace(*line_ptr)) line_ptr++;

    if (*line_ptr == '\0') {
        next_token = TOKEN_END;
        return;
    }

    if (isalpha(*line_ptr)) { // IDENT (identifier)
        char* start = line_ptr;
        while (isalnum(*line_ptr)) {
            line_ptr++;
        }
        int len = line_ptr - start;
        strncpy(token_string, start, len);

        token_string[len] = '\0';
        next_token = TOKEN_IDENT;
        return;
    }
    else if (isdigit(*line_ptr)) { // CONST (constant)
        char* start = line_ptr;
        while (isdigit(*line_ptr)) {
            line_ptr++;
        }
        int len = line_ptr - start;
        strncpy(token_string, start, len);
        token_string[len] = '\0';
        next_token = TOKEN_CONST;
        return;
    }
    else { // Operators and delimiters
        switch (*line_ptr) {
        case ':':
            if (*(line_ptr + 1) == '=') {
                next_token = TOKEN_ASSIGN_OP;
                strcpy(token_string, ":=");
                line_ptr += 2;

            }
            else if (*(line_ptr + 2) == '=') {
                next_token = TOKEN_ASSIGN_OP;
                strcpy(token_string, ":=");
                line_ptr += 3;
            }
            break;
        case '+':
            next_token = TOKEN_ADD_OP;
            strcpy(token_string, "+");
            line_ptr++;
            break;
        case '-':
            next_token = TOKEN_MIN_OP;
            strcpy(token_string, "-");
            line_ptr++;
            break;
        case '*':
            next_token = TOKEN_MULT_OP;
            strcpy(token_string, "*");
            line_ptr++;
            break;
        case '/':
            next_token = TOKEN_DIVI_OP;
            strcpy(token_string, "/");
            line_ptr++;
            break;
        case '(':
            next_token = TOKEN_LEFT_PAREN;
            strcpy(token_string, "(");
            line_ptr++;
            break;
        case ')':
            next_token = TOKEN_RIGHT_PAREN;
            strcpy(token_string, ")");
            line_ptr++;
            break;
        case ';':
            next_token = TOKEN_SEMI_COLON;
            strcpy(token_string, ";");
            line_ptr++;
            break;
        default:
            line_ptr++;
        }
    }
    return;
}

void modify_line(char* statement) {
    char* src = statement; // ���� ������
    char* dest = statement; // ������ ���ڿ��� ������ ��ġ

    while (*src != '\0') {
        if (*src == '=' && *(src - 1) != ':') {
            // ���� ���ڰ� '='�ε�, ���� ���ڰ� ':'�� �ƴ� ��� ':='�� ��ȯ
            duplicateop = 5;
            *dest++ = ':';
            *dest++ = '=';
            src+=2;
        }
        else if (*src == '+' && (*(src + 1) == '+' || isspace(*(src + 1)) && *(src + 2) == '+')) {
            // ���� ���ڰ� '+'�̰� ������ '+' �Ǵ� ���� �� '+'�� �����ϴ� ���
            duplicateop = 1;
            *dest++ = '+';
            src += (*(src + 1) == '+') ? 2 : 3; // �ߺ��� '+'�� ���� �ǳʶٱ�
        }
        else if (*src == '-' && (*(src + 1) == '-' || isspace(*(src + 1)) && *(src + 2) == '-')) {
            // ���� ���ڰ� '-'�̰� ������ '-' �Ǵ� ���� �� '-'�� �����ϴ� ���
            duplicateop = 2;
            *dest++ = '-';
            src += (*(src + 1) == '-') ? 2 : 3; // �ߺ��� '-'�� ���� �ǳʶٱ�
        }
        else if (*src == '*' && (*(src + 1) == '*' || isspace(*(src + 1)) && *(src + 2) == '*')) {
            duplicateop = 3;
            *dest++ = '*';
            src += (*(src + 1) == '*') ? 2 : 3; 
        }
        else if (*src == '/' && (*(src + 1) == '/' || isspace(*(src + 1)) && *(src + 2) == '/')) {
            duplicateop = 4;
            *dest++ = '/';
            src += (*(src + 1) == '/') ? 2 : 3;
        }
        else {
            // ������ ���� ����
            *dest++ = *src++;
        }
    }
    *dest = '\0'; // ������ ���ڿ� ����
}


// identifier�� ������ �ش� index ������ -1��ȯ
int find_ident(char* name) {
    for (int i = 0; i < num_idents; i++) {
        if (strcmp(idArray[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int find_or_add_ident(char* name) {
    int index = find_ident(name);
    if (index != -1) {
        return index;
    }

    strcpy(idArray[num_idents].name, name);
    strcpy(idArray[num_idents].value, "Unknown");
    return num_idents++;
}

void parse() {
    int issemicolon = 0;
    do {
        // ������ ���� �����ݷ� �߰� (���� ���)
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';  // ���� ���� ����
            len--;  // ���� ����
        }

        for (int i = 0; i < len; i++) {
            if (line[i] == ';')  issemicolon++;
        }

        if (len > 0 && line[len-1] != ';') {
            line[len] = ';';
            line[len + 1] = '\0';
        }

        line_ptr = line;
        // ident_count, const_count, op_count, error �ʱ�ȭ
        ident_count = const_count = op_count = error = 0;

        // �����ݷ��� �������� ������
        char* statement = strtok(line, ";");
        while (statement != NULL) {
            // ���� ���� �� ó��
            while (isspace(*statement)) statement++;  // �� ���� ����
            if (*statement == '\0') { // �� ������ �ǳʶ�
                statement = strtok(NULL, ";");
                continue;
            }

            // ���� statement ó��
            modify_line(statement);  // �ʿ� �� ����
            line_ptr = statement;    // ���� statement

            // ���� �м�
            while (1) {
                lexical();  // ���� �м�
                if (next_token == TOKEN_END || next_token == TOKEN_SEMI_COLON) break;
                parse_statement();  // ���� �м�
            }

            // ��� ���
            size_t statement_len = strlen(statement);
            if (issemicolon) {  // �����ݷ��� ������ �߰�
                statement[statement_len] = ';';
                statement[statement_len + 1] = '\0';  // �� ���� �߰�
                issemicolon--;
            }

            printResultByLine(statement, ident_count, const_count, op_count);

            // ���� ó��
            if (error == 0 && iderror == 0) {
                printOK();  // ����
            }
            else if (iderror) {
                printIDError(errorID);  // �ĺ��� ����
                iderror = 0;  // �ʱ�ȭ
            }
            if (duplicateop) {
                printOPWarning(duplicateop);  // �ߺ� ������ ���
                duplicateop = 0;  // �ʱ�ȭ
            }

            // ���� statement�� �Ѿ
            statement = strtok(NULL, ";");
        }

    } while (fgets(line, sizeof(line), file) != NULL);  // ���� ������ �ݺ�

    // �ĺ��� ���
    printIdent(num_idents);  // �ɺ� ���̺� ���
}


void parse_statement() {
    int ident_index = find_or_add_ident(token_string); // �ɺ����̺� ������Ʈ

    lexical(); // ���� ��ū: :=

    if (next_token == TOKEN_ASSIGN_OP) {
        lexical(); // ���� ��ū
        ident_count++;
        int value = parse_expression(); // �� ���
        if (isdiviedby0 || error || iderror || isunknown) {
            strcpy(idArray[ident_index].value, "Unknown");
        }
        else sprintf(idArray[ident_index].value, "%d", value); // ��� ��� ����
    }
    else if (next_token == '=') { // �߸��� �Ҵ� ������
        duplicateop = 5; // :=�� ��ü
        lexical();         // ���� �� ��� ����
        ident_count++;
        int value = parse_expression();
        if (isdiviedby0 || error || iderror || isunknown) {
            strcpy(idArray[ident_index].value, "Unknown");
        }
        else {
            sprintf(idArray[ident_index].value, "%d", value); // ��� ��� ����
        }
    }
}

int parse_expression() {
    int value = parse_term();

    while (next_token == TOKEN_ADD_OP || next_token == TOKEN_MIN_OP) {
        op_count++; // �߰��� �κ�
        int operator_token = next_token;
        lexical();
        int term_value = parse_term();

        if (operator_token == TOKEN_ADD_OP) {
            value += term_value;
        }
        else if (operator_token == TOKEN_MIN_OP) {
            value -= term_value;
        }
    }
    return value;
}

int parse_term() {
    int value = parse_factor();
    lexical();
    while (next_token == TOKEN_MULT_OP || next_token == TOKEN_DIVI_OP) {
        int operator_token = next_token; // ���� ������ ����
        op_count++;
        lexical(); // ���� ��ū���� �̵�
        int factor_value = parse_factor(); // ���� �� ���

        // ���� ����
        if (operator_token == TOKEN_MULT_OP) {
            value *= factor_value;
        }
        else if (operator_token == TOKEN_DIVI_OP) {
            value /= factor_value;
        }
    }
    return value; // ��� ���� �Ϸ� �� ��ȯ
}

int parse_factor() {
    if (next_token == TOKEN_LEFT_PAREN) {
        lexical(); // ��ȣ ���η� �̵�
        int value = parse_expression(); // ��ȣ ���� �� ���
        if (next_token == TOKEN_RIGHT_PAREN) {
            //lexical(); // �ݴ� ��ȣ�� ó��
            return value;
        }
        lexical();
        return value;
    }
    else if (next_token == TOKEN_IDENT) {
        ident_count++;
        int ident_index = find_ident(token_string);
        if (ident_index == -1) {
            error++;
            iderror = 1;
            isunknown = 1;
            find_or_add_ident(token_string);
            strcpy(errorID,token_string);
            return 0;
        }
        if (strcmp(idArray[ident_index].value, "Unknown") == 0) { // ���� Unknown�� ���
            error++;
            isunknown = 1;
            return 0;
        }
        return atoi(idArray[ident_index].value);
    }
    else if (next_token == TOKEN_CONST) {
        const_count++;
        return atoi(token_string); // ��� ��ȯ
    }
    return 0;
}

void parse_V() {
    do {
        line_ptr = line;
        while (1) {
            lexical();
            if (next_token == TOKEN_END) break;
            printToken(token_string);
        }

    } while (fgets(line, sizeof(line), file) != NULL);
}

// �־��� �ڵ�
void printResultByLine(char* line, int ID, int CON, int OP) {
    printf("%s\n", line);
    printf("ID: %d; CONST: %d; OP: %d;\n", ID, CON, OP);
}

void printOPWarning(int code) {
    switch (code) {

    case 1:
        printf("(Warning) \"Eliminating duplicate operator (+)\"\n");
        break;
    case 2:
        printf("(Warning) \"Eliminating duplicate operator (-)\"\n");
        break;
    case 3:
        printf("(Warning) \"Eliminating duplicate operator (*)\"\n");
        break;
    case 4:
        printf("(Warning) \"Eliminating duplicate operator (/)\"\n");
        break;
    case 5:
        printf("(Warning) \"Substituting assignment operator (:=)\"\n");
        break;
    }
}

void printOK() {
    printf("(OK)\n");
}

void printOPError() {
    printf("(Error)cannot do operation\"\"\n");
}

void printIDError(char* name) {
    printf("(Error) \"referring to undefined identifiers(%s)\"\n", name);
}

void printIdent(int num_ident) {
    int i;
    printf("Result ==>");
    for (i = 0; i < num_ident; i++) {
        printf(" %s: %s;", idArray[i].name, idArray[i].value);
    }
}

void printToken(char* token) {
    printf("%s\n", token);
}