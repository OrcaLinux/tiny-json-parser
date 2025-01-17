#include "json_tokenizer.h"
#include "json_logging.h"
#include "json_utils.h"
#include <string.h>
#include <stdio.h>

const char *json_token_type_to_string(JsonTokenType token_type)
{
    switch (token_type)
    {
    case TOKEN_NONE:
        return "TOKEN_NONE";
    case TOKEN_LEFT_BRACE:
        return "TOKEN_LEFT_BRACE";
    case TOKEN_RIGHT_BRACE:
        return "TOKEN_RIGHT_BRACE";
    case TOKEN_LEFT_BRACKET:
        return "TOKEN_LEFT_BRACKET";
    case TOKEN_RIGHT_BRACKET:
        return "TOKEN_RIGHT_BRACKET";
    case TOKEN_COLON:
        return "TOKEN_COLON";
    case TOKEN_COMMA:
        return "TOKEN_COMMA";
    case TOKEN_STRING:
        return "TOKEN_STRING";
    case TOKEN_NUMBER:
        return "TOKEN_NUMBER";
    case TOKEN_TRUE:
        return "TOKEN_TRUE";
    case TOKEN_FALSE:
        return "TOKEN_FALSE";
    case TOKEN_NULL:
        return "TOKEN_NULL";
    case TOKEN_EOF:
        return "TOKEN_EOF";
    case TOKEN_ERROR:
        return "TOKEN_ERROR";
    default:
        return "UNKNOWN_TOKEN";
    }
}

void json_tokenizer_init(JsonTokenizer *tokenizer, const char *json)
{
    tokenizer->json = json;
    tokenizer->pos = 0;
}

void json_tokenizer_reset(JsonTokenizer *tokenizer, const char *json)
{
    tokenizer->json = json;
    tokenizer->pos = 0;
}

static void skip_whitespace(JsonTokenizer *tokenizer)
{
    while (json_is_whitespace(tokenizer->json[tokenizer->pos]))
    {
        DEBUG_PRINT("Tokenizer: Skipping whitespace at position %zu\n", tokenizer->pos);
        tokenizer->pos++;
    }
}

JsonToken json_get_next_token(JsonTokenizer *tokenizer)
{
    skip_whitespace(tokenizer);
    JsonToken token;
    token.value = NULL;

    char current = tokenizer->json[tokenizer->pos];
    DEBUG_PRINT("Tokenizer: Current char '%c' at position %zu\n", current, tokenizer->pos);

    switch (current)
    {
    case '\0':
        token.type = TOKEN_EOF;
        DEBUG_PRINT("Tokenizer: TOKEN_EOF\n");
        break;
    case '{':
        token.type = TOKEN_LEFT_BRACE;
        tokenizer->pos++;
        DEBUG_PRINT("Tokenizer: TOKEN_LEFT_BRACE\n");
        break;
    case '}':
        token.type = TOKEN_RIGHT_BRACE;
        tokenizer->pos++;
        DEBUG_PRINT("Tokenizer: TOKEN_RIGHT_BRACE\n");
        break;
    case '[':
        token.type = TOKEN_LEFT_BRACKET;
        tokenizer->pos++;
        DEBUG_PRINT("Tokenizer: TOKEN_LEFT_BRACKET\n");
        break;
    case ']':
        token.type = TOKEN_RIGHT_BRACKET;
        tokenizer->pos++;
        DEBUG_PRINT("Tokenizer: TOKEN_RIGHT_BRACKET\n");
        break;
    case ':':
        token.type = TOKEN_COLON;
        tokenizer->pos++;
        DEBUG_PRINT("Tokenizer: TOKEN_COLON\n");
        break;
    case ',':
        token.type = TOKEN_COMMA;
        tokenizer->pos++;
        DEBUG_PRINT("Tokenizer: TOKEN_COMMA\n");
        break;
    case '"':
    {
        // Parse string
        tokenizer->pos++; // Skip opening quote
        size_t start = tokenizer->pos;
        while (tokenizer->json[tokenizer->pos] != '"' && tokenizer->json[tokenizer->pos] != '\0')
        {
            if (tokenizer->json[tokenizer->pos] == '\\')
            {
                DEBUG_PRINT("Tokenizer: Escaped character '\\' at position %zu\n", tokenizer->pos);
                tokenizer->pos += 2; // Skip escaped character
            }
            else
            {
                tokenizer->pos++;
            }
        }
        size_t length = tokenizer->pos - start;
        if (tokenizer->json[tokenizer->pos] == '"')
        {
            token.type = TOKEN_STRING;
            token.value = json_strdup_range(tokenizer->json + start, length);
            tokenizer->pos++; // Skip closing quote
            DEBUG_PRINT("Tokenizer: TOKEN_STRING with value '%s'\n", token.value);
        }
        else
        {
            token.type = TOKEN_ERROR;
            DEBUG_PRINT("Tokenizer: TOKEN_ERROR while parsing string at position %zu\n", tokenizer->pos);
        }
        break;
    }
    case 't':
        if (strncmp(&tokenizer->json[tokenizer->pos], "true", 4) == 0)
        {
            token.type = TOKEN_TRUE;
            tokenizer->pos += 4;
            DEBUG_PRINT("Tokenizer: TOKEN_TRUE\n");
        }
        else
        {
            token.type = TOKEN_ERROR;
            DEBUG_PRINT("Tokenizer: TOKEN_ERROR while parsing 't' at position %zu\n", tokenizer->pos);
        }
        break;
    case 'f':
        if (strncmp(&tokenizer->json[tokenizer->pos], "false", 5) == 0)
        {
            token.type = TOKEN_FALSE;
            tokenizer->pos += 5;
            DEBUG_PRINT("Tokenizer: TOKEN_FALSE\n");
        }
        else
        {
            token.type = TOKEN_ERROR;
            DEBUG_PRINT("Tokenizer: TOKEN_ERROR while parsing 'f' at position %zu\n", tokenizer->pos);
        }
        break;
    case 'n':
        if (strncmp(&tokenizer->json[tokenizer->pos], "null", 4) == 0)
        {
            token.type = TOKEN_NULL;
            tokenizer->pos += 4;
            DEBUG_PRINT("Tokenizer: TOKEN_NULL\n");
        }
        else
        {
            token.type = TOKEN_ERROR;
            DEBUG_PRINT("Tokenizer: TOKEN_ERROR while parsing 'n' at position %zu\n", tokenizer->pos);
        }
        break;
    default:
        if ((current >= '0' && current <= '9') || current == '-')
        {
            // Parse number
            size_t start = tokenizer->pos;
            if (current == '-')
                tokenizer->pos++;
            while (tokenizer->json[tokenizer->pos] >= '0' && tokenizer->json[tokenizer->pos] <= '9')
            {
                tokenizer->pos++;
            }
            if (tokenizer->json[tokenizer->pos] == '.')
            {
                tokenizer->pos++;
                while (tokenizer->json[tokenizer->pos] >= '0' && tokenizer->json[tokenizer->pos] <= '9')
                {
                    tokenizer->pos++;
                }
            }
            size_t length = tokenizer->pos - start;
            token.type = TOKEN_NUMBER;
            token.value = json_strdup_range(tokenizer->json + start, length);
            DEBUG_PRINT("Tokenizer: TOKEN_NUMBER with value '%s'\n", token.value);
        }
        else
        {
            token.type = TOKEN_ERROR;
            DEBUG_PRINT("Tokenizer: TOKEN_ERROR with unrecognized character '%c' at position %zu\n", current, tokenizer->pos);
        }
        break;
    }

    return token;
}

void json_token_free(JsonToken *token)
{
    if (token->value)
    {
        json_free(token->value);
        token->value = NULL;
    }
}
