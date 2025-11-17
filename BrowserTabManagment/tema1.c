// BAHACIU RAISA-GEORGIANA 313CC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct page
{
    int id;
    char url[50];
    char *description;
} page;

typedef struct stack
{
    page *currentPage;
    struct stack *nextPage;
} *stack;

typedef struct tab
{
    int id;
    page *currentPage;
    stack backwardStack;
    stack forwardStack;
} *tab;

typedef struct tabList
{
    tab currentTab;
    struct tabList *previousTab;
    struct tabList *nextTab;
} *tabList;

typedef struct browser
{
    tab currentTab;
    tabList list;
} *browser;

page *findPage(page Pages[], int id, int nr_pages)
{
    page *found = NULL;
    for (int i = 0; i <= nr_pages; i++) {
        if (id == Pages[i].id)
        {
            found = &Pages[i];
        }
    }
    return found;
}
void remove_newline(char *s)
{
    size_t len = strlen(s);
    if (len > 0 && s[len - 1] == '\n')
    {
        s[len - 1] = '\0';
    }
}

// functia adauga un tab in lista de taburi a browserului
void add_in_browser_list(browser b, tab t)
{ // cazul in care lita e goala si doar adaugam un nod pe care
    // il conectam circular la el insusi
    if (b->list == NULL)
    {
        tabList tab_aux = (tabList)malloc(sizeof(struct tabList));
        tab_aux->currentTab = t;
        b->list = (tabList)malloc(sizeof(struct tabList));
        b->list->currentTab = t;
        b->list->nextTab = tab_aux;
        b->list->previousTab = tab_aux;
        tab_aux->nextTab = tab_aux;
        tab_aux->previousTab = tab_aux;
        b->list = tab_aux;
    }
    // adaugare clasica in lista dublu inlantuita circulara
    else {
        tabList tab_aux = (tabList)malloc(sizeof(struct tabList));
        tab_aux->currentTab = t;
        tabList contor_aux = b->list;
        while (contor_aux->nextTab->currentTab->id != -1)
            contor_aux = contor_aux->nextTab;
        tab_aux->nextTab = contor_aux->nextTab;
        contor_aux->nextTab->previousTab = tab_aux;
        tab_aux->previousTab = contor_aux;
        contor_aux->nextTab = tab_aux;
        b->currentTab = t;
        b->list = tab_aux;
    }
}

// functia returneaza un tab nou initializat cu pagina trimisa
tab create_new_tab(page *Pages, int *last_id)
{
    tab new_tab = (tab)malloc(sizeof(struct tab));
    new_tab->backwardStack = NULL;
    new_tab->forwardStack = NULL;
    new_tab->currentPage = Pages;
    new_tab->id = *last_id;
    (*last_id)++;
    return new_tab;
}

// functia creeaza un tab nou in browser folosid ultimul id nefolosit
//  pana in momentul respectiv
void NEW_TAB(browser b, page *Pages, int *last_id)
{
    tab tab_to_add = create_new_tab(Pages, last_id);
    b->currentTab = tab_to_add;
    add_in_browser_list(b, tab_to_add);
}

// functia inchide tab-ul curent
void CLOSE(browser b, FILE *f)
{ // verificam daca tabul curent este tabul 0
    if (b->list->currentTab->id == 0)
    {
        fprintf(f, "403 Forbidden\n");
    } else {
        tabList contor_aux = b->list;
        while (contor_aux->currentTab->id != b->currentTab->id)
            contor_aux = contor_aux->nextTab;
        b->list = b->list->previousTab;
        b->currentTab = contor_aux->previousTab->currentTab;
        contor_aux->previousTab->nextTab = contor_aux->nextTab;
        contor_aux->nextTab->previousTab = contor_aux->previousTab;
    }
}

// functia descide tabul cu id ul primit
void OPEN(browser b, int id_to_open, FILE *f)
{
    if (b->currentTab->id != id_to_open)
    { /*folosim faptul ca daca am ajuns de 2 ori la santinela clar am parcurs o
          data toate taburile si e clar ca nu am gasit tabul cerut*/
        int passed_santinela = 0;
        tabList contor_aux = b->list;
        while (contor_aux->currentTab->id != id_to_open && passed_santinela < 2) {
            if (contor_aux->currentTab->id == -1)
            {
                passed_santinela++;
            }
            contor_aux = contor_aux->nextTab;
        }
        // contor_aux este tabul cu id ul cerut
        if (passed_santinela != 2)
        {
            b->currentTab = contor_aux->currentTab;
            b->list = contor_aux;
        }
        // passed_santinela==2 si deci nu am gasit tabul
        else {
            fprintf(f, "403 Forbidden\n");
        }
    }
}

void PRINT(browser b, FILE *f)
{ // retinem b->list in aux pt a nu il modifica
    tabList aux = b->list;
    int first = 1; // folosim first ca sa nu se adauge spatiu la inceputul liniei
    do {
        if (aux->currentTab->id != -1) // se sare peste santinela
        {
            if (!first)
                fprintf(f, " ");
            fprintf(f, "%d", aux->currentTab->id); // printeaza id urile
            first = 0;
        }
        aux = aux->nextTab;
    } while (aux->currentTab->id != b->currentTab->id);

    fprintf(f, "\n%s\n", b->currentTab->currentPage->description);
}

void NEXT(browser b)
{ // sarim peste santinela daca suntem la final de b->list
    if (b->list->nextTab->currentTab->id == -1)
    {
        b->list = b->list->nextTab->nextTab;
        b->currentTab = b->list->currentTab;
    } else {
        b->list = b->list->nextTab;
        b->currentTab = b->list->currentTab;
    }
}

void PREV(browser b)
{ // sarim peste sandinela daca suntem in primul tab de dupa santinela
    if (b->list->previousTab->currentTab->id == -1)
    {
        b->list = b->list->previousTab->previousTab;
        b->currentTab = b->list->currentTab;
    } else {
        b->list = b->list->previousTab;
        b->currentTab = b->list->currentTab;
    }
}

void NEW_PAGE(tab t, page Pages[], int id_to_add, int pages_ids, FILE *f)
{ // verificam daca id ul exista in id urile paginilor
    page *found = findPage(Pages, id_to_add, pages_ids);
    if (found == NULL)
    {
        fprintf(f, "403 Forbidden\n");
    }
    // daca t->currentPage nu e null atunci trebuie sa facem toate mutarile
    //  aferente nu doar sa punem pagina in t->currentPage
    else if (t->currentPage != NULL) {
        stack aux = (stack)malloc(sizeof(struct stack));

        aux->currentPage = t->currentPage;
        aux->nextPage = t->backwardStack;
        t->backwardStack = aux;
    }
    while (t->forwardStack != NULL) {

        stack tmp = t->forwardStack;
        t->forwardStack = t->forwardStack->nextPage;
        free(tmp);
    }
    t->currentPage = found;
}

void BACKWARD(browser b, FILE *f)
{ // daca nu avem taburi in backwardstack nu putem face operatia
    if (b->currentTab->backwardStack == NULL || b->currentTab->currentPage == NULL)
    {
        fprintf(f, "403 Forbidden\n");
        return;
    } else {
        // salvam pagina curenta in aux
        stack aux = (stack)malloc(sizeof(struct stack));
        aux->currentPage = b->currentTab->currentPage;
        aux->nextPage = b->currentTab->forwardStack;
        b->currentTab->forwardStack = aux;
        // mergem la pagina anterioara
        stack going_to = b->currentTab->backwardStack;
        b->currentTab->currentPage = going_to->currentPage;
        b->currentTab->backwardStack = going_to->nextPage;
        free(going_to);
    }
}

void FORWARD(browser b, FILE *f)
{ // daca nu avem taburi in fowardstack nu putem face operatia
    if (b->currentTab->forwardStack == NULL || b->currentTab->currentPage == NULL)
    {
        fprintf(f, "403 Forbidden\n");
        return;
    } else {
        // salvam pagina curenta in aux
        stack aux = (stack)malloc(sizeof(struct stack));
        aux->currentPage = b->currentTab->currentPage;
        // aux pointeaza la backwardstack
        aux->nextPage = b->currentTab->backwardStack;
        // aux devine noul backwardstack adaugand fosta pagina curenta
        b->currentTab->backwardStack = aux;
        // salvam pagina din forward in going_to pt claritate
        stack going_to = b->currentTab->forwardStack;
        b->currentTab->currentPage = going_to->currentPage;
        b->currentTab->forwardStack = going_to->nextPage;
        free(going_to);
    }
}

tab find_tab(browser *b, int id_to_find, FILE *f)
{
    int passed_santinela = 0;
    tabList contor_aux = (*b)->list;
    while (passed_santinela < 2) {
        // am ajuns la santinela
        if (contor_aux->currentTab->id == -1)
            passed_santinela++;
        // am gasit tabul cautat si il returnam
        if (contor_aux->currentTab->id == id_to_find)
            return contor_aux->currentTab;
        contor_aux = contor_aux->nextTab;
    }
    // am iesit din while si nu am gasit tabul cautat
    fprintf(f, "403 Forbidden\n");
    return NULL;
}

void push(stack *s, page *p)
{
    stack aux = (stack)malloc(sizeof(struct stack));
    aux->currentPage = p;
    aux->nextPage = *s;
    *s = aux;
}

page *pop(stack *s)
{
    if (*s == NULL)
    {
        return NULL;
    }
    stack aux = *s;
    page *popped = aux->currentPage;
    *s = aux->nextPage;
    free(aux);
    return popped;
}

void print_stiva(stack s, FILE *f)
{ // printam stiva trimisa in ordinea inlantuirii nodurilor
    stack aux = s;
    while (aux != NULL) {
        fprintf(f, "%s\n", aux->currentPage->url);
        aux = aux->nextPage;
    }
}

void forward_reversed(stack *s, FILE *f)
{
    stack aux = NULL;
    // se creeaza stackul aux care va fi stack ul initial invers
    while (*s != NULL) {
        page *p = pop(s);
        push(&aux, p);
    }

    print_stiva(aux, f);
    // se compune din nou stackul initial inversand aux prin aceeasi logica
    while (aux != NULL) {
        page *p = pop(&aux);
        push(s, p);
    }
}

void PRINT_HISTORY(browser *b, int id_to_find, FILE *f)
{ // se printeaza tabul curent
    tab tab_to_print = find_tab(b, id_to_find, f);
    if (tab_to_print != NULL)
    {
        forward_reversed(&tab_to_print->forwardStack, f);
        fprintf(f, "%s\n", tab_to_print->currentPage->url);
        print_stiva(tab_to_print->backwardStack, f);
    }
}

browser create_browser(page *Pages, int *last_id)
{ // se aloca memorie browser
    browser my_browser = (browser)malloc(sizeof(struct browser));
    // se adauga tabul default cu pages[0](trimis in functie)
    tab new_tab_aux = create_new_tab(Pages, last_id);
    my_browser->currentTab = new_tab_aux;
    my_browser->list = NULL;
    // se initializaza tabul santinela
    tab santinela = (tab)malloc(sizeof(struct tab));
    santinela->backwardStack = NULL;
    santinela->forwardStack = NULL;
    santinela->currentPage = NULL;
    santinela->id = -1;
    // se adauga santinala si tabul default in lista de taburi
    add_in_browser_list(my_browser, santinela);
    add_in_browser_list(my_browser, my_browser->currentTab);
    return my_browser;
}

void identify_command(char command[], int *last_id, browser B, page Pages[], int nr_pages, FILE *fout)
{ // identificam comanda si apelam functia corespunzatoare

    if (!strcmp(command, "NEW_TAB"))
        NEW_TAB(B, &Pages[0], last_id);
    if (!strcmp(command, "PRINT"))
        PRINT(B, fout);
    if (!strcmp(command, "CLOSE"))
        CLOSE(B, fout);
    if (!strncmp(command, "OPEN ", 5))
    {
        char aux[3];
        strcpy(aux, command + 5);
        int id_to_add = atoi(aux);
        OPEN(B, id_to_add, fout);
    }
    if (!strcmp(command, "PREV"))
        PREV(B);
    if (!strcmp(command, "NEXT"))
        NEXT(B);
    if (!strcmp(command, "BACKWARD"))
        BACKWARD(B, fout);
    if (!strcmp(command, "FORWARD"))
        FORWARD(B, fout);
    if (!strncmp(command, "PAGE ", 5))
    {
        char aux[3];
        strcpy(aux, command + 5);
        int id_to_add = atoi(aux);

        NEW_PAGE(B->currentTab, Pages, id_to_add, nr_pages, fout);
    }
    if (!strncmp(command, "PRINT_HISTORY ", 14))
    {
        char aux[3];
        strcpy(aux, command + 14);
        int id_to_find = atoi(aux);
        PRINT_HISTORY(&B, id_to_find, fout);
    }
}

void read_file(int *nr_pages, page Pages[], int *last_id, browser B, FILE *fout)
{
    FILE *f = fopen("tema1.in", "r");
    // citim numarul de pagini
    char aux[4];
    fgets(aux, sizeof(aux), f);
    *nr_pages = atoi(aux);
    // initializam pagina 0 default
    Pages[0].id = 0;
    strcpy(Pages[0].url, "https://acs.pub.ro/");
    char desc_aux[200];
    strcpy(desc_aux, "Computer Science");
    Pages[0].description = (char *)malloc((strlen(desc_aux) + 1) * sizeof(char));
    strcpy(Pages[0].description, desc_aux);
    // verificam ca avem pagini de citit
    if (nr_pages)
    { // citim fiecare pagina
        for (int i = 1; i <= *nr_pages; i++) {
            // id
            char aux[4];
            fgets(aux, sizeof(aux), f);
            Pages[i].id = atoi(aux);
            // url
            fgets(Pages[i].url, sizeof(Pages[i].url), f);
            remove_newline(Pages[i].url);
            // description
            char desc_aux[200];
            fgets(desc_aux, sizeof(desc_aux), f);
            remove_newline(desc_aux);
            Pages[i].description = (char *)malloc((strlen(desc_aux) + 1) * sizeof(char));
            strcpy(Pages[i].description, desc_aux);
        }
    }
    // citim nr de comenzi
    char tmp[4];
    fgets(tmp, sizeof(tmp), f);
    int nr_commands = atoi(tmp);
    char command[50];
    // citim comenzile
    for (int i = 0; i < nr_commands; i++) {
        if (fgets(command, sizeof(command), f) != NULL)
        {
            remove_newline(command);
            // facem operatia aferenta comenzii
            identify_command(command, last_id, B, Pages, *nr_pages, fout);
        }
    }
    fclose(f);
}

int main()
{ // initializam vectorul de pagini
    page Pages[51];
    int nr_pages;
    int id_nu_pointer = 0;
    int *last_id = &id_nu_pointer;
    browser B = create_browser(&Pages[0], last_id);
    // deschidem fisierul de output
    FILE *fout = fopen("tema1.out", "w");
    read_file(&nr_pages, Pages, last_id, B, fout);
    fclose(fout);
    return 0;
}