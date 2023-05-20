#include "request.h"
#include "helpers.h"

static int requestParseLine(Request* req, char* line)
{
    if (!isRequestLine(line))
        return (-1);

    char* token = NULL;
    size_t length;
    unsigned i = 0;
    char lineCopy[HEADER_FIELD_MAX];
    strcpy(lineCopy, line);

    do {
        if (token == NULL)
            token = strtok(lineCopy, " ");
        length = strlen(token) + 1;
        switch (i++) {
        case 0:
            req->method = malloc(length);
            strcpy(req->method, token);
            break;
        case 1:
            req->uri = malloc(length);
            strcpy(req->uri, token);
            break;
        case 2:
            req->httpVersion = malloc(length);
            strcpy(req->httpVersion, token);
            break;
        }
    } while ((token = strtok(NULL, " ")) != NULL);

    return 0;
}

static int requestSetBody(Request* req, char* data)
{
    if (data == NULL || req == NULL)
        return (-1);

    char buffer[HEADER_FIELD_MAX];
    char* offset = data;
    size_t bodyLength;

    while ((offset = readField(buffer, offset, HEADER_FIELD_MAX)) != NULL) {
        if (readField(buffer, offset, HEADER_FIELD_MAX) == NULL)
            break;
    }

    if (strstr(offset, "\r\n") != NULL)
        offset += 2;

    bodyLength = strlen(offset) + 1;
    req->body = malloc(bodyLength);
    strcpy(req->body, offset);
    return 0;
}

static int requestAddHeader(Request* req, char* line)
{
    if (!isRequestHeader(line))
        return (-1);

    char *name, *value, *delim = " ";
    size_t nameLength, valueLength;
    char lineCopy[HEADER_FIELD_MAX];

    strcpy(lineCopy, line);

    name = strtok(lineCopy, delim);
    nameLength = strlen(name);
    name[nameLength - 1] = '\0'; // removes the ':' character
    nameLength--;

    value = strtok(NULL, delim);
    value = line + (value - lineCopy);
    valueLength = strlen(value);

    Header *header, *ptr;

    header = malloc(sizeof *header);
    header->name = malloc(nameLength + 1);
    header->value = malloc(valueLength + 1);
    header->next = NULL;

    strcpy(header->name, name);
    strcpy(header->value, value);

    if (req->headers == NULL) {
        req->headers = header;
    } else {
        for (ptr = req->headers; ptr->next != NULL; ptr = ptr->next)
            ;
        ptr->next = header;
    }

    return 0;
}

/* Public functions */

void requestInit(Request* req)
{
    req->method = NULL;
    req->uri = NULL;
    req->httpVersion = NULL;
    req->headers = NULL;
    req->body = NULL;
}

Request* requestParse(Request* req, char* data)
{
    char field[HEADER_FIELD_MAX];
    char* offset = data;

    if (!isValidRequest(data)) {
        fprintf(stderr, "Error: Invalid request\n");
        return NULL;
    }

    while ((offset = readField(field, offset, HEADER_FIELD_MAX)) != NULL) {
        if (isRequestLine(field))
            requestParseLine(req, field);
        else if (isRequestHeader(field))
            requestAddHeader(req, field);
        else
            fprintf(stderr, "Error: Couldn't parse header: %s\n", field);
    }

    if (httpHasBody(data))
        requestSetBody(req, data);

    printf("\x1b[33mRequest URI: %s\x1b[0m\n", req->uri);
    return req;
}

void requestDestroy(Request* req)
{
    if (req->method != NULL)
        free(req->method);
    if (req->uri != NULL)
        free(req->uri);
    if (req->httpVersion != NULL)
        free(req->httpVersion);
    if (req->body != NULL)
        free(req->body);

    Header *header, *temp;
    header = req->headers;
    req->headers = NULL;

    while (header != NULL) {
        temp = header;
        header = header->next;
        free(temp->name);
        free(temp->value);
        free(temp);
    }
}
