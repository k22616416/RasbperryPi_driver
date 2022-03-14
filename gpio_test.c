#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <time.h>
#include <limits.h>

#define BCM2708_PERI_BASE 0x3F000000
#define GPIO_BASE (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */

#define PAGE_SIZE (4 * 1024)
#define BLOCK_SIZE (4 * 1024)

#define DHT_PIN 4
#define DHT11_TIMEOUT UINT_MAX

int mem_fd;
void *gpio_map;

/* I/O access */
volatile unsigned *gpio;

/* GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y) */
#define INP_GPIO(g) *(gpio + ((g) / 10)) &= ~(7 << (((g) % 10) * 3))
#define OUT_GPIO(g) *(gpio + ((g) / 10)) |= (1 << (((g) % 10) * 3))
#define GET_GPIO(g) (*(gpio + 13) & (1 << g))
#define SET_GPIO_ALT(g, a) *(gpio + (((g) / 10))) |= (((a) <= 3 ? (a) + 4 : (a) == 4 ? 3  \
                                                                                     : 2) \
                                                      << (((g) % 10) * 3))

#define GPIO_SET *(gpio + 7)  /* sets   bits which are 1 ignores bits which are 0 */
#define GPIO_CLR *(gpio + 10) /* clears bits which are 1 ignores bits which are 0 */

/**
 * Set up a memory regions to access GPIO
 *
 */
void *mulitThread(void *argu)
{
        int i = 0;
        INP_GPIO(2);
        printf("No.2 thread start.\n");
        for (i = 0; i < 30; i++)
        {
                OUT_GPIO(2);
                GPIO_SET = 1 << 2;
                sleep(1);
                GPIO_CLR = 1 << 2;
                sleep(1);
                printf("No.2:Flash %d", i);
        }
        pthread_exit(NULL);
}

void setup_io()
{
        /* open /dev/mem */
        if ((mem_fd = open("/dev/mem", O_RDWR | O_SYNC)) < 0)
        {
                printf("can't open /dev/mem \n");
                exit(-1);
        }

        /* mmap GPIO */
        gpio_map = mmap(
            NULL,                   /* Any adddress in our space will do */
            BLOCK_SIZE,             /* Map length */
            PROT_READ | PROT_WRITE, /* Enable reading & writting to mapped memory */
            MAP_SHARED,             /* Shared with other processes */
            mem_fd,                 /* File to map */
            GPIO_BASE               /* Offset to GPIO peripheral */
        );

        close(mem_fd); /* No need to keep mem_fd open after mmap */

        if (gpio_map == MAP_FAILED)
        {
                printf("mmap error %d\n", (int)gpio_map); /* errno also set! */
                exit(-1);
        }

        /* Always use volatile pointer! */
        gpio = (volatile unsigned *)gpio_map;
}

void CreatWave(int status, double time)
{
}

u_int16_t readWave(u_int8_t pin, u_int8_t target)
{
        u_int16_t count = 0;
        u_int8_t force = !!target;
        while ((!!GET_GPIO(pin)) == force)
        {
                if (count++ > DHT11_TIMEOUT)
                {
                        return (u_int16_t)DHT11_TIMEOUT;
                }
        }
        return count;
}

int main(int argc, char **argv)
{
        pthread_t thread1;
        int i;
        /* Set up gpi pointer for direct register access */
        setup_io();

        // pthread_create(&thread1,NULL,&mulitThread,NULL);
        /* Must use INP_GPIO before we can use OUT_GPIO */

        INP_GPIO(4);
        /*
        for(i=0;i<10;i++)
        {
                OUT_GPIO(4);
                GPIO_SET = 1 << 4;
                //usleep(1)     //delay 1us
                //nanosleep(1)  //delay 1ns
                sleep(1);       //delay 1s

                GPIO_CLR = 1 << 4;


                sleep(1);
        }
        */

        OUT_GPIO(4);
        GPIO_SET = 1 << 4;
        GPIO_CLR = 1 << 4;
        usleep(18000); // 1. LOW 18ms
        GPIO_SET = 1 << 4;
        usleep(30); // 2. HIGH 20~40us

        // while (1)
        // {
        //         i = GET_GPIO(4);

        //         printf("Port 4 is %d\n", i);
        //         sleep(1);
        // }

        // pthread_join(thread1,NULL);
        return 0;
}
