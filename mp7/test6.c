
/* 
 * CS 241
 * The University of Illinois
 */

#define _GNU_SOURCE
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "libmapreduce.h"

#define CHARS_PER_INPUT 30000
#define INPUTS_NEEDED 10

static const char *long_key = "Longest_Word";

/* Takes input string and maps the longest
 * word to the key, long_key.
 */

typedef struct _list_t
{
	struct _list_t *next;
	char *word;
	int count;
} list_t;
list_t *head = NULL;


void map(int fd, const char *data)
{
	list_t *thru;

	while (1)
	{
		char *word = malloc(100); int word_len = 0;
		const char *word_end = data;

		/* Get each word... */
		while (1)
		{
			if ( ((*word_end == ')' || *word_end == '(' ||*word_end == '/' ||*word_end == ' ' || *word_end == '!' || *word_end == '-' || *word_end == ',' || *word_end == '.' || *word_end == '\n') && word_len > 0) || *word_end == '\0' || word_len == 99)
				break;
			else if (isalpha(*word_end))
				word[word_len++] = tolower(*word_end);

			word_end++;
		}

		if (*word_end == '\0') { break; }

		word[word_len] = '\0';

		/* Update our linked list... */
		thru = head;
		while (thru != NULL)
		{
			if ( strcmp(thru->word, word) == 0 )
			{
				thru->count++;
				break;
			}

			thru = thru->next;
		}

		if (thru == NULL)
		{
			list_t *node = malloc(sizeof(list_t));
			node->next = head;
			node->word = word;
			node->count = 1;

			head = node;
		}
		else
			//free(word);


		data = word_end + 1;
	}



	thru = head;
	char s[200];
	while (thru != NULL)
	{
		//printf("going to add something\n");	
		// ORIGINAL ---> int len = snprintf(s, 200, "%s: %d\n", thru->word, thru->count);
		int len = snprintf(s, 200, "%s: %s\n", long_key, thru->word);	
		write(fd, s, len);

		list_t *thru_next = thru->next;
		//printf("These are the values.... fd-> %d, word -> %s, strlen -> %d, thru_next -> %p\n", fd, thru->word,(int) strlen(thru->word), thru_next);

		//free(thru->word)
;		//free(thru);
		thru = thru_next;
	}

	close(fd);
}

/* Takes two words and reduces to the longer of the two
 */
const char *reduce(const char *value1, const char *value2)
{
	//    char * newtarget1 = strdup(target1);
	//printf("called\n");
	 
	if (value1 == NULL){char * back = strdup(value2);return back;}
	if (value2 == NULL){char * back = strdup(value1);return back;}


	//printf("comparing THIS value1 %s AND value2 %s\n", value1, value2);
	if (strlen(value1) > strlen(value2)){char * back = strdup(value1);return back;}
	char * back = strdup(value2);
	return back; 
}


int main()
{
	FILE *file = fopen("alice.txt", "r");
	char s[1024];
	int i;

	char **values = malloc(INPUTS_NEEDED * sizeof(char *));
	int values_cur = 0;
	
	values[0] = malloc(CHARS_PER_INPUT + 1);
	values[0][0] = '\0';

	while (fgets(s, 1024, file) != NULL)
	{
		if (strlen(values[values_cur]) + strlen(s) < CHARS_PER_INPUT)
			strcat(values[values_cur], s);
		else
		{
			values_cur++;
			values[values_cur] = malloc(CHARS_PER_INPUT + 1);
			values[values_cur][0] = '\0';
			strcat(values[values_cur], s);
		}
	}

	values_cur++;
	values[values_cur] = NULL;
	
	fclose(file);

	mapreduce_t mr;
	mapreduce_init(&mr, map, reduce);

	mapreduce_map_all(&mr, (const char **)values);
	mapreduce_reduce_all(&mr);
	
	const char *result_longest = mapreduce_get_value(&mr, long_key);

	if (result_longest == NULL) { printf("MapReduce returned (null).  The longest word was not found.\n"); }
	else { printf("Longest Word: %s\n", result_longest); free((void *)result_longest); }

	mapreduce_destroy(&mr);

	for (i = 0; i < values_cur; i++)
		free(values[i]);
	free(values);

	return 0;
}
