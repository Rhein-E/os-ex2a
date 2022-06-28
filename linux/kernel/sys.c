/*
 *  linux/kernel/sys.c
 *
 *  (C) 1991  Linus Torvalds
 */

#include <errno.h>

#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/kernel.h>
#include <asm/segment.h>
#include <sys/times.h>
#include <sys/utsname.h>
#include <fcntl.h>

int sys_ftime() { return -ENOSYS; }

int sys_break() { return -ENOSYS; }

int sys_ptrace() { return -ENOSYS; }

int sys_stty() { return -ENOSYS; }

int sys_gtty() { return -ENOSYS; }

int sys_rename() { return -ENOSYS; }

int sys_prof() { return -ENOSYS; }

int sys_setregid(int rgid, int egid) {
    if (rgid > 0) {
        if ((current->gid == rgid) || suser())
            current->gid = rgid;
        else
            return (-EPERM);
    }
    if (egid > 0) {
        if ((current->gid == egid) || (current->egid == egid) || suser()) {
            current->egid = egid;
            current->sgid = egid;
        } else
            return (-EPERM);
    }
    return 0;
}

int sys_setgid(int gid) {
    /*	return(sys_setregid(gid, gid)); */
    if (suser())
        current->gid = current->egid = current->sgid = gid;
    else if ((gid == current->gid) || (gid == current->sgid))
        current->egid = gid;
    else
        return -EPERM;
    return 0;
}

int sys_acct() { return -ENOSYS; }

int sys_phys() { return -ENOSYS; }

int sys_lock() { return -ENOSYS; }

int sys_mpx() { return -ENOSYS; }

int sys_ulimit() { return -ENOSYS; }

int sys_time(long *tloc) {
    int i;

    i = CURRENT_TIME;
    if (tloc) {
        verify_area(tloc, 4);
        put_fs_long(i, (unsigned long *)tloc);
    }
    return i;
}

/*
 * Unprivileged users may change the real user id to the effective uid
 * or vice versa.
 */
int sys_setreuid(int ruid, int euid) {
    int old_ruid = current->uid;

    if (ruid > 0) {
        if ((current->euid == ruid) || (old_ruid == ruid) || suser())
            current->uid = ruid;
        else
            return (-EPERM);
    }
    if (euid > 0) {
        if ((old_ruid == euid) || (current->euid == euid) || suser()) {
            current->euid = euid;
            current->suid = euid;
        } else {
            current->uid = old_ruid;
            return (-EPERM);
        }
    }
    return 0;
}

int sys_setuid(int uid) {
    /*	return(sys_setreuid(uid, uid)); */
    if (suser())
        current->uid = current->euid = current->suid = uid;
    else if ((uid == current->uid) || (uid == current->suid))
        current->euid = uid;
    else
        return -EPERM;
    return (0);
}

int sys_stime(long *tptr) {
    if (!suser())
        return -EPERM;
    startup_time = get_fs_long((unsigned long *)tptr) - jiffies / HZ;
    return 0;
}

int sys_times(struct tms *tbuf) {
    if (tbuf) {
        verify_area(tbuf, sizeof *tbuf);
        put_fs_long(current->utime, (unsigned long *)&tbuf->tms_utime);
        put_fs_long(current->stime, (unsigned long *)&tbuf->tms_stime);
        put_fs_long(current->cutime, (unsigned long *)&tbuf->tms_cutime);
        put_fs_long(current->cstime, (unsigned long *)&tbuf->tms_cstime);
    }
    return jiffies;
}

int sys_brk(unsigned long end_data_seg) {
    if (end_data_seg >= current->end_code && end_data_seg < current->start_stack - 16384)
        current->brk = end_data_seg;
    return current->brk;
}

/*
 * This needs some heave checking ...
 * I just haven't get the stomach for it. I also don't fully
 * understand sessions/pgrp etc. Let somebody who does explain it.
 */
int sys_setpgid(int pid, int pgid) {
    int i;

    if (!pid)
        pid = current->pid;
    if (!pgid)
        pgid = current->pid;
    for (i = 0; i < NR_TASKS; i++)
        if (task[i] && task[i]->pid == pid) {
            if (task[i]->leader)
                return -EPERM;
            if (task[i]->session != current->session)
                return -EPERM;
            task[i]->pgrp = pgid;
            return 0;
        }
    return -ESRCH;
}

int sys_getpgrp(void) { return current->pgrp; }

int sys_setsid(void) {
    if (current->leader && !suser())
        return -EPERM;
    current->leader = 1;
    current->session = current->pgrp = current->pid;
    current->tty = -1;
    return current->pgrp;
}

int sys_getgroups() { return -ENOSYS; }

int sys_setgroups() { return -ENOSYS; }

int sys_uname(struct utsname *name) {
    static struct utsname thisname = {"linux .0", "nodename", "release ", "version ", "machine "};
    int i;

    if (!name)
        return -ERROR;
    verify_area(name, sizeof *name);
    for (i = 0; i < sizeof *name; i++)
        put_fs_byte(((char *)&thisname)[i], i + (char *)name);
    return 0;
}

int sys_sethostname() { return -ENOSYS; }

int sys_getrlimit() { return -ENOSYS; }

int sys_setrlimit() { return -ENOSYS; }

int sys_getrusage() { return -ENOSYS; }

int sys_gettimeofday() { return -ENOSYS; }

int sys_settimeofday() { return -ENOSYS; }

int sys_umask(int mask) {
    int old = current->umask;

    current->umask = mask & 0777;
    return (old);
}

struct linux_dirent {
    long d_ino;
    off_t d_off;
    unsigned short d_reclen;
    char d_name[14];
};

int sys_getdents(unsigned int fd, struct linux_dirent *dirp, unsigned int count) {
    //  printk("sys_getdents\n");

    int nsize, ret, i;
    char buf[16 + 8];
    unsigned int oldfs;
    long ino, off;
    short reclen;

    ret = 0;
    while (count >= 16) {
        oldfs = get_fs();
        set_fs(get_ds());
        nsize = sys_read(fd, buf, 16);
        set_fs(oldfs);

        if (!nsize)
            break;
        count -= nsize;
        ret += nsize;
        buf[nsize] = '\0';
        ino = *(unsigned short *)buf;
        off = ret;
        reclen = sizeof(struct linux_dirent);

        put_fs_long(ino, &dirp->d_ino);
        put_fs_long(off, &dirp->d_off);
        put_fs_word(reclen, &dirp->d_reclen);
        for (i = 0; buf[i + 2]; ++i)
            put_fs_byte(buf[i + 2], dirp->d_name + i);
        put_fs_byte(0, dirp->d_name + i);

        ++dirp;
    }

    return ret;
}


int sys_sleep(unsigned int seconds)
{
	sys_signal(SIGALRM, SIG_IGN);
	sys_alarm(seconds);
	if (sys_pause() != -1)
		return 0;
	return -1;
}

char *sys_getcwd(char *buf, size_t size) {
    //  printk("sys_getcwd\n");

    if (!buf)
        return NULL;

    char fname[32], fbuf[32];
    int top, oldfs;
    unsigned long inum[32], izone[32];
    unsigned short idev;
    unsigned long fpos;

    oldfs = get_fs();
    set_fs(get_ds());
    top = 0;
    inum[top] = current->pwd->i_num;
    izone[top] = current->pwd->i_zone[0];
    idev = current->pwd->i_dev;
    while (1) {
        fpos = izone[top] * 1024 + 16;
        block_read(idev, &fpos, fbuf, 16); // entry of ".."
        inum[top + 1] = *(unsigned short *)fbuf;
        if (inum[top + 1] == inum[top]) // root
            break;
        fpos = 0x1400 + (inum[top + 1] - 1) * 32;
        block_read(idev, &fpos, fbuf, 32);               // inode of ".."
        izone[top + 1] = *(unsigned short *)(fbuf + 14); // i_zone[0] of ".."
        ++top;
    }
    set_fs(oldfs);

    char *pwrite = buf;
    if (!top) // root
        put_fs_byte('/', pwrite++);
    else
        while (top) {
            put_fs_byte('/', pwrite++);
            fpos = izone[top] * 1024 + 32;
            while (1) {
                oldfs = get_fs();
                set_fs(get_ds());
                if (!block_read(idev, &fpos, fbuf, 16)) { // end of block
                    set_fs(oldfs);
                    break;
                }
                set_fs(oldfs);

                if (*(unsigned short *)fbuf == inum[top - 1]) { // same inode number
                    char *pread = fbuf + 2;
                    while (*pread) {
                        put_fs_byte(*pread, pwrite++);
                        ++pread;
                    }
                    break;
                }
            }
            --top;
        }
    return buf;
}
