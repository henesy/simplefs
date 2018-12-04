#define Cmdwidth	1024
#define Ncmd		4
#define Nfiles		2

// Represents a 9p file
typedef struct File9 File9;
struct File9 {
	Ref;
	int		id;		// index in array; qid.path
	char	*name;	// of file
};

// All available files on the 9p fs
extern File9 *files[Nfiles];
