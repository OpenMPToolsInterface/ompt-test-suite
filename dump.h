#include <stdlib.h>

#include <ompt.h>

void dobreak()
{
}

ompt_get_task_frame_t ompt_get_task_frame;
ompt_get_parallel_id_t ompt_get_parallel_id;
ompt_get_task_id_t ompt_get_task_id;

void dump_frames(int rank, int level)
{
	char *env = getenv("OMP_RANK");
	int myrank = (env && atoi(env) == rank);

	if (!env || myrank) {
		int i = 0;
		while(1) {
			if (myrank) dobreak();
			ompt_frame_t *frame = ompt_get_task_frame(i);
			uint64_t id = ompt_get_parallel_id(i);
			if (frame == 0) break;
			printf("%d(%d): parallel id = %llx frame = %p exit = %p reenter = %p\n",
					rank, level, id, frame,
					frame ? frame->exit_runtime_frame : 0,
					frame ? frame->reenter_runtime_frame : 0);
			i++;
		}
	}
}


void dump_tasks(int rank, int level)
{
	char *env = getenv("OMP_RANK");
        int myrank = (env && atoi(env) == rank);

	if (!env || myrank) {
		int i = 0;
		while(1) {
			if (myrank) dobreak();
			ompt_frame_t *frame = ompt_get_task_frame(i);
			uint64_t id = ompt_get_task_id(i);
			if (frame == 0) break;
			printf("%d(%d): task id = %llx frame = %p exit = %p reenter = %p\n",
					rank, level, id, frame,
					frame ? frame->exit_runtime_frame : 0,
					frame ? frame->reenter_runtime_frame : 0);
			i++;
		}
	}
}

