
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/loop.h>

static int loop_umount(const char* path)
{
	FILE *f;
	struct stat file_stat;
	char dev_name[256];
    int count;
    char device[256];
    char mount_path[256];
    char rest[256];
    int result = 0;
    int path_length = strlen(path);
    
    f = fopen("/proc/mounts", "r");
    if (!f) {
        fprintf(stdout, "could not open /proc/mounts\n");
        return -1;
    }

    do {
        count = fscanf(f, "%255s %255s %255s\n", device, mount_path, rest);
        if (count == 3) {
            if (strcmp(path, mount_path) == 0) {
				result = stat(path, &file_stat);
				if(result < 0)
					continue;

				if(major(file_stat.st_rdev) == 7) // 7 is loopback, riiight? :P
				{ 
					// free the loop device
					int loop_fd = open(device, O_RDONLY);

					if (loop_fd < -1) {
						perror("open loop device failed");
						return 1;
					}

					if (ioctl(loop_fd, LOOP_CLR_FD, 0) < 0) {
						perror("ioctl LOOP_CLR_FD failed");
						return 1;
					}

					close(loop_fd);
					result = 0;
				}
                break;
            }
        }
    } while (count == 3);

    fclose(f);
    return result;
}

int umount_main(int argc, char *argv[])
{
    if(argc != 2) {
        fprintf(stderr,"umount <path>\n");
        return 1;
    }

    if(umount(argv[1])){
        fprintf(stderr,"failed.\n");
        return 1;
    }

	if(loop_umount(argv[1]))
	{
		fprintf(stderr, "loop umount failed.\n");
		return 1;
	}

    return 0;
}
