#include "stdio.h"
#include "libc.h"
#include "elk.h"
#include "gengo.h"
#include "shell.h"

static jsval_t js_print(struct js *js,jsval_t *args,int nargs){
  for(int i=0;i<nargs;i++){
    if(i)putchar(' ');
    if(js_type(args[i])==JS_STR){
      size_t len;
      char *str=js_getstr(js,args[i],&len);
      for(size_t j=0;j<len;j++)putchar(str[j]);
    }else{
      const char *str=js_str(js,args[i]);
      while(*str)putchar(*str++);
    }
  }
  putchar('\n');
  return js_mkundef();
}

static int input(char *source,int size){
  int ch,len=0;
  printf("> ");

  while((ch=getchar())!='\r'&&ch!='\n'){
    if((ch==8||ch==127)&&len)
      printf("\b \b"),len--;
    else if(ch>=32&&ch<127&&len<size-1)
      source[len++]=(char)ch,putchar((char)ch);
  }

  putchar('\n');
  source[len]=0;
  return len;
}

void main(void){
  static unsigned char memory[16*1024*1024],heap[1024*1024];
  char source[256*1024];

  heap_init(heap,sizeof(heap));
  struct js *js=js_create(memory,sizeof(memory));

#define SET(name,function) js_set(js,js_glob(js),name,js_mkfun(function))
  SET("print",js_print);
#undef SET

  jsval_t result=js_eval(js,(char *)gengo_js,gengo_js_len);

  if(js_type(result)==JS_ERR){
    printf("%s\n",js_str(js,result));
    exit(1);
  }

  js_set(js,js_glob(js),"source",
         js_mkstr(js,(char *)shell_gengo,shell_gengo_len));

  result=js_eval(js,"program(source);",~0U);

  if(js_type(result)==JS_ERR){
    printf("%s\n",js_str(js,result));
    exit(1);
  }

  printf("\nGengo lang REPL (try 1+2, type exit to quit)\n");

  for(;;){
    int len=input(source,sizeof(source));
    if(!len)continue;

    if(len==4&&!memcmp(source,"exit",4))
      exit(0);

    js_set(js,js_glob(js),"source",js_mkstr(js,source,len));
    result=js_eval(js,"program(source);",~0U);

    if(js_type(result)==JS_ERR)
      printf("%s\n",js_str(js,result));
  }
}