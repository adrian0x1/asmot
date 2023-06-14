#include "response.h"

#define NONE 0

void responseInit(Response* res)
{
    res->headers = NULL;
    res->body = NULL;
    res->statusCode = NONE;
}

void responseSetHeader(Response* res, const char* name, const char* value)
{
    Header *header, *ptr;
    header = malloc(sizeof *header);
    header->name = malloc(strlen(name) + 1);
    header->value = malloc(strlen(value) + 1);

    strcpy(header->name, name);
    strcpy(header->value, value);
    header->next = NULL;

    if (res->headers == NULL)
        res->headers = header;
    else {
        for (ptr = res->headers; ptr->next != NULL; ptr = ptr->next)
            ;
        ptr->next = header;
    }
}

void responseSendBytes(Response* res, const char* bytes, size_t size)
{
    char sizeStr[64];
    sprintf(sizeStr, "%d", size);
    responseSetHeader(res, "content-length", sizeStr);
    res->body = malloc(size);
    memcpy(res->body, bytes, size);
    if (res->statusCode == NONE)
        res->statusCode = HTTP_OK;
    res->contentLength = size;
}

void responseSendStatus(Response* res, int status)
{
    res->statusCode = status;
}

void responseSendFile(Response* res, const char* filepath)
{
    size_t size;
    FILE* file;

    /* Set MIME Type for file */
    const int EXT = 0, TYPE = 1;
    char* MIME[][2] = {
        { "html", "text/html" },
        { "js", "text/javascript" },
        { "css", "text/css" },
        { "json", "application/json" },
        { "ico", "image/x-icon" },
        { "avif", "image/avif" }
    };

    char fileExt[8] = { 0 };
    for (int i = 0; i < strlen(filepath); i++) {
        if (filepath[i] != '.')
            continue;
        strcpy(fileExt, &filepath[++i]);
        break;
    }

    for (int i = 0; i < ARRAY_SIZE(MIME); i++) {
        if (strcmp(fileExt, MIME[i][EXT]) == 0)
            responseSetHeader(res, "content-type", MIME[i][TYPE]);
    }

    file = fopen(filepath, "rb");

    if (file == NULL) {
        fprintf(stderr, "File couldn't be read\n");
        return responseSendStatus(res, HTTP_NOT_FOUND);
    }
    /**************************/

    /* Read file into memory */
    fseek(file, 0L, SEEK_END);
    size = ftell(file);
    rewind(file);

    res->body = malloc(size + 1);

    fread(res->body, size, 1, file);
    fclose(file);

    res->body[size] = '\0';

    char sizestr[64];
    sprintf(sizestr, "%ld", size);
	printf("Size: %ld\n", size);
    responseSetHeader(res, "content-length", sizestr);
    if (res->statusCode == NONE)
        res->statusCode = HTTP_OK;

    res->contentLength = size;
}

char* responseToBuffer(Response* res, char* buffer, size_t bufsize)
{
    // Universal headers for now
    // TODO: Caching and compression if supported by client
    responseSetHeader(res, "server", "asmot_v0.1.1");
    responseSetHeader(res, "connection", "close");
	// Acces Policy
	responseSetHeader(res, "Access-Control-Allow-Origin", "*");

    const int STATUS_CODE = 0, STATUS_PHRASE = 1;
    char* statusPhrase;

    void* phrases[][2] = {
        { &(int) { HTTP_OK }, "OK" },
        { &(int) { HTTP_MOVED_PERMANENTLY }, "Moved Permanently" },
        { &(int) { HTTP_BAD_REQUEST }, "Bad Request" },
        { &(int) { HTTP_UNAUTHORIZED }, "Unauthorized" },
        { &(int) { HTTP_FORBIDDEN }, "Forbidden" },
        { &(int) { HTTP_NOT_FOUND }, "Not Found" },
        { &(int) { HTTP_INTERNAL_SERVER_ERROR }, "Internal Server Error" },
        { &(int) { HTTP_NOT_IMPLEMENTED }, "Not Implemented" },
        { &(int) { HTTP_BAD_GATEWAY }, "Bad Gateway" },
        { &(int) { HTTP_SERVICE_UNAVAILABLE }, "Service Unavailable" },
        { &(int) { HTTP_GATEWAY_TIMEOUT }, "Gateway Timeout" }
    };

    if (res->statusCode == NONE)
        res->statusCode = HTTP_INTERNAL_SERVER_ERROR;

    memset(buffer, 0, bufsize);

    /* Status Line */
    int i = 0;
    for (; *(int*)phrases[i][STATUS_CODE] != res->statusCode; i++)
        ;
    statusPhrase = phrases[i][STATUS_PHRASE];
    sprintf(buffer, "%s %d %s\r\n",
        "HTTP/1.1", res->statusCode, statusPhrase);

    /* Headers */
    for (Header* h = res->headers; h != NULL; h = h->next)
        sprintf(buffer + strlen(buffer), "%s: %s\r\n", h->name, h->value);

    sprintf(buffer + strlen(buffer), "\r\n");

    /* Set the response length (status line + headers)*/
    res->length = strlen(buffer);

    /* Body */
    // FIXME: Make sure the buffer will hold the total response length
    //  before copying it
    if (res->body != NULL)
        memcpy(buffer + res->length, res->body, res->contentLength);
    else
        res->contentLength = 0;

    printf("\x1b[32mResponse Length: %ld bytes\n", res->length);
    printf("Content Length: %ld bytes\n", res->contentLength);
    printf("Total: %ld bytes\x1b[0m\n", res->length + res->contentLength);

    return buffer;
}

void responseDestroy(Response* res)
{
    if (res->body != NULL)
        free(res->body);

    if (res->headers != NULL) {
        Header *header, *temp;
        header = res->headers;
        res->headers = NULL;

        while (header != NULL) {
            temp = header;
            header = header->next;
            free(temp->name);
            free(temp->value);
            free(temp);
        }
    }
}
