#include <StormLib.h>
#include <stdio.h>

int main(int argc, char*argv[]) {
	if(argc!=2) {
		printf("# args %d != expected args 2\n",argc); 
		return -1;
	}
	
	HANDLE archive = NULL;
	if(!SFileOpenArchive(argv[1],0,0,&archive)) {
		printf("failed opening archive %s with error %d\n",argv[1],GetLastError());
		return -1;
	}
	printf("opened %s\n",argv[1]);
	SFILE_FIND_DATA file_data;
	HANDLE search_handle = SFileFindFirstFile(archive, "*", &file_data, NULL);
	if(!search_handle) {
		printf("failed to find file in archive with error %d\n",GetLastError());
	}
	printf("found file %s\n",file_data.cFileName);
	while(SFileFindNextFile(search_handle,&file_data)) {
		printf("found file %s\n",file_data.cFileName);
	}
	printf("failed to find next file in archive with error %d\n", GetLastError());
	if(!SFileFindClose(search_handle)) {
		printf("failed to close search with error %d\n",GetLastError());
	}
}
