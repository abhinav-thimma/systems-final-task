int noOfUsers = 0;
int postsStartingLocation = -1;


typedef struct
{
	short userid;		//id to uniquely identify users who commented on my post
	char del;			// del = 0 then comment deleted ,   del > 0 then it is comment size
	char text[50];
}comment;


typedef struct
{
	//short postid;				//this id uniquely identifies the post
	short userid;				//this iunique;ly identifies a user
	short likes;				//no of likes for this post
	char filename[20];			//if filename == ""  then this post is deleted , else it consisits of filename
	int noOfComments;				//this offset refers to the position of 
	int size;
	int maxSize;
	comment comments[20];
} post;


typedef struct
{
	short userid;
	short noOfPosts;
	char username[20];
	char password[20];
	int userPostOffsets[20];
}user;

