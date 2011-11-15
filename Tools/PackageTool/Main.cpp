#include "Core.h"
#include "UnrealClasses.h"
#include "UnPackage.h"

/*-----------------------------------------------------------------------------
	Main function
-----------------------------------------------------------------------------*/

struct FileInfo
{
	int		Ver;
	int		LicVer;
	int		Count;
	char	FileName[512];
};

static int InfoCmp(const FileInfo *p1, const FileInfo *p2)
{
	int dif = p1->Ver - p2->Ver;
	if (dif) return dif;
	return p1->LicVer - p2->LicVer;
}

TArray<FileInfo> PkgInfo;

static bool ScanPackage(const CGameFileInfo *file)
{
	guard(ScanPackage);

	FArchive *Ar = appCreateFileReader(file);

	unsigned Tag, FileVer;
	*Ar << Tag;
	if (Tag == PACKAGE_FILE_TAG_REV)
		Ar->ReverseBytes = true;
	else if (Tag != PACKAGE_FILE_TAG)	//?? possibly Lineage2 file etc
		return true;
	*Ar << FileVer;

	FileInfo Info;
	Info.Ver    = FileVer & 0xFFFF;
	Info.LicVer = FileVer >> 16;
	Info.Count  = 0;
	strcpy(Info.FileName, file->RelativeName);
//	printf("%s - %d/%d\n", file->RelativeName, Info.Ver, Info.LicVer);
	int Index = INDEX_NONE;
	for (int i = 0; i < PkgInfo.Num(); i++)
	{
		FileInfo &Info2 = PkgInfo[i];
		if (Info2.Ver == Info.Ver && Info2.LicVer == Info.LicVer)
		{
			Index = i;
			break;
		}
	}
	if (Index == INDEX_NONE)
		Index = PkgInfo.AddItem(Info);
	// update info
	PkgInfo[Index].Count++;
	// combine filename
	char *s = PkgInfo[Index].FileName;
	char *d = Info.FileName;
	while (*s == *d && *s != 0)
	{
		s++;
		d++;
	}
	*s = 0;

	delete Ar;
	return true;

	unguardf(("%s", file->RelativeName));
}


int main(int argc, char **argv)
{
#if DO_GUARD
	TRY {
#endif

	guard(Main);

	// display usage
	if (argc < 2)
	{
	help:
		printf(	"Unreal package scanner\n"
				"http://www.gildor.org/\n"
				"Usage: pkgtool <path to game files>\n"
		);
		exit(0);
	}

	// parse command line
//	bool dump = false, view = true, exprt = false, listOnly = false, noAnim = false, pkgInfo = false;
	int arg = 1;
/*	for (arg = 1; arg < argc; arg++)
	{
		if (argv[arg][0] == '-')
		{
			const char *opt = argv[arg]+1;
			if (!stricmp(opt, "dump"))
			{
			}
			else if (!stricmp(opt, "check"))
			{
			}
			else
				goto help;
		}
		else
		{
			break;
		}
	} */
	const char *argPkgDir = argv[arg];
	if (!argPkgDir) goto help;

	appSetRootDirectory(argPkgDir);

	// scan packages
	appEnumGameFiles(ScanPackage);

	PkgInfo.Sort(InfoCmp);
	printf("Version summary:\n"
		   "%-9s  %-9s  %s   %s\n", "Ver", "LicVer", "Count", "Filename");
	for (int i = 0; i < PkgInfo.Num(); i++)
	{
		const FileInfo &Info = PkgInfo[i];
		printf("%3d (%3X)  %3d (%3X)  %4d    %s%s\n",
			Info.Ver, Info.Ver, Info.LicVer, Info.LicVer, Info.Count, Info.FileName,
			Info.Count > 1 && Info.FileName[0] ? "..." : "");
	}

	unguard;

#if DO_GUARD
	} CATCH {
		if (GErrorHistory[0])
		{
//			printf("ERROR: %s\n", GErrorHistory);
			appNotify("ERROR: %s\n", GErrorHistory);
		}
		else
		{
//			printf("Unknown error\n");
			appNotify("Unknown error\n");
		}
		exit(1);
	}
#endif
	return 0;
}