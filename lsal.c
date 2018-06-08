#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <unistd.h>

int maxSizeLen(char * directory);
int fileCount(char * directory);
int maxNameLen(char * directory);
int maxGroupLen(char * directory);
int maxNlinkLen(char * directory);

int MAX = 1024;

struct Table {
	char * nlink;
	char * user;
	char * group;
	char * size;
	char * permissions;
	char * date;
	char * fdate;
	char * name;
	char * rpath;
	int isLink;
};

void sortEntries(struct Table ** table, int entries);

int main(int argc, char ** argv) {
	struct Table ** table;
	int entry = 0;	
	if (!argv[1]){
		argv[1] = ".";
	}
	int entries=0;
	DIR * d;
	d = opendir(argv[1]);
	struct dirent * dir;
	struct stat st;
	if (d) {
		entries = fileCount(argv[1]);
		table = malloc(sizeof(struct Table *) * entries);
		//get max lengths for padding fields
		int maxSizeL = maxSizeLen(argv[1]);
		int maxNameL = maxNameLen(argv[1]);
		int maxGroupL = maxGroupLen(argv[1]);
		int maxNlinkL = maxNlinkLen(argv[1]);		

		int blocks = 0;

		
		while ((dir = readdir(d)) != NULL) {
			int isLink=0;
			table[entry] = malloc(sizeof(struct Table));
			table[entry]->nlink = malloc(sizeof(char) * maxNlinkL);
			table[entry]->user = malloc(sizeof(char) * MAX);
			table[entry]->group = malloc(sizeof(char) * MAX);
			table[entry]->size = malloc(sizeof(char) * MAX);
			table[entry]->permissions = malloc(sizeof(char) * 10);
			table[entry]->date = malloc(sizeof(char) * 20);
			table[entry]->fdate = malloc(sizeof(char) * 13);
			table[entry]->name = malloc(sizeof(char) * MAX);
			table[entry]->rpath = malloc(sizeof(char) * MAX);			

			char * filepath = malloc(sizeof(char) * MAX);
			sprintf(filepath, "%s/%s", argv[1], dir->d_name);			
			
			lstat(filepath, &st);

			blocks += st.st_blocks;
			int perm = st.st_mode;
			if (S_ISDIR(st.st_mode)) {
				strcpy(table[entry]->permissions, "d");
			}
			else if (S_ISREG(st.st_mode)) {
				strcpy(table[entry]->permissions, "-");
			}
			else if (S_ISLNK(st.st_mode)) {
				strcpy(table[entry]->permissions, "l");
				char * buf = malloc(sizeof(char)*MAX);
				realpath(filepath, buf);
				strcpy(table[entry]->rpath, " -> ");
				strcat(table[entry]->rpath, buf);
				free(buf);
				isLink=1;
			}
			//convert to binary
			int i = 8;
			long dec = perm+0.0;
			char * bin = malloc(sizeof(char) * 9);
			long quot = dec; 
			while (quot != 0) {
				bin[i--] = quot % 2 + '0';
				quot = quot / 2;
			}
			//concatenate permissions
			for (i=0; i < 9; i++) {
				if (bin[i] == '0') {
					strcat(table[entry]->permissions, "-");
				}
				else {
					switch(i%3) {
						case 0: strcat(table[entry]->permissions, "r");
							break;
						case 1: strcat(table[entry]->permissions, "w");
							break;
						case 2: strcat(table[entry]->permissions, "x");
							break;
					}
				}
			}

			//move strings into struct
			sprintf(table[entry]->nlink, "%d\0", st.st_nlink);
			if (getpwuid(st.st_uid) && getpwuid(st.st_uid)->pw_name != NULL) {
				strcpy(table[entry]->user, getpwuid(st.st_uid)->pw_name);
			}
			else {
				strcpy(table[entry]->user, "root");
			}
			if (getgrgid(st.st_gid) != NULL && getgrgid(st.st_gid)->gr_name != NULL) {
				strcpy(table[entry]->group, getgrgid(st.st_gid)->gr_name);
			}
			else {
				strcpy(table[entry]->group, "root");
			}
			sprintf(table[entry]->size, "%d\0", st.st_size);
			sprintf(table[entry]->date, "%s\0", ctime(&st.st_mtime));
			strncpy(table[entry]->fdate, (table[entry]->date)+4, 12);
			table[entry]->fdate[12]='\0';
			strcpy(table[entry]->name, dir->d_name);
			int last = strlen(dir->d_name);
			table[entry]->name[last]='\0';
			entry++;
		}
		sortEntries(table, entries);
		int i = 0;
		printf("total %d\n",blocks); 
		for (i=0; i < entries; i++) {
			if (table[i]->isLink == 0 || i==0 || i==1){
				printf("%-10s %*s %-*s %-*s %*s%*s %s\n",
								table[i]->permissions,
					maxNlinkL,		table[i]->nlink,
					maxNameL,		table[i]->user,
					maxGroupL, 		table[i]->group,
					maxSizeL,		table[i]->size,
					13, 			table[i]->fdate,
								table[i]->name);
			}
			else{
				
				printf("%-10s %*s %-*s %-*s %*s%*s %s%s\n",
								table[i]->permissions,
					maxNlinkL,		table[i]->nlink,
					maxNameL,		table[i]->user,
					maxGroupL, 		table[i]->group,
					maxSizeL,		table[i]->size,
					13, 			table[i]->fdate,
								table[i]->name,
								table[i]->rpath);
			}
		}
		closedir(d);	
	}
	else{
		entries=1;
		table = malloc(sizeof(struct Table *) * entries);
		table[0] = malloc(sizeof(struct Table));
		table[0]->nlink = malloc(sizeof(char) * MAX);
		table[0]->user = malloc(sizeof(char) * MAX);
		table[0]->group = malloc(sizeof(char) * MAX);
		table[0]->size = malloc(sizeof(char) * MAX);
		table[0]->permissions = malloc(sizeof(char) * 10);
		table[0]->date = malloc(sizeof(char) * 20);
		table[0]->fdate = malloc(sizeof(char) * 13);
		table[0]->name = malloc(sizeof(char) * MAX);
		table[0]->rpath = malloc(sizeof(char) * MAX);
		if (!(lstat(argv[1], &st))){	
			int perm = st.st_mode;
				if (S_ISDIR(st.st_mode)) {
					strcpy(table[0]->permissions, "d");
				}
				else if (S_ISREG(st.st_mode)) {
					strcpy(table[0]->permissions, "-");
				}
				else if (S_ISLNK(st.st_mode)) {
					strcpy(table[0]->permissions, "l");
					char * buf = malloc(sizeof(char)*MAX);
					realpath(argv[1], buf);
					strcpy(table[0]->rpath, " -> ");
					strcat(table[0]->rpath, buf);
					free(buf);
				}
				//convert to binary
				int i = 8;
				long dec = perm+0.0;
				char * bin = malloc(sizeof(char) * 9);
				long quot = dec; 
				while (quot != 0) {
					bin[i--] = quot % 2 + '0';
					quot = quot / 2;
				}
				//concatenate permissions
				for (i=0; i < 9; i++) {
					if (bin[i] == '0') {
						strcat(table[0]->permissions, "-");
					}
					else {
						switch(i%3) {
							case 0: strcat(table[0]->permissions, "r");
								break;
							case 1: strcat(table[0]->permissions, "w");
								break;
							case 2: strcat(table[0]->permissions, "x");
								break;
						}
					}
				}
	
				//move strings into struct
				sprintf(table[entry]->nlink, "%d\0", st.st_nlink);
				if (getpwuid(st.st_uid) && getpwuid(st.st_uid)->pw_name != NULL) {
					strcpy(table[0]->user, getpwuid(st.st_uid)->pw_name);
				}
				else {
					strcpy(table[entry]->user, "root");
				}
				if (getgrgid(st.st_gid) != NULL && getgrgid(st.st_gid)->gr_name != NULL) {
					strcpy(table[0]->group, getgrgid(st.st_gid)->gr_name);
				}
				else {
					strcpy(table[0]->group, "root");
				}
				sprintf(table[0]->size, "%d\0", st.st_size);
				sprintf(table[0]->date, "%s\0", ctime(&st.st_mtime));
				strncpy(table[0]->fdate, (table[0]->date)+4, 12);
				table[0]->fdate[12]='\0';
				char * temp = malloc(sizeof(char)*MAX);
				strcpy(temp, argv[1]);
				while(strchr(temp, '/')){
					temp++;
				}	
				strcpy(table[0]->name, temp);
				printf("%-10s %*s %-*s %-*s %*s%*s %s%s\n",
									table[0]->permissions,
					strlen(table[0]->nlink),	table[0]->nlink,
					strlen(table[0]->user),		table[0]->user,
					strlen(table[0]->group),	table[0]->group,
					strlen(table[0]->size),		table[0]->size,
					13, 				table[0]->fdate,
									table[0]->name,
									table[0]->rpath);		
		}
		else {
			fprintf(stderr, "%s: canot access \'%s\': No such file or directory\n", argv[0], argv[1]);
			exit(1);
		}	
	}
	fflush(stdout);
	return 0;
}

void sortEntries(struct Table ** table, int entries){
	int i = 0;
	int j = 0;
	for (i = 0; i < entries; i++) {
		if (strcmp(table[i]->name, ".") == 0) {
			struct Table * temp = malloc(sizeof(struct Table *));
			temp = table[i];
			table[i] = table[0];
			table[0] = temp;
		}
		if (strcmp(table[i]->name, "..") == 0) {
			struct Table * temp = malloc(sizeof(struct Table *));
			temp = table[i];
			table[i] = table[1];
			table[1] = temp;
		}

	}
	if (entries > 2) {
		for (i = 2; i < entries; i++) {
			for (j = i+1; j < entries; j++){
				char a = table[i]->name[0];
				char b = table[j]->name[0];
				char * left = malloc(sizeof(table[i]->name));
				char * right = malloc(sizeof(table[j]->name));
				strcpy(left, table[i]->name);
				strcpy(right, table[j]->name);
				if (a == '.') {
					strcpy(left, left+1);
				}		
				if (b == '.') {
					strcpy(right, right+1);
				}
				if (strcmp(left, right) > 0) {
					struct Table * temp = malloc(sizeof(struct Table *));
					temp = table[i];
					table[i] = table[j];
					table[j] = temp;
				}
			}
		}
	}
}

int maxNlinkLen(char * directory) {
	DIR * d;
	struct dirent * dir;
	d = opendir(directory);
	int max = 0;
	struct stat st;
	while ((dir = readdir(d)) != NULL) {	
		char * filepath = malloc(sizeof(char) * MAX);
			sprintf(filepath, "%s/%s", directory, dir->d_name);			
			
			lstat(filepath, &st);
		int nlink = st.st_nlink;
		if (nlink > max) max = nlink;
	}
	char * maxNlink = malloc(sizeof(char));
	sprintf(maxNlink, "%d\0", max);
	closedir(d);
	return strlen(maxNlink);
}

int maxSizeLen(char * directory) {
	DIR * d;
	struct dirent * dir;
	d = opendir(directory);
	int max = 0;
	while ((dir = readdir(d)) != NULL) {
		struct stat st;
		char * filepath = malloc(sizeof(char) * MAX);
			sprintf(filepath, "%s/%s", directory, dir->d_name);			
			
			lstat(filepath, &st);
		if (st.st_size > max) max = st.st_size;
	}
	char * maxSize = malloc(sizeof(char));
	sprintf(maxSize, "%d\0", max);
	closedir(d);
	return strlen(maxSize);
}

int fileCount(char * directory) {
	DIR * d;
	struct dirent * dir;
	d = opendir(directory);
	int count = 0;
	while((dir = readdir(d)) != NULL) {
		count++;
	}
	closedir(d);
	return count;
}

int maxNameLen(char * directory) {
	DIR * d;
	struct dirent * dir;
	d = opendir(directory);
	int max = 0;
	while((dir = readdir(d)) != NULL) {
		struct stat st;
		char * filepath = malloc(sizeof(char) * MAX);
			sprintf(filepath, "%s/%s", directory, dir->d_name);			
			
			lstat(filepath, &st);
		if (getpwuid(st.st_uid)->pw_name != NULL) {
			if (strlen(getpwuid(st.st_uid)->pw_name) > max)
				max=strlen(getpwuid(st.st_uid)->pw_name);
		}
		else if (4 > max) {
			max = 4;
		}
	}
	closedir(d);
	return max;
}

int maxGroupLen(char * directory) {
	DIR * d;
	struct dirent * dir;
	d = opendir(directory);
	int max = 0;
	while((dir = readdir(d)) != NULL) {
		struct stat st;
		char * filepath = malloc(sizeof(char) * MAX);
			sprintf(filepath, "%s/%s", directory, dir->d_name);			
			
			lstat(filepath, &st);
		if (getgrgid(st.st_gid) != NULL && getgrgid(st.st_gid)->gr_name != NULL) {
			if (strlen(getgrgid(st.st_gid)->gr_name) > max)
				max = strlen(getgrgid(st.st_gid)->gr_name);
		}
		else if (4 > max) {
			max = 4;
		}
	}
	closedir(d);
	return max;
}