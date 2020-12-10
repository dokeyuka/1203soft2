#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h> // for error catch


// Structure for canvas
typedef struct
{
  int width;
  int height;
  char **canvas;
  char pen;
} Canvas;

// Command 構造体と History構造体
// [*]
typedef struct command Command;
struct command{
  char *str;
  size_t bufsize;
  Command *next;
};

typedef struct
{
  Command *begin;
  size_t bufsize; // [*] : この方が効率的ですね。一部の方から指摘ありました。
} History;

// functions for Canvas type
Canvas *init_canvas(int width, int height, char pen);
void reset_canvas(Canvas *c);
void print_canvas(FILE *fp, Canvas *c);
void free_canvas(Canvas *c);

// display functions
void rewind_screen(FILE *fp,unsigned int line); 
void clear_command(FILE *fp);
void clear_screen(FILE *fp);

// enum for interpret_command results
typedef enum res{ EXIT, NORMAL, COMMAND, UNKNOWN, ERROR} Result;

int max(const int a, const int b);
void draw_line(Canvas *c, const int x0, const int y0, const int x1, const int y1);
Result interpret_command(const char *command, History *his, Canvas *c);
void save_history(const char *filename, History *his);

// [*] list.c のpush_backと同じ
Command *push_command(History *his, const char *str);

int main(int argc, char **argv)
{
  //for history recording
  
  const int bufsize = 1000;

  // [*]
  History his = (History){ .begin = NULL, .bufsize = bufsize};

  int width;
  int height;
  if (argc != 3){
    fprintf(stderr,"usage: %s <width> <height>\n",argv[0]);
    return EXIT_FAILURE;
  } else{
    char *e;
    long w = strtol(argv[1],&e,10);
    if (*e != '\0'){
      fprintf(stderr, "%s: irregular character found %s\n", argv[1],e);
      return EXIT_FAILURE;
    }
    long h = strtol(argv[2],&e,10);
    if (*e != '\0'){
      fprintf(stderr, "%s: irregular character found %s\n", argv[2],e);
      return EXIT_FAILURE;
    }
    width = (int)w;
    height = (int)h;    
  }
  
  char pen = '*';

  FILE *fp;
  char buf[bufsize];
  fp = stdout;
  Canvas *c = init_canvas(width,height, pen);

  fprintf(fp,"\n"); // required especially for windows env
  while(1){
    // [*]
    // hsize はひとまずなし
    // 作る場合はリスト長を調べる関数を作っておく
    print_canvas(fp,c);
    printf("* > ");
    if(fgets(buf, bufsize, stdin) == NULL) break;
    
    const Result r = interpret_command(buf, &his, c);

    if (r == EXIT) break;   
    if (r == NORMAL) {
      // [*]
      push_command(&his,buf);
    }

    rewind_screen(fp,2); // command results
    clear_command(fp); // command itself
    rewind_screen(fp, height+2); // rewind the screen to command input

  }

  clear_screen(fp);
  free_canvas(c);
  fclose(fp);

  return 0;
}

Canvas *init_canvas(int width,int height, char pen)
{
  Canvas *new = (Canvas *)malloc(sizeof(Canvas));
  new->width = width;
  new->height = height;
  new->canvas = (char **)malloc(width * sizeof(char *));

  char *tmp = (char *)malloc(width*height*sizeof(char));//tmpのメモリをキャンバスの面積分確保
  memset(tmp, ' ', width*height*sizeof(char));//tmpのメモリに、キャンバスの面積分空白をセット
  for (int i = 0 ; i < width ; i++){
    new->canvas[i] = tmp + i * height;
  }
  
  new->pen = pen;
  return new;
}

void reset_canvas(Canvas *c)
{
  const int width = c->width;
  const int height = c->height;
  memset(c->canvas[0], ' ', width*height*sizeof(char));
}


void print_canvas(FILE *fp, Canvas *c)
{
  const int height = c->height;
  const int width = c->width;
  char **canvas = c->canvas;
  
  // 上の壁
  fprintf(fp,"+");
  for (int x = 0 ; x < width ; x++)
    fprintf(fp, "-");
  fprintf(fp, "+\n");

  // 外壁と内側
  for (int y = 0 ; y < height ; y++) {
    fprintf(fp,"|");
    for (int x = 0 ; x < width; x++){
      const char c = canvas[x][y];
      fputc(c, fp);
    }
    fprintf(fp,"|\n");
  }
  
  // 下の壁
  fprintf(fp, "+");
  for (int x = 0 ; x < width ; x++)
    fprintf(fp, "-");
  fprintf(fp, "+\n");
  fflush(fp);
}

void free_canvas(Canvas *c)
{
  free(c->canvas[0]); //  for 2-D array free
  free(c->canvas);
  free(c);
}

void rewind_screen(FILE *fp,unsigned int line)
{
  fprintf(fp,"\e[%dA",line);
}

void clear_command(FILE *fp)
{
  fprintf(fp,"\e[2K");
}

void clear_screen(FILE *fp)
{
  fprintf(fp, "\e[2J");
}


int max(const int a, const int b)
{
  return (a > b) ? a : b;
}
void draw_line(Canvas *c, const int x0, const int y0, const int x1, const int y1)
{
  const int width = c->width;
  const int height = c->height;
  char pen = c->pen;
  
  const int n = max(abs(x1 - x0), abs(y1 - y0));
  c->canvas[x0][y0] = pen;
  for (int i = 1; i <= n; i++) {
    const int x = x0 + i * (x1 - x0) / n;
    const int y = y0 + i * (y1 - y0) / n;
    if ( (x >= 0) && (x< width) && (y >= 0) && (y < height))
      c->canvas[x][y] = pen;
  }
}

void draw_rect(Canvas *c, const int x0, const int y0, const int width,const int height){
  char pen = c->pen;
int x, y;
  c->canvas[y0][x0] = pen;
  for(int i = 0; i < height; i++){
    y = y0 + i;
    if(y == y0 || y == y0+height-1){//上下の辺
      for(int j = 1; j < width; j++){
      x = x0 + j;
      c->canvas[y][x] = pen;
      } 
    } else {//左右の辺
      c->canvas[y][x0] = pen;
      c->canvas[y][x0+width-1] = pen;
    }
  }
}


void draw_circle(Canvas *c, const int x0,const int y0, const int r){
  char pen = c->pen;
 
  int x = r;
  int y = 0;
  int F = -2 * r + 3;

  while ( x >= y ) {
    c->canvas[y0+y][x0+x] = pen;
    c->canvas[y0+y][x0-x] = pen;
    c->canvas[y0-y][x0-x] = pen;
    c->canvas[y0-y][x0+x] = pen;
    c->canvas[y0-x][x0+y] = pen;
    c->canvas[y0-x][x0-y] = pen;
    c->canvas[y0+x][x0-y] = pen;
    c->canvas[y0+x][x0+y] = pen;
    c->canvas[y0-x][x0+y] = pen;

    if ( F >= 0 ) {
      x--;
      F -= 4 * x;
    }
    y++;
    F += 4 * y + 2;
  }
}




void save_history(const char *filename, History *his)
{
  const char *default_history_file = "history.txt";
  if (filename == NULL)
    filename = default_history_file;
  
  FILE *fp;
  if ((fp = fopen(filename, "w")) == NULL) {
    fprintf(stderr, "error: cannot open %s.\n", filename);
    return;
  }
  // [*] 線形リスト版
  for (Command *p = his->begin ; p != NULL ; p = p->next){
    fprintf(fp, "%s", p->str);
  }

  fclose(fp);
}

Result interpret_command(const char *command, History *his, Canvas *c)
{
  char buf[his->bufsize];
  strcpy(buf, command);
  buf[strlen(buf) - 1] = 0; // remove the newline character at the end

  const char *s = strtok(buf, " ");
  if (s == NULL){ // 改行だけ入力された場合
    printf("\n");
    return UNKNOWN;
  }
  // The first token corresponds to command
  if (strcmp(s, "line") == 0) {
    int p[5] = {0}; // p[0]: x0, p[1]: y0, p[2]: x1, p[3]: x1 
    char *b[4];
    for (int i = 0 ; i < 4; i++){//b[i]に切り出した文字列を格納(コマンド次の数字が入っていく）
      b[i] = strtok(NULL, " ");//nullで次の文字列を指定（２回目以降のきりだしはこうする）
        if (b[i] == NULL){
	clear_command(stdout);
        printf("the number of point is not enough.\n");
        return ERROR;
      }
    }
    for (int i = 0 ; i < 4 ; i++){//char b[i]に格納された数字をvに代入しp[i]に入れる
      char *e;
      long v = strtol(b[i],&e, 10);
      if (*e != '\0'){
	clear_command(stdout);
	printf("Non-int value is included.\n");
	return ERROR;
      }
      p[i] = (int)v;
    }

    draw_line(c,p[0],p[1],p[2],p[3]);//引数にしてp[i]を代入
    clear_command(stdout);
    printf("1 line drawn\n");
    return NORMAL;
  }
  

  
  // The first token corresponds to command
  if (strcmp(s, "rect") == 0) {
    int p[4] = {0}; // p[0]: x0, p[1]: y0, p[2]: width, p[3]: height
    char *b[4];
    for (int i = 0 ; i < 4; i++){//b[i]に切り出した文字列を格納(コマンド次の数字が入っていく）
      b[i] = strtok(NULL, " ");//nullで次の文字列を指定（２回目以降のきりだしはこうする）
        if (b[i] == NULL){
	clear_command(stdout);
        printf("the number of point is not enough.\n");
        return ERROR;
      }
    }
    for (int i = 0 ; i < 4 ; i++){//char b[i]に格納された数字をvに代入しp[i]に入れる
      char *e;
      long v = strtol(b[i],&e, 10);
      if (*e != '\0'){
	clear_command(stdout);
	printf("Non-int value is included.\n");
	return ERROR;
      }
      p[i] = (int)v;
    }

    draw_rect(c,p[0],p[1],p[2],p[3]);//引数にしてp[i]を代入
    clear_command(stdout);
    printf("1 rect drawn\n");
    return NORMAL;
  }

  if (strcmp(s, "circle") == 0) {
    int p[3] = {0}; // p[0]: x0, p[1]: y0, p[2]: r 
    char *b[3];
    for (int i = 0 ; i < 3; i++){//b[i]に切り出した文字列を格納(コマンド次の数字が入っていく）
      b[i] = strtok(NULL, " ");//nullで次の文字列を指定（２回目以降のきりだしはこうする）
        if (b[i] == NULL){
	clear_command(stdout);
        printf("the number of point is not enough.\n");
        return ERROR;
      }
    }
    for (int i = 0 ; i < 3 ; i++){//char b[i]に格納された数字をvに代入しp[i]に入れる
      char *e;
      long v = strtol(b[i],&e, 10);
      if (*e != '\0'){
	clear_command(stdout);
	printf("Non-int value is included.\n");
	return ERROR;
      }
      p[i] = (int)v;
    }

    draw_circle(c,p[0],p[1],p[2]);//引数にしてp[i]を代入
    clear_command(stdout);
    printf("1 circle drawn\n");
    return NORMAL;
  }
  
  if(strcmp(s,"load") == 0){
    
    s = strtok(NULL, " ");//sにファイル名が入る
    
    if(s == NULL){//引数なしの場合history.txtを読み込む
      FILE *fp;
      if((fp = fopen("history.txt","r")) != NULL){
       char buf1[100] = {0};
       while(fgets(buf1, 100, fp) != NULL){//buf1にline　10　20　10　10　などを1行入れた
        //buf1[0]はload以外のコマンド名
        save_history(&buf1[0],his);
        interpret_command(&buf1[0],his,c);
       }  

      } else {//存在しない場合エラー
        printf("Cannot open file, history.txt\n");
        return -1;
      }
    } else {//引数ありの場合そのファイルを読み込む
    FILE *fp;
    if((fp = fopen(s,"r")) != NULL){
      
      char buf1[100] = {0};
       while(fgets(buf1, 100, fp) != NULL){//buf1にline　10　20　10　10　などを1行入れた
        //buf1[0]はload以外のコマンド名
        save_history(&buf1[0],his);
        interpret_command(&buf1[0],his,c);
       }  

    } else {
     printf("Cannot open file %s\n",s);
        return -1;
    }

    fclose(fp);      
    }

    clear_command(stdout);
    printf("loaded from \"%s\"\n",(s==NULL)?"history.txt":s);
    return COMMAND;
  }



  if (strcmp(s, "save") == 0) {
    s = strtok(NULL, " ");
    save_history(s, his);
    clear_command(stdout);
    printf("saved as \"%s\"\n",(s==NULL)?"history.txt":s);
    return COMMAND;
  }
  
  if (strcmp(s, "chpen") == 0) {
    save_history(s, his);
    s = strtok(NULL, " ");//sに文字種が入る
    c->pen = *s;

    clear_command(stdout);
    printf("changed pen as %s", s);
    return COMMAND;
  }
if (strcmp(s, "chcolor") == 0) {
    save_history(s, his);
    s = strtok(NULL, " ");//sに文字種が入る
    if(strcmp(s, "red")){
     c->pen = "\x1b[34mpen\034[m\n";
    }

    clear_command(stdout);
    printf("changed color as %s", s);
    return COMMAND;
  }

  if (strcmp(s, "undo") == 0) {
    reset_canvas(c);
    //[*] 線形リストの先頭からスキャンして逐次実行
    // pop_back のスキャン中にinterpret_command を絡めた感じ
    Command *p = his->begin;
    if (p == NULL){
      clear_command(stdout);
      printf("no command in history\n");
      return COMMAND;
    }
    else{
      Command *q = NULL; // 新たな終端を決める時に使う
      while (p->next != NULL){ // 終端でないコマンドは実行して良い
	interpret_command(p->str, his, c);
	q = p;
	p = p->next;
      }
      // 1つしかないコマンドのundoではリストの先頭を変更する
      if (q == NULL) {
	his->begin = NULL;
      }
      else{
	q->next = NULL;
      }
      free(p->str);
      free(p);	
      clear_command(stdout);
      printf("undo!\n");
      return COMMAND;
    }  
  }

  if (strcmp(s, "quit") == 0) {
    return EXIT;
  }
  clear_command(stdout);
  printf("error: unknown command.\n");
  return UNKNOWN;
}


// [*] 線形リストの末尾にpush する
Command *push_command(History *his, const char *str){
  Command *c = (Command*)malloc(sizeof(Command));
  char *s = (char*)malloc(his->bufsize);
  strcpy(s, str);
  
  *c = (Command){ .str = s, .bufsize = his->bufsize, .next = NULL};

  Command *p = his->begin;

  if ( p == NULL) {
    his->begin = c;
  }
  else{
    while (p->next != NULL){
      p = p->next;
    }
    p->next = c;
  }
  return c;
}


