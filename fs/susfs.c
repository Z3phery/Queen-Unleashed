#include <linux/version.h>
#include <linux/cred.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
#include <linux/printk.h>
#include <linux/namei.h>
#include <linux/list.h>
#include <linux/init_task.h>
#include <linux/spinlock.h>
#include <linux/stat.h>
#include <linux/uaccess.h>
#include <linux/fdtable.h>
#include <linux/statfs.h>
#include <linux/random.h>
#include <linux/susfs.h>
#include <linux/export.h>
#include "mount.h"

extern bool susfs_is_current_ksu_domain(void);

/* Global State Symbols */
u32 susfs_ksu_sid = 0;
u32 susfs_kernel_sid = 0;
bool susfs_is_avc_log_spoofing_enabled = false;
bool susfs_hide_sus_mnts_for_all_procs = true;
bool susfs_is_log_enabled = true;

const struct qstr susfs_fake_qstr_name = QSTR_INIT("..5.u.S", 7);

EXPORT_SYMBOL_GPL(susfs_ksu_sid);

/* --- Core Utility Functions --- */

bool susfs_starts_with(const char *str, const char *prefix) {
    while (*prefix) {
        if (*str++ != *prefix++)
            return false;
    }
    return true;
}
EXPORT_SYMBOL_GPL(susfs_starts_with);

/* --- Path & Inode Hooks --- */

bool susfs_is_inode_sus_path(struct inode *inode) { 
    (void)inode; 
    return false; 
}
EXPORT_SYMBOL_GPL(susfs_is_inode_sus_path);

void susfs_add_sus_path_loop(void __user **user_info) { 
    (void)user_info; 
}
EXPORT_SYMBOL_GPL(susfs_add_sus_path_loop);

void susfs_set_i_state_on_external_dir(void __user **user_info) { 
    (void)user_info; 
}
EXPORT_SYMBOL_GPL(susfs_set_i_state_on_external_dir);

void susfs_add_sus_path(void __user **user_info) { 
    (void)user_info; 
}
EXPORT_SYMBOL_GPL(susfs_add_sus_path);

void susfs_run_sus_path_loop(uid_t uid) { 
    (void)uid; 
}
EXPORT_SYMBOL_GPL(susfs_run_sus_path_loop);

/* --- Dentry Hooks --- */

bool susfs_is_base_dentry_android_data_dir(struct dentry* base) { return false; }
EXPORT_SYMBOL_GPL(susfs_is_base_dentry_android_data_dir);

bool susfs_is_sus_android_data_d_name_found(const char *d_name) { return false; }
EXPORT_SYMBOL_GPL(susfs_is_sus_android_data_d_name_found);

bool susfs_is_base_dentry_sdcard_dir(struct dentry* base) { return false; }
EXPORT_SYMBOL_GPL(susfs_is_base_dentry_sdcard_dir);

bool susfs_is_sus_sdcard_d_name_found(const char *d_name) { return false; }
EXPORT_SYMBOL_GPL(susfs_is_sus_sdcard_d_name_found);

/* --- Spoofing & Redirection --- */

void susfs_spoof_uname(struct new_utsname* tmp) { (void)tmp; }
EXPORT_SYMBOL_GPL(susfs_spoof_uname);

int susfs_spoof_cmdline_or_bootconfig(struct seq_file *m) { return 1; }
EXPORT_SYMBOL_GPL(susfs_spoof_cmdline_or_bootconfig);

struct filename* susfs_get_redirected_path(unsigned long ino) { 
    (void)ino; 
    return ERR_PTR(-ENOENT); 
}
EXPORT_SYMBOL_GPL(susfs_get_redirected_path);

void susfs_sus_ino_for_generic_fillattr(unsigned long ino, struct kstat *stat) { 
    (void)ino; 
    (void)stat; 
}
EXPORT_SYMBOL_GPL(susfs_sus_ino_for_generic_fillattr);

void susfs_sus_ino_for_show_map_vma(unsigned long ino, dev_t *out_dev, unsigned long *out_ino) { 
    (void)ino; 
    (void)out_dev; 
    (void)out_ino; 
}
EXPORT_SYMBOL_GPL(susfs_sus_ino_for_show_map_vma);

/* --- IOC / Supercalls --- */

void susfs_enable_log(void __user **user_info) { (void)user_info; }
EXPORT_SYMBOL_GPL(susfs_enable_log);

void susfs_set_avc_log_spoofing(void __user **user_info) { (void)user_info; }
EXPORT_SYMBOL_GPL(susfs_set_avc_log_spoofing);

void susfs_update_sus_kstat(void __user **user_info) { (void)user_info; }
EXPORT_SYMBOL_GPL(susfs_update_sus_kstat);

void susfs_add_sus_kstat(void __user **user_info) { (void)user_info; }
EXPORT_SYMBOL_GPL(susfs_add_sus_kstat);

void susfs_add_open_redirect(void __user **user_info) { (void)user_info; }
EXPORT_SYMBOL_GPL(susfs_add_open_redirect);

void susfs_get_enabled_features(void __user **user_info) { (void)user_info; }
EXPORT_SYMBOL_GPL(susfs_get_enabled_features);

void susfs_set_uname(void __user **user_info) { (void)user_info; }
EXPORT_SYMBOL_GPL(susfs_set_uname);

void susfs_show_version(void __user **user_info) { (void)user_info; }
EXPORT_SYMBOL_GPL(susfs_show_version);

void susfs_show_variant(void __user **user_info) { (void)user_info; }
EXPORT_SYMBOL_GPL(susfs_show_variant);

void susfs_add_sus_map(void __user **user_info) { (void)user_info; }
EXPORT_SYMBOL_GPL(susfs_add_sus_map);

void susfs_set_cmdline_or_bootconfig(void __user **user_info) { (void)user_info; }
EXPORT_SYMBOL_GPL(susfs_set_cmdline_or_bootconfig);

void susfs_add_try_umount(void __user *arg) { (void)arg; }
EXPORT_SYMBOL_GPL(susfs_add_try_umount);

void susfs_try_umount(void) { }
EXPORT_SYMBOL_GPL(susfs_try_umount);

void susfs_set_hide_sus_mnts_for_all_procs(void __user **user_info) { (void)user_info; }
EXPORT_SYMBOL_GPL(susfs_set_hide_sus_mnts_for_all_procs);

/* --- Initialization --- */

void susfs_init(void) {
    pr_info("susfs: initialized\n");
}
EXPORT_SYMBOL_GPL(susfs_init);
