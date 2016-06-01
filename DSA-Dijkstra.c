#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <time.h>

/* Dijkstra's algorithm shortest path implementation */


/* A vertex is either a block or not a block
   To implement Dijkstra's algorithm, also need to know:
   - Distance from source node
   - Predecessor node
   - Position in priority queue
   - List of adjacent vertices
   - Unique identifier */

typedef struct vertex {
    int block;
    int dist;
    int pos; //position in heap, counts from 1 not from 0
    int ident;
    struct vertex* pre;
    struct adjnode* edge;
} vertex;


/*Node in adjacency list for each vertex */

typedef struct adjnode {
    struct vertex* data;
    int weight;
    struct adjnode* next;
} adjnode;


/* A graph is a 2d array of vertices (array of vertex pointers) */

typedef struct graph {
    struct vertex** vertices;
} graph;


int citysize = 0, heapsize = 0, arraysize = 1;
graph g;


/* Sets graph size to number given in line 1 of file using malloc
   initialises graph */

void makeGraph () {

    int i,j;

    //create array[citysize]
    g.vertices = malloc(citysize*sizeof(vertex*));
    for (i = 0; i < citysize; i++) {
        //each array[i] is itself an array[citysize]
        g.vertices[i] = malloc(citysize*sizeof(vertex));
    }
    for (i = 0; i < citysize; i++) {
        for (j = 0; j < citysize; j++) {
            g.vertices[i][j].block = 0;
            g.vertices[i][j].dist = INT_MAX;
            g.vertices[i][j].pre = NULL;
            g.vertices[i][j].edge = NULL;
            g.vertices[i][j].ident = (i*citysize) + j;
        }
    }
    
}


/*Adds a node to adjacency list*/

void addNode (vertex* u, vertex* v, int weight) {

   adjnode** e = &u->edge;

    //empty list, add new element at start
    if (*e == NULL) {
        *e = malloc(sizeof(adjnode));
        if (*e == NULL) {
            printf("Error allocating adjacency list");
            exit(EXIT_FAILURE);
        }
    }
    //non-empty list, iterate to end then add new element
    else {
        while ((*e)->next != NULL) {
            e = &((*e)->next);
        }
        (*e)->next = malloc(sizeof(adjnode));
        e = &((*e)->next);
    }

    (*e)->data = v;
    (*e)->next = NULL;
    (*e)->weight = weight;

}


/* Reads location of blocks and teleports from file*/

void readFile (FILE* fp) {

    char type;
    int x1,y1,x2,y2,weight,i,j;

    fgetc(fp);
    while ((type = fgetc(fp)) != EOF) {
        fscanf(fp,"%d",&x1);
        fscanf(fp,"%d",&y1);
        fscanf(fp,"%d",&x2);
        fscanf(fp,"%d",&y2);

        if (type == 'b') {
            for (i = (x1-1); i < x2; i++) {
                for (j = (y1-1); j < y2; j++) {
                    g.vertices[j][i].block = 1;  
                }
            }
        }
        else if (type == 't') {
            fscanf(fp,"%d",&weight);
            addNode(&g.vertices[y1-1][x1-1], &g.vertices[y2-1][x2-1], weight);
        }
    fgetc(fp);
    }
}

/*Populates adjacency list (teleports already added at this point)*/

void makeAdjList () {

    int i,j;

    //adjacent nodes are either above, below, left or right
    for (i = 0; i < citysize; i++) {
        for (j = 0; j < citysize; j++) {
            if (g.vertices[i][j].block == 0) {
                if (j+1 < citysize && g.vertices[i][j+1].block == 0) {
                    addNode(&g.vertices[i][j], &g.vertices[i][j+1], 1);
                }
                if (i+1 < citysize && g.vertices[i+1][j].block == 0) {
                    addNode(&g.vertices[i][j], &g.vertices[i+1][j], 1);
                }
                if (j-1 >= 0 && g.vertices[i][j-1].block == 0) {
                    addNode(&g.vertices[i][j], &g.vertices[i][j-1], 1);
                }
                if (i-1 >= 0  && g.vertices[i-1][j].block == 0) {
                    addNode(&g.vertices[i][j], &g.vertices[i-1][j], 1);
                }
            }
        }
    }
}    


/* Frees the memory allocated to a graph */

void freeGraph () {

    int i;

    for (i = 0; i < citysize; i++) {
        free(g.vertices[i]);
    }

    free(g.vertices);
}


/* Find left child of node in heap */

int left (int i) {

    return (2*i);

}


/* Find right child of node in heap */

int right (int i) {

    return ((2*i)+1);

}


/* Find parent of node in heap */

int parent (int i) {

    return ((int)(i/2));

}


/* heapify function as in lecture (ensure heap property holds for tree rooted
   in given vertex) */

vertex** heapify (vertex** h, int pos) {

    int i,j,smallest = 0;
    vertex* temp;
    i = left(pos);
    j = right(pos);

    if ((i <= heapsize) && (h[i-1]->dist < h[pos-1]->dist)) {
        smallest = left(pos);
    }
    else {
        smallest = pos;
    }
    if ((j <= heapsize) && (h[j-1]->dist < h[smallest-1]->dist)) {
        smallest = right(pos);
    }
    if (smallest != pos) {
        temp = h[pos-1];
        h[pos-1] = h[smallest-1];
        h[pos-1]->pos = pos;
        h[smallest-1] = temp;
        h[smallest-1]->pos = smallest;
        heapify(h,smallest);
    }

    return h;
}


/* buildHeap operation as defined in lectures (ensure heap property
   holds for entire heap) */

vertex** buildHeap (vertex** h) {

    int i;

    for (i = (int)(heapsize/2); i > 0; i--) {
        heapify(h,i);
    }

    return h;
}


/* DecreaseKey as defined in lectures */

void decreaseKey (vertex** h, vertex* vp, int kdist) {

    vertex* temp;
    vertex v = (*vp);

    if (kdist > v.dist) {
        printf("Error: new key (%d) is larger than current key (%d)\n",kdist,v.dist);
    }
    else {
        h[v.pos-1]->dist = kdist;
        while (v.pos > 1 && (h[parent(v.pos)-1]->dist > h[v.pos-1]->dist)) {
            //swap
            temp = h[v.pos-1];
            h[v.pos-1] = h[parent(v.pos)-1];
            h[v.pos-1]->pos = v.pos;
            h[parent(v.pos)-1] = temp;
            h[parent(v.pos)-1]->pos = parent(v.pos);
            v.pos = parent(v.pos);
        }
    }
    
}


/* Insert as defined in lectures */

void insert (vertex** h, vertex* v) {

    vertex** newh;

    /*heapsize = heapsize + 1;
    printf("before realloc loop\n");
    fflush(stdout);
    if (heapsize > arraysize) {
        arraysize = arraysize*2;
        newh = realloc(h,arraysize*sizeof(vertex*));

        if (!newh) {
            free(h);
            printf("Array reallocation failed");
            fflush(stdout);
            exit(EXIT_FAILURE);
        }

        h = newh;
    }*/

    //printf("heapsize is %d\n",heapsize);
    //printf("arraysize is %d\n", arraysize);
    //fflush(stdout);
    v->pos = heapsize;
    //printf("assigns position\n");
    h[heapsize-1] = v;
    printf("inserts vertex\n");
    fflush(stdout);

    decreaseKey(h,v,v->dist);
    printf("key is decreased\n");
    fflush(stdout);
}


/* ExtractMin as defined in lectures */

vertex* extractMin (vertex** h) {

    vertex* min;

    if (heapsize < 1) {
        printf("Error: Heap Underflow");
        exit(EXIT_FAILURE);
    }
    else {
        min = h[0];
        h[0] = h[heapsize-1];
        h[0]->pos = 1;
        heapsize = heapsize-1;
        heapify(h,1);
        return min;
    }

}


/* Relax function as defined in lectures */

void relax (vertex** h, vertex* u, adjnode* vp) {

    adjnode v = *vp;
    int newdist = 0;

    if (v.data->dist > u->dist + v.weight) {
        newdist = u->dist + v.weight;
        v.data->pre = u;
        decreaseKey(h,v.data,newdist);
    }

}


/* Carry out Dijkstra's algorithm */

void dijkstra () {

    vertex** h = malloc(citysize*citysize*sizeof(vertex*));
    vertex* vp;
    adjnode* ep;
    int i,j;

    g.vertices[0][0].dist = 0;

    for (i = 0; i < citysize; i++) {
        for (j = 0; j < citysize; j++) {
            if (g.vertices[i][j].block == 0) {
                g.vertices[i][j].pos = heapsize + 1;
                insert(h, &g.vertices[i][j]);
                //h[heapsize] = &g.vertices[i][j];
                heapsize = heapsize + 1;
            }
        }
    }
    //printf("final heapsize is %d\n",heapsize);
    buildHeap(h);
    
    while (heapsize > 0) {
        vp = extractMin(h);
        ep = vp->edge;
        while (vp->dist != INT_MAX && ep != NULL) {
            relax(h,vp,ep);
            ep = ep->next;
        }
    }
    //free the heap
    free(h);
}


/* Various tests*/

void test () {

    srand(time(NULL));
    int i,correct = 1;
    vertex* v;
    vertex* vp;
    vertex vn;

    vertex** h = malloc(heapsize*sizeof(vertex*));

    vertex v1,v2,v3,v4,v5;
    v1.dist = 11;
    insert(h,&v1);
    v2.dist = 7;
    insert(h,&v2);
    v3.dist = 5;
    insert(h,&v3);
    v4.dist = 9;
    insert(h,&v4);
    v5.dist = 3;
    insert(h,&v5);
  
    for (i = 0; i < heapsize; i++) {
        printf("%d has dist %d\n", h[i]->pos, h[i]->dist);
    }
    
    v = h[3];
    decreaseKey(h,v,3); 
 
    vn.dist = 9;
    insert(h,&vn); 

    v = h[0];
    vp = extractMin(h);

    //check extractMin returns the correct element
    if ((*v).dist != (*vp).dist) {
        correct = 0;
    }

    //check heap property holds
    for (i = heapsize; i > 1; i--) {
        if ((h[i-1])->dist < (h[parent(i)-1])->dist) {
            correct = 0;
        }
    }

    //check position is updated correctly
    for (i = heapsize; i > 0; i--) {
        if ((*h[i-1]).pos != i) {
            correct = 0;
        }
    }

    if (correct == 1) {
        printf("correct\n");
    }

    free(h);
}


/* Follows path from bottom-right to top-left recursively,
   prints direction after evaluating each recursive call */

void printRoute (vertex current) {

    vertex pre;

    if (current.pre != NULL) {
        pre = (*current.pre);
        if (pre.ident == current.ident -1 && current.ident % citysize != 0) {
            printRoute(pre);
            printf("E");
        }
        else if (pre.ident == current.ident + 1 && pre.ident % citysize != 0) {
            printRoute(pre);
            printf("W");
        }
        else if (pre.ident == current.ident - citysize) {
            printRoute(pre);
            printf("S");
        }
        else if (pre.ident == current.ident + citysize) {
            printRoute(pre);
            printf("N");
        }
        else {
            printRoute(pre);
            printf("T");
        }            
    }
}


/*Main function */
 
int main (int argc, char *argv[]) {

    FILE *fp;

    if (argc != 2) {
        printf("Need to run with file as an argument\n");
        exit(EXIT_FAILURE);
    }

    fp = fopen(argv[1],"r");
    fscanf(fp,"%d",&citysize);
    makeGraph();
    readFile(fp);
    makeAdjList(); 
    dijkstra();
    
    //test();

    printRoute(g.vertices[citysize-1][citysize-1]);
    printf("\n");
    fclose(fp);
    freeGraph();

    return 0;
}







