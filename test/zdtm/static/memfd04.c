#include <fcntl.h>
#include <linux/memfd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/vfs.h>
#include <unistd.h>

#include "zdtmtst.h"

const char *test_doc = "mmap with memfd_secret";
const char *test_author = "Suraj Shirvankar <surajshirvankar@gmail.com>";

#define err(exitcode, msg, ...)                \
	({                                     \
		pr_perror(msg, ##__VA_ARGS__); \
		exit(exitcode);                \
	})

static int _memfd_secret(unsigned int flags)
{
	return syscall(SYS_memfd_secret, flags);
}

int main(int argc, char *argv[])
{
	int fd;
	void *addr;
	char data[] = "secret_data";
        size_t DATA_SIZE = sizeof(data);

	task_waiter_t t;

	test_init(argc, argv);

	task_waiter_init(&t);

	fd = _memfd_secret(O_CLOEXEC);
	if (fd < 0)
		pr_perror("Can't call memfd_secret.Enable it by passing in ");

	ftruncate(fd, DATA_SIZE);
        addr = mmap(NULL, DATA_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (addr == MAP_FAILED)
		pr_perror("Unable to mmap");

	strncpy(addr, data, DATA_SIZE);

	test_daemon();
	test_waitsig();

	if(strcmp(addr, data)) {
		fail("Data in memfd_secret is incorrect");
		return 1;
	}

	pass();

	return 0;
}
