#ifndef __DEFINE__
#define __DEFINE__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h> 
#include <pthread.h>

#define FAIL(fmt, ...) printf("[\x1b[31mFAIL\x1b[0m]"fmt "\n", ##__VA_ARGS__)
#define SUCCESS(fmt, ...) printf("[\x1b[32mSUCCESS\x1b[0m]"fmt "\n", ##__VA_ARGS__)
#define DEBUG(fmt, ...) printf("[\x1b[35mINFO\x1b[0m]"fmt "\n", ##__VA_ARGS__)

#define COMMON_PORT 4096
#define TEMP_BUF_SIZE 1024
#define POLL_TIMEOUT_CNT 10

typedef enum
{
    STEP1,
    STEP2,
    STEP3,
    STEP4,
    STEP5,
    STEP6,
    STEP7,
    STEP8,
    STEP9,
    STEP10,
}STEP;

#endif