#define DEF_NOFILE 256


/* @author      :yangkun
 * @description :get max number of file descriptor that a process can hold
 * @return      :max number of file descriptor
 * @time        :2018-7-10
 * */
unsigned long GetMX_NOFILE(void)
{
	struct rlimit rl;
	if(getrlimit(RLIMIT_NOFILE,&rl)<0)
		return DEF_NOFILE;
	else
		return rl.rlim_max;
}
