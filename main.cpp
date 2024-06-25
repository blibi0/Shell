#include <QCoreApplication>
#include "smsh.h"

#define MAXARGS 20  //cmdline args
#define ARGLEN 100  //token length
#define DEL_PROMPT ">"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

        char *cmdline,**arglist;
        int result;
        void setup();

        const char *prompt = DEL_PROMPT;
        setup();

        while ((cmdline = next_cmd(prompt,stdin)) != NULL) {
            if((arglist = splitline(cmdline)) != NULL){
                result = execute5(arglist);
                freelist(arglist);
            }
            free(cmdline);
        }

        return a.exec();

}

void setup(){
    signal(SIGINT,SIG_IGN);
    signal(SIGQUIT,SIG_IGN);
}

void fatal(const char *s1,const char *s2,int n){
    std::fprintf(stderr,"Error:%s,%s\n",s1,s2);
    exit(n);
}






