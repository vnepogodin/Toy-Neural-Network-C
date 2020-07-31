#include "random_seed.h"

# include <stdlib.h> /* arc4random, rand_r */

#ifdef __linux__
# include <fcntl.h> /* openat, O_RDONLY */
# include <unistd.h> /* pread, close */
#elif _WIN32
# include <windows.h>
# include <bcrypt.h> /* BCryptGenRandom */
#endif

int get_random_seed(void) {
#ifdef __linux__
    register int fd = openat(0, "/dev/urandom", O_RDONLY, 0);
    unsigned char buf[1] = { 0U };

    if (fd != -1) {
        pread(fd, buf, 1, 0);
        close(fd);
    }

    unsigned int __random = buf[0];

    return rand_r(&__random);
#elif _WIN32
    UINT __random = 0U;

    BCryptGenRandom(NULL, (BYTE*)&__random, sizeof(UINT), BCRYPT_USE_SYSTEM_PREFERRED_RNG);

    return (int)__random;
#else
    return (int)arc4random();
#endif
}
