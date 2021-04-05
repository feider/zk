#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "cinireader.h"

#define MAX_LINE_LENGTH 10000

Cini * cini_init()
{
    Cini * cini = (Cini*) malloc(sizeof(Cini));
    cini->first_section = NULL;
    return cini;
}

int find_char_pos(char c, const char * str)
{
    for(int i = 0; i<strlen(str); i++)
    {
        if(c == str[i])
            return i;
    }
    return -1;
}

int find_first_non(char c, const char * str)
{
    for(int i = 0; i<strlen(str); i++)
    {
        if(c != str[i])
            return i;
    }
    return -1;
}

int cini_read(const char * filepath, Cini * cini)
{
    FILE * file;
    file = fopen(filepath, "r");
    if(file == NULL)
        return 1;

    ssize_t line_size;
    size_t linebuf_size = MAX_LINE_LENGTH;
    char * linebuf = NULL;


    char keybuf[MAX_LINE_LENGTH];
    char valbuf[MAX_LINE_LENGTH];
    char secbuf[MAX_LINE_LENGTH];

    line_size = getline(&linebuf, &linebuf_size, file);
    linebuf[line_size] = '\0';

    int secbeg, secend, eq;

    Cini_Section * csec = NULL;
    Cini_Entry * centry = NULL;

    while(line_size>=0)
    {
        secbeg = find_char_pos('[', linebuf);
        secend = find_char_pos(']', linebuf);
        eq = find_char_pos('=', linebuf);

        // Case 1: we have found section
        if( (secbeg == 0) && (secend > 0) )
        {
            // get the segment title
            strncpy(secbuf, linebuf+1, secend-1);
            secbuf[secend-1] = '\0';

            // create a new segment
            if(csec == NULL)
            {
                csec = (Cini_Section*) malloc(sizeof(Cini_Section));
                cini->first_section = csec;
            }
            else
            {
                csec->next = (Cini_Section*) malloc(sizeof(Cini_Section));
                csec = csec->next;
            }
            csec->next = NULL;

            // copy the name
            csec->title = malloc((strlen(secbuf)+1) * sizeof(char) );
            strcpy(csec->title, secbuf);

            // reset the current entry
            centry = NULL;
        }

        // Case 2: we have found a new entry (and already have a section present)
        else if((eq > 0) && (csec != NULL))
        {
            // copy key and value to buffers
            strncpy(keybuf, linebuf, eq);
            keybuf[eq] = '\0';

            int l_val = strlen(linebuf+eq)-2;
            strncpy(valbuf, linebuf+eq+1, l_val);
            valbuf[l_val] = '\0';

            // create a new entry
            if(centry == NULL)
            {
                centry = (Cini_Entry*) malloc(sizeof(Cini_Entry));
                csec->first_entry = centry;
            }
            else
            {
                centry->next = (Cini_Entry*) malloc(sizeof(Cini_Entry));
                centry = centry->next;
            }
            centry->next = NULL;

            // copy key and value
            centry->key = malloc((strlen(keybuf)+1) * sizeof(char) );
            strcpy(centry->key, keybuf);

            centry->value = malloc((strlen(valbuf)+1) * sizeof(char) );
            strcpy(centry->value, valbuf);
        }

        line_size = getline(&linebuf, &linebuf_size, file);
        linebuf[line_size] = '\0';
    }


    free(linebuf);
    fclose(file);

}

const char * cini_get(const char * section, const char * key, Cini * cini)
{
    Cini_Section * csec = NULL;
    Cini_Entry * centry = NULL;

    // traverse sections
    for(csec = cini->first_section; csec != NULL; csec = csec->next)
    {
        if(strcmp(csec->title, section) == 0)
            // traverse entries
            for(centry = csec->first_entry; centry != NULL; centry = centry->next)
            {
                if(strcmp(centry->key, key) == 0)
                    return centry->value;
            }
    }
    return NULL;
}


void cini_print(Cini * cini)
{
    Cini_Section * csec = NULL;
    Cini_Entry * centry = NULL;

    // traverse sections
    for(csec = cini->first_section; csec != NULL; csec = csec->next)
        // traverse entries
        for(centry = csec->first_entry; centry != NULL; centry = centry->next)
        {
            printf("[%s]\t%s\t=\t%s\n", csec->title, centry->key, centry->value);
        }

}

void cini_free(Cini * cini)
{
    Cini_Section * csec = cini->first_section;
    Cini_Entry * centry = NULL;


    // traverse sections
    while(csec != NULL)
    {
        free(csec->title);

        centry = csec->first_entry;
        while(centry != NULL)
        {
            free(centry->key);
            free(centry->value);

            Cini_Entry * e = centry;
            centry = centry->next;
            free(e);
        }

        Cini_Section * s = csec;
        csec = csec->next;
        free(s);

    }

    free(cini);

}
