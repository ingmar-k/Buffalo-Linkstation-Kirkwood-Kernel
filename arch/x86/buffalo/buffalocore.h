#if !defined(_BUFFALOCORE_H_)
#define _BUFFALOCORE_H_

#if defined(CONFIG_ARCH_FEROCEON_KW)
struct bf_proc_entry_tab {
  const char *name;
  int isDirectory;
  read_proc_t *read_proc;
  write_proc_t *write_proc;
  void *data;
  MV_BOARD_GPP_CLASS class;
};
#endif

struct proc_dir_entry *get_proc_buffalo(void);

#endif
