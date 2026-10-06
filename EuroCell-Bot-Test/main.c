#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winhttp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#pragma comment(lib, "winhttp.lib")
#define LIMIT (4u * 1024u * 1024u)
#define BOUNDARY "EuroCellLocalTest216"

/* Fixed local connection only. No redirects, browser cookies, authentication,
 * proxies or retries. Reading response chunks does not submit extra requests. */
static int request(HINTERNET connection, const wchar_t *method,
    char *payload, DWORD length, char **body, DWORD *status)
{
    HINTERNET handle = NULL;
    DWORD disabled = WINHTTP_DISABLE_REDIRECTS | WINHTTP_DISABLE_COOKIES |
        WINHTTP_DISABLE_AUTHENTICATION;
    DWORD size = sizeof(*status), received;
    size_t used = 0;
    ULONGLONG started = GetTickCount64();
    int ok = 0;
    *body = NULL;
    *status = 0;
    handle = WinHttpOpenRequest(connection, method, L"/", NULL,
        WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
    if (!handle) goto done;
    if (!WinHttpSetOption(handle, WINHTTP_OPTION_DISABLE_FEATURE,
        &disabled, sizeof(disabled))) goto done;
    if (!WinHttpSendRequest(handle, payload ?
        L"Content-Type: multipart/form-data; boundary=EuroCellLocalTest216\r\n" :
        L"Accept: text/html\r\n", (DWORD)-1L, payload, length, length, 0)) goto done;
    if (!WinHttpReceiveResponse(handle, NULL)) goto done;
    if (!WinHttpQueryHeaders(handle, WINHTTP_QUERY_STATUS_CODE |
        WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX,
        status, &size, WINHTTP_NO_HEADER_INDEX)) goto done;
    *body = malloc(LIMIT + 1u);
    if (!*body) { SetLastError(ERROR_NOT_ENOUGH_MEMORY); goto done; }
    for (;;) {
        char chunk[8192];
        if (GetTickCount64() - started > 30000) {
            SetLastError(ERROR_TIMEOUT); goto done;
        }
        if (!WinHttpReadData(handle, chunk, sizeof(chunk), &received)) goto done;
        if (!received) break;
        if (used + received > LIMIT) {
            SetLastError(ERROR_INSUFFICIENT_BUFFER); goto done;
        }
        memcpy(*body + used, chunk, received);
        used += received;
    }
    (*body)[used] = '\0';
    ok = 1;
done:
    if (!ok) {
        printf("Windows request error: %lu\n", (unsigned long)GetLastError());
        free(*body);
        *body = NULL;
    }
    if (handle && !WinHttpCloseHandle(handle))
        printf("Close request error: %lu\n", (unsigned long)GetLastError());
    return ok;
}

/* Attribute parsing supports either quote style and arbitrary attribute order.
 * Unknown entities fail closed instead of changing the submitted hidden value. */
static int attribute(const char *p, const char *end, const char *key,
    char *output, size_t capacity)
{
    while (p < end) {
        const char *name, *name_end, *value;
        char quote;
        size_t used = 0;
        while (p < end && (isspace((unsigned char)*p) || *p == '/')) ++p;
        name = p;
        while (p < end && !isspace((unsigned char)*p) && *p != '=') ++p;
        if (p == name) break;
        name_end = p;
        while (p < end && isspace((unsigned char)*p)) ++p;
        if (p == end || *p != '=') continue;
        ++p;
        while (p < end && isspace((unsigned char)*p)) ++p;
        quote = p < end && (*p == '\'' || *p == '"') ? *p++ : 0;
        value = p;
        while (p < end && (quote ? *p != quote : !isspace((unsigned char)*p))) ++p;
        if ((size_t)(name_end - name) == strlen(key) &&
            _strnicmp(name, key, strlen(key)) == 0) {
            while (value < p) {
                char ch = *value++;
                if (ch == '&') {
                    const char *entity = value - 1;
                    if (p - entity >= 5 && strncmp(entity, "&amp;", 5) == 0) { ch = '&'; value = entity + 5; }
                    else if (p - entity >= 6 && strncmp(entity, "&quot;", 6) == 0) { ch = '"'; value = entity + 6; }
                    else if (p - entity >= 5 && strncmp(entity, "&#39;", 5) == 0) { ch = '\''; value = entity + 5; }
                    else if (p - entity >= 4 && strncmp(entity, "&lt;", 4) == 0) { ch = '<'; value = entity + 4; }
                    else if (p - entity >= 4 && strncmp(entity, "&gt;", 4) == 0) { ch = '>'; value = entity + 4; }
                    else return 0;
                }
                if (used + 1 >= capacity) return 0;
                output[used++] = ch;
            }
            output[used] = 0;
            return 1;
        }
        if (quote && p < end) ++p;
    }
    return 0;
}

static int input_value(const char *p, const char *limit, const char *name,
    char *value, size_t capacity)
{
    while (p < limit && (p = strchr(p, '<')) != NULL && p < limit) {
        const char *end = strchr(p, '>');
        char field[256];
        if (!end || end >= limit) break;
        if (_strnicmp(p, "<input", 6) == 0 && isspace((unsigned char)p[6]) &&
            attribute(p + 6, end, "name", field, sizeof(field)) &&
            strcmp(field, name) == 0)
            return attribute(p + 6, end, "value", value, capacity);
        p = end + 1;
    }
    return 0;
}

static int add_field(char *body, size_t capacity, size_t *used,
    const char *name, const char *value)
{
    int count = snprintf(body + *used, capacity - *used,
        "--" BOUNDARY "\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
        name, value);
    if (count < 0 || (size_t)count >= capacity - *used) return 0;
    *used += (size_t)count;
    return 1;
}

/* FILETIME counts 100-nanosecond intervals since 1601. Convert to Unix
 * milliseconds to reproduce JavaScript Date.now(), not a CAPTCHA token. */
static int submission_start_time(char *output, size_t capacity)
{
    FILETIME current_time;
    ULARGE_INTEGER ticks;
    ULONGLONG milliseconds;
    int count;
    GetSystemTimeAsFileTime(&current_time);
    ticks.LowPart = current_time.dwLowDateTime;
    ticks.HighPart = current_time.dwHighDateTime;
    if (ticks.QuadPart < 116444736000000000ULL) return 0;
    milliseconds = (ticks.QuadPart - 116444736000000000ULL) / 10000ULL;
    count = snprintf(output, capacity, "%llu", milliseconds);
    return count >= 0 && (size_t)count < capacity;
}

/* Exclude HTML attributes and scripts/styles to avoid printing hidden tokens.
 * Output the visible response text, including the server's form messages. */
static void print_response(const char *p)
{
    size_t printed = 0;
    int space = 1;
    puts("Response text (maximum 16000 characters):");
    while (*p && printed < 16000) {
        if (*p == '<') {
            const char *end = strchr(p, '>');
            if (!end) break;
            if (_strnicmp(p, "<script", 7) == 0 || _strnicmp(p, "<style", 6) == 0) {
                const char *closing = _strnicmp(p, "<script", 7) == 0 ? "</script" : "</style";
                p = end + 1;
                while (*p && _strnicmp(p, closing, strlen(closing)) != 0) ++p;
                if (!*p) break;
                end = strchr(p, '>');
                if (!end) break;
            }
            p = end + 1;
            if (!space) { putchar('\n'); ++printed; space = 1; }
        } else {
            unsigned char ch = (unsigned char)*p++;
            if (isspace(ch)) {
                if (!space) { putchar(' '); ++printed; space = 1; }
            } else if (ch >= 32) { putchar(ch); ++printed; space = 0; }
        }
    }
    puts("\nHTTP 200 alone does not prove CAPTCHA rejection or form acceptance.");
    puts("Inspect the form message and verify no accepted Everest Forms entry exists.");
}

int main(void)
{
    HINTERNET session = NULL, connection = NULL;
    char *html = NULL, *response = NULL;
    char nonce[256], referer[2048], start_time[256], payload[16384];
    const char *form, *form_end = NULL;
    size_t used = 0;
    DWORD status = 0;
    int result = EXIT_FAILURE;
    int referer_found;
    puts("EuroCell form 216: one local POST with an empty reCAPTCHA field.");
    session = WinHttpOpen(L"EuroCell-Local-Security-Test/1.0",
        WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);
    if (!session) { printf("Open error: %lu\n", (unsigned long)GetLastError()); goto cleanup; }
    if (!WinHttpSetTimeouts(session, 5000, 5000, 10000, 10000)) {
        printf("Timeout setup error: %lu\n", (unsigned long)GetLastError()); goto cleanup;
    }
    connection = WinHttpConnect(session, L"eurocell-local.local", 80, 0);
    if (!connection) { printf("Connect error: %lu\n", (unsigned long)GetLastError()); goto cleanup; }
    if (!request(connection, L"GET", NULL, 0, &html, &status)) {
        printf("GET result: failed; HTTP status: %lu\nNonce found: no\nPOST result: skipped\n", (unsigned long)status);
        goto cleanup;
    }
    printf("GET result: completed; HTTP status: %lu\n", (unsigned long)status);
    /* Only use hidden values from the form containing the current nonce. */
    form = html;
    while ((form = strstr(form, "<form")) != NULL) {
        form_end = strstr(form, "</form>");
        if (!form_end) break;
        if (input_value(form, form_end, "_wpnonce216", nonce, sizeof(nonce)) && nonce[0]) break;
        form = form_end + 7;
        form_end = NULL;
    }
    printf("Nonce found: %s\n", form && form_end ? "yes" : "no");
    referer_found = form && form_end &&
        input_value(form, form_end, "_wp_http_referer", referer, sizeof(referer));
    printf("_wp_http_referer found: %s\n", referer_found ? "yes" : "no");
    if (status != 200 || !form || !form_end ||
        !referer_found) {
        puts("POST result: skipped; successful GET and hidden input values are required.");
        goto cleanup;
    }
    /* The browser fills this initially valueless input with Date.now(). */
    if (!submission_start_time(start_time, sizeof(start_time))) {
        puts("POST result: skipped; could not generate submission start time.");
        goto cleanup;
    }
    puts("evf_submission_start_time: generated from current Unix milliseconds.");
#define FIELD(name, value) do { if (!add_field(payload, sizeof(payload), &used, name, value)) { puts("POST result: skipped; payload too large."); goto cleanup; } } while (0)
    FIELD("_wpnonce216", nonce);
    FIELD("_wp_http_referer", referer);
    FIELD("evf_submission_start_time", start_time);
    FIELD("everest_forms[form_fields][fullname]", "Bot Security Test");
    FIELD("everest_forms[form_fields][email]", "bot-test@example.invalid");
    FIELD("everest_forms[form_fields][subject]", "Automated Security Test");
    FIELD("everest_forms[form_fields][message]", "Authorized local anti-spam test");
    /* Deliberately empty: no token generation, copying, solving or replay. */
    FIELD("everest_forms[recaptcha]", "");
    FIELD("everest_forms[id]", "216");
    FIELD("everest_forms[author]", "2");
    FIELD("everest_forms[post_id]", "627");
    FIELD("everest_forms[submit]", "evf-submit");
#undef FIELD
    {
        int count = snprintf(payload + used, sizeof(payload) - used, "--" BOUNDARY "--\r\n");
        if (count < 0 || (size_t)count >= sizeof(payload) - used) goto cleanup;
        used += (size_t)count;
    }
    /* Match normal form waiting time before the only POST attempt. */
    puts("Waiting 11 seconds before the single POST attempt.");
    Sleep(11000);
    /* The only POST call. Failure or redirect never causes another attempt. */
    if (!request(connection, L"POST", payload, (DWORD)used, &response, &status)) {
        printf("POST result: failed; response status: %lu\n", (unsigned long)status);
        goto cleanup;
    }
    printf("POST result: completed; response status: %lu\n", (unsigned long)status);
    print_response(response);
    result = EXIT_SUCCESS; /* Transport completed, not a security test pass. */
cleanup:
    free(html);
    free(response);
    if (connection && !WinHttpCloseHandle(connection))
        printf("Close connection error: %lu\n", (unsigned long)GetLastError());
    if (session && !WinHttpCloseHandle(session))
        printf("Close session error: %lu\n", (unsigned long)GetLastError());
    return result;
}
