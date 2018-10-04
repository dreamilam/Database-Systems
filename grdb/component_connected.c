#include "graph.h"
#define BUFSIZE		(1 << 12)
#define O_RDONLY 0x0000
#define O_RDWR 0x0000
#include "stdbool.h"
#include <assert.h>


int
component_connected_strong(
        char *grdbdir, int gidx, int cidx, vertexid_t id1, vertexid_t id2)
{
        char s[BUFSIZE];
	struct component c;
	int fd;

	off_t off, off1;
	ssize_t len, size;
	vertexid_t edge_id1, edge_id2;
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
        buf = malloc(readlen);
        
        int connectedness = 0;
        FILE *fp, *fv;
        fp = fopen("dfs_stack","wb+");
        fv = fopen("visited","wb+");
   
        // Add first vertex to the stack
        vertexid_t *v1;
        v1 = &id1;
        fwrite(v1, sizeof(vertexid_t), 1, fp);

        vertexid_t *v = malloc(sizeof(vertexid_t)); 
        vertexid_t vertex;

        fseek(fp,0, SEEK_END);
        size_t stacksize = ftell(fp);

        while(stacksize != 0){
                fseek(fp,-sizeof(vertexid_t), SEEK_END);
                fread(v,sizeof(vertexid_t),1,fp);
                vertex = *v;
                //printf("Polled %llu from stack\n", vertex);
                
                if(vertex == id2)
                {
                        connectedness = 1;
                        break;
                }
                else
                {  
                        // Pop from stack
                        fseek(fp,-sizeof(vertexid_t),SEEK_END);
                        off_t position = ftell(fp);
                        ftruncate(fileno(fp), position);
                        //printf("stack Popped\n");

                        // Loop to see if the popped vertex has been visited
                        bool is_visited = false;
                        fseek(fv, 0, SEEK_SET);

                        *v = NULL;
                        for(off = 0;; off += sizeof(vertexid_t)){
                                lseek(fv, off, SEEK_SET);
		                if (fread(v, sizeof(vertexid_t), 1,fv) == 0)
			                break;
                                else{
                                        //printf("Read %llu from visited\n", *v);
                                        if (vertex == *v)
                                        {
                                                is_visited = true;
                                                break;
                                        }
                                }
                        }
                        
                        if(!is_visited)
                        {   
                                //printf("Vertex %llu has not been visited\n", vertex);    
                                //add vertex to visited
                                fwrite(&vertex, sizeof(vertexid_t), 1, fv); 
                                //printf("vertex %llu added to visited\n", vertex);
                        }

                        //get neighbors and push to the stack
                        #if 0
                                free(buf);
                        #endif

                        assert (buf != NULL);
                        memset(buf, 0, readlen);

                        for (off = 0;; off += readlen) {
                                lseek(c.efd, off, SEEK_SET);
                                len = read(c.efd, buf, readlen);
                                if (len <= 0)
                                        break;
                                edge_id1 = *((vertexid_t *) buf);
                                edge_id2 = *((vertexid_t *) (buf + sizeof(vertexid_t)));
                                
                                if (edge_id1 == vertex)
                                {       
                                        // Check if the neigbor is visited
                                        bool is_visited1 = false;
                                        fseek(fv, 0, SEEK_SET);

                                        *v = NULL;
                                        for(off1 = 0;; off1 += sizeof(vertexid_t)){
                                                lseek(fv, off1, SEEK_SET);
                                                if (fread(v, sizeof(vertexid_t), 1,fv) == 0)
                                                        break;
                                                else{
                                                        //printf("Read %llu from visited\n", *v);
                                                        if (edge_id2 == *v)
                                                        {
                                                                is_visited1 = true;
                                                        }
                                                }
                                        }

                                        if(!is_visited1)
                                        {
                                                fwrite(&edge_id2, sizeof(vertexid_t), 1, fp);
                                                //printf("%llu added to stack\n", edge_id2);
                                        }
                                                
                                } 
                        } 
                }   
                // printf("\n");  
                // sleep(1);  
                fseek(fp,0, SEEK_END);
                stacksize = ftell(fp);     
        } 
        fclose(fp);
        fclose(fv);

        if(connectedness == 1)
                printf("True\n");
        else
                printf("False\n");

	return connectedness;
}

int
component_connected_weak(
        char *grdbdir, int gidx, int cidx, vertexid_t id1, vertexid_t id2)
{
        //Old Procedure
        /*
        int connectedness1 = 0;
        int connectedness2 = 0;

        connectedness1=component_connected_strong( grdbdir,  gidx,  cidx,  id1,  id2);
        connectedness2=component_connected_strong( grdbdir,  gidx,  cidx,  id2,  id1);

        if(connectedness1==1 || connectedness2==1)
        {
                printf("True\n");
        }
        else
        {
                printf("False\n");
        }
	return 0;
        */

        // Same procedure as Strongly connected
        // Only difference is while adding the neighbor to the stack in DFS
        // In this case, We consider a vertex as a neighbor if there is an edge from the current vertex to the other or vice versa
        char s[BUFSIZE];
	struct component c;
	int fd;

	off_t off, off1;
	ssize_t len, size;
	vertexid_t edge_id1, edge_id2;
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
        buf = malloc(readlen);

        int connectedness = 0;
        FILE *fp, *fv;
        fp = fopen("dfs_stack_weak","wb+");
        fv = fopen("visited_weak","wb+");
   
        vertexid_t *v1;
        v1 = &id1;
        fwrite(v1, sizeof(vertexid_t), 1, fp);
        vertexid_t *v = malloc(sizeof(vertexid_t)); 
        vertexid_t vertex;

        fseek(fp,0, SEEK_END);
        size_t stacksize = ftell(fp);
        
        while(stacksize !=0 ){
                fseek(fp,-sizeof(vertexid_t), SEEK_END);
                fread(v,sizeof(vertexid_t),1,fp);
                vertex = *v;
                //printf("Polled %llu from stack\n", vertex);
                
                if(vertex == id2)
                {
                        connectedness = 1;
                        break;
                }
                else
                {  
                        // Pop from stack
                        fseek(fp,-sizeof(vertexid_t),SEEK_END);
                        off_t position = ftell(fp);
                        ftruncate(fileno(fp), position);
                        //printf("stack Popped\n");

                        bool is_visited = false;
                        fseek(fv, 0, SEEK_SET);

                        *v = NULL;
                        for(off = 0;; off += sizeof(vertexid_t)){
                                lseek(fv, off, SEEK_SET);
		                if (fread(v, sizeof(vertexid_t), 1,fv) == 0)
			                break;
                                else{
                                        //printf("Read %llu from visited\n", *v);
                                        if (vertex == *v)
                                        {
                                                is_visited = true;
                                                break;
                                        }
                                }
                        }
                        
                        if(!is_visited)
                        {   
                                //printf("Vertex %llu has not been visited\n", vertex);    
                                //add vertex to visited
                                fwrite(&vertex, sizeof(vertexid_t), 1, fv); 
                                //printf("vertex %llu added to visited\n", vertex);
                        }

                        //get neighbors and push to the stack
                        #if 0
                                free(buf);
                        #endif

                        assert (buf != NULL);
                        memset(buf, 0, readlen);

                        for (off = 0;; off += readlen) {
                                lseek(c.efd, off, SEEK_SET);
                                len = read(c.efd, buf, readlen);
                                if (len <= 0)
                                        break;
                                edge_id1 = *((vertexid_t *) buf);
                                edge_id2 = *((vertexid_t *) (buf + sizeof(vertexid_t)));

                                if (edge_id1 == vertex)
                                {       
                                        bool is_visited1 = false;
                                        fseek(fv, 0, SEEK_SET);

                                        *v = NULL;
                                        for(off1 = 0;; off1 += sizeof(vertexid_t)){
                                                lseek(fv, off1, SEEK_SET);
                                                if (fread(v, sizeof(vertexid_t), 1,fv) == 0)
                                                        break;
                                                else{
                                                        //printf("Read %llu from visited\n", *v);
                                                        if (edge_id2 == *v)
                                                        {
                                                                is_visited1 = true;
                                                        }
                                                }
                                        }

                                        if(!is_visited1)
                                        {
                                                fwrite(&edge_id2, sizeof(vertexid_t), 1, fp);
                                                //printf("%llu added to stack\n", edge_id2);
                                        }
                                                
                                } 
                                else if (edge_id2 == vertex)
                                {       
                                        bool is_visited1 = false;
                                        fseek(fv, 0, SEEK_SET);

                                        *v = NULL;
                                        for(off1 = 0;; off1 += sizeof(vertexid_t)){
                                                lseek(fv, off1, SEEK_SET);
                                                if (fread(v, sizeof(vertexid_t), 1,fv) == 0)
                                                        break;
                                                else{
                                                        //printf("Read %llu from visited\n", *v);
                                                        if (edge_id1 == *v)
                                                        {
                                                                is_visited1 = true;
                                                        }
                                                }
                                        }

                                        if(!is_visited1)
                                        {
                                                fwrite(&edge_id1, sizeof(vertexid_t), 1, fp);
                                                //printf("%llu added to stack\n", edge_id1);
                                        }
                                                
                                } 
                        } 
                }   
                // printf("\n");  
                // sleep(1);  
                fseek(fp,0, SEEK_END);
                stacksize = ftell(fp);     
        } 
        fclose(fp);
        fclose(fv);

        if(connectedness == 1)
                printf("True\n");
        else
                printf("False\n");

	return connectedness;
}
