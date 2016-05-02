#ifndef LINUXSTAT_H
#define LINUXSTAT_H

/* shorted version of linux's stat.h file
 * relevant data here for determining permissions
 */

#define	S_IRWXU	0x1C0			/* RWX mask for owner */
#define	S_IRUSR	0x100			/* R for owner */
#define	S_IWUSR	0x80			/* W for owner */
#define	S_IXUSR	0x40			/* X for owner */

#define	S_IRWXG	0x38			/* RWX mask for group */
#define	S_IRGRP	0x20			/* R for group */
#define	S_IWGRP	0x10			/* W for group */
#define	S_IXGRP	0x8			/* X for group */

#define	S_IRWXO	0x7			/* RWX mask for other */
#define	S_IROTH	0x4			/* R for other */
#define	S_IWOTH	0x2			/* W for other */
#define	S_IXOTH	0x1			/* X for other */



#endif // LINUXSTAT_H
