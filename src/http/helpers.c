#include "helpers.h"
#include "common.h"

char* readField(char* buffer, char* data, size_t bufsize)
{
    const char* crlf = "\r\n";
    char* eoh;
    unsigned long end;

    if ((eoh = strstr(data, crlf)) != NULL) {
        if ((end = (unsigned long)eoh - (unsigned long)data) == 0
            || end >= bufsize)
            return NULL;

        memcpy(buffer, data, end);
        buffer[end] = '\0';
        return eoh + strlen(crlf);
    }

    return NULL;
}

bool isHttpMethod(const char* field)
{
    const char* methods[] = {
        "GET", "HEAD", "POST", "PUT", "DELETE",
        "CONNECT", "OPTIONS", "TRACE", "PATCH"
    };

    for (int i = 0; i < ARRAY_SIZE(methods); i++) {
        if (strcmp(field, methods[i]) == 0)
            return true;
    }

    return false;
}

// TODO: Implement these
static bool isAuthority(const char* field)
{
    return true;
}

static bool isAbsoluteUri(const char* field)
{
    return true;
}

static bool isAbsolutePath(const char* field)
{
    return true;
}

bool isRequestUri(const char* field)
{
    if (strcmp(field, "*") == 0
        || isAbsoluteUri(field)
        || isAbsolutePath(field)
        || isAuthority(field))
        return true;

    return false;
}

bool isHttpVersion(const char* field)
{
    const int VERSION_HEADER_LENGTH = 8;
    const char* http = "HTTP/";
    size_t httpOffset = strlen(http);

    if (strlen(field) != VERSION_HEADER_LENGTH)
        return false;

    if (strstr(field, http) != NULL) {
        if (isdigit(field[httpOffset])
            && (field[httpOffset + 1] == '.')
            && isdigit(field[httpOffset + 2]))
            return true;
    }

    return false;
}

bool isRequestLine(char* field)
{
    char fieldCopy[HEADER_FIELD_MAX];
    strcpy(fieldCopy, field);
    int count = 0;
    const char* delim = " ";
    char* token = strtok(fieldCopy, delim);

    if (token != NULL) {
        do {
            if ((count == 0 && !isHttpMethod(token))
                || (count == 1 && !isRequestUri(token))
                || (count == 2 && !isHttpVersion(token))
                || (count > 2))
                return false;
            count++;
        } while ((token = strtok(NULL, delim)) != NULL);
    }

    return true;
}

bool isValidRequest(char* data)
{
    char buffer[HEADER_FIELD_MAX];

    if ((readField(buffer, data, HEADER_FIELD_MAX) != NULL)
        && isRequestLine(buffer))
        return true;

    return false;
}

bool isRequestHeader(char* field)
{
    char fieldCopy[HEADER_FIELD_MAX];
    strcpy(fieldCopy, field);
    char separators[] = "()<>@,;:\\\"/[]?={} \t";

    char* token = strtok(fieldCopy, " ");
    size_t end = strlen(token) - 1;

    for (int i = 0; i < strlen(token); i++) {
        if (iscntrl(token[i]) || token[end] != ':')
            return false;

        for (int j = 0; j < sizeof separators && i < end; j++) {
            if (token[i] == separators[j])
                return false;
        }
    }

    return true;
}

bool httpHasBody(char* data)
{
    char buffer[HEADER_FIELD_MAX];
    char* offset = data;

    while ((offset = readField(buffer, offset, HEADER_FIELD_MAX)) != NULL) {
        if (readField(buffer, offset, HEADER_FIELD_MAX) == NULL)
            break;
    }

    if (strstr(offset, "\r\n") != NULL)
        offset += 2;

    return strlen(offset) != 0 ? true : false;
}
