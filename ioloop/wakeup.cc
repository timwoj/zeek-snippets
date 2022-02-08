#include <stdio.h>
#include <unistd.h>
#include <uv.h>

uv_check_t check;
uv_prepare_t prepare;

void check_hdlr(uv_check_t* handle)
	{
	printf("check\n");
	}

void prepare_hdlr(uv_prepare_t* handle)
	{
	printf("prepare\n");
	}

void sig_handler(int signo)
	{
	printf("signal\n");
	uv_prepare_init(uv_default_loop(), &prepare);
	uv_prepare_start(&prepare, prepare_hdlr);
	}

int main(int argc, char** argv)
	{
	(void) signal(SIGTSTP, sig_handler);
	
	uv_check_init(uv_default_loop(), &check);
	uv_check_start(&check, check_hdlr);
	
	uv_run(uv_default_loop(), UV_RUN_DEFAULT);

	return 0;
	}
