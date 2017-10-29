#ifndef __SMALLFS_H__
#define __SMALLFS_H__

#include "flash.h"

// STM defs
typedef int bool;
#define false 0
#define true 1
#define NULL ((void*)0)

/** SmallFS filesystem magic */
#define SMALLFS_MAGIC 0x50411F50

#ifndef SEEK_SET
# define SEEK_SET   0
# define SEEK_CUR  1
# define SEEK_END  2
#endif

struct smallfs_header {
	unsigned int magic /** big-endian, magic number **/;
	unsigned int numfiles;
}__attribute__((packed));

struct smallfs_entry {
	unsigned int offset;
	unsigned int size;
	unsigned char namesize;
	char name[0];
} __attribute__((packed));

struct smallfs {
    struct smallfs_header hdr;
    unsigned fsstart;
    unsigned offset;
    flash_control_t fc;
#ifdef __linux__
    int fd;
#endif
};

struct smallfsentry {
    unsigned m_offset;
    unsigned m_index;
    struct smallfs *fs;
};

struct smallfsfile {
    int flashoffset;
    int filesize;
    int seekpos;
    struct smallfs *fs;
};

static inline void smallfsfile__initempty(struct smallfsfile *f)
{
    f->flashoffset=-1;
    f->seekpos=0;
    f->fs = NULL;
}

static inline void smallfsfile__init(struct smallfsfile *f, struct smallfs *fs,unsigned o,unsigned size)
{
    f->flashoffset = o;
    f->filesize = size;
    f->seekpos = 0;
    f->fs = fs;
}

static inline bool smallfsfile__valid(struct smallfsfile*f) {
    return f->flashoffset>=0;
}

int smallfsfile__read(struct smallfsfile*f,void *buf, int size);

void smallfsfile__seek(struct smallfsfile*f,int pos, int whence);

static inline int smallfsfile__size(const struct smallfsfile*f) {
    return f->filesize;
}

int smallfsfile__readCallback(struct smallfsfile*f,int size, void (*callback)(unsigned char, void*), void *data);

uint8_t smallfsfile__readByte(struct smallfsfile*f);

static inline int smallfsfile__getOffset(const struct smallfsfile*f) {
    return f->flashoffset;
}

static inline int smallfsfile__getSize(const struct smallfsfile*f)
{
    return f->filesize;
}

static inline void smallfsentry__init(struct smallfsentry *e, struct smallfs *fs)
{
    e->m_offset=0;
    e->fs = fs;
}

static inline bool smallfsentry__valid(const struct smallfsentry *e)
{
    return e->m_offset>0;
}

bool smallfsentry__hasNext(const struct smallfsentry *e);
bool smallfsentry__equals(const struct smallfsentry *e,const char *name);
bool smallfsentry__endsWith(const struct smallfsentry *e,const char *end);
bool smallfsentry__startsWith(const struct smallfsentry *e,const char *end);

void smallfsentry__getName(const struct smallfsentry *e,char *dest);

int smallfsentry__open(struct smallfsentry *e, struct smallfsfile *f);
void smallfsentry__advance(struct smallfsentry *e,int);
void smallfsentry__back(struct smallfsentry *e);

static inline void smallfsentry__initadv(struct smallfsentry *e, unsigned offset, unsigned idx) {
    e->m_offset=offset;
    e->m_index=idx;
}

int smallfs__begin(struct smallfs *fs, unsigned offset);
void smallfs__end(struct smallfs *fs);

static inline unsigned int smallfs__getCount(const struct smallfs *fs)
{
    return fs->hdr.numfiles;
}
static inline unsigned int smallfs__getFSStart(const struct smallfs *fs)
{
    return fs->fsstart;
}

void smallfs__read(struct smallfs *fs, unsigned address, void *target, unsigned size);
uint8_t smallfs__readByte(struct smallfs *fs, unsigned address);
int smallfs__open(struct smallfs *fs, struct smallfsfile *f, const char *name);
int smallfs__openByOffset(struct smallfs *fs, struct smallfsfile *f, unsigned offset);
int smallfs__getFirstEntry(struct smallfs *fs, struct smallfsentry *e);

void smallfs__seek(struct smallfs *fs, unsigned address);

struct smallfs *smallfs__getfs();
int smallfs__start();

#endif
