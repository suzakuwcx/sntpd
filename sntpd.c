#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <ctype.h>
#include <netdb.h>

#include <sys/time.h>
#include <sys/socket.h>


static int dry_run = 0;

static inline int on_string_positive(const char *str) {
    int len = strlen(str);
    int i;

    for (i = 0; i < len; ++i) {
        if (isdigit(str[i]) == 0)
            return 1;
    }

    if (len > 1) {
        if (str[0] == '0') 
            return 1;
    }

    return 0;
}

static inline int on_string_minus(const char *str) {
    return on_string_positive(str + 1);
}

/**
 * @brief check if a string a valid digital 
 *
 * @return 0 if string a valid digital
 *         1 if not valid
 *
 * @code
 * if (is_string_digital("123") == 0) // return 0
 *     // atos("123")
 * if (is_string_digital("aaa") == 0) {} // return 1
 * if (is_string_digital("001") == 0) {} // return 1
 * if (is_string_digital("-1") == 0) {} // return 0
 * @endcode
 */

static int is_string_digital(const char *str) {
    int len;

    if (str == NULL)
        return 1;

    len = strlen(str);

    if (len == 0)
        return 1;

    if (len == 1) {
        if (!(isdigit(str[0]) != 0))
            return 1;
        else 
            return 0;
    }

    if (str[0] == '-')
        return on_string_minus(str);
    else
        return on_string_positive(str);
}

/**
 * @brief check if host string a number and dot notation
 * 
 * @return 0 if host are number and dot notation
 *         1 if host are domain
 * @code
 * if (is_host_number_and_dot("127.0.0.1") == 0)
 *    // create sockaddr_in
 * else
 *    // try to resolve dns
 * @endcode
 */

static int is_host_number_and_dot(const char *host) {
    char *buff = strdup(host);
    char *tmp;
    int ret = 0;

    tmp = strtok(buff, ".");
    while (tmp != NULL) {
        if (!(is_string_digital(tmp) == 0)) {
            ret = 1;
            goto clean;
        }
        tmp = strtok(NULL, ".");
    }
    
clean:
    free(buff);
    return ret;
}

/**
 * @brief create a udp connecion
 *
 * @code
 * int sock_fd = udp_connect("127.0.0.1:12345");
 * if (sock_fd < 0) {
 *     fprintf(stderr, "error occur\n");
 * }
 * char buf[] = "123";
 * send(sock_fd, buf, sizeof(buf), 0);
 * @endcode
 */
static int udp_connect(const char *sockaddr) {
    char *buff = strdup(sockaddr);
    char *host;
    char *port;
    int sock_fd;

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("socket");
        return -1;
    }

    host = strtok(buff, ":");
    port = strtok(NULL, ":");

    struct sockaddr_in saddr = {
        .sin_family = AF_INET,
        .sin_addr = inet_addr("0.0.0.0"),
        .sin_port = INADDR_ANY
    };

    if (bind(sock_fd, (const struct sockaddr *) &saddr, sizeof(saddr)) < 0) {
        perror("bind");
        return -1;
    }

    struct sockaddr_in daddr = {
        .sin_family = AF_INET,
        .sin_addr = inet_addr(host),
        .sin_port = htons(atoi(port))
    };

    if (connect(sock_fd, (const struct sockaddr *) &daddr, sizeof(daddr)) < 0) {
        perror("connect");
        return -1;
    }

    free(buff);
    return sock_fd;
}

/**
 * @brief: create udp connection
 * 
 * @param hostname host string or a number and dot notation 
 *                    (e.g. 127.0.0.1 or localhost)
 * @param service service name or port number (e.g. 22 or ssh)
 * 
 * @code
 * int sock_fd = udp_connect2("localhost", "12345");
 * if (sock_fd < 0) {
 *     fprintf(stderr, "error occur\n");
 * }
 * @endcode
 */
static int udp_connect2(const char *hostname, const char *service) {
    int sock_fd;
    int ret;
    struct addrinfo *result;
    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_DGRAM
    };

    if (getaddrinfo(hostname, service, &hints, &result) < 0) {
        perror("getaddrinfo");
        return -1;
    }

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("sock_fd");
        ret = -1;
        goto clean;
    }

    struct sockaddr_in saddr = {
        .sin_family = AF_INET,
        .sin_addr = inet_addr("0.0.0.0"),
        .sin_port = INADDR_ANY
    };

    if (bind(sock_fd,(const struct sockaddr *) &saddr, sizeof(saddr)) < 0) {
        perror("bind");
        ret = -1;
        goto clean;
    }

    if (connect(sock_fd, (const struct sockaddr *) result->ai_addr, sizeof(struct sockaddr_in)) < 0) {
        perror("connect");
        ret = -1;
        goto clean;
    }

    ret = sock_fd;
clean:
    freeaddrinfo(result);
    return ret;
}

/**
 * @brief set socket recive timeout and send timeout
 *
 * @param fd an connected socket file descriptor
 * @param second timeout value
 *
 * @return -1 if error occur
 *          0 if success
 *
 * @code
 * ...
 * connect(sock_fd, ...);
 * set_sock_timeout(sock_fd, 30); // set the connection time out for 30s
 * ...
 * @endcode
 */
static int set_sock_timeout(int fd, int second) 
{
    struct timeval tx_tv = { .tv_sec = second }, rx_tv = { .tv_sec = second };
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tx_tv, sizeof tx_tv) < 0) {
        perror("setsockopt");
        return -1;
    }
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &rx_tv, sizeof rx_tv) < 0) {
        perror("setsockopt");
        return -1;
    }
    return 0;
}

/**
 * @brief unpack ntp time stamps format 
 *
 * Timestamps. The 64-bit binary fixed-point timestamps used by NTP consist 
 * of a 32-bit part for seconds and a 32-bit part for fractional second.
 *
 * for ntp v3 header: unsigned char h[56]
 * 
 * @see rfc4330
 * Receive Timestamp(T2):  h[40] ~ h[47]
 * Transmit Timestamp(T3): h[48] ~ h[56]
 *
 * @param p the 32 bit ntp timestamp
 *
 * @return time stamp value since January 1, 1900
 *
 * @code
 * unsigned char h[48] = { 0x1b, };
 * get_ntp_reply(h)
 * unsigned long T3_second = unpack32(&h[40])
 * unsigned long T3_fractional = unpack32(&h[42])
 * @endcode
 */
static inline unsigned long unpack32(unsigned char *p) {
    unsigned long l = 0;
    l |= ((unsigned long)p[0]) << 24;
    l |= ((unsigned long)p[1]) << 16;
    l |= ((unsigned long)p[2]) <<  8;
    l |= ((unsigned long)p[3]) <<  0;
    return l;
}


/**
 * @brief update system time
 *
 * This call is restricted to the super-user.
 *
 * @param t time, unit is second
 *
 * @return -1 if error occur
 *          0 if success
 * 
 * @code
 * time_t t;
 * time(&t); // get system current time, you can also get from sntp
 * if (update_system_time((unsigned long)t) < 0) {
 *     fprintf(stderr, "error occur\n"); 
 * }
 * @endcode
 */
static int update_system_time(unsigned long t) {
    struct timeval tv = {.tv_sec = t, .tv_usec = 0};
    int ret = settimeofday(&tv, NULL);
    if (ret < 0) {
        perror("settimeofday");
        return -1;
    }
    return 0;
}

/*
 * NTP uses an epoch of January 1, 1900. But, linux system clock
 * use the Epoch of (1970-01-01 00:00 UTC)
 */
#define DELTA (2208988800ull)

/**
 * @brief request T3 time second from ntp server
 *
 * @param hostname the ntp server hostname
 *                    if leave NULL, use default "uk.pool.ntp.org"
 *
 * @return -1 if error occur
 *         T3 second is success
 */
static int sntp_request(const char *hostname)
{    
    int sock_fd;
    char *buff;
    int second = 0;

    if (hostname == NULL)
        buff = strdup("in.pool.ntp.org");
    else
        buff = strdup(hostname);

    sock_fd = udp_connect2(buff, "123");
    if (sock_fd < 0) {
        fprintf(stderr, "error occur\n");
        second = -1;
    }

    set_sock_timeout(sock_fd, 30);

    unsigned char h[48] = { 0x1b, };
    if (send(sock_fd, h, sizeof(h), 0) < 0) {
        perror("send");
        second = -1;
        goto clean;
    }
    if (recv(sock_fd, h, sizeof(h), 0) < 0) {
        perror("recv");
        second = -1;
        goto clean;
    };

    second = unpack32(&h[40]) - DELTA;

clean:
    free(buff);
    close(sock_fd);
    return second;
}

static int usage(int code)
{
    FILE *fp = stdout;

    if (code)
        fp = stderr;

    fprintf(fp,
        "Usage:\n"
        "  %s [-h] [-d] [-n] [-m main_ntp_host] [-s side_ntp_host] [-t request_interval] \n"
        "\n"
        "Options:\n"
        "  -h       Show summary of command line options and exit\n"
        "  -d       Use default parameter to start\n"
        "  -n       Dry run, will print time to stdout instead of changing system time\n"
        "  -m       Main ntp server host (default: cn.pool.ntp.org)\n"
        "  -s       Side ntp server host (default: pool.ntp.org)\n"
        "  -t       Request interval (second, default: 3600)\n"
        "\n", "sntpd");

    return code;
}

static int main_loop(int loop_period, const char *main, const char *side) {
    int ret = 0;
    while (1) {
        ret = sntp_request(main);
        if (ret < 0) 
            ret = sntp_request(side);

        if (ret < 0) {
            fprintf(stderr, "both ntp request is failed\n");
        }
        else if (dry_run) {
            fprintf(stdout, "time from sntp: %d\n", ret);
        }
        else {
            ret = update_system_time((unsigned long)ret);
            if (ret < 0)
                fprintf(stderr, "update system time failed\n");            
        }
            
        sleep(loop_period);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int show_help = 1;
    int loop_period = 3600;
    char *main = "cn.pool.ntp.org";
    char *side = "pool.ntp.org";

    while (1) {
        int c;

        c = getopt(argc, argv, "hdnm:s:t:");
        if (c == EOF)
            break;

        switch (c) {
        case 'h':
            return usage(0);
        case 'd':
            show_help = 0;
        case 'n':
            show_help = 0;
            dry_run = 1;
        case 'm':
            show_help = 0;
            main = optarg;
            break;
        case 's':
            show_help = 0;
            side = optarg;
            break;
        case 't':
            show_help = 0;
            if (is_string_digital(optarg) == 0) {
                loop_period = atoi(optarg);
                /* the minimal request period should greater than 64 seconds */
                if (loop_period < 64) {
                    fprintf(stdout, "The loop period is too short, adjust to 64\n");
                    loop_period = 64;                    
                }
            }
            else {
                fprintf(stderr, "Invalid value %s\n", optarg);
                return 1;
            }
            break;
        case '?':
            fprintf(stderr, "Unknown option %c\n", optopt);
            return usage(1);
        default:
            return usage(1);
        }
    }

    if (show_help)
        return usage(1);

    main_loop(loop_period, main, side);

    return 0;
}
