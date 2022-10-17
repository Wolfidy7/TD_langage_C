#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "i_circular_buffer_repository.h"
#include "circular_buffer_infra_file_service.h"
#include "i_circular_buffer.h"

#define MAX_RECORD 1024
#define INDEX_SUFFIX ".ndx"
#define DATA_SUFFIX ".rec"
#define FILE_DB_REPO "../Persistence/FileDB/CircularBuffer/CIRCULAR_BUFFER"

struct RECORD_CB
{
    unsigned long data_length;
    int offset_head;
    int offset_current;
    char data[]; //new in c99 !!
};

struct circular_buffer
{
    char *tail;
    unsigned long length;
    char *head;
    char *current;
    bool isFull;
};

struct index
{
    long recordStart;
    size_t recordLength;
};

static FILE *index_stream;
static FILE *data_stream;

int ICircularBufferRepository_save(circular_buffer cb)
{
	if (!ICircularBufferRepository_open(FILE_DB_REPO))
		return 0;
	ICircularBufferRepository_append(cb);
	return 1;
}

void ICircularBufferRepository_close(void)
{
	fclose(data_stream);
	fclose(index_stream);
}

static FILE *auxiliary_open(char *prefix, char *suffix)
{
	int prefix_length = strlen(prefix);
	int suffix_length = strlen(suffix);
	char name[prefix_length + suffix_length + 1];
	strncpy(name, prefix, prefix_length);
	strncpy(name + prefix_length, suffix, suffix_length + 1);
	FILE *flux = fopen(name, "r+");
	if (flux == NULL)
		flux = fopen(name, "w+");
	if (flux == NULL)
		perror(name);
	return flux;
}

int ICircularBufferRepository_open(char *name)
{
	data_stream = auxiliary_open(name, DATA_SUFFIX);
	if (data_stream == NULL)
	return 0;
	index_stream = auxiliary_open(name, INDEX_SUFFIX);
	if (index_stream == NULL)
	{
		fclose(data_stream);
		return 0;
	}
	return 1;
}

int ICircularBufferRepository_append(circular_buffer cb)
{
	struct index index;
	int myBuffer[5] = {*(cb->tail), cb->length, *(cb->head), *(cb->current), cb->isFull};
	size_t length_1 = sizeof(myBuffer); //length est déjà utilisé dans myBuffer
	fseek(data_stream, 0L, SEEK_END);
	index.recordStart = ftell(data_stream);
	index.recordLength = length_1;
	fwrite(myBuffer, sizeof(myBuffer), 1, data_stream);
	fseek(index_stream, 0L, SEEK_END);
	fwrite(&index, sizeof index, 1, index_stream);
	return 1;
}
