#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <errno.h>

/* @author      :yangkun
 * @description :make current process to be a daemon process
 * @return      :void
 * @time        :2018-7-10
 * */
void daemonize(const char *cmd)
{
	int i,fd0,fd1,fd2;
	pid_t pid;
	struct rlimit rl;
	struct sigaction sa;

	/*1-文件创建掩码清除*/
	umask(0);

	/*2-获取当前最大文件描述符数量*/
	if(getrlimit(RLIMIT_NOFILE,&rl)<0)
		{
			perror("can not get file limit.");
			exit(-1);
		}
	
	/*3-为了和当前终端tty脱离关系，把自己设置成为一个新会话的会话首进程，一个tty对应一个会话，
 	 *  因此这里设置会话就相当于和之前的会话脱离，即和tty脱离 
 	 *  下面这个fork的意义:只有非进程组组长才能成为会话首进程，这里fork一次以使子进程不可能成为
 	 *  进程组组长，为后面的setsid做准备
	 */
	pid=fork();
	if(pid<0)
		{
			perror("can not fork at setsid.");
			exit(-1);
		}
	if(pid>0)	
		{
			exit(0);	/*父进程退出*/
		}
	setsid();		/*子进程设置自己为新会话的会话首进程，这个动作有如下3个隐藏含义：
				*  1）成为会话首进程；2）成为进程组组长；3）脱离控制终端，不再被控制
				*/

	/*4-重设SIGHUP动作*/
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags=0;
	if(sigaction(SIGHUP,&sa,NULL)<0)
		{
			perror("can not ignore SIGHUP.");
			exit(-1);
		}
	if((pid=fork())<0)
		{
			perror("can not fork at ignore SIGHUP.");
			exit(-1);
		}
	else if(pid!=0)
		{
			exit(0);
		}


	/*5-切换当前工作目录为根目录，这样可以防止某个文件系统无法被卸载
 	 *  解释：从父进程继承过来的当前工作目录可能挂在某个文件系统上，又因为守护进程一般都是长时间运
 	 *  行，如果我想提前卸载某个文件系统，但是守护进程还在运行，这个时候就会提示无法卸载。	
 	 * */
	if(chdir("/")<0)
		{
			perror("can noe change directory to /.");
			exit(-1);
		}


	/*6-关闭所有已经打开的文件描述符*/
	if(rl.rlim_max == RLIM_INFINITY)
		rl.rlim_max = 1024;
	for(i=0;i<rl.rlim_max;i++)
		close(i);


	/*7-让0,1,2都指向/dev/null文件，用来丢弃所有试图通过打开这三个文件描述符而与进程通讯的动作*/
	fd0=open("/dev/null",O_RDWR);
	fd1=dup(0);
	fd2=dup(0);


	/*8-设置日志输出到系统日志中————>/var/log/messages*/
	openlog(cmd,LOG_CONS,LOG_DAEMON);
	if(fd0!=0||fd1!=1||fd2!=2){
		syslog(LOG_ERR,"unex[ected file descriptors %d %d %d",fd0,fd1,fd2);
		exit(-1);
	}
}
