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
#include "mount.h"

extern bool susfs_is_current_ksu_domain(void);

/* Global State Symbols */
u32 susfs_ksu_sid = 0;
u32 susfs_kernel_sid = 0;
bool susfs_is_avc_log_spoofing_enabled = false;
bool susfs_hide_sus_mnts_for_all_procs = true;

/* Required by namei.c */
const struct qstr susfs_fake_qstr_name = QSTR_INIT("..5.u.S", 7);

#ifdef CONFIG_KSU_SUSFS_ENABLE_LOG
bool susfs_is_log_enabled __read_mostly = true;
#define SUSFS_LOGI(fmt, ...) if (susfs_is_log_enabled) pr_info("susfs:[%u][%d][%s] " fmt, current_uid().val, current->pid, __func__, ##__VA_ARGS__)
#define SUSFS_LOGE(fmt, ...) if (susfs_is_log_enabled) pr_err("susfs:[%u][%d][%s]" fmt, current_uid().val, current->pid, __func__, ##__VA_ARGS__)
#else
#define SUSFS_LOGI(fmt, ...) 
#define SUSFS_LOGE(fmt, ...) 
#endif

bool susfs_starts_with(const char *str, const char *prefix) {
    while (*prefix) {
        if (*str++ != *prefix++)
            return false;
    }
    return true;
}

/* sus_path */
#ifdef CONFIG_KSU_SUSFS_SUS_PATH
static LIST_HEAD(LH_SUS_PATH_LOOP);
static LIST_HEAD(LH_SUS_PATH_ANDROID_DATA);
static LIST_HEAD(LH_SUS_PATH_SDCARD);

bool susfs_is_base_dentry_android_data_dir(struct dentry* base) { (void)base; return false; }
bool susfs_is_base_dentry_sdcard_dir(struct dentry* base) { (void)base; return false; }

bool susfs_is_sus_android_data_d_name_found(const char *d_name) { (void)d_name; return false; }
bool susfs_is_sus_sdcard_d_name_found(const char *d_name) { (void)d_name; return false; }

void susfs_run_sus_path_loop(uid_t uid) { (void)uid; }

/* Inode hook required by namei.c */
bool susfs_is_inode_sus_path(struct inode *inode) { (void)inode; return false; }

void susfs_set_i_state_on_external_dir(void __user **user_info) { (void)user_info; }
void susfs_add_sus_path(void __user **user_info) { (void)user_info; }
void susfs_add_sus_path_loop(void __user **user_info) { (void)user_info; }
#endif

/* sus_mount */
#ifdef CONFIG_KSU_SUSFS_SUS_MOUNT
void susfs_set_hide_sus_mnts_for_all_procs(void __user **user_info) { (void)user_info; }
#endif

/* sus_kstat */
#ifdef CONFIG_KSU_SUSFS_SUS_KSTAT
void susfs_add_sus_kstat(void __user **user_info) { (void)user_info; }
void susfs_update_sus_kstat(void __user **user_info) { (void)user_info; }
void susfs_sus_ino_for_generic_fillattr(unsigned long ino, struct kstat *stat) { (void)ino; (void)stat; }
void susfs_sus_ino_for_show_map_vma(unsigned long ino, dev_t *out_dev, unsigned long *out_ino) { (void)ino; (void)out_dev; (void)out_ino; }
#endif

/* spoof_uname */
#ifdef CONFIG_KSU_SUSFS_SPOOF_UNAME
void susfs_set_uname(void __user **user_info) { (void)user_info; }
void susfs_spoof_uname(struct new_utsname* tmp) { (void)tmp; }
#endif

/* spoof_cmdline_or_bootconfig */
#ifdef CONFIG_KSU_SUSFS_SPOOF_CMDLINE_OR_BOOTCONFIG
void susfs_set_cmdline_or_bootconfig(void __user **user_info) { (void)user_info; }
int susfs_spoof_cmdline_or_bootconfig(struct seq_file *m) { (void)m; return 1; }
#endif

/* open_redirect */
#ifdef CONFIG_KSU_SUSFS_OPEN_REDIRECT
void susfs_add_open_redirect(void __user **user_info) { (void)user_info; }
struct filename* susfs_get_redirected_path(unsigned long ino) { (void)ino; return ERR_PTR(-ENOENT); }
#endif

/* sus_map */
#ifdef CONFIG_KSU_SUSFS_SUS_MAP
void susfs_add_sus_map(void __user **user_info) { (void)user_info; }
#endif

/* General supercalls functions */
void susfs_enable_log(void __user **user_info) { (void)user_info; }
void susfs_show_version(void __user **user_info) { (void)user_info; }
void susfs_show_variant(void __user **user_info) { (void)user_info; }
void susfs_get_enabled_features(void __user **user_info) { (void)user_info; }

void susfs_set_avc_log_spoofing(void __user **user_info) {
	struct st_susfs_avc_log_spoofing info;
	if (!copy_from_user(&info, (struct st_susfs_avc_log_spoofing __user*)*user_info, sizeof(info))) {
		susfs_is_avc_log_spoofing_enabled = info.enabled;
	}
}

void susfs_init(void) {
	SUSFS_LOGI("susfs initialized\n");
}
