#include <linux/string.h>
#include "buffalo_proc_entry.h"

struct proc_dir_entry *
get_proc_buffalo(void)
{
	static struct proc_dir_entry *buffalo = NULL;

	if (buffalo == NULL)
		buffalo = proc_mkdir("buffalo", NULL);

	return buffalo;
}

void make_proc_entry(struct proc_entry_data *entry)
{
	char *s, *p;
	struct proc_dir_entry *ent, *parent;

	parent = get_proc_buffalo();
	s = entry->path;
	while (1) {
		p = strchr(s, '/');
		if (p) {
			*p = '\0';
			parent = proc_mkdir(s, parent);
			*p = '/';
			s = p++;
		} else {
			ent = create_proc_entry(s, entry->mode, parent);
			break;
		}
	}
	if (ent) {
		ent->read_proc = entry->readproc;
		ent->write_proc = entry->writeproc;
		ent->data = entry->data;
	}
}

struct proc_dir_entry *
search_proc_entry(char *path)
{
	char *s, *p;
	struct proc_dir_entry **ent, *parent;

	parent = get_proc_buffalo();
	s = path;
	while (1) {
		p = strchr(s, '/');
		if (p)
			*p = '\0';
		for (ent = &parent->subdir; *ent; ent = &(*ent)->next) {
			if (strcmp((*ent)->name, s) == 0) {
				parent = *ent;
				break;
			}
		}
		if (p) {
			*p = '/';
			s = p++;
		}
		if (!*ent || !p) {
			break;
		}
	}

	return *ent;
}

static void __delete_proc_entry(struct proc_dir_entry* target)
{
	struct proc_dir_entry **ent;

	for (ent = &target->subdir; *ent; ) {
		if ((*ent)->subdir) {
			__delete_proc_entry(*ent);
		} else {
			remove_proc_entry((*ent)->name, (*ent)->parent);
		}
		
	}
}

void delete_proc_entry(char *path)
{
	struct proc_dir_entry *target;

	target = search_proc_entry(path);
	if (target)
		__delete_proc_entry(target);
}
