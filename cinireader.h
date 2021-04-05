#ifndef CINIREADER_H
#define CINIREADER_H

typedef struct Cini_Entry Cini_Entry;
typedef struct Cini_Section Cini_Section;

struct Cini_Entry{
    char * key;
    char * value;
    Cini_Entry * next;

};

struct Cini_Section{
    char * title;
    Cini_Entry * first_entry;
    Cini_Section * next;
};

typedef struct {
    Cini_Section * first_section;
} Cini;

Cini * cini_init();
int cini_read(const char * filepath, Cini * cini);
const char * cini_get(const char * section, const char * key, Cini * cini);
void cini_print(Cini * cini);
void cini_free(Cini * cini);

#endif
