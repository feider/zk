#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <curses.h>
#include <locale.h>
#include <wchar.h>

#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>


// configuration
#include "config.h"

// init reader
#include "cinireader.h"

#define DEBUG 0

#define true 1
#define false 0


int print_zettel_folder(){ DIR *d; struct dirent *dir;
    d = opendir(zettelkasten_path);
    if(d){
        while( (dir = readdir(d)) != NULL ) {
            if(dir->d_name[0] != '.')
                printf("%s\n", dir->d_name);
        }
        closedir(d);
    }
    else
    {
        printf("could not search folder\n");
        exit(1);
    }
    return 0;
}

int add_zettel(){
    // get new zettel name by time and date
    time_t rawtime;
    struct tm * timeinfo;
    char zettel_name[20];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
strftime(zettel_name, 20, "%Y_%m_%d_%H_%M_%S", timeinfo);
    printf("%s\n", zettel_name);
    char * full_command = malloc(strlen(editor_command) + 1 + strlen(zettelkasten_path) + 1 + strlen(zettel_name) + 1);
    full_command[0] = 0;
    strcat(full_command, editor_command);
    strcat(full_command, " ");
    strcat(full_command, zettelkasten_path);
    strcat(full_command, "/");
    strcat(full_command, zettel_name);

    //printf("full command: %s\n", full_command);

    FILE * execute;
    char result[100];

    // TODO: Check if file already exists
    // If yes: append something to the end

    execute = popen(full_command, "r");
    if(execute == NULL)
    {
        printf("failed to create new zettel\n");
        exit(1);
    }
    
    while(fgets(result, sizeof(result), execute) != NULL){
        printf("%s", result);
    }
    pclose(execute);


    free(full_command);
}


// #########################################
// tags
// #########################################

typedef struct {
    char * title;
    size_t * zettel;
    size_t num_zettel;
    size_t max_zettel;

} Tag;

typedef struct {
    Tag ** array;

    size_t size;
    size_t used;
} Tag_Array;

void init_tags(Tag_Array * tags, size_t init_size)
{
    tags->used = 0;
    tags->size = init_size;
    tags->array = (Tag **) malloc(init_size * sizeof(Tag*));
}

void append_tag(Tag_Array * tags, const char * tag_str)
{
    if(DEBUG)
        printf("adding tag %s\n", tag_str);
    if (tags->used == tags->size)
    {
        if(DEBUG)
            printf("increasing tag array\nOld size:%zu\n", tags->size);
        tags->size *= 2;
        if(DEBUG)
            printf("new size: %zu\n", tags->size);
        tags->array = (Tag**) realloc(tags->array, tags->size * sizeof(Tag*));
    }
    Tag * tag = (Tag * ) malloc(sizeof(Tag));
    tag->title = (char *) malloc(strlen(tag_str) * sizeof(char));
    strcpy(tag->title, tag_str);
    tag->max_zettel=1;
    tag->num_zettel=0;
    tag->zettel = (size_t*) malloc(tag->max_zettel * sizeof(size_t));

    tags->array[tags->used] = tag;
    tags->used ++;
}

size_t find_tag(Tag_Array * tags, const char * tag_str)
{
    for(size_t i = 0; i<tags->used; i++)
        if (strcmp(tags->array[i]->title, tag_str) == 0)
            return i;
    return SIZE_MAX;
}




void print_tags(Tag_Array * tags)
{
    for(size_t i = 0; i<tags->used; i++)
    {
        //printf("Tag %zu: %s, Number of Zettel: %zu\n", i, tags->array[i]->title, tags->array[i]->num_zettel);
        printf("[%s] ", tags->array[i]->title);
    }
    printf("\n");
}


// #########################################
// zettel
// #########################################

typedef struct {
    size_t * tags;
    size_t num_tags;
    size_t max_tags;
    char * text;
} Zettel;

typedef struct {
    Zettel ** array;

    size_t size;
    size_t used;
} Zettel_Array;

void init_zettels(Zettel_Array * zets, size_t init_size)
{
    zets->used = 0;
    zets->size = init_size;
    zets->array = (Zettel **) malloc(init_size * sizeof(Zettel*));
}

size_t append_zettel(Zettel_Array * zettels, const char * text)
{
    if (zettels->used == zettels->size)
    {
        if(DEBUG)
            printf("increasing tag array\nOld size:%zu\n", zettels->size);
        zettels->size *= 2;
        if(DEBUG)
            printf("new size: %zu\n", zettels->size);
        zettels->array = (Zettel**) realloc(zettels->array, zettels->size * sizeof(Zettel*));
    }
    Zettel * zet = (Zettel*) malloc(sizeof(Zettel));
    zet->text = (char * ) malloc(strlen(text) * sizeof(char));
    strcpy(zet->text, text);
    zet->max_tags=1;
    zet->num_tags=0;
    zet->tags = (size_t*) malloc(zet->max_tags * sizeof(size_t));
    size_t zet_id = zettels->used;
    zettels->array[zet_id] = zet;
    zettels->used ++;
    return zet_id;
}

void tag_zettel(Tag_Array * tags, Zettel_Array * zets, size_t tag_id, size_t zettel_id)
{
    Tag * tag = tags->array[tag_id];
    if (tag->num_zettel == tag->max_zettel){
        tag->max_zettel *= 2;
        tag->zettel = (size_t*) realloc(tag->zettel, tag->max_zettel * sizeof(size_t));
    }
    tag->zettel[tag->num_zettel] = zettel_id;
    tag->num_zettel ++;

    Zettel * zet = zets->array[zettel_id];
    if(zet->num_tags == zet->max_tags){
        zet->max_tags *= 2;
        zet->tags = (size_t*) realloc(zet->tags, zet->max_tags * sizeof(size_t));
    }
    zet->tags[zet->num_tags] = tag_id;
    zet->num_tags ++;
}

void print_zettel(Zettel_Array * zettel)
{
    for(size_t i = 0; i<zettel->used; i++)
    {
        printf("Zettel %zu: %s\nNumber of Tags: %zu\n", i, zettel->array[i]->text, zettel->array[i]->num_tags);
    }
}

void read_zettel(Tag_Array * tags, Zettel_Array * zets, const char * fullfile)
{
    char fbuffer[MAX_NOTE_SIZE];
    char zettel_content[MAX_NOTE_SIZE];
    zettel_content[0] = 0;


    // Read content
    FILE * fp = fopen(fullfile, "r");
    while(fgets(fbuffer, sizeof(fbuffer), fp))
    {
        if(fbuffer[0] == '#')
        {
            continue;
        }
        strcat(zettel_content, fbuffer);
    }
    fclose(fp);
    size_t cend = strlen(zettel_content)-1;
    while(zettel_content[cend] == '\n')
    {
        zettel_content[cend] = 0;
        cend --;
    }
    size_t zet = append_zettel(zets, zettel_content);

    // Read Tags
    size_t tag;
    fp = fopen(fullfile, "r");
    while(fgets(fbuffer, sizeof(fbuffer), fp))
    {
        if(fbuffer[0] != '#')
        {
            continue;
        }
        fbuffer[strlen(fbuffer) -1 ] = 0; // remove newline
        strcpy(zettel_content, fbuffer+1); // copy without first character (#)
        tag = find_tag(tags, zettel_content);
        if(tag == SIZE_MAX)
        {
            append_tag(tags, zettel_content);
            tag = find_tag(tags, zettel_content);
        }
        tag_zettel(tags, zets, tag, zet);
        
    }
    fclose(fp);
}

void read_zettel_from_folder(Tag_Array * tags, Zettel_Array * zets, const char * folder)
{
    DIR *d;
    struct dirent *dir;
    char ** zet_names = NULL;
    size_t zets_in_folder = 0;
    size_t current_zet = 0;

    // count the number of existing zettels
    d = opendir(zettelkasten_path);
    if(d){
        while( (dir = readdir(d)) != NULL ) {
            if(dir->d_name[0] != '.'){
                zets_in_folder ++;
            }
        }
        closedir(d);
    }
    else
    {
        printf("could not search folder\n");
        exit(1);
    }

    zet_names = (char **) malloc(zets_in_folder * sizeof(char*));
    
    //actually read the zettel names
    d = opendir(zettelkasten_path);
    if(d){
        while( (dir = readdir(d)) != NULL ) {
            if(dir->d_name[0] != '.'){
                zet_names[current_zet] = (char*) malloc((strlen(dir->d_name)+1)*sizeof(char));
                strcpy(zet_names[current_zet], dir->d_name);
                current_zet++;
            }
        }
        closedir(d);
    }
    else
    {
        printf("could not search folder\n");
        exit(1);
    }

    // load the zettel
    // TODO Optimize, move malloc and free out of for loop 
    for(size_t i = 0; i<zets_in_folder; i++)
    {
        char * fullpath = malloc((strlen(folder) + 1 + strlen(zet_names[i] + 1)) * sizeof(char));
        fullpath[0]=0;
        strcat(fullpath, folder);
        strcat(fullpath, "/");
        strcat(fullpath, zet_names[i]);
        
        read_zettel(tags, zets, fullpath);

        free(fullpath);
    }


    // job is done, now free the zettel name space
    for(size_t i = 0; i<zets_in_folder; i++)
    {
        free(zet_names[i]);
    }
    free(zet_names);
} 

void browse_tag(Tag_Array * tags, Zettel_Array * zets, const char * tag_str)
{
    size_t tag_id = find_tag(tags, tag_str);
    if(tag_id == SIZE_MAX)
    {
        printf("No entries for %s found\n", tag_str);
        return;
    }
    Tag * tag = tags->array[tag_id];
    Zettel * zet = NULL;

    size_t nz = tag->num_zettel;
    int zet_id;
    size_t ztag_id;
    Tag * ztag;
    for(size_t z = 0; z<nz; z++)
    {
        zet_id = tag->zettel[z];
        zet = zets->array[zet_id];
        printf("Zettel %zu of %zu:\n", z+1, nz);
        printf("%s\nTags: ", zet->text);

        // print tags
        for(size_t t = 0; t<zet->num_tags; t++)
        {
            ztag_id = zet->tags[t];
            ztag = tags->array[ztag_id];
            printf("[%s] ", ztag->title);
        }
        printf("\n\n");

    }
}

typedef enum
{
    TAG_CHOICE
} C_Fsm;

typedef struct{
    int running;
    int sx, sy;
    C_Fsm fsm;
    int cursor_y;
    int tag_window;

} C_State;

int c_init(C_State * c_state)
{
    initscr();
    getmaxyx(stdscr, c_state->sy, c_state->sx);
    curs_set(false);
    c_state->running = true;
    c_state->fsm = TAG_CHOICE;
    c_state->cursor_y = 0;
    c_state->tag_window = 0;

    return 0;
}

int c_update(C_State * c_state, Tag_Array * tags, Zettel_Array * zets)
{
    char c = getch();

    switch(c_state->fsm)
    {
        case TAG_CHOICE:
            switch(c)
            {
                case 'j':
                    c_state->cursor_y ++;
                    if(c_state->cursor_y == c_state->sy)
                    {
                        c_state->cursor_y = c_state->sy-1;
                        c_state->tag_window++;
                        if(c_state->tag_window > tags->used-c_state->sy) c_state->tag_window = tags->used-c_state->sy;
                    }
                    break;

                case 'k':
                    c_state->cursor_y--;
                    if(c_state->cursor_y == -1)
                    {
                        c_state->cursor_y = 0;
                        c_state->tag_window --;
                        if(c_state->tag_window == -1) c_state->tag_window = 0;
                    }
                    break;
                case 'G':
                    c_state->cursor_y = c_state->sy-1;
                    c_state->tag_window = tags->used-c_state->sy;
                    break;
                case 'g':
                    if(getch() == 'g')
                    {
                        c_state->cursor_y = 0;
                        c_state->tag_window = 0;
                    }
                    break;
                case '/':;
                    char searchterm[64];
                    mvgetnstr(10, 10, searchterm, 63);
                    break;


            }
            break;
    }

    
    if ( c == 'q' ) c_state->running = false;
    return 0;
}

int c_render(C_State * c_state, Tag_Array * tags, Zettel_Array * zets)
{
    clear();
    switch(c_state->fsm)
    {
        case TAG_CHOICE:
            mvaddstr(c_state->cursor_y, 0, "=>");
            for(int y = 0; y < tags->used; y++)
            {
                wchar_t tag[100];
                swprintf(tag, 100, L"%hs", tags->array[y]->title);

                //mvaddwstr(y-c_state->tag_window, 2, tag);
                mvaddstr(y-c_state->tag_window, 2, tags->array[y]->title);
            }
            break;
    }
    refresh();
    return 0;
}

int c_exit()
{
    endwin();
    return 0;
}
int interactive()
{
    printf("prepare for interactive mode!");

    Tag_Array tags;
    Zettel_Array zets;
    init_tags(&tags, 1);
    init_zettels(&zets, 1);
    read_zettel_from_folder(&tags, &zets, zettelkasten_path);

    C_State c_state;
    c_init(&c_state);
    while(c_state.running)
    {
        c_render(&c_state, &tags, &zets);
        c_update(&c_state, &tags, &zets);
    }
    c_exit();

}

int main(int argc, char * argv[])
{

    Cini * cini = cini_init();

    {
        char settings_folder[1000];
        const char *homedir;
        if ((homedir = getenv("HOME")) == NULL) {
            homedir = getpwuid(getuid())->pw_dir;
        }
        strcpy(settings_folder, homedir);
        strcpy(settings_folder+strlen(settings_folder), "/.config/zk/zk.conf");
        cini_read(settings_folder, cini);
    }

    
    setlocale(LC_ALL, "");

        

    zettelkasten_path = cini_get("general", "zkpath", cini);
    editor_command = cini_get("general", "editor_command", cini);



    char command[42];
    char tag_str[200];
    if(argc < 2)
    {
        printf("Welcome to Zettelkasten!\n\nCommands:\n");
        printf("add - add Zettel\n");
        printf("list - list all Zettel\n");
        printf("tags - list all tags\n");
        printf("browse [tag] - browse all zettel tagged with [tag]\n");
        printf("build - update the tag list\n");
        return 0;
    }

    strncpy(command, argv[1], 42);

    if(strcmp(command, "add") == 0)
    {
        add_zettel();
    }
    else if(strcmp(command, "list") == 0)
    {
        print_zettel_folder();
    }
    else if(strcmp(command, "tags") == 0)
    {
        printf("Available tags:\n");
        Tag_Array tags;
        Zettel_Array zets;
        init_tags(&tags, 1);
        init_zettels(&zets, 1);
        read_zettel_from_folder(&tags, &zets, zettelkasten_path);
        print_tags(&tags);
    }
    else if(strcmp(command, "browse") == 0)
    {
        Tag_Array tags;
        Zettel_Array zets;
        init_tags(&tags, 1);
        init_zettels(&zets, 1);
        if (argc != 3)
        {
            printf("Usage: zk browse [tag]\n");
            exit(0);
        }

        read_zettel_from_folder(&tags, &zets, zettelkasten_path);
        strcpy(tag_str, argv[2]);
        browse_tag(&tags, &zets, tag_str);
    }
    else if(strcmp(command, "test") == 0)
    {
        Tag_Array tags;
        Zettel_Array zets;
        init_tags(&tags, 1);
        init_zettels(&zets, 1);
        append_tag(&tags, "test");
        append_tag(&tags, "test2");
        append_tag(&tags, "test3");
        append_tag(&tags, "test4");
        append_tag(&tags, "test5");

        size_t zet = append_zettel(&zets, "This is a test entry for 3 and 5");
        tag_zettel(&tags, &zets, find_tag(&tags, "test3"), zet);
        tag_zettel(&tags, &zets, find_tag(&tags, "test5"), zet);

        zet = append_zettel(&zets, "This is a test_entry for 2 and 5");
        tag_zettel(&tags, &zets, find_tag(&tags, "test2"), zet);
        tag_zettel(&tags, &zets, find_tag(&tags, "test5"), zet);
        printf("\n");
        print_tags(&tags);
        printf("\n");
        print_zettel(&zets);
    }
    else if(strcmp(command, "test2") == 0)
    {
        Tag_Array tags;
        Zettel_Array zets;
        init_tags(&tags, 1);
        init_zettels(&zets, 1);
        read_zettel_from_folder(&tags, &zets, zettelkasten_path);
        printf("\n");
        print_tags(&tags);
        printf("\n");
        print_zettel(&zets);
    }
    else if(strcmp(command, "interactive") == 0)
    {
        interactive();
    }

    cini_free(cini);
    return 0;
}
