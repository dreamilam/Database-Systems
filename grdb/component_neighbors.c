#include "graph.h"
#include <assert.h>
#define BUFSIZE		(1 << 12)
#define O_RDONLY 0x0000
#define O_RDWR 0x0000


void
component_neighbors(char *grdbdir, int gidx, int cidx, vertexid_t id)
{
	char s[BUFSIZE];
	struct component c;
	int fd;

	off_t off;
	ssize_t len, size;
	vertexid_t id1, id2;
	char *buf;
	int readlen;

	component_init(&c);

	/* Load enums */
	c.efd = enum_file_open(grdbdir, gidx, cidx);
	if (c.efd >= 0) {
		enum_list_init(&(c.el));
		enum_list_read(&(c.el), c.efd);
	}

	/* Load edge schema */
	memset(s, 0, BUFSIZE);
	sprintf(s, "%s/%d/%d/se", grdbdir, gidx, cidx);
	fd = open(s, O_RDONLY);
	if (fd >= 0) {
		c.se = schema_read(fd, c.el);
		close(fd);
	}

	/* Open edge file */
	memset(s, 0, BUFSIZE);
	sprintf(s, "%s/%d/%d/e", grdbdir, gidx, cidx);
	c.efd = open(s, O_RDWR);

	/* Edges */
	if (c.se == NULL)
		size = 0;
	else
		size = schema_size(c.se);

	readlen = (sizeof(vertexid_t) << 1) + size;
#if 0
	free(buf);
#endif
	buf = malloc(readlen);
	assert (buf != NULL);
	memset(buf, 0, readlen);

	printf("(");
	int is_first_element = 1;

	for (off = 0;; off += readlen) {
		lseek(c.efd, off, SEEK_SET);
		len = read(c.efd, buf, readlen);
		if (len <= 0)
			break;

		id1 = *((vertexid_t *) buf);
		id2 = *((vertexid_t *) (buf + sizeof(vertexid_t)));
		if (id1 == id){
			if(is_first_element != 1)
				printf(", ");
			else
				is_first_element = 0;
		 	printf("%llu",id2);
		} 
		// else if (id2 == id){
		//  	printf("(%llu)",id1);
		// }
	}
	printf(")\n");
	return;
}
