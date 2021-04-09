#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // for read/write/close
#include <sys/types.h>  // standard system types
#include <netinet/in.h> // Internet address structures
#include <sys/socket.h> // socket interface functions
#include <netdb.h>      // host to IP resolution

#define BUFLEN 100
#define h_addr h_addr_list[0] /* for backward compatibility */

int splitUrl(char *url, char **host_ptr, int *port, char **file_path);

int get_p_index(int argc, char *argv[]);

int get_r_index(int argc, char *argv[]);

int get_r_index_from(int argc, char *argv[], int ind);

void error(char *error);

void errorUsage();

void insertToPost(char *src, char **dest);

int isEqualCharAtValidPlace(char *word);

int isNumber(char *number);

int insertToParameters(int argc, char *argv[], int r_index, char **parameters);

int checkIfArgcIsValid(int argc, char *argv[], int p, int r);

int findUrlIndex(int argc, char *argv[], int p, int r);

void buildRequest(char *host, char *path, char *post, char *parameters, int p, int r, char **req);

int sendRequest(char *request, char *host, int port);

int main(int argc, char *argv[])
{
    char *request;
    int check = 0;
    int url_ind = -1;
    char *host = NULL;
    char *file_path = NULL;
    int port = 0;

    int p_ind = -1;
    char *post = NULL;
    int r_ind = -1;
    char *parameters = NULL;

    // takes care for "-p" flag
    p_ind = get_p_index(argc, argv);
    if (p_ind == argc - 1)
        errorUsage();
    else if (p_ind >= 0)
        insertToPost(argv[p_ind + 1], &post);

    // takes care for "-r" flag
    r_ind = get_r_index(argc, argv);
    if (p_ind >= 0 && p_ind + 1 == r_ind) // if the text of p is "-r"
    {
        r_ind = -1;
        r_ind = get_r_index_from(argc, argv, p_ind + 2);
    }

    if (r_ind == argc - 1)
    {
        free(post);
        errorUsage();
    }
    else if (r_ind >= 0)
    {
        check = insertToParameters(argc, argv, r_ind, &parameters);
        if (check == -1) // failed
        {
            free(post);
            free(parameters);
            errorUsage();
        }
    }

    check = checkIfArgcIsValid(argc, argv, p_ind, r_ind);
    if (check == -1) // failed
    {
        free(post);
        free(parameters);
        errorUsage();
    }

    // takes care for URL
    url_ind = findUrlIndex(argc, argv, p_ind, r_ind);
    if (url_ind == -1) // failed
    {
        free(post);
        free(parameters);
        errorUsage();
    }
    check = splitUrl(argv[url_ind], &host, &port, &file_path);
    
    if (check == -1) // failed
    {
        free(host);
        free(parameters);
        free(file_path);
        free(post);
        return EXIT_FAILURE;
    }

    // build the request
    buildRequest(host, file_path, post, parameters, p_ind, r_ind, &request);

    // socket part
    check = sendRequest(request, host, port);

    free(host);
    free(file_path);
    free(post);
    free(parameters);
    free(request);

    if (check == -1)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

void error(char *error)
{
    perror(error);
    exit(1);
}

void errorUsage()
{
    printf("Usage: client [-p <text>] [-r n <pr1=value1 pr2=value2 …>] <URL>\n");
    exit(0);
}

int splitUrl(char *url, char **host_ptr, int *port, char **file_path)
{
    int is_port = 0;
    char port_arr[100] = "";
    const int SEVEN = 7; // "http://" is 7 chars
    int host_chars = 0;
    char *req = NULL;

    int i = SEVEN;
    while (url[i] != '\0' && url[i] != '/')
    {
        if (url[i] == ':')
        {
            is_port = 1;
            break;
        }
        host_chars++;
        i++;
    }

    // takes care for host
    char *host = (char *)malloc(sizeof(char) * (host_chars + 1));
    if (host == NULL)
        error("malloc of host\n");

    for (i = 0; i < host_chars; i++)
    {
        host[i] = url[i + SEVEN];
    }
    host[i] = '\0';
    *host_ptr = host;

    // takes care for port
    i = SEVEN + host_chars;
    if (is_port == 1)
    {
        int j = 0;
        i = SEVEN + host_chars + 1;
        while (url[i] != '\0' && url[i] != '/')
        {
            port_arr[j] = url[i];
            j++;
            i++;
        }
        if (port_arr != NULL)
            *port = atoi(port_arr);
    }
    else
    {
        *port = 80;
    }

    // takes care for request
    int req_index = i;
    int k = 0;

    while (url[i] != '\0') // count number of chars
    {
        k++;
        i++;
    }

    if (k == 0) // there is no request put "\"
    {
        req = (char *)malloc(sizeof(char) * 2);
        if (req == NULL)
            error("malloc of request\n");
        req[0] = '/';
        req[1] = '\0';
    }

    else
    {
        k++;
        req = (char *)malloc(sizeof(char) * k);
        if (req == NULL)
            error("malloc of request\n");

        for (int j = 0; j < k; j++)
            req[j] = url[j + req_index];
        req[k - 1] = '\0';
    }

    *file_path = req;
    if (*port <= 0)
    {
        printf("port number must be bigger than 0\n");
        return -1;
    }

    return 0;
}

int get_p_index(int argc, char *argv[])
{
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-p") == 0)
        {
            return i;
        }
    }
    return -1;
}

int get_r_index(int argc, char *argv[])
{
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-r") == 0)
        {
            return i;
        }
    }
    return -1;
}

int get_r_index_from(int argc, char *argv[], int ind)
{
    for (int i = ind; i < argc; i++)
    {
        if (strcmp(argv[i], "-r") == 0)
        {
            return i;
        }
    }
    return -1;
}

void insertToPost(char *src, char **dest)
{
    int i = 0;
    while (src[i] != '\0')
        i++;

    char *temp = (char *)malloc(sizeof(char) * (i + 1));
    strcpy(temp, src);

    *dest = temp;
}

int isEqualCharAtValidPlace(char *word)
{
    int len = (int)strlen(word);
    for (int i = 1; i < len - 1; i++)
    {
        if (word[i] == '=')
            return 0;
    }
    return -1;
}

int isNumber(char *number)
{
    int len = strlen(number);
    for (int i = 0; i < len; i++)
    {
        if (number[i] != '0' && number[i] != '1' && number[i] != '2' && number[i] != '3' && number[i] != '4' && number[i] != '5' && number[i] != '6' && number[i] != '7' && number[i] != '8' && number[i] != '9')
            return -1;
    }
    return 0;
}

int insertToParameters(int argc, char *argv[], int r_index, char **parameters)
{
    char *temp = NULL;
    int check = isNumber(argv[r_index + 1]);
    if (check == -1)
        return -1;
    int num_of_params = atoi(argv[r_index + 1]);
    if (num_of_params < 0)
        return -1;
    if (num_of_params == 0)
    {
        return 1;
    }

    if (r_index + 1 + num_of_params >= argc)
        return -1;

    int num_of_chars = 0;
    for (int i = r_index + 2; i < r_index + 2 + num_of_params; i++)
    {
        if (isEqualCharAtValidPlace(argv[i]) == -1)
            return -1;

        num_of_chars += strlen(argv[i]);
    }

    num_of_chars += num_of_params;
    temp = (char *)malloc(sizeof(char) * (num_of_chars + 1));
    if (temp == NULL)
        error("malloc at insert to parameters\n");

    temp[0] = '?';
    temp[1] = '\0';
    for (int i = r_index + 2; i < r_index + 2 + num_of_params; i++)
    {
        strcat(temp, argv[i]);
        if (i != r_index + 2 + num_of_params - 1)
            strcat(temp, "&");
    }

    *parameters = temp;
    return 0;
}

int checkIfArgcIsValid(int argc, char *argv[], int p, int r)
{
    int count = 2; // program name + url
    if (p >= 0)
        count += 2; // -p + [text]

    if (r >= 0)
    {
        int num_of_params = atoi(argv[r + 1]);
        if (num_of_params < 0)
            return -1;
        count += 2; // -r + [num]
        count += num_of_params;
    }

    if (count != argc)
        return -1;
    return 0;
}

int findUrlIndex(int argc, char *argv[], int p, int r)
{

    // calculate forbiden zones
    int from1 = -1;
    int to1 = -1;
    if (p >= 0)
    {
        from1 = p;
        to1 = p + 1;
    }

    int from2 = -1;
    int to2 = -1;
    if (r >= 0)
    {
        from2 = r;
        to2 = r + 1 + atoi(argv[r + 1]);
    }

    for (int i = 0; i < argc; i++)
    {
        if (strstr(argv[i], "http://") != NULL)
        {
            if (i >= from1 && i <= to1)
                continue;
            if (i >= from2 && i <= to2)
                continue;

            return i;
        }
    }

    return -1;
}

void buildRequest(char *host, char *path, char *post, char *parameters, int p, int r, char **req)
{
    int size = 200;

    if (host != NULL)
        size += strlen(host);
    if (path != NULL)
        size += strlen(path);
    if (post != NULL)
        size += strlen(post);
    if (parameters != NULL)
        size += strlen(parameters);

    char *temp = (char *)malloc(sizeof(char) * size);
    if (temp == NULL)
        error("malloc at build request\n");

    temp[0] = '\0';
    if (p >= 0)
        strcat(temp, "POST ");
    else
        strcat(temp, "GET ");

    strcat(temp, path);

    if (r >= 0 && parameters != NULL)
        strcat(temp, parameters);

    strcat(temp, " HTTP/1.0\r\n");
    strcat(temp, "Host: ");
    strcat(temp, host);
    strcat(temp, "\r\n");

    if (p >= 0)
    {
        char len[50] = "\0";
        sprintf(len, "%d", (int)strlen(post));
        strcat(temp, "Content-length:");
        strcat(temp, len);
        strcat(temp, "\r\n\r\n");
        strcat(temp, post);
    }

    else
        strcat(temp, "\r\n");

    *req = temp;
}

int sendRequest(char *request, char *host, int port)
{
    int sockfd;
    int check;
    char rbuf[BUFLEN];
    struct sockaddr_in serv_addr;
    struct hostent *server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket failed");
        return -1;
    }

    // connect to server
    server = gethostbyname(host);
    if (server == NULL)
    {
        herror("ERROR, no such host\n");
        close(sockfd);
        return -1;
    }
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    check = connect(sockfd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (check < 0)
    {
        perror("connect failed:");
        close(sockfd);
        return -1;
    }
    printf("HTTP request =\n%s\nLEN = %d\n", request, (int)strlen(request));

    check = write(sockfd, request, strlen(request) + 1);
    if (check <= 0)
    {
        perror("write error\n");
        close(sockfd);
        return -1;
    }

    int response_size = 0;
    do
    {
        check = read(sockfd, rbuf, BUFLEN - 1);
        if (check == -1)
        {
            perror("read error\n");
            close(sockfd);
            return -1;
        }
        rbuf[check] = '\0';
        printf("%s", rbuf);
        response_size += check;
    } while (check > 0);

    close(sockfd);
    printf("\n Total received response bytes: %d\n", response_size);
    return 0;
}