#include <u.h>
#include <libc.h>
#include <auth.h>
#include <fcall.h>
#include <thread.h>
#include <9p.h>
#include "fs.h"

File9 *files[Nfiles];

// Prototypes for 9p handler functions
static void		fsattach(Req *r);
static int		getdirent(int n, Dir *d, void *);
static void		fsread(Req *r);
static void		fswrite(Req *r);
static char*	fswalkl(Fid * fid, char *name, Qid *qid);
static char*	fsclone(Fid *fid, Fid *newfid);
static void		fsstat(Req *r);

// Srv structure to handle incoming 9p communications
static Srv srvfs = 
{
	.attach		=	fsattach,
	.read		=	fsread,
	.write		=	fswrite,
	.walk1		=	fswalkl,
	.stat		=	fsstat,
};


// Usage output
void
usage(void)
{
	fprint(2, "usage: %s [-D] [-s srv] [-m mnt]\n", argv0);
	threadexitsall("usage");
}


/* A simple 9p fileserver to show a minimal set of operations */
void
threadmain(int argc, char *argv[])
{
	char	*mnt, *srv;

	srv = nil;
	mnt = "/mnt/simplefs";

	ARGBEGIN {
	case 'D':
		chatty9p++;
		break;
	case 's':
		srv = EARGF(usage());
		break;
	case 'm':
		mnt = EARGF(usage());
		break;
	default:
		usage();
	} ARGEND;

	if(argc != 0)
		usage();

	// Setup ctl file
	File9 ctl = (File9) { (Ref){ 0 }, 0, "ctl" };
	files[0] = &ctl;
	
	// Setup log file
	File9 log = (File9) { (Ref){ 0 }, 1, "log" };
	files[1] = &log;

	threadpostmountsrv(&srvfs, srv, mnt, MREPL|MCREATE);
	threadexits(nil);
}


// Handle 9p attach
static void
fsattach(Req *r)
{
	r->fid->qid = (Qid) { 0, 0, QTDIR };
	r->ofcall.qid = r->fid->qid;
	respond(r, nil);
}

// Get directory entries for stat and such
static int
getdirent(int n, Dir *d, void *)
{
	d->atime = time(nil);
	d->mtime = d->atime;
	d->uid = estrdup9p(getuser());
	d->gid = estrdup9p(d->uid);
	d->muid = estrdup9p(d->uid);
	if(n == -1){
		d->qid = (Qid) {0, 0, QTDIR};
		d->mode = 0775;
		d->name = estrdup9p("/");
		d->length = 0;
	}else if(n >= 0 && n < Nfiles && files[n] != nil){
		d->qid = (Qid) {n, 0, 0};
		d->mode = 0664;
		d->name = estrdup9p(files[n]->name);
	}else
		return -1;
	return 0;
}

// Handle 9p read
static void
fsread(Req *r)
{
	Fid		*fid;
	Qid		q;
	char	readmsg[] = "Nothing special in this read.\n";

	fid = r->fid;
	q = fid->qid;
	if(q.type & QTDIR){
		dirread9p(r, getdirent, nil);
		respond(r, nil);
		return;
	}

	switch(q.path){
	case 0:
		// ctl file
		strcpy(readmsg, "ctl file is unreadable.\n");
		break;
	case 1:
		// log file
		// TODO -- attach stdout to write to the read stream
		strcpy(readmsg, "log shows prior commands.\n");
		break;
	}
	
	// Set the read reply string
	readstr(r, readmsg);
	
	// Respond to the 9p request
	respond(r, nil);
}

// Handle 9p write
static void
fswrite(Req *r)
{
	Fid		*fid;
	Qid		q;
	char	str[10];

	fid = r->fid;
	q = fid->qid;
	if(q.type & QTDIR){
		respond(r, "permission denied.");
		return;
	}
	if(r->ifcall.count > sizeof(str) - 1){
		respond(r, "string too large");
		return;
	}
	memmove(str, r->ifcall.data, r->ifcall.count);
	str[r->ifcall.count] = 0;
	
	print("%s", str);

	respond(r, nil);
}

// Handle 9p walk
static char *
fswalkl(Fid * fid, char *name, Qid *qid)
{
	Qid q;
	int i;

	q = fid->qid;
	if(!(q.type && QTDIR)){
		if(!strcmp(name, "..")){
			fid->qid = (Qid) {0, 0, QTDIR};
			*qid = fid->qid;
			fid->aux = nil;
			return nil;
		}
	}else{
		for(i = 0; i < Nfiles; i++)
			if(files[i] && !strcmp(name, files[i]->name)){
				fid->qid = (Qid){i, 0, 0};
				incref(files[i]);
				fid->aux = files[i];
				*qid = fid->qid;
				return nil;
			}
	}
	return "no such directory.";
}

// Handle 9p stat
static void
fsstat(Req *r)
{
	Fid *fid;
	Qid q;

	fid = r->fid;
	q = fid->qid;
	if(q.type & QTDIR)
		getdirent(-1, &r->d, nil);
	else
		getdirent(q.path, &r->d, nil);
	respond(r, nil);
}
