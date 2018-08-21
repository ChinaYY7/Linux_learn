//打印系统限制值
#include "apue.h"
#include <errno.h>
#include <limits.h>
int main(int argc, char *argv[])
{
	long int variate_value;
	int errno_l;
	errno_l = errno;
	if(argv[1][0] == 'S')
	{
		if(argc != 2)
		{
			printf("Please Input One Paramter\n");
			exit(0);
		}
		if(strcmp(argv[1],"SC_CLK_TCK") == 0)
			variate_value = sysconf(_SC_CLK_TCK);
		else if(strcmp(argv[1],"SC_ARG_MAX") == 0)
			variate_value = sysconf(_SC_ARG_MAX);
		else if(strcmp(argv[1],"SC_ATEXIT_MAX") == 0)
			variate_value = sysconf(_SC_ATEXIT_MAX);
		else if(strcmp(argv[1],"SC_CHILD_MAX") == 0)
			variate_value = sysconf(_SC_CHILD_MAX);
		else if(strcmp(argv[1],"SC_COLL_WEIGHTS_MAX") == 0)
			variate_value = sysconf(_SC_COLL_WEIGHTS_MAX);
		else if(strcmp(argv[1],"SC_DELAYTIMER_MAX") == 0)
			variate_value = sysconf(_SC_DELAYTIMER_MAX);
		else if(strcmp(argv[1],"SC_HOST_NAME_MAX") == 0)
			variate_value = sysconf(_SC_HOST_NAME_MAX);
		else if(strcmp(argv[1],"SC_IOV_MAX") == 0)
			variate_value = sysconf(_SC_IOV_MAX);
		else if(strcmp(argv[1],"SC_LINE_MAX") == 0)
			variate_value = sysconf(_SC_LINE_MAX);
		else if(strcmp(argv[1],"SC_LOGIN_NAME_MAX") == 0)
			variate_value = sysconf(_SC_LOGIN_NAME_MAX);
		else if(strcmp(argv[1],"SC_NGROUPS_MAX") == 0)
			variate_value = sysconf(_SC_NGROUPS_MAX);
		else if(strcmp(argv[1],"SC_OPEN_MAX") == 0)
			variate_value = sysconf(_SC_OPEN_MAX);
		else if(strcmp(argv[1],"SC_PAGESIZE_MAX") == 0)
			variate_value = sysconf(_SC_PAGESIZE);
		else if(strcmp(argv[1],"SC_PAGR_SIZE_MAX") == 0)
			variate_value = sysconf(_SC_PAGE_SIZE);
		else if(strcmp(argv[1],"SC_RE_DUP_MAX") == 0)
			variate_value = sysconf(_SC_RE_DUP_MAX);
		else if(strcmp(argv[1],"SC_RTSIG_MAX") == 0)
			variate_value = sysconf(_SC_RTSIG_MAX);
		else if(strcmp(argv[1],"SC_SEM_NSEMS_MAX") == 0)
			variate_value = sysconf(_SC_SEM_NSEMS_MAX);
		else if(strcmp(argv[1],"SC_SEM_VALUE_MAX") == 0)
			variate_value = sysconf(_SC_SEM_VALUE_MAX);
		else if(strcmp(argv[1],"SC_SIGQUEUE_MAX") == 0)
			variate_value = sysconf(_SC_SIGQUEUE_MAX);
		else if(strcmp(argv[1],"SC_STREAM_MAX") == 0)
			variate_value = sysconf(_SC_STREAM_MAX);
		else if(strcmp(argv[1],"SC_SYMLOOP_MAX") == 0)
			variate_value = sysconf(_SC_SYMLOOP_MAX);
		else if(strcmp(argv[1],"SC_TIMER_MAX") == 0)
			variate_value = sysconf(_SC_TIMER_MAX);
		else if(strcmp(argv[1],"SC_TTY_NAME_MAX") == 0)
			variate_value = sysconf(_SC_TTY_NAME_MAX);
		else if(strcmp(argv[1],"SC_TZNAME_MAX") == 0)
			variate_value = sysconf(_SC_TZNAME_MAX);
		else
		{
			printf("Invaild Paramter\n");
			exit(0);
		}
	}
	else if(argv[1][0] == 'P')
	{
		if(argc != 3)
		{
			printf("Please Input Two Paramter\n");
			exit(0);
		}
		if(strcmp(argv[1],"PC_FILESIZEBITS") == 0)
			variate_value = pathconf(argv[2],_PC_FILESIZEBITS);
		else if(strcmp(argv[1],"PC_LINK_MAX") == 0)
			variate_value = pathconf(argv[2],_PC_LINK_MAX);
		else if(strcmp(argv[1],"PC_MAX_CANON") == 0)
			variate_value = pathconf(argv[2],_PC_MAX_CANON);
		else if(strcmp(argv[1],"PC_MAX_INPUT") == 0)
			variate_value = pathconf(argv[2],_PC_MAX_INPUT);
		else if(strcmp(argv[1],"PC_NAME_MAX") == 0)
			variate_value = pathconf(argv[2],_PC_NAME_MAX);
		else if(strcmp(argv[1],"PC_PATH_MAX") == 0)
			variate_value = pathconf(argv[2],_PC_PATH_MAX);
		else if(strcmp(argv[1],"PC_PIPE_BUF") == 0)
			variate_value = pathconf(argv[2],_PC_PIPE_BUF);
		//else if(strcmp(argv[1],"PC_TIMESTAMP_RESOLUTION") == 0)
		//	variate_value = pathconf(argv[2],_PC_TIMESTAMP_RESOLUTION);
		else if(strcmp(argv[1],"PC_SYMLINK_MAX") == 0)
			variate_value = pathconf(argv[2],_PC_SYMLINK_MAX);
		else
		{
			printf("Invaild Paramter\n");
			exit(0);
		}

	}
	else
	{
		printf("Please Input Correct Paramter\n");
		exit(0);
	}
	if(variate_value == -1)
	{
		if(errno != errno_l)
			perror("This Constant Is Incorrect");
		printf("%s is Numerical Uncertainty\n",argv[1]);
		exit(0);
	}
	printf("%s = %ld\n",argv[1],variate_value);
	exit(0);
}

