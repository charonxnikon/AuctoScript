#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>

#define IGNORE 0
#define GET_PRICE 1
#define GET_AMOUNT 2
#define BUF_SIZE 1024

enum state{
   waiting, 
   working,
   bankrupt,
   disconnect, 
   finish
};

enum stat_wait{
   erro,
   def,
   exe
};

enum play_group{
   on,
   off
};

enum cmd{
   market,
   player,
   online,
   info,
   me,
   prod,
   buy, 
   sell,
   build,
   back,
   ext,
   turn,
   help,
   err,
   miss 
};

struct level{
   double price;
   double amount;
	int lvl;
   struct level *next;
};

struct list{
   int number;
   struct list *next; 
};

struct offer{
   int count;
   int price;
};

struct request{
   enum cmd name;
   int op1;
   int op2;
};

struct offer_l{
   int count; 
   int price;
   struct info_ *usr; 
};

struct market{
   int materials; 
   int production;
   int min;
   int max;
};

struct game{
   int month;
   int count;
   struct level *lvl;
   enum play_group play; 
};

struct info_{
   int id;
   char buf[BUF_SIZE];
   int buf_use; 
   int fd;
   int money;
   int materials; 
   int product;
   int factory; 
   int order;
   int sold;
   int bought;
   struct offer sale;
   struct offer buy;
   int build[5];
   enum state stat;
   struct info_ *next;  
};

struct list *
get_lastli(struct list *lis)
{
   struct list *p = NULL;
   while (lis){
      p = lis;
      lis = lis->next;
   }
   return p;
}

struct list *
expand_list(struct list *p, int num)
{
   struct list *q; 
   if (p){
      q = get_lastli(p);
      q->next = malloc(sizeof(struct list));
   }else{
      q = malloc(sizeof(struct list));
      p = q; 
   }
   q->next = NULL; 
   q->number = num;
   return p; 
}

void
free_list(struct list *p)
{
   struct list *q; 
   while (p){
      q = p; 
      p = p->next;
      free(q);
   }
}

int mirrow(int count)
{
    int num=0;
    while (count){
        num*=10;
        num+=count%10;
        count/=10;
    }
    return num;
}

int lenint(int count)
{
    int length=0;
    if (count==0){
        return 1;
    }
    if (count<0){
        length++;
        count*=(-1);
    }
    while (count){
        length++;
        count/=10;
    }
    return length;
}

char *int2str(int count)
{
    int i = 0, N = lenint(count);
    char *str = malloc(N+1);
    if (count == 0){
        str[0] = '0';
        str[1] = '\0';
        return str;
    }
    if (count<0){
        str[0] = '-';
        i++;
        count *= (-1);
    }
    count = mirrow(count);
    while (i<N){
        str[i] = count%10+'0';
        count /= 10;
        i++;
    }
    str[N] = '\0';
    return str;
}

int
size_number(int number)
{
   int i;
   for (i = 0; number; i++){
      number = number/10;
   }
   return i;
}


void
reverse_str(char *s)
{
   int i, j;
   char c;
   for (i = 0, j = strlen(s)-1; i < j; i++, j--){
      c = s[i];
      s[i] = s[j];
      s[j] = c;
   }
}

void
itost(int n, char *s)
{
   int i, sign;
   if ((sign = n) < 0){
      n = -n;
   }
   i = 0;
   do {
      s[i++] = n % 10 + '0';
   } while ((n /= 10) > 0);
   if (sign < 0){
      s[i++] = '-';
   }
   s[i] = '\0';
   reverse_str(s);
}


int
strtoi(char *s)
{
    int n = 0;
    while( *s >= '0' && *s <= '9' ) {
        n *= 10;
        n += *s++;
        n -= '0';
    }
    return n;
}

int
get_conline(struct info_ *data)
{
   int i;
   for(i = 0; data; data = data->next){
      if (data->stat != bankrupt){
         i++;
      }
   }
   return i;
}

int
size_str(const char *str)
{
   int i = 0;
   if (str){ 
      for (; str[i] != '\0'; i++){
         ;
      }
   }
   return i;
}

void 
free_dellist(struct list **first)
{
   struct list *tmp; 
   while (*first){
      tmp = *first;
      *first = (*first)->next;
      free(tmp);
   }
   *first = NULL; 
}

int 
str_cmp(const char *first, const char *second)
{
   int i;
   if (first && second){
      if (size_str(first) == size_str(second)){
         for (i = 0; first[i] != '\0'; i++){
            if (first[i] != second[i]){
               return 1;  
            }
         }
         return 0;
      } 
   }
   return 1;
}

void
send_msg(struct info_ *tmp, const char *msg)
{
   write(tmp->fd, msg, size_str(msg));
}

int 
send_cnt(struct info_ *tmp, int count)
{
   char *msg = int2str(count);
   int len = lenint(count);
   send_msg(tmp,msg);
   free(msg);
   return len;
}

void
send_space(struct info_ *tmp,int size)
{
    const char space = ' ';
    if (size<=0)
        size = 1;
    while (size>0){
        write(tmp->fd,&space,1);
        size--;
    }
}

int 
check_id(struct info_ *data, int id)
{
   while (data){
      if (data->id == id){
         return 1;
      }
      data = data->next; 
   }
   return 0;
}

int
get_id(struct info_ *data)
{
   int id = 1; 
   while (check_id(data, id )){
      id++; 
   }
   return id;
}

void 
start_kit(struct info_ *tmp)
{
   int i;
   tmp->money = 10000; 
   tmp->materials = 4; 
   tmp->product = 2; 
   tmp->factory = 2; 
   for (i = 0; i < 5; i++){
      tmp->build[i] = 0; 
   }
}

void 
reset_re(struct info_ *tmp)
{
   tmp->sale.count = 0; 
   tmp->sale.price = 0; 
   tmp->buy.count = 0;
   tmp->buy.price = 0;
   tmp->sold = 0; 
   tmp->bought = 0;
   tmp->order = 0; 
}

enum cmd
get_cmd(char *cmd)
{
   static const char a0[] = "\0";
   static const char a1[] = "market";
   static const char a2[] = "player";
   static const char a3[] = "online";
   static const char a4[] = "exit";
   static const char a5[] = "prod";
   static const char a6[] = "buy";
   static const char a7[] = "sell";
   static const char a8[] = "build";
   static const char a9[] = "back";
   static const char a10[] = "me";
   static const char a11[] = "turn";
   static const char a12[] = "help";
   if (!str_cmp(cmd,"info")){
       return info;
   }
   if (!str_cmp(cmd, a0)){
      return miss;
   }
   if (!str_cmp(cmd, a1)){
      return market; 
   }
   if (!str_cmp(cmd, a2)){
      return player;
   }
   if (!str_cmp(cmd, a3)){
      return online; 
   }
   if (!str_cmp(cmd, a4)){
      return ext; 
   }
   if (!str_cmp(cmd, a5)){
      return prod; 
   }
   if (!str_cmp(cmd, a6)){
      return buy; 
   }
   if (!str_cmp(cmd, a7)){
      return sell; 
   }
   if (!str_cmp(cmd, a8)){
      return build; 
   }
   if (!str_cmp(cmd, a9)){
      return back; 
   }
   if (!str_cmp(cmd, a10)){
      return me; 
   }
   if (!str_cmp(cmd, a11)){
      return turn; 
   }
   if (!str_cmp(cmd, a12)){
      return help; 
   }
   return err; 
}

int 
endw(char symbol)
{
   return (symbol == ' ' || symbol == '\t' ||  symbol == '\0');
}

int 
get_w(char *cmd, char *str)
{
   int i; 
   for (i = 0; !(endw(str[i])) && i < 8; i++){
      cmd[i] = str[i];
   }
   cmd[i] = '\0';
   if (!endw(str[i])){
      return -1; 
   }
   return i; 
}

int 
skip_spaces(char *str)
{
   int i = 0;
   while (str[i] == ' ' || str[i] == '\t'){
      i++; 
   }
   return i; 
}

struct request
analyze(char *str)
{
   int rc = 0; 
   char a[9]; 
   struct request cmd; 
   cmd.op1 = 0; 
   cmd.op2 = 0; 
   rc += skip_spaces(str+rc);
   rc += get_w(a, str+rc);
   if (rc == -1){
      cmd.name = err; 
      return cmd;
   }
   cmd.name = get_cmd(a);
   rc += skip_spaces(str+rc);
   rc += get_w(a, str+rc);
   if (rc == -1){
      cmd.name = err; 
      return cmd;
   }
   cmd.op1 = strtoi(a);
   rc += skip_spaces(str+rc);
   rc += get_w(a, str+rc);
   if (rc == -1){
      cmd.name = err; 
      return cmd;
   }
   cmd.op2 = strtoi(a);
   rc += skip_spaces(str+rc);
   if (str[rc] != '\0'){
      cmd.name = err; 
   }
   return cmd;
}

int 
check_err(struct info_ *tmp, struct request cmd)
{
   static const char a1[] = "Nikon: >> WRONG COMMAND\n";
   static const char a2[] = "Nikon: >> WRONG ARGUMENT\n";
   static const char a3[] = "Nikon: >> Use command <help> \n\n"; 
   int errarg = 0; 
   if (cmd.name == err || cmd.op1 == -1 || cmd.op2 == -1){
      send_msg(tmp, a1);
      send_msg(tmp, a3); 
      return 1; 
   }
   switch (cmd.name)
   {
      case market: 
      case online:
      case info: 
      case back: 
      case turn:
      case ext:
      case me:
      case help:
         if (cmd.op1 || cmd.op2){
            errarg = 1;
         }
         break;
      case player: 
      case build:
      case prod:
         if (!cmd.op1){
            errarg = 1;
         }
         break; 
      case buy: 
      case sell: 
         if (!cmd.op1 || !cmd.op2){
            errarg = 1;
         }
            break; 
      default:
         errarg = 0; 
   }
   if (errarg){
      send_msg(tmp, a2); 
      send_msg(tmp, a3);
   }
   return errarg; 
}

enum stat_wait 
check_access(struct info_ *tmp, struct request cmd)
{
   static const char a1[] = "Nikon: >> Not available now\n";
   switch (cmd.name)
   {
      case sell:
      case buy: 
      case prod: 
      case build: 
      case back: 
      case turn: 
         if (tmp->stat == finish || tmp->stat == bankrupt){
            send_msg(tmp, a1);
            return erro; 
         }
      default: 
         if (tmp->stat == finish || tmp->stat == bankrupt){
            return exe;
         }
         return def;
   }
}

struct market situation(struct level *lvl, int active)
{
   struct market shop;
/* 
   switch (lvl)
   {
      case 1: 
         shop.materials = 1*active;
         shop.production = 3*active; 
         shop.min = 800; 
         shop.max = 6500; 
         break;
      case 2: 
         shop.materials = (int)(1.5*active);
         shop.production = (int)(2.5*active); 
         shop.min = 650; 
         shop.max = 6000; 
         break;
      case 3: 
         shop.materials = 2*active;
         shop.production = 2*active; 
         shop.min = 500; 
         shop.max = 5500; 
         break;
      case 4: 
         shop.materials = (int)(2.5*active);
         shop.production = (int)(1.5*active); 
         shop.min = 400; 
         shop.max = 5000; 
         break;
      case 5: 
         shop.materials = 3*active;
         shop.production = 1*active; 
         shop.min = 300; 
         shop.max = 4500; 
         break;
   }
*/
	shop.materials = lvl->amount;
   shop.production = lvl->amount;
   shop.min = lvl->price;
   shop.max = lvl->price-2;
   return shop;
}

void
exec_market(struct info_ *tmp, struct game bank, int active)
{
   static const char a1[] = " CURRENT MONTH : "; 
   static const char a2[] = " th\n";
   static const char a3[] = " Players still active:\n";
   static const char a4[] = " Bank sell:  items  minimal\n";
   static const char a5[] = " Bank buys:  items  maximum\n";
   static const char a6[] = "%                ";
   static const char a7[] = "\n"; 
   static const char a8[] = "   ";
   struct market shop = situation(bank.lvl, active);
   send_msg(tmp, a1);
   send_cnt(tmp, bank.month);
   send_msg(tmp, a2);
   send_msg(tmp, a3);
   send_msg(tmp, a6);
   send_cnt(tmp, active);
   send_msg(tmp, a7);
   send_msg(tmp, a4);
   send_msg(tmp, a6);
   send_cnt(tmp, shop.materials);
   send_msg(tmp, a8);
   send_cnt(tmp, shop.min);
   send_msg(tmp, a7);
   send_msg(tmp, a5);
   send_msg(tmp, a6);
   send_cnt(tmp, shop.production);
   send_msg(tmp, a8);
   send_cnt(tmp, shop.max);
   send_msg(tmp,"\n\n\r");
}

void exec_info(struct info_ *tmp,struct info_ *data)
{
    static const char s1[] = "Player  Money   Raw  Prod  Fact\n";
    static const char s2[] = "%";
    int res;
    send_msg(tmp, s1);
    while (data){
        if (data->stat != bankrupt){
            send_msg(tmp, s2);
            res = send_cnt(tmp, data->id);
            send_space(tmp, 7-res);
            res = send_cnt(tmp, data->money);
            send_space(tmp, 8-res);
            res = send_cnt(tmp, data->materials);
            send_space(tmp, 5-res);
            res = send_cnt(tmp, data->product);
            send_space(tmp, 6-res);
            res = send_cnt(tmp, data->factory);
            send_msg(tmp, "\n");
        }
        data = data->next;
    }
    send_msg(tmp, "\n\r");
}

void
exec_player(struct info_ *tmp, struct info_ *data, struct request cmd)
{
   static const char a1[] = " money : ";
   static const char a2[] = " \n materials : ";
   static const char a3[] = " \n products : ";
   static const char a4[] = " \n factory : ";
   static const char a5[] = " \n\n";
   static const char a10[] = "Nikon: >> NOT FOUND\n\n"; 
   while (data){
      if (cmd.op1 == data->id){
         send_msg(tmp, a1);
         send_cnt(tmp, data->money);
         send_msg(tmp, a2);
         send_cnt(tmp, data->materials);
         send_msg(tmp, a3);
         send_cnt(tmp, data->product);
         send_msg(tmp, a4);
         send_cnt(tmp, data->factory);
         send_msg(tmp, a5);
         return; 
      }
      data = data->next; 
   }
   send_msg(tmp, a10);
}

void 
exec_online(struct info_ *tmp, struct info_ *data)
{
   static const char a1[] = " Users still in server: ";
   static const char a2[] = " ";
   static const char a3[] = "\n\n"; 
   static const char a4[] = "%";
   send_msg(tmp, a1);
   while (data){
      send_msg(tmp, a4); 
      send_cnt(tmp, data->id); 
      send_msg(tmp, a2); 
      data = data->next; 
   }
   send_msg(tmp, a3);
}

void
exec_prod(struct info_ *tmp, struct request cmd)
{
   static const char a2[] = "Nikon: >> NOT ENOUGH MATERIALS\n "; 
   static const char a4[] = "Nikon: >> Factory accept\n"; 
   int count = tmp->order+cmd.op1;
   if (tmp->materials-count < 0){
      send_msg(tmp, a2);
      return;
   }
   tmp->order += cmd.op1;
   send_msg(tmp, a4); 
}

void 
exec_buy(struct info_ *tmp, struct request cmd, struct level *lvl, int active)
{
   static const char a1[] = "Nikon: >> BANK SELLS LESS MATERIALS\n"; 
   static const char a2[] = "Nikon: >> TOO LOW PRICE\n "; 
   static const char a3[] = "Nikon: >> accept\n";
   struct market shop = situation(lvl, active);
   if (cmd.op1 > shop.materials){
      send_msg(tmp, a1);
      return; 
   }
   if (cmd.op2 < shop.min){
      send_msg(tmp, a2);
      return;
   }
   send_msg(tmp, a3);
   tmp->buy.count = cmd.op1; 
   tmp->buy.price = cmd.op2;
}

void
exec_sell(struct info_ *tmp, struct request cmd, struct level *lvl, int active)
{
   static const char a1[] = "Nikon: >> BANK BUYS LESS PRODUCT\n"; 
   static const char a2[] = "Nikon: >> TOO HIGH PRICE\n "; 
   static const char a3[] = "Nikon: >> NOT ENOUGH PRODUCT\n "; 
   static const char a4[] = "Nikon: >> accept\n";
   struct market shop = situation(lvl, active);
   if (cmd.op1 > shop.production){
      send_msg(tmp, a1);
      return;
   } 
   if (cmd.op1 > shop.max){
      send_msg(tmp, a2);
      return;
   } 
   if (cmd.op1 > tmp->product){
      send_msg(tmp, a3);
      return; 
   }
   send_msg(tmp, a4);
   tmp->sale.count = cmd.op1; 
   tmp->sale.price = cmd.op2;
}

void 
exec_me(struct info_ *tmp, struct info_ *data)
{
   static const char a1[] = " money: ";
   static const char a2[] = " \n materials: ";
   static const char a3[] = " \n products: ";
   static const char a4[] = " \n factory:";
   static const char a5[] = "\n\n";
   while (data){
      if (tmp->id == data->id){
         send_msg(tmp, a1);
         send_cnt(tmp, data->money);
         send_msg(tmp, a2);
         send_cnt(tmp, data->materials);
         send_msg(tmp, a3);
         send_cnt(tmp, data->product);
         send_msg(tmp, a4);
         send_cnt(tmp, data->factory);
         send_msg(tmp, a5);
         return; 
      }
      data = data->next; 
   }
}

void
exec_exit(struct info_ *tmp)
{
      tmp->stat = disconnect;
}

void
exec_build(struct info_ *tmp, struct request cmd)
{
   static const char a1[] = "Nikon: >> NOT ENOUGH MONEY\n";  
   static const char a3[] = "\n\n"; 
   static const char a2[] = "Nikon: >> accept\n"; 
   if (tmp->money >= 2500*cmd.op1){
      tmp->money -= 2500*cmd.op1;
      tmp->build[4] += cmd.op1;
      send_msg(tmp, a2);
      send_cnt(tmp, tmp->money);
      send_msg(tmp, a3);
   }else{
      send_msg(tmp, a1);
   }
}

void
exec_back(struct info_ *tmp)
{
   static const char a[] = "Nikon: >> all canceled\n";
   reset_re(tmp);
   send_msg(tmp, a);
}

void 
exec_turn(struct info_ *tmp)
{
   static const char a[] = "Nikon: >> Wait please, use info commands\n";
   tmp->stat = finish; 
   send_msg(tmp, a);
}

void 
exec_help(struct info_ *tmp)
{
   static const char a1[] = "   >>> Nikon: use this\n\n"; 
   static const char a2[] = "*To find out situation on market : <market>\n"; 
   static const char a3[] = "*To get info about user : <player> <id>\n"; 
   static const char a4[] = "*To see active users : <online>\n"; 
   static const char a5[] = "*To see info about you : <me>\n"; 
   static const char a6[] = "*To disconnect : <exit>\n"; 
   static const char a7[] = "*To produce in factiry : <prod> <count>\n"; 
   static const char a8[] = "*To try buy : <buy> <count>\n"; 
   static const char a9[] = "*To build factiry : <build> <count>\n"; 
   static const char a10[] = "*To cancell all moves in month : <back>\n"; 
   static const char a11[] = "*To finish current month : <turn> "; 
   static const char a12[] = "*To try sell : <sell> <count>\n";
   static const char a13[] = "*To get help : <help>\n\n ";  
   send_msg(tmp, a1);
   send_msg(tmp, a2);
   send_msg(tmp, a3);
   send_msg(tmp, a4);
   send_msg(tmp, a5);
   send_msg(tmp, a6);
   send_msg(tmp, a7);
   send_msg(tmp, a8);
   send_msg(tmp, a12);
   send_msg(tmp, a9);
   send_msg(tmp, a10);
   send_msg(tmp, a11);
   send_msg(tmp, a13);
}

int 
waiting2(struct info_ *tmp, int active, int c_start)
{
   static const char a1[] = " from ";
   static const char a2[] = " connected\n";
   static const char a3[] = " Waiting .. \n\n";
   if (tmp->stat == waiting){
      send_cnt(tmp, active);
      send_msg(tmp, a1);
      send_cnt(tmp, c_start);
      send_msg(tmp, a2); 
      send_msg(tmp, a3);
      return 1;
   }
   return 0;
}

void
execute(char *str, struct info_ *tmp, struct info_ *data, struct game bank)
{
   int active = get_conline(data);
   struct request cmd = analyze(str);
   if (waiting2(tmp, active, bank.count)){
      return;
   }
   if (check_err(tmp, cmd)){
      return; 
   }
   if (check_access(tmp, cmd) == erro){
      return; 
   }
	if (check_access(tmp,cmd) == exe){
		switch (cmd.name)
		{
			case market:
				exec_market(tmp, bank, active);
				break;
			case player: 
				exec_player(tmp, data, cmd); 
				break; 
			case info:
				exec_info(tmp, data);
				break; 
 			case me:
				exec_me(tmp, data); 
				break; 
			case ext:
				exec_exit(tmp); 
				break; 
			case online:
				exec_online(tmp, data); 
				break; 
			case help:
				exec_help(tmp); 
				break; 
			default:
				break; 
		}
		exec_turn(tmp);
		return; 
	}
   switch (cmd.name)
   {
      case market:
         exec_market(tmp, bank, active);
         break;
      case player: 
         exec_player(tmp, data, cmd);
         break;
      case info:
         exec_info(tmp, data);
         break;
      case me:
         exec_me(tmp, data);
         break;
      case ext:
         exec_exit(tmp);
         break;
      case online: 
         exec_online(tmp, data);
         break;
      case prod:
         exec_prod(tmp, cmd);
         break;
      case buy:
         exec_buy(tmp, cmd, bank.lvl, active);
         break;
      case sell: 
         exec_sell(tmp, cmd, bank.lvl, active);
         break;
      case build: 
         exec_build(tmp, cmd);
         break;
      case back: 
         exec_back(tmp);
         break;
      case turn: 
         exec_turn(tmp);
         break;
      case help: 
         exec_help(tmp);
         break;
      default:
         return;
   }
}

void
shake(struct offer_l *arr, int len)
{
   int i, j;
   struct offer_l tmp; 
   for (i = len-1; i > 0; i--){
      j = (int)((i+1.0)*rand()/(RAND_MAX+1.0));
      tmp = arr[j];
      arr[j] = arr[i];
      arr[i] = tmp;
   } 
}

void 
sort_inc(struct offer_l *arr, int len)
{
   int i, j; 
   struct offer_l tmp; 
   for (i = 0; i < len-1; i++){
      for (j = 0; j < len-i-1; j++){
         if (arr[j].price > arr[j+1].price){
            tmp = arr[j]; 
            arr[j] = arr[j+1];
            arr[j+1] = tmp; 
         }
      }
   }
}

void
sort_dec(struct offer_l *arr, int len)
{
  int i, j; 
   struct offer_l tmp; 
   for (i = 0; i < len-1; i++){
      for (j = 0; j < len-i-1; j++){
         if (arr[j].price < arr[j+1].price){
            tmp = arr[j]; 
            arr[j] = arr[j+1];
            arr[j+1] = tmp; 
         }
      }
   }
}

void
buying_up(struct offer_l *arr, int len, int prod)
{
   int i;
   for (i = 0; i < len; i++){
      if (prod >= arr[i].count){
         arr[i].usr->sold = arr[i].count;
         prod -= arr[i].count; 
      }else{
         if (prod > 0){
            arr[i].usr->sold = prod;
            prod = 0; 
         }else{
            arr[i].usr->sold = 0;
         }
      }
   }
}

void send_results(struct info_ *data)
{
    static const char s1[]="Player  Sold  Price  Bought  Price\n";
    static const char s2[]="%";
    struct info_ *tmp,*client=data;
    int res;
    while (client){
        send_msg(client,s1);
        tmp=data;
        while (tmp){
            if (tmp->stat!=bankrupt){
                send_msg(client,s2);
                res=send_cnt(client,tmp->id);
                send_space(client,7-res);
                res=send_cnt(client,tmp->sold);
                send_space(client,6-res);
                res=send_cnt(client,tmp->sale.price);
                send_space(client,7-res);
                res=send_cnt(client,tmp->bought);
                send_space(client,8-res);
                res=send_cnt(client,tmp->buy.price);
                send_msg(client,"\n");
            }
            tmp=tmp->next;
        }
        send_msg(client,"\n");
        client=client->next;
    }
}

void
creat_cmd(char *cmd, char *buf, int j)
{ 
   int i; 
   for (i = 0; i < j; i++){
      cmd[i] = buf[i];
   }
   cmd[j] = '\0'; 
}

void
displace_buf(struct info_ *tmp, int j)
{
   int i, k;
   for (i = j+1, k = 0; i < tmp->buf_use; i++, k++){
      tmp->buf[k] = tmp->buf[i];
   }
   tmp->buf_use = tmp->buf_use-(j+1);
}

void
serve_buf(struct info_ *tmp, struct info_ *data, struct game bank)
{
   char *cmd;
   int i, pos; 
   for(;;){
      pos=-1;
      for (i = 0; i < tmp->buf_use; i++){
          if (tmp->buf[i] == '\n'){
             pos = i;
             break; 
          }
      }
      if (pos == -1){
         return;
      }
      cmd = malloc(pos+1);
      creat_cmd(cmd, tmp->buf, pos);
      displace_buf(tmp, pos);
      if (cmd[pos-1] == '\r'){
         cmd[pos-1] = '\0';
      }
      fprintf(stderr,"%s\n",cmd);
      execute(cmd, tmp, data, bank);
      free(cmd);
   }
}

int
serve_usr(struct info_ *tmp, struct info_ *data, struct game bank)
{
   int rc, busy = tmp->buf_use;
   static const char err_msg[] = "Nikon server: >> too long message\n";
   rc = read(tmp->fd, tmp->buf+busy, BUF_SIZE-busy);
   if (rc == 0){
      return -1;
   }
   tmp->buf_use = tmp->buf_use+rc;
   serve_buf(tmp, data, bank);
   if (tmp->stat == disconnect){
      return -1;
   }
   if (tmp->buf_use >= BUF_SIZE){
      send_msg(tmp, err_msg);
      tmp->buf_use = 0;
   }
   return 0;
}

struct info_ *
close_info(struct info_ *p, int fd)
{
   struct info_ *tmp, *dl;
   if (p){
      if (p->fd == fd){
         tmp = p->next;
         free(p);
         return tmp;
      }
      dl = p;
      while (dl){
         if (dl->fd == fd){
            break;
         }
         dl = dl->next;
      }
      tmp = p;
      while (tmp->next != dl){
         tmp = tmp->next;
      }
      tmp->next = dl->next;
      free(dl);
   }
   return p;
}

struct info_ *
rm_usrs(struct info_ *data, struct list **dellist)
{
   struct list *tmp;
   tmp = *dellist;  
   while (tmp){
      data = close_info(data, tmp->number);
      shutdown(tmp->number, 2);
      close(tmp->number);
      tmp = tmp->next;
      fprintf(stderr,  "user out\n");
   }
   free_dellist(dellist);
   return data;
}

struct info_ *
get_last(struct info_ *data)
{
   while(data->next){
      data = data->next;
   }
   return data;
}

struct info_ *
add_usr(struct info_ *data, int fd)
{
   struct info_ *tmp; 
   static const char greeting[] = "Welcome to Nikon server \n";
   static const char sayid[] = "Your id is:\n";
   fprintf(stderr, "New user in server\n");
   if (data){
      tmp = get_last(data);
      tmp->next = malloc(sizeof(struct info_));
      tmp = tmp->next;
   }else{
      tmp = malloc(sizeof(struct info_));
      data = tmp; 
   }
   tmp->next = NULL; 
   tmp->fd = fd; 
   tmp->id = 0;
   tmp->id = get_id(data); 
   tmp->buf_use = 0;
   tmp->stat = waiting;
   start_kit(tmp);
   reset_re(tmp);
   send_msg(tmp,greeting);
   send_msg(tmp, sayid);
   send_msg(tmp,"%    ");
   send_cnt(tmp, tmp->id);
   send_msg(tmp,"\n\n");
   return data; 
}

void
add_2del(struct list **first, struct list **last, int fd)
{
   struct list *tmp; 
   tmp = malloc(sizeof(struct list));
   tmp->next = NULL; 
   tmp->number = fd;
   if (!(*first)){
      *first = tmp; 
      *last = tmp; 
   }else{
      (*last)->next = tmp;
      *last = tmp;
   }
}

void 
sell_out(struct offer_l *arr, int len, int mat)
{
   int i;
   for (i = 0; i < len; i++){
      if (mat >= arr[i].count){
         arr[i].usr->bought = arr[i].count;
         mat -= arr[i].count; 
      }else{
         if (mat > 0){
            arr[i].usr->bought = mat;
            mat = 0; 
         }else{
            arr[i].usr->bought = 0;
         }
      }
   }
}

void
auction(struct info_ *data, struct level *lvl)
{
   int i = 0; 
   int active = get_conline(data);
   struct offer_l arr[active];
   struct market shop = situation(lvl, active);
   struct info_ *tmp; 
   tmp = data; 
   while (tmp){
      arr[i].count = tmp->sale.count; 
      arr[i].price = tmp->sale.price; 
      arr[i].usr = tmp; 
      tmp = tmp->next; 
      i++;
   }

   shake(arr, active);

   sort_inc(arr, active);
   buying_up(arr, active, shop.production);
   i = 0;
   tmp = data;
   while (tmp){
      arr[i].count = tmp->buy.count; 
      arr[i].price = tmp->buy.price;
      arr[i].usr = tmp; 
      tmp = tmp->next;
      i++;
   }

   shake(arr, active);

   sort_dec(arr, active);
   sell_out(arr, active, shop.materials);
   send_results(data);
}

int
build_factory(struct info_ *tmp)
{
   static const char a1[] = "Construct completed :  ";
   static const char a2[] = "Payment :  ";
   static const char a3[] = "\n";
   static const char a4[] = " \n\n";
   int price = tmp->build[0]*2500;
   int i;
   if (tmp->build[0]){
      send_msg(tmp, a1);
      send_cnt(tmp, tmp->build[0]);
      send_msg(tmp, a3);
      send_msg(tmp, a2);
      send_cnt(tmp, price);
      send_msg(tmp, a4);
      tmp->factory += tmp->build[0];
   }
   for (i = 0; i < 4; i++){
      tmp->build[i] = tmp->build[i+1]; 
   }
   tmp->build[4] = 0; 
   return price;
}

void
finance(struct info_ *data)
{
   static const char a1[] = "Nikon: >> You earn : ";
   static const char a2[] = "Payment : ";
   static const char a3[] = "Total Money : ";
   static const char a4[] = "$\n";
   static const char a5[] = "\n";
   int pay, earn; 
   struct info_ *tmp = data;
   while (tmp){
      pay = 0; 
      earn = (tmp->sold)*(tmp->sale.price);
      tmp->product -= tmp->sold; 
      tmp->materials -= tmp->order; 
      tmp->product += tmp->order; 
      tmp->materials += tmp->bought; 
      pay += (tmp->bought)*(tmp->buy.price);
      tmp->money -= pay;
      tmp->money += earn; 
      send_msg(tmp ,a1);
      send_cnt(tmp, earn);
      send_msg(tmp, a4);
      send_msg(tmp, a2);
      send_cnt(tmp, pay);
      send_msg(tmp, a4);
      send_msg(tmp, a3);
      send_cnt(tmp, tmp->money);
      send_msg(tmp, a4);
      send_msg(tmp, a5);
      tmp = tmp->next; 
   }
}

int
usr_active(struct info_ *data)
{
   int i; 
   for (i = 0; data; data = data->next){
      if (data->stat != bankrupt){
         i++;
      }
   }
   return i;
}

struct info_ *
get_last_id(struct info_ *data)
{
   struct info_ *tmp = NULL; 
   while (data){
      if (data->stat != bankrupt){
         tmp = data; 
         return tmp; 
      }
      data = data->next; 
   }
   return tmp; 
}

struct info_ *
update(struct info_ *data)
{
   static const char a1[] = "  Nikon: >>> You are bankrupt \n\n ";
   static const char a2[] = "User ";
   static const char a3[] = "are bankrupt \n\n ";
   static const char a4[] = "  Nikon: >>> YOU WIN\n\n";
   struct info_ *tmp, *q;
   struct list *del = NULL, *last = NULL;  
   tmp = data; 
   while (tmp){
      if (tmp->money < 0){
         q = data; 
         while (q){
            send_msg(q, a2);
            send_cnt(q, tmp->id);
            send_msg(q, a3);
            q = q->next; 
         }
         tmp->stat = bankrupt;
         send_msg(tmp, a1);
      }
      tmp = tmp->next;  
   }
   if (usr_active(data) == 1){
      tmp = get_last_id(data);
      send_msg(tmp, a4);
      tmp = data; 
      while (tmp){
         add_2del(&del, &last, tmp->fd);
         tmp = tmp->next; 
      } 
      data = rm_usrs(data, &del);
   }
   return data;
}

struct game
def_set(int c_start,struct level *ptr)
{
   struct game bank; 
   bank.lvl = ptr;
   bank.month = 1;
   bank.play = off;
   bank.count = c_start;
   return bank;
}

int
change_lvl(struct level **lvl)
{
/*
   static const int distribution[5][5] = {
      {4,4,2,1,1},
      {3,4,3,1,1},
      {1,3,4,3,1},
      {1,1,3,4,3},
      {1,1,2,4,4}
   };
   int i, sum = 0; 
   int r = 1+(int)(12.0*rand()/(RAND_MAX+1.0));
   for (i = 0; i < 5; i++){
      sum += distribution[lvl-1][i];
      if (sum >= r){
         break; 
      }
   }
   return i+1;
*/
	*lvl = (*lvl)->next;
	return (*lvl)->lvl;  
	
}

int check_fin(struct info_ *data)
{
   while (data){
      if (data->stat == working){
         return 1;
      }
      data = data->next; 
   }
   return 0; 
}

void
wait_2work(struct info_ *data)
{
   static const char a[] = "Nikon: >>> GAME STARTED \n\n";
   while (data){
      send_msg(data, a);
      send_msg(data,"\r");
      data->stat = working; 
      data = data->next; 
   }
}

void 
fin_2work(struct info_ *data)
{
   while (data){
      if (data->stat != bankrupt && data->stat !=disconnect){
         data->stat = working; 
         send_msg(data,"\r");
      }
      data = data->next; 
   }
}

void
reset_all(struct info_ *data)
{
   while (data){
      reset_re(data);
      data = data->next; 
   }
}

void
get_data(struct info_ *data, struct game *b)
{	
	int fd, buf;
	char space = ' ';
	char ent = '\n';
	char *str; 
	fd = open("prot.txt",O_WRONLY|O_APPEND|O_CREAT,0644);
   if (fd == -1){
   	perror("prot.txt");
      exit(1);
   }
	while (data){
		buf = data->id;
      str = int2str(buf);
      write(fd,str,size_str(str));
      write(fd,&space,sizeof(space));
      free(str);

		buf = data->money;
		str = int2str(buf);
		write(fd,str,size_str(str));
		write(fd,&space,sizeof(space));
		free(str);

		buf = data->materials;
		str = int2str(buf);
      write(fd,str,size_str(str));
      write(fd,&space,sizeof(space));
		free(str);

		buf = data->sold;  
		str = int2str(buf); 	
      write(fd,str,size_str(str));
      write(fd,&space,sizeof(space));
		free(str);

		buf = data->bought;
		str = int2str(buf);
      write(fd,str,size_str(str));
      write(fd,&space,sizeof(space));
		free(str);

		buf = (b->lvl)->price;
		str = int2str(buf);
      write(fd,str,size_str(str));
      write(fd,&space,sizeof(space));
		free(str);

		buf = (b->lvl)->amount;
		str = int2str(buf);
      write(fd,str,size_str(str));
      write(fd,&space,sizeof(space));
		free(str);

		write(fd,&ent,sizeof(ent));
		data = data->next;
	}
	close(fd);
}

struct info_ *
gameplay(struct info_ *data, struct game *bank, struct level *ptr)
{  
   if (bank->play == off){
      if (get_conline(data) < bank->count){
         return data;
      }
      wait_2work(data);
      bank->play = on;
      return data;
   }
   if (check_fin(data)){
      return data;
   }
   auction(data, bank->lvl);
   finance(data); 
   get_data(data, bank);
   data = update(data);
   reset_all(data);
   fin_2work(data);
   bank->month++;
   bank->lvl = bank->lvl->next;
   if (!data){
      *bank = def_set(bank->count,ptr);
   }
   return data;
}

void
start(int ls, int count, struct level *ptr)
{
   struct info_ *tmp, *data = NULL;
   struct game bank = def_set(count,ptr);
   struct list *dellist = NULL, *last_indel = NULL;
   int maxd, fd, result;
   for(;;){
      fd_set usrfds;
      FD_ZERO(&usrfds); 
      FD_SET(ls, &usrfds);
      maxd = ls;
      tmp = data;
      while (tmp){
         FD_SET(tmp->fd, &usrfds);
         if (tmp->fd > maxd){
            maxd = tmp->fd; 
         }
         tmp = tmp->next; 
      }
      result = select(maxd+1, &usrfds, NULL, NULL, NULL);
      if (result < 1){
         continue;
      }
      if (FD_ISSET(ls, &usrfds)){
         fd = accept(ls, NULL, NULL);
         if ( fd != -1){
            if (bank.play!=on){
                data = add_usr(data, fd);
            }
            else{
                shutdown(fd, 2);
                close(fd);
            }
         }else{
            perror("accept");
         }
      }
      tmp = data;
      dellist = NULL;
      while (tmp){
         if (FD_ISSET(tmp->fd, &usrfds)){
            result = serve_usr(tmp, data, bank);
            if (result == -1){
               add_2del(&dellist, &last_indel, tmp->fd);
            }
         }
         tmp = tmp->next; 
      }
      data = rm_usrs(data, &dellist);
      last_indel = NULL;
      data = gameplay(data, &bank, ptr);
      if (!bank.lvl){
          break;
      }
   }
}

struct level *add_elem(struct level *ptr, double a_price, double a_amount)
{
    struct level *tmp;
    if (ptr){
        tmp = ptr;
        while (tmp->next){
            tmp = tmp->next;
        }
        tmp->next = malloc(sizeof(*tmp));
        tmp = tmp->next;
    }else{
        tmp = malloc(sizeof(*tmp));
        ptr = tmp;
    }
    tmp->next = NULL;
    tmp->price = a_price;
    tmp->amount = a_amount;
    return ptr;
}

void dispose(struct level *ptr)
{
    struct level *tmp;
    while (ptr){
        tmp = ptr;
        ptr = ptr->next;
        free(tmp);
    }
}

void print(struct level *ptr)
{
    while (ptr){
        fprintf(stderr,"%f / %f\n",ptr->price,ptr->amount);
        ptr = ptr->next;
    }
}

struct level *scanner()
{
    char buff[128]; 
    int c, i, comma = 0, flag = IGNORE;
    double price, amount;
    struct level *ptr = NULL;
    while((c = getchar()) != EOF){
        switch (flag)
        {
            case GET_PRICE:
                if (c == ','){
                    buff[i] = '\0';
                    price = atof(buff);
                    i = 0;
                    flag = GET_AMOUNT;
                }else{
                    buff[i] = c;
                    i++;
                }
                break;
            case GET_AMOUNT:
                if (c == '\n'){
                    buff[i] = '\0';
                    amount = atof(buff);
                    i = 0;
                    flag = IGNORE;
                    ptr = add_elem(ptr, price, amount);
                }else{
                    buff[i] = c;
                    i++;
                }
                break;
            case IGNORE:
                if (c == ','){
                    comma++;
                }
                if (comma == 5){
                    flag = GET_PRICE;
                    comma = 0;
                    i = 0;
                }
        }
    }
    return ptr;
}

int
main(int argc, char **argv)
{
   int fd, ls, opt = 1, port, count;
   struct sockaddr_in addr;
   struct level *ptr;
   if (argc!=4){
      fprintf(stderr,"Wrong amount of arguments\n");
      exit(1);
   }
   fd = open(argv[3], O_RDONLY);
   if (fd==-1){
      perror(argv[3]);
      exit(1);
   }
   dup2(fd,0);
   close(fd);
   port = strtoi(argv[1]);
   count = strtoi(argv[2]);
   addr.sin_family = AF_INET;
   addr.sin_port = htons(port);
   addr.sin_addr.s_addr = htonl(INADDR_ANY);
   ls = socket(AF_INET, SOCK_STREAM, 0);
   if (ls == -1){
      perror("socket");
      exit(1);
   }
   setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
   if (-1 == bind(ls, (struct sockaddr*)&addr, sizeof(addr))){
      perror("bind");
      exit(1);
   }
   if (-1 == listen(ls, 16)){
      perror("listen");
      exit(1);
   }
   ptr=scanner();
   start(ls, count, ptr);
   dispose(ptr);
   return 0;
}

