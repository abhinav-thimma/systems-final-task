#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "MemoryAlloc.h"
#include "Structure.h"
#define FILE_NAME "abc.bin"


 /*--------------------------------------------BLOB FILE--------------------------------------------------*/

FILE* createBlobFile(char* filename)
{
	FILE* fp = fopen(filename, "wb");
	fseek(fp, 1024*1024*100 - 1, SEEK_SET);

	fwrite("", 1, sizeof(char), fp);
	fclose(fp);
	return fp;
}







/*---------------------------------------------GLOBAL DATA------------------------------------------------*/


void storeGlobalsToFile()
{
	FILE* fp = fopen(FILE_NAME, "rb+");
	fwrite(&noOfUsers, sizeof(int), 1, fp);
	fwrite(&postsStartingLocation, sizeof(int), 1, fp);
	fclose(fp);
}

//this function reads the user count and start position of posts and plaaces in globals
void fetchGlobalsFromFile()
{
	FILE* fp = fopen(FILE_NAME,"rb+");
	fread(&noOfUsers,sizeof(int),1,fp);
	fread(&postsStartingLocation, sizeof(int), 1, fp);
	fclose(fp);
}


//gets the write offset for user node
int getWriteOffSet()
{
	int offset = noOfUsers*sizeof(user) + 2 * sizeof(int);

	return offset;
}






/*------------------------------------------------EXTRAS--------------------------------------------------*/


long getFileSize(char* filename)
{
	FILE* fp = fopen(filename, "r");
	fseek(fp, 0, SEEK_END);
	long noOFBytes = ftell(fp);
	fclose(fp);


	return noOFBytes;
}


//finds ythe user offset for given username
int getUserOffset(char* username)
{

	int offset = 2 * sizeof(int);
	FILE* fp = fopen(FILE_NAME, "r");
	fseek(fp, 2 * sizeof(int), SEEK_SET);

	for (int i = 0; i <= noOfUsers; i++)
	{
		user u;
		fread(&u, sizeof(user), 1, fp);

		if (strcmp(u.username, username) == 0)
		{
			return offset;
		}
		offset += sizeof(user);
	}
}


//finds the userid for given username
int getUserId(char* username)
{

	FILE* fp = fopen(FILE_NAME, "r");
	fseek(fp, 2 * sizeof(int), SEEK_SET);

	for (int i = 0; i < noOfUsers; i++)
	{
		user u;
		fread(&u, sizeof(user), 1, fp);

		if (strcmp(u.username, username) == 0)
		{
			return u.userid;
		}
	}
}














/*-------------------------------------------------USERS--------------------------------------------------*/

//this function creates a user and stores in file
void createUser(char* username,char* password)
{
	user newUser;// = (user*)malloc(sizeof(user));
	newUser.noOfPosts = 0;
	newUser.userid = noOfUsers + 1;
	strcpy(newUser.username, username);
	strcpy(newUser.password,password);
	for (int i = 0; i < 20; i++)
		newUser.userPostOffsets[i] = -1;


	int writeOffset = getWriteOffSet();
	

	FILE* fp = fopen(FILE_NAME,"rb+");
	fseek(fp,writeOffset,SEEK_SET);
	fwrite(&newUser,sizeof(user),1,fp);
	//fclose(fp);


	noOfUsers++;

	//fp = fopen(FILE_NAME, "rb+");
	fseek(fp, 0, SEEK_SET);
	fwrite(&noOfUsers, sizeof(int), 1, fp);
	fclose(fp);



}



//this function is used for checking user credentials
int userLogin(char* username,char* password)
{
	int offset = getUserOffset(username);

	user u;
	FILE* fp = fopen(FILE_NAME, "rb+");
	fseek(fp, offset, SEEK_SET);

	fread(&u, sizeof(user), 1, fp);
	fclose(fp);

	if (strcmp(u.password, password) == 0)
	{
		return  1;
	}
	else return 0;
}










/*--------------------------------------------------POSTS----------------------------------------------------*/


//this function will be used to create a new post for a specific user
void createPost(char* username,char* filename)
{
	long size = getFileSize(filename);
	char *buff = (char*)AB_malloc(sizeof(char)*(size+1));

	FILE* infp = fopen(filename, "rb");
	fread(buff, size*sizeof(char), 1, infp);
	buff[size] = '\0';
	fclose(infp);

	//AB_free(buff);


	FILE* fp = fopen(FILE_NAME,"rb+");
	fseek(fp,-postsStartingLocation,SEEK_END);

	post po; int off = postsStartingLocation; int c;
	while (c = fread(&po, sizeof(post), 1, fp))
	{
		off += sizeof(post);
		int si = po.maxSize;
		
		if (strcmp(po.filename, "") == 0  && size <= po.maxSize)
		{
			break;
		}

		off += po.maxSize;
		char* buff = (char*)AB_malloc(sizeof(char)*(si + 1));
		fread(buff, si, 1, fp);
	}

	if (c == 0)
	{

		int postOffset = 0;

		FILE* fp = fopen(FILE_NAME, "rb+");
		if (postsStartingLocation == -1)
		{
			fseek(fp, 0, SEEK_END);
			postOffset = size;
		}
		else
		{
			fseek(fp, -postsStartingLocation, SEEK_END);
			postOffset = postsStartingLocation + size;
		}

		//seeking to curr - file size so as to write the file
		fseek(fp, -size, SEEK_CUR);
		fwrite(buff, size*sizeof(char), 1, fp);
		fclose(fp);



		int offset = getUserOffset(username);
		user u;
		FILE* newfp = fopen(FILE_NAME, "rb+");
		fseek(newfp, offset, SEEK_SET);
		fread(&u, sizeof(user), 1, newfp);



		//creating the post sturtcure
		post newPost;
		strcpy(newPost.filename, filename);
		newPost.likes = 0;
		if (size > newPost.maxSize)
			newPost.maxSize = size;
		newPost.size = size;
		newPost.userid = u.userid;
		newPost.noOfComments = 0;
		for (int i = 0; i < 20; i++)
		{
			newPost.comments[i].userid = -1;
			newPost.comments[i].del = 0;
		}


		//writing the post structure to bin file
		fp = fopen(FILE_NAME, "rb+");
		int pend = (postOffset + sizeof(post) + 1);
		fseek(fp, -pend, SEEK_END);
		fwrite(&newPost, sizeof(post), 1, fp);
		fclose(fp);

		postsStartingLocation = postOffset + sizeof(post) + 1;

		fp = fopen(FILE_NAME, "rb+");
		fseek(fp, sizeof(int), SEEK_SET);
		fwrite(&postsStartingLocation, sizeof(int), 1, fp);
		fclose(fp);


		//updating the respective user node in file
		int j;
		for (j = 0; j < 20; j++)
		{
			if (u.userPostOffsets[j] == -1)
				break;
		}
		u.userPostOffsets[j] = postsStartingLocation;
		u.noOfPosts++;
		int sizeofUser = sizeof(user);
		fseek(newfp, -sizeofUser, SEEK_CUR);
		fwrite(&u, sizeof(user), 1, newfp);

		fclose(newfp);
	}
	else
	{


		int offset = getUserOffset(username);
		user u;
		FILE* newfp = fopen(FILE_NAME, "rb+");
		fseek(newfp, offset, SEEK_SET);
		fread(&u, sizeof(user), 1, newfp);


		strcpy(po.filename,filename);
		po.size = size;
		po.userid = getUserId(username);
		po.likes = 0;
		for (int i = 0; i < 20; i++)
		{
			po.comments[i].del = 0;
			po.comments[i].userid = 0;
		}
		po.noOfComments = 0;
		int postsize = sizeof(post);
		fseek(fp, -postsize, SEEK_CUR);

		fwrite(&po, sizeof(post), 1, fp);



		int j;
		for (j = 0; j < 20; j++)
		{
			if (u.userPostOffsets[j] == -1)
				break;
		}
		u.userPostOffsets[j] = postsStartingLocation;
		u.noOfPosts++;
		int sizeofUser = sizeof(user);
		fseek(newfp, -sizeofUser, SEEK_CUR);
		fwrite(&u, sizeof(user), 1, newfp);

		fclose(newfp);
	}
}


//this function returns the index in the userPostOffset array inside user where the changes have to be made
int findPostToModify(int noOfPosts,user u)
{
	int count = 0;

	for (int i = 0; i < 20; i++)
	{
		if (u.userPostOffsets[i] == -1)
			continue;
		count++;
		if (count == noOfPosts)
			return count;
	}
}

//this function will create a copy of given file 
void downloadPost(char* username,char *filename,char* destFilename)
{
	int offset = getUserOffset(username);

	user u;
	FILE* fp = fopen(FILE_NAME,"rb+");
	fseek(fp,offset,SEEK_SET);

	fread(&u, sizeof(user), 1, fp);
	//fclose(fp);

	for (int i = 0; i < 20; i++)
	{
		int offset = u.userPostOffsets[i];


		fseek(fp,-offset,SEEK_END);
		post p;
		fread(&p,sizeof(post),1,fp);


		if (strcmp(p.filename, "") != 0)
		{
			if (strcmp(p.filename, filename) == 0)
			{
				int size = p.size;
				char* buff = (char*)malloc(sizeof(char)*(size + 1));
				fread(buff, size, 1, fp);
				//buff[size] = '\0';

				FILE* destfp = fopen(destFilename, "wb");

				fwrite(buff+1, size, 1, destfp);
				fclose(destfp);

				return;
			}
		}
	}
	printf("download operation failed!\n");

}


//this functionn shows all the files names uploaded by a particular user
void showPosts(char* username)
{
	int offset = getUserOffset(username);

	user u;
	FILE* fp = fopen(FILE_NAME, "rb+");
	fseek(fp, offset, SEEK_SET);

	fread(&u, sizeof(user), 1, fp);
	//fclose(fp);

	for (int i = 0; i < 20; i++)
	{
		int offset = u.userPostOffsets[i];


		fseek(fp, -offset, SEEK_END);
		post p;
		fread(&p, sizeof(post), 1, fp);


		if (offset!= -1 && strcmp(p.filename,"") != 0)
		{
			printf("%s\n", p.filename);
		}
	}

	fclose(fp);
}


//this function is used to delete a specific post of a user
void deletePost(char* username,char* filename)
{
	int offset = getUserOffset(username);

	user u;
	FILE* fp = fopen(FILE_NAME, "rb+");
	fseek(fp, offset, SEEK_SET);

	fread(&u, sizeof(user), 1, fp);
	//fclose(fp);

	for (int i = 0; i < 20; i++)
	{
		int offset = u.userPostOffsets[i];

		if (offset > 0)
		{
			fseek(fp, -offset, SEEK_END);
			post p;
			fread(&p, sizeof(post), 1, fp);
			if (strcmp(filename, p.filename) == 0)
			{
				strcpy(p.filename, "");
				p.size = -1;
				fwrite(&p, sizeof(post), 1, fp);
				u.userPostOffsets[i] = -1;
				break;
			}
		}
	}

	fseek(fp, offset, SEEK_SET);
	fwrite(&u, sizeof(user), 1, fp);

	fclose(fp);
}









/*----------------------------------------COMMENTS--------------------------------------------------*/

//this function adds a comment to a specific post
void createComment(char* username,char * postFileName,char* text)
{
	int offset = getUserOffset(username);

	user u;
	FILE* fp = fopen(FILE_NAME, "rb+");

	if (fp == NULL)
	{
		printf("\tCannot open File\n");
	}
	fseek(fp, offset, SEEK_SET);

	fread(&u, sizeof(user), 1, fp);
	//fclose(fp);

	for (int i = 0; i < 20; i++)
	{
		int offset = u.userPostOffsets[i];


		fseek(fp, -offset, SEEK_END);
		post p;
		fread(&p, sizeof(post), 1, fp);


		if (offset != -1 && strcmp(p.filename, postFileName) == 0)
		{

			for (int j = 0; j < 20; j++)
			{
				if (p.comments[j].del == 0)
				{
					strcpy(p.comments[j].text, text);
					p.comments[j].userid = 123;
					p.comments[j].del = 1;
					p.noOfComments++;
					

					int postsize = sizeof(post);
					fseek(fp, -offset, SEEK_END);
					fwrite(&p, sizeof(post), 1, fp);

					fclose(fp);



					return;
				}
			}
		}
	}

	fclose(fp);
}


//this function prints all comments for a specific post
void showComments(char* username,char* postFileName)
{
	int offset = getUserOffset(username);

	user u;
	FILE* fp = fopen(FILE_NAME, "rb+");
	fseek(fp, offset, SEEK_SET);

	fread(&u, sizeof(user), 1, fp);
	//fclose(fp);



	for (int i = 0; i < 20; i++)
	{
		int offset = u.userPostOffsets[i];


		fseek(fp, -offset, SEEK_END);
		post p;
		fread(&p, sizeof(post), 1, fp);


		if (offset != -1 && strcmp(p.filename, postFileName) == 0)
		{
			for (int j = 0; j < 20; j++)
			{
				if (p.comments[j].del == 1)
				{
					printf("\t%d \t%s\n",j,p.comments[j].text);
				}
			}
			fclose(fp);
			return;
		}
		
	}
	printf("\tno such post exists\n");
	fclose(fp);
}


//this function is used to delete a specific comment on a users post
void deleteComment(char* username, char* postFileName,int commentIndex)
{
	int offset = getUserOffset(username);

	user u;
	FILE* fp = fopen(FILE_NAME, "rb+");
	fseek(fp, offset, SEEK_SET);

	fread(&u, sizeof(user), 1, fp);
	//fclose(fp);

	for (int i = 0; i < 20; i++)
	{
		int offset = u.userPostOffsets[i];


		fseek(fp, -offset, SEEK_END);
		post p;
		fread(&p, sizeof(post), 1, fp);


		if (offset != -1 && strcmp(p.filename, postFileName) == 0)
		{
			p.comments[commentIndex].del = 0;
			fseek(fp, -offset, SEEK_END);
			fwrite(&p,sizeof(post),1,fp);
			fclose(fp);
			return;
		}
	}
	printf("\tno such post exists to delete\n");
	fclose(fp);
}









/*--------------------------------------------LIKES---------------------------------------------------*/


//this function is used to increment the likes of a post
void incrementLikes(char* username,char* postFileName)
{
	int offset = getUserOffset(username);

	user u;
	FILE* fp = fopen(FILE_NAME, "rb+");
	fseek(fp, offset, SEEK_SET);

	fread(&u, sizeof(user), 1, fp);
	//fclose(fp);



	for (int i = 0; i < 20; i++)
	{
		int offset = u.userPostOffsets[i];


		fseek(fp, -offset, SEEK_END);
		post p;
		fread(&p, sizeof(post), 1, fp);


		if (offset != -1 && strcmp(p.filename, postFileName) == 0)
		{
			p.likes++;
			fseek(fp, -offset, SEEK_END);
			fwrite(&p, sizeof(post), 1, fp);
			fclose(fp);

			printf("\t%s got %d likes\n", postFileName, p.likes);
			break;
		}
	}

}




/*----------------------------------------------UI----------------------------------------------------*/


//
//void ui()
//{
//	while (1)
//	{
//		system("cls");
//
//		int option = 0;
//		printf("\tYou can do:\n\t 1.create user  \n\t2.login \n\t3.create post \n\t4.add comment  \n\t5.view posts \n\t6.display comments  \n\t7.delete comment  \n\t8.delete post\n\t Enter option:");
//		scanf("%d", &option);
//
//		switch (option)
//		{
//		case 1:
//			system("cls");
//			char un[20],pas[20];
//			printf("Enter username:");
//			scanf("%s", un);
//			printf("\nEnter Password:");
//			scanf("%s", pas);
//
//			createUser(un,pas);
//			break;
//
//
//		case 2:
//			system("cls");
//			char un[20], pas[20];
//			printf("Enter username:");
//			scanf("%s", un);
//			printf("\nEnter Password:");
//			scanf("%s", pas);
//
//			int validate = userLogin(un,pas);
//			if (validate == 0)
//			{
//				printf("\t invalid username or password\n");
//				exit(0);
//			}
//			else
//				printf("\t\n Welcome %s ", un);
//			break;
//
//		case 3:
//
//		case 4:
//		case 5:
//		case 6:
//		case 7:
//		case 8:
//
//		}
//	}
//}


/*-------------------------------------------TESTING--------------------------------------------------*/


void test()
{
	/*createBlobFile("abc.bin");
	storeGlobalsToFile();


	createUser("abhinav");


	FILE* fp = fopen(FILE_NAME, "r");
	user u;
	fseek(fp,2*sizeof(int),SEEK_SET);
	fread(&u,sizeof(user),1,fp);

	printf("%s", u.username);

	fclose(fp);
	

	createPost("abhinav", "text1.txt");
	createPost("abhinav", "text2.txt");
	*/
	createBlobFile(FILE_NAME);
	storeGlobalsToFile();
	createUser("abhinav","1234");
	createPost("abhinav", "text3.txt");
	createPost("abhinav", "text2.txt");
	showPosts("abhinav");

	downloadPost("abhinav", "text3.txt", "dest1.txt");
	createComment("abhinav","text3.txt","heyyyyy yoo");
	deleteComment("abhinav","text3.txt",0);
	createComment("abhinav", "text3.txt", "bwhjdvbjh");
	showComments("abhinav","text3.txt");
	deletePost("abhinav","text3.txt");


	

	showComments("abhinav", "text3.txt");
	incrementLikes("abhinav","text2.txt");
	incrementLikes("abhinav","text2.txt");
	createComment("abhinav","text2.txt","asdf");
	showComments("abhinav","text2.txt");
	deleteComment("abhinav","text2.txt",0);
	/*FILE* fp = fopen(FILE_NAME,"rb");
	
	int o =  sizeof(post) + 4 ;
	fseek(fp,-o,SEEK_END);
	post p;
	fread(&p, sizeof(post), 1, fp);
	*/

	//showPosts("abhinav");
	//downloadPost("abhinav", "text3.txt", "newText.txt");
	/*FILE* fp = fopen(FILE_NAME, "rb+");
	int pend = (3+ 1*sizeof(post));
	fseek(fp, -pend, SEEK_END);
	post p;
	fread(&p, sizeof(post), 1, fp);



	user u;
	fseek(fp,8, SEEK_SET);
	fread(&u, sizeof(user), 1, fp);
	printf("%s  %d", p.filename,u.noOfPosts);
	fclose(fp);
	*/

}


int main()
{
	
	test();
	getchar();
}