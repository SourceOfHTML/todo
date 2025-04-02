#include <string.h>

#include <errno.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define println(m_x) printf("%s\n", m_x)

#define LIST_COMMAND "list"
#define LIST_COMMAND_SIZE strlen(LIST_COMMAND)
#define ADD_COMMAND "add"
#define ADD_COMMAND_SIZE strlen(ADD_COMMAND)
#define REMOVE_COMMAND "remove"
#define REMOVE_COMMAND_SIZE strlen(REMOVE_COMMAND)

#define WRITE_BUFFER_SIZE 6
#define GROUP_SEPERATOR '\n'
#define UNIT_SEPERATOR (char)31

// The format for each item is thus:
// $id: $name{delimiter}$desc\n

// TODO: ADD A DATABASE OF TAKEN IDs FOR EASE OF LIFE!!!!!

typedef struct todo_item{
	uint16_t id;
	char* name;
	char* desc;
	bool is_finished;
} todo_t;

enum success_state{
	SUCCEEDED,
	FAILED
};

enum success_state add(char *p_name, char* p_desc);
enum success_state remove_todo(short p_id);
void list(void);
bool has(short *, short, int);

int main(int argc, char **argv)
{
	if (argc < 1)
	{
		println("You somehow called this program with no (or negative) arguments. Congratulations");
		return 1;
	}
	if (argc == 1)
	{
		println("This is a help message");
		return 0;
	}

	char *first_arg = argv[1];

	int is_list = strncmp(first_arg, LIST_COMMAND, LIST_COMMAND_SIZE) == 0;
	int is_add = strncmp(first_arg, ADD_COMMAND, ADD_COMMAND_SIZE) == 0;
	int is_remove = strncmp(first_arg, REMOVE_COMMAND, REMOVE_COMMAND_SIZE) == 0;

	if (!is_add && !is_list && !is_remove)
	{
		println("Invalid command, use the program name by itself to get the help message");
		return 0;
	}

	if ((is_add && argc != 4) || (is_remove && argc != 3))
	{
		println("Wrong number of arguments, needs two");
		return 0;
	}

	if(is_add)
	{
		enum success_state success = add(argv[2], argv[3]);
		if (success == SUCCEEDED)
		{
			println("Successfully added the item to the list");
		} else
		{
			println("Failed to add the item to the list, exiting");
		}
		return 0;
	}
	
	if(is_remove)
	{
		enum success_state success = remove_todo(strtoll(argv[2], NULL, 10));
		if (success == SUCCEEDED)
		{
			println("Successfully removed the item from the list");
		} else
		{
			println("Failed to remove the item from the list");
		}
		return 0;
	}

	if(is_list)
	{
		list();
		return 0;
	}

}

void list(void)
{
	FILE * file_in = fopen("todo_list", "r");

	if (file_in == NULL)
	{
		perror("Failed to open todo_list");
		return;
	}

	char id_in_file[WRITE_BUFFER_SIZE];
	while ( fgets(id_in_file, WRITE_BUFFER_SIZE, file_in) )
	{
		short id = strtoull(id_in_file, NULL, 10);

		fgetc(file_in);
		fgetc(file_in);
		
		char ch;

		printf("%d: ", id);
		while ( (ch = fgetc(file_in)) != UNIT_SEPERATOR )
		{
			putchar(ch);
		}
		putchar('\n'); putchar('\t');
		while ( (ch = fgetc(file_in)) != GROUP_SEPERATOR )
		{
			putchar(ch);
		}

		putchar('\n');
	}
}

enum success_state add(char *p_name, char* p_desc)
{
	FILE * file_in = fopen("todo_list", "a+");

	if (file_in == NULL)
	{
		println("ERROR: failed to open file");
		perror("Errno says");
		return FAILED;
	}
	
	fseek(file_in, 0, SEEK_SET);
	
	short id = 0;
	short invalid_ids[512];
	int counter = 0;
	char id_in_file[WRITE_BUFFER_SIZE];

	while ( fgets(id_in_file, WRITE_BUFFER_SIZE, file_in) )
	{
		short temp_id = (short)strtoll(id_in_file, NULL, 10);
		if (!has(invalid_ids, temp_id, counter))
		{
			invalid_ids[counter] = temp_id;
			counter++;
			if (counter >= 512)
			{
				break;
			}
		}

		while( fgetc(file_in) != '\n')
		{

		}
		fgetc(file_in);
	}

	while(has(invalid_ids, id, counter))
	{
		id++;
	}
	

	fseek(file_in, 0, SEEK_END);
	fprintf(file_in, "%.5d: %s%c%s\n", id, p_name, 31, p_desc);

	fclose(file_in);
	return SUCCEEDED;
}

enum success_state remove_todo(short p_id)
{
	if (p_id >= 100000 || p_id < 0)
	{
		println("Invalid ID");
		return FAILED;
	}

	FILE * file_in = fopen("todo_list", "r");
	FILE * file_copy = fopen("temp_todo_list", "w");

	if (file_in == NULL || file_copy == NULL)
	{
		return FAILED;
	}

	char id_in_file[WRITE_BUFFER_SIZE];
	while ( fgets(id_in_file, WRITE_BUFFER_SIZE, file_in))
	{
		short temp_id = strtoull(id_in_file, NULL, 10);
		if ( temp_id == p_id )
		{
			while ( fgetc(file_in) != '\n') {}
			continue;
		}
		fprintf(file_copy, "%.5d", temp_id);
		
		char ch;
		while ( (ch = fgetc(file_in)) != '\n')
		{
			fputc(ch, file_copy);
		}

		fputc(ch, file_copy);
	}
	
	if ( remove("todo_list") != 0)
	{
		perror("Failed to remove item");
		remove("temp_todo_list");
		return FAILED;
	}
	file_in = NULL;

	rename("temp_todo_list", "todo_list");
	return SUCCEEDED;
}

bool has(short * nums, short num, int size)
{
	for(int i = 0; i < size; i++)
	{
		if (nums[i] == num)
		{
			return true;
		}
	}
	return false;
}