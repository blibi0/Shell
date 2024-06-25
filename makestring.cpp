#include "smsh.h"

void freelist(char **list);
void *erealloc(void *p,size_t n);
char *newstr(char *s,int l);

enum states   { NEUTRAL, WANT_THEN, THEN_BLOCK ,ELSE_BLOCK};
enum results  { SUCCESS, FAIL };

static int if_state  = NEUTRAL;
static int if_result = SUCCESS;
static int last_stat = 0;

char *makestring(char *buf){
    char *cp;

    buf[strlen(buf)-1] = '\0';
    cp = (char *)malloc(strlen(buf)+1);
    if (cp == NULL){
        fprintf(stderr,"no memory\n");
        exit(1);
    }
    strncpy(cp,buf,sizeof(cp));
    return cp;
}


char *next_cmd(const char *prompt,FILE *fp){
    char *buf;
    int bufspace = 0;
    int pos = 0;
    int c;

    std::cout<<prompt;
    while ((c = getc(fp)) != EOF) {
        if(pos + 1 >= bufspace){
            if(bufspace == 0)
                buf = reinterpret_cast<char*>(emalloc(BUFSIZ));
            else
                buf = reinterpret_cast<char*>(erealloc(buf,bufspace + BUFSIZ));
            bufspace += BUFSIZ;
        }
        if(c == '\n')
            break;
        buf[pos++] = c;
    }
    if(c == EOF && pos == 0)
        return NULL;
    buf[pos] = '\0';
    return buf;
}

#define is_delim(x) ((x) == ' '||(x) == '\t')

char **splitline(char *line){
    char **args;
    int spots = 0;
    int bufspace = 0;
    int argnum = 0;
    char *cp = line;
    char *start;
    int len;

    if(line == NULL)
        return NULL;

    args = reinterpret_cast<char**>(emalloc(BUFSIZ));
    bufspace = BUFSIZ;
    spots = BUFSIZ/sizeof(char *);

    while(*cp != '\0'){
        while (is_delim(*cp)) {
            cp++;
        }
        if(*cp == '\0')
            break;

        if(argnum +1 >= spots){
            args =reinterpret_cast<char**>(erealloc(args,bufspace+BUFSIZ));
            bufspace += BUFSIZ;
            spots += BUFSIZ/sizeof(char *);
        }

        start = cp;
        len = 1;
        while(*++cp != '\0' && !(is_delim(*cp)) ){
            len++;
        }
        args[argnum++] = newstr(start,len);
    }
    args[argnum] = NULL;
    return args;
}

char *newstr(char *s,int l){
    char *rv = reinterpret_cast<char*>(emalloc(l+1));
    rv[l] = '\0';
    strncpy(rv,s,l);
    return rv;
}

void freelist(char **list){
    char **cp = list;
    while(*cp)
        free(*cp++);
    free(list);
}

void *emalloc(size_t n){
    void *rv;
    if((rv = malloc(n)) == NULL)
        fatal("out of memory","",1);
    return rv;
}

void *erealloc(void *p,size_t n){
    void *rv;
    if((rv = realloc(p,n)) == NULL)
        fatal("realloc() failed","",1);
    return rv;
}


// 实现if-else流程控制

int	syn_err(char *);

int ok_to_execute(){
    int	rv = 1;

    if ( if_state == WANT_THEN ){
        err("then expected");
        rv = 0;
    }
    else if ( if_state == THEN_BLOCK && if_result == SUCCESS )
        rv = 1;
    else if ( if_state == THEN_BLOCK && if_result == FAIL )
        rv = 0;
    return rv;
}

int is_control_command(char *s){
    return (strcmp(s,"if")==0 || strcmp(s,"then")==0 || strcmp(s,"fi")==0);
}


int control_command(char **args){  //处理关键字
    char *cmd = args[0];
    int	rv = -1;

    if( strcmp(cmd,"if")==0 ){
        if ( if_state != NEUTRAL )
            rv = err("if unexpected");
        else {
            last_stat = execute5(args+1);
            if_result = (last_stat == 0 ? SUCCESS : FAIL );
            if_state = WANT_THEN;
            rv = 0;
        }
    }
    else if ( strcmp(cmd,"then")==0 ){
        if ( if_state != WANT_THEN )
            rv = err("then unexpected");
        else {
            if_state = THEN_BLOCK;
            rv = 0;
        }
    }
    else if ( strcmp(cmd,"else")==0 ){
        if ( if_state != THEN_BLOCK )
            rv = err("else unexpected");
        else {
            if_result = ELSE_BLOCK;
            rv = 0;
        }
    }
    else if ( strcmp(cmd,"fi")==0 ){
        if ( if_state != ELSE_BLOCK )
            rv = err("fi unexpected");
        else {
            if_state = NEUTRAL;
            rv = 0;
        }
    }
    else
        fatal("internal error processing:", cmd, 2);
    return rv;
}

int err(const char *msg){
    if_state = NEUTRAL;
    fprintf(stderr,"syntax error: %s\n", msg);
    return -1;
}









