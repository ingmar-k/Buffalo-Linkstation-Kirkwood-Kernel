#if !defined(_BUFFALO_PROC_ENTRY_H_)
#define _BUFFALO_PROC_ENTRY_H_

#include <linux/proc_fs.h>

struct proc_entry_data {
	char		path[64];
	mode_t		mode;
	read_proc_t*	readproc;
	write_proc_t*	writeproc;
	void*		data;
};

struct proc_dir_entry* get_proc_buffalo(void);
void make_proc_entry(struct proc_entry_data *entry);
struct proc_dir_entry* search_proc_entry(char *path);
void delete_proc_entry(char *path);

#endif
