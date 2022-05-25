#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define INBUFSIZE 1024

struct buffer{
    	char buf[INBUFSIZE];
    	int buf_used;
};

enum mark{
	keyword,
	constant,
	operation,
	defect,
	punctuator,
	string,
	identifier
};

struct lex_item{
	char *word; 
	mark type;
	int line; 
	lex_item *next; 
};

class Info_;
class RPNElem;

struct RPNItem{
    	RPNElem *elem;
    	RPNItem *next;
};

struct item{
	int count;
	struct item *next;
};

class Error{
	char *msg;
	lex_item *lex; 
public:
	Error(const char *str);
	~Error(); 
	Error(const char *str, lex_item *q);
	Error(const Error& Drop);
	void Report();
};

class Scan{
	enum state{
		beg,
		keywd,
		quote,
		count,
		ident,
		equal,
		negation,
		negup,
		negunder,
		up,
		under,
		error
	};
	struct item{
		char symbol;
		item *next;
	};
	state flag; 
	item *q;
	bool check;
	int cur_line;
	lex_item *f; 
private:
	void Step(char c);
	void Handl_beg(char c);
	void Handl_keywd(char c);
	void Handl_quote(char c);
	void Handl_count(char c);
	void Handl_ident(char c);
	void Handl_equal(char c);
	void Handl_negation(char c);
	void Handl_negup(char c);
	void Handl_negunder(char c);
	void Handl_up(char c);
	void Handl_under(char c);
	bool Check_ident();
	bool Check_keywd();
	bool Is_op(char c);
	void Add_lex(mark note);
	void Append(char c);
	void Cln();
	int Length();
	char *Get_word();
public:
	Scan();
	~Scan();
	void Feed(char c);
	lex_item *Get_lex();
};

class Parser{
	lex_item *cur_lex;
    RPNItem *stack;
    RPNItem *prog;
private:
	void next();
	void S();
	void A();
	void B();
	void D1();
	void D2();
	void C();
	void C1();
	void C2();
	void C3();
	void H();
	bool Is_lex(const char *str);
	bool Is_var();
	bool Is_func();
	bool Is_const();
	bool Is_str();
    	void Push(RPNElem *c);
    	RPNElem *Pop();
    	void Add(RPNElem *c);
    	RPNItem *Blank();
    	RPNItem *GetLast();
	RPNElem *Create();
public:
	Parser(lex_item *q);
	RPNItem *Analyze();
};

int 
len_str(const char *str);

bool 
cmp_str(const char *str1, const char *str2);

char *
dup_str(const char *str);

int
strtoi(char *s);

class Info_{
	struct list_var{
		const char *word;
		int value; 
		int index; 
		struct list_var *next;
	};
    list_var *data;
public: 
	Info_();
	~Info_();
	void AddVar(const char *word, int index, int value);
	int FindValue(const char *word, int index);
	void AssignValue(const char *word, int value, int index);
}; 

class GameInfo{
    struct PlayerData{
        int id;
        int cash;
        int raw;
        int product;
        int factory;
        PlayerData *next;
    };
    struct AuctionResult{
        int id;
        int sold;
        int bought;
        int selling_price;
        int purchase_price;
        AuctionResult *next;
    };
    int id;
    int active;
    int sells;
    int buys;
    int min_price;
    int max_price;
    int sell_price;
    int sell_count;
    int buy_price;
    int buy_count;
    int prod;
    int build;
    bool game_over;
    PlayerData *data;
    AuctionResult *result;
private:
    void CleanBid();
    void CleanPlayerData();
    void CleanAuctionResult();
public:
    GameInfo(int num);
    ~GameInfo();
    void SetMarket(int *mass,int size);
    void SetPlayer(int *arr,int size);
    void SetResult(int *arr,int size);
    void FinishGame();
    bool GameIsOver() const;
    int SSellPrice() const;
    int SSellCount() const;
    int SBuyPrice() const;
    int SBuyCount() const;
    int SProd() const;
    int SBuild() const;
    void Sell(int count,int price);
    void Buy(int count,int price);
    void Prod(int count);
    void Build(int count);
    int ID() const;
    int Money(int num) const;
    int Raw(int num) const;
    int Production(int num) const;
    int Factories(int num) const;
    int ActivePlayers() const;
    int Supply() const;
    int Demand() const;
    int RawPrice() const;
    int ProdPrice() const;
    int RProdBought(int num) const;
    int RProdPrice(int num) const;
    int RRawPrice(int num) const;
    int RRawSold(int num) const;
};

GameInfo::GameInfo(int num)
{
    id = num;
    data = 0;
    result = 0;
    CleanBid();
    game_over=false;
}

GameInfo::~GameInfo()
{
    CleanPlayerData();
    CleanAuctionResult();
}

void GameInfo::SetMarket(int *mas,int size)
{
    CleanBid();
    active = mas[0];
    sells = mas[1];
    min_price = mas[2];
    buys = mas[3];
    max_price = mas[4];
}

void GameInfo::SetPlayer(int *arr,int size)
{
    int i;
    PlayerData *tmp;
    CleanPlayerData();
    for(i = 0; i<size/5; i++){
        if (data){
            tmp = data;
            while (tmp->next){
                tmp = tmp->next;
            }
            tmp->next = new PlayerData;
            tmp = tmp->next;
        }
        else{
            data = new PlayerData;
            tmp = data;
        }
        tmp->next = 0;
        tmp->id = arr[i*5];
        tmp->cash = arr[i*5+1];
        tmp->raw = arr[i*5+2];
        tmp->product = arr[i*5+3];
        tmp->factory = arr[i*5+4];
    }
}

void GameInfo::SetResult(int *arr,int size)
{
    int i;
    AuctionResult *tmp;
    CleanAuctionResult();
    for(i = 0; i<size/5; i++){
        if (result){
            tmp = result;
            while (tmp->next){
                tmp = tmp->next;
            }
            tmp->next = new AuctionResult;
            tmp = tmp->next;
        }else{
            result = new AuctionResult;
            tmp = result;
        }
        tmp->next = 0;
        tmp->id = arr[i*5];
        tmp->sold = arr[i*5+1];
        tmp->selling_price = arr[i*5+2];
        tmp->bought = arr[i*5+3];
        tmp->purchase_price = arr[i*5+4];
    }
}

void GameInfo::FinishGame()
{
    game_over = true;
}

bool GameInfo::GameIsOver() const
{
    return game_over;
}

int GameInfo::SSellPrice() const
{
    return sell_price;
}

int GameInfo::SSellCount() const
{
    return sell_count;
}

int GameInfo::SBuyPrice() const
{
    return buy_price;
}

int GameInfo::SBuyCount() const
{
    return buy_count;
}

int GameInfo::SProd() const
{
    return prod;
}

int GameInfo::SBuild() const
{
    return build;
}

void GameInfo::Sell(int count,int price)
{
    sell_count = count;
    sell_price = price;
}

void GameInfo::Buy(int count,int price)
{
    buy_count = count;
    buy_price = price;
}

void GameInfo::Prod(int count)
{
    prod = count;
}

void GameInfo::Build(int count)
{
    build = count;
}

int GameInfo::ID() const
{
    return id;
}

int GameInfo::Money(int num) const
{
    PlayerData *tmp;
    tmp=data;
    while(tmp){
        if (tmp->id == num){
            return tmp->cash;
        }
        tmp = tmp->next;
    }
    return 0;
}

int GameInfo::Raw(int num) const
{
    PlayerData *tmp;
    tmp = data;
    while(tmp){
        if (tmp->id == num){
            return tmp->raw;
        }
        tmp = tmp->next;
    }
    return 0;
}

int GameInfo::Production(int num) const
{
    PlayerData *tmp;
    tmp = data;
    while(tmp){
        if (tmp->id == num){
            return tmp->product;
        }
        tmp = tmp->next;
    }
    return 0;
}

int GameInfo::Factories(int num) const
{
    PlayerData *tmp;
    tmp = data;
    while(tmp){
        if (tmp->id == num){
            return tmp->factory;
        }
        tmp = tmp->next;
    }
    return 0;
}

int GameInfo::ActivePlayers() const
{
    return active;
}

int GameInfo::Supply() const
{
    return sells;
}

int GameInfo::Demand() const
{
    return buys;
}

int GameInfo::RawPrice() const
{
    return min_price;
}

int GameInfo::ProdPrice() const
{
    return max_price;
}

int GameInfo::RProdPrice(int num) const
{
    AuctionResult *tmp;
    tmp = result;
    while (tmp){
        if (tmp->id == num){
            return tmp->selling_price;
        }
        tmp = tmp->next;
    }
    return 0;
}

int GameInfo::RProdBought(int num) const
{
    AuctionResult *tmp;
    tmp = result;
    while (tmp){
        if (tmp->id == num){
            return tmp->sold;
        }
        tmp = tmp->next;
    }
    return 0;
}

int GameInfo::RRawPrice(int num) const
{
    AuctionResult *tmp;
    tmp = result;
    while (tmp){
        if (tmp->id == num){
            return tmp->purchase_price;
        }
        tmp = tmp->next;
    }
    return 0;
}

int GameInfo::RRawSold(int num) const
{
    AuctionResult *tmp;
    tmp=result;
    while (tmp){
        if (tmp->id == num){
            return tmp->bought;
        }
        tmp = tmp->next;
    }
    return 0;
}

void GameInfo::CleanBid()
{
    sell_price = 0;
    sell_count = 0;
    buy_price = 0;
    buy_count = 0;
    prod = 0;
    build = 0;
}

void GameInfo::CleanPlayerData()
{
    PlayerData *tmp;
    while (data){
        tmp = data;
        data = data->next;
        delete tmp;
    }
}

void GameInfo::CleanAuctionResult()
{
    AuctionResult *tmp;
    while (result){
        tmp = result;
        result = result->next;
        delete tmp;
    }
}

class RPNElem{
public:
	RPNElem(){}
	virtual ~RPNElem(){}
	virtual void Evaluate(RPNItem **stack,RPNItem **cur_cmd, 
							GameInfo *G, Info_ *T) const = 0;
protected:
	static void Push(RPNItem **stack, RPNElem *elem);
	static RPNElem* Pop(RPNItem **stack);
};

void RPNElem::
Push(RPNItem **stack, RPNElem *elem)
{
    	RPNItem *tmp;
    	tmp = new RPNItem;
    	tmp->elem = elem;
    	tmp->next =* stack;
    	*stack = tmp;
}

RPNElem *RPNElem::
Pop(RPNItem **stack)
{
    	if (*stack){
        	RPNElem *elem = (*stack)->elem;
        	RPNItem *tmp = *stack;
        	*stack = (*stack)->next;
        	delete tmp;
       	 	return elem;
    	}
    	return 0;
}


class RPNConst : public RPNElem{
public:
	virtual ~RPNConst(){}
	virtual void Evaluate(RPNItem **stack, RPNItem **cur_cmd, 
							GameInfo *G, Info_ *T) const
	{
		Push(stack, Clone());
		*cur_cmd = (*cur_cmd)->next; 
	}
	virtual RPNElem* Clone() const = 0;
};

class RPNFunction : public RPNElem{
public: 
	virtual ~RPNFunction(){}
	virtual RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T)															const = 0;
	virtual void Evaluate(RPNItem **stack, RPNItem **cur_cmd, 
							GameInfo *G, Info_ *T) const 
	{
		RPNElem *res = EvaluateFun(stack, G, T);
		if (res)
			Push(stack, res);
		*cur_cmd = (*cur_cmd)->next;
	}
};

class RPNInt : public RPNConst{
	int value; 
public:
	RPNInt(){}
	virtual ~RPNInt(){}
	RPNInt(int a){value = a;}
	virtual RPNElem* Clone() const {return new RPNInt(value);}
	int Get() const {return value;}
};

class RPNString : public RPNConst{
	char* value; 
public: 
	RPNString(){}
	virtual ~RPNString(){}
	RPNString(char *a){value = a;}
	virtual RPNElem* Clone() const {return new RPNString(value);}
	char* Get() const {return value;}
};

class RPNLabel : public RPNConst{
	RPNItem* value;
public:
	virtual ~RPNLabel(){}
	RPNLabel(RPNItem* a) {value = a;}
	virtual RPNElem* Clone() const {return new RPNLabel(value);}
	RPNItem* Get() const {return value;}
};
	
class RPNOpGo : public RPNElem{
public:
	RPNOpGo(){}
	virtual ~RPNOpGo(){}
	void Evaluate(RPNItem **stack,RPNItem **cur_cmd, 
					GameInfo *G, Info_ *T) const 
	{
		RPNElem *op1 = Pop(stack);
		RPNLabel *lab = dynamic_cast<RPNLabel*>(op1);
		if (!lab) throw Error("Error in RPNOpGo");
		RPNItem *addr = lab->Get();
		*cur_cmd = addr; 
		delete op1;
	}
};		

class RPNOpGoFalse : public RPNElem{
public:
	RPNOpGoFalse(){}
	virtual ~RPNOpGoFalse(){}
	void Evaluate(RPNItem **stack, RPNItem **cur_cmd, 
					GameInfo *G, Info_ *T) const 
	{
		RPNElem *op1 = Pop(stack);
		RPNInt *i1 = dynamic_cast<RPNInt*>(op1);
		if (!i1) throw Error("Error in RPNGoFalse");
		RPNElem *op2 = Pop(stack);
		RPNLabel *lab = dynamic_cast<RPNLabel*>(op2);
		if (!lab) throw Error("Error in RPNGoFalse");
		if (!i1->Get()){
			RPNItem *addr = lab->Get();
			*cur_cmd = addr;
		}else{
			*cur_cmd = (*cur_cmd)->next;  
        }
		delete op1;
		delete op2;
	}
};	

class RPNFunVar : public RPNFunction{
public:
	RPNFunVar(){}
	~RPNFunVar(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const
	{
		RPNElem *op1 = Pop(stack); 
		RPNInt *i1 = dynamic_cast<RPNInt*>(op1);
		if (!i1) throw Error("Error in RPNFunVar");
		RPNElem *op2 = Pop(stack);
		RPNString *i2 = dynamic_cast<RPNString*>(op2);
		if (!i2) throw Error("Error in RPNFunVar");
		int res = T->FindValue(i2->Get(), i1->Get());
		delete op1;
		delete op2;
		return new RPNInt(res);
	}
};

class RPNFunAssign : public RPNFunction{
public:
	RPNFunAssign(){}
	~RPNFunAssign(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const 
	{
		RPNElem *op1 = Pop(stack); 
		RPNInt *i1 = dynamic_cast<RPNInt*>(op1);
		if (!i1) throw Error("Error in RPNFunAssign");
		RPNElem *op2 = Pop(stack);
		RPNInt *i2 = dynamic_cast<RPNInt*>(op2);
        if (!i2) throw Error("Error in RPNFunAssign");
		RPNElem *op3 = Pop(stack);
		RPNString *i3 = dynamic_cast<RPNString*>(op3);
		if (!i3) throw Error("Error in RPNFunAssign");
		T->AssignValue(i3->Get(), i1->Get(), i2->Get());
		delete op1;
		delete op2;
		delete op3;
		return 0;
	}
};
	
class RPNFunPlus : public RPNFunction{
public:
	RPNFunPlus(){}
	virtual ~RPNFunPlus(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const 
	{
		RPNElem *op1 = Pop(stack); 
		RPNInt *i1 = dynamic_cast<RPNInt*>(op1);
		if (!i1) throw Error("Error in RPNFunPlus");
		RPNElem *op2 = Pop(stack);
		RPNInt *i2 = dynamic_cast<RPNInt*>(op2);
		if (!i2) throw Error("Error in RPNFunPlus");
		int res = i1->Get() + i2->Get();
		delete op1;
		delete op2;
		return new RPNInt(res);
	}
};
	
class RPNFunMinus : public RPNFunction{
public:
	RPNFunMinus(){}
	virtual ~RPNFunMinus(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const 
	{
		RPNElem *op1 = Pop(stack); 
		RPNInt *i1 = dynamic_cast<RPNInt*>(op1);
		if (!i1) throw Error("Error in RPNFunMinus");
		RPNElem *op2 = Pop(stack);
		RPNInt *i2 = dynamic_cast<RPNInt*>(op2);
		if (!i2) throw Error("Error in RPNFunMinus");
		int res = i2->Get() - i1->Get();
		delete op1;
		delete op2;
		return new RPNInt(res);
	}
};		

class RPNFunMultiply : public RPNFunction{
public:
	RPNFunMultiply(){}
	virtual ~RPNFunMultiply(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const 
	{
		RPNElem *op1 = Pop(stack); 
		RPNInt *i1 = dynamic_cast<RPNInt*>(op1);
		if (!i1) throw Error("Error in RPNFunMultiply");
		RPNElem *op2 = Pop(stack);
		RPNInt *i2 = dynamic_cast<RPNInt*>(op2);
		if (!i2) throw Error("Error in RPNFunMultiply");
		int res = i1->Get() * i2->Get();
		delete op1;
		delete op2;
		return new RPNInt(res);
	}
};

class RPNFunDivision : public RPNFunction{
public:
	RPNFunDivision(){}
	virtual ~RPNFunDivision(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const 
	{
		RPNElem *op1 = Pop(stack); 
		RPNInt *i1 = dynamic_cast<RPNInt*>(op1);
		if (!i1) throw Error("Error in RPNFunDivision");
		RPNElem *op2 = Pop(stack);
		RPNInt *i2 = dynamic_cast<RPNInt*>(op2);
		if (!i2) throw Error("Error in RPNFunDivision");
		int res = i2->Get() / i1->Get();
		delete op1;
		delete op2;
		return new RPNInt(res);
	}
};

class RPNFunNegation : public RPNFunction{
public:
	RPNFunNegation(){}
	virtual ~RPNFunNegation(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const 
	{
		RPNElem *op1 = Pop(stack); 
		RPNInt *i1 = dynamic_cast<RPNInt*>(op1);
		if (!i1) throw Error("Error in RPNFunNegation");
		int res = !(i1->Get());
		delete op1;
		return new RPNInt(res);
	}
};

class RPNFunOr : public RPNFunction{
public:
    RPNFunOr(){}
    virtual ~RPNFunOr(){}
    RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const
    {
        	RPNElem *op1 = Pop(stack);
        	RPNInt *i1 = dynamic_cast<RPNInt*>(op1);
        	if (!i1) throw Error("Error in RPNFunOr");
        	RPNElem *op2 = Pop(stack);
        	RPNInt *i2 = dynamic_cast<RPNInt*>(op2);
        	if (!i2) throw Error("Error in RPNFunOr");
        	int res = i2->Get() || i1->Get();
        	delete op1;
        	delete op2;
        	return new RPNInt(res);
    }
};

class RPNFunAnd : public RPNFunction{
public:
    RPNFunAnd(){}
    virtual ~RPNFunAnd(){}
    RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const
    {
        	RPNElem *op1 = Pop(stack);
        	RPNInt *i1 = dynamic_cast<RPNInt*>(op1);
        	if (!i1) throw Error("Error in RPNFunAnd");
        	RPNElem *op2 = Pop(stack);
        	RPNInt *i2 = dynamic_cast<RPNInt*>(op2);
        	if (!i2) throw Error("Error in RPNFunAnd");
       	 	int res = i2->Get() & i1->Get();
        	delete op1;
        	delete op2;
        	return new RPNInt(res);
    }
};

class RPNFunUnMinus : public RPNFunction{
public:
    RPNFunUnMinus(){}
    virtual ~RPNFunUnMinus(){}
    RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const
    {
        	RPNElem *op1 = Pop(stack);
        	RPNInt *i1 = dynamic_cast<RPNInt*>(op1);
        	if (!i1) throw Error("Error in RPNFunUnMinus");
        	int res = -(i1->Get());
       	 	delete op1;
        	return new RPNInt(res);
    }
};

class RPNFunZero : public RPNFunction{
public:
    	RPNFunZero(){}
    	virtual ~RPNFunZero(){}
    	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const
    	{
        	return 0;
    	}
};

class RPNFunUp : public RPNFunction{
public:
	RPNFunUp(){}
	virtual ~RPNFunUp(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const 
	{
		RPNElem *op1 = Pop(stack); 
		RPNInt *i1 = dynamic_cast<RPNInt*>(op1);
		if (!i1) throw Error("Error in RPNFunUp");
		RPNElem *op2 = Pop(stack);
		RPNInt *i2 = dynamic_cast<RPNInt*>(op2);
		if (!i2) throw Error("Error in RPNFunUp");
		int res = i2->Get() > i1->Get();
		delete op1;
		delete op2;
		return new RPNInt(res);
	}
};

class RPNFunUnder : public RPNFunction{
public:
	RPNFunUnder(){}
	virtual ~RPNFunUnder(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const 
	{
		RPNElem *op1 = Pop(stack); 
		RPNInt *i1 = dynamic_cast<RPNInt*>(op1);
		if (!i1) throw Error("Error in RPNFunUnder");
		RPNElem *op2 = Pop(stack);
		RPNInt *i2 = dynamic_cast<RPNInt*>(op2);
		if (!i2) throw Error("Error in RPNFunUnder");
		int res = i2->Get() < i1->Get();
		delete op1;
		delete op2;
		return new RPNInt(res);
	}
};

class RPNFunNegUp : public RPNFunction{
public:
	RPNFunNegUp(){}
	virtual ~RPNFunNegUp(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const 
	{
		RPNElem *op1 = Pop(stack); 
		RPNInt *i1 = dynamic_cast<RPNInt*>(op1);
		if (!i1) throw Error("Error in RPNFunNegUp");
		RPNElem *op2 = Pop(stack);
		RPNInt *i2 = dynamic_cast<RPNInt*>(op2);
		if (!i2) throw Error("Error in RPNFunNegUp");
		int res = i2->Get() <= i1->Get();
		delete op1;
		delete op2;
		return new RPNInt(res);
	}
};

class RPNFunNegUnder : public RPNFunction{
public:
	RPNFunNegUnder(){}
	virtual ~RPNFunNegUnder(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const 
	{
		RPNElem *op1 = Pop(stack); 
		RPNInt *i1 = dynamic_cast<RPNInt*>(op1);
		if (!i1) throw Error("Error in RPNFunNegUnder");
		RPNElem *op2 = Pop(stack);
		RPNInt *i2 = dynamic_cast<RPNInt*>(op2);
		if (!i2) throw Error("Error in RPNFunNegUnder");
		int res = i2->Get() >= i1->Get();
		delete op1;
		delete op2;
		return new RPNInt(res);
	}
};

class RPNFunEqual : public RPNFunction{
public:
	RPNFunEqual(){}
	virtual ~RPNFunEqual(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const 
	{
		RPNElem *op1 = Pop(stack); 
		RPNInt *i1 = dynamic_cast<RPNInt*>(op1);
		if (!i1) throw Error("Error in RPNFunEqual");
		RPNElem *op2 = Pop(stack);
		RPNInt *i2 = dynamic_cast<RPNInt*>(op2);
		if (!i2) throw Error("Error in RPNFunEqual");
		int res = i1->Get() == i2->Get();
		delete op1;
		delete op2;
		return new RPNInt(res);
	}
};

class RPNFunNegEqual : public RPNFunction{
public:
	RPNFunNegEqual(){}
	virtual ~RPNFunNegEqual(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const 
	{
		RPNElem *op1 = Pop(stack); 
		RPNInt *i1 = dynamic_cast<RPNInt*>(op1);
		if (!i1) throw Error("Error in RPNFunNegEqual");
		RPNElem *op2 = Pop(stack);
		RPNInt *i2 = dynamic_cast<RPNInt*>(op2);
		if (!i2) throw Error("Error in RPNFunNegEqual");
		int res = i2->Get() != i1->Get();
		delete op1;
		delete op2;
		return new RPNInt(res);
	}
};

class RPNFunMyId : public RPNFunction{
public:
	RPNFunMyId(){}
	virtual ~RPNFunMyId(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const 
	{
		int res = G->ID(); 
		return new RPNInt(res);
	}
};

class RPNFunActivePlayers : public RPNFunction{
public:
	RPNFunActivePlayers(){}
	virtual ~RPNFunActivePlayers(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const 
	{
		int res = G->ActivePlayers(); 
		return new RPNInt(res);
	}
};

class RPNFunSupply : public RPNFunction{
public:
	RPNFunSupply(){}
	virtual ~RPNFunSupply(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const 
	{
		int res = G->Supply(); 
		return new RPNInt(res);
	}
};

class RPNFunRawPrice : public RPNFunction{
public:
	RPNFunRawPrice(){}
	virtual ~RPNFunRawPrice(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const 
	{
		int res = G->RawPrice(); 
		return new RPNInt(res);
	}
};

class RPNFunDemand : public RPNFunction{
public:
	RPNFunDemand(){}
	virtual ~RPNFunDemand(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const 
	{
		int res = G->Demand(); 
		return new RPNInt(res);
	}
};

class RPNFunProdPrice : public RPNFunction{
public:
	RPNFunProdPrice(){}
	virtual ~RPNFunProdPrice(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const 
	{
		int res = G->ProdPrice(); 
		return new RPNInt(res);
	}
};

class RPNFunMoney : public RPNFunction{
public:
	RPNFunMoney(){}
	virtual ~RPNFunMoney(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const 
	{
		int res; 
		RPNElem *op1 = Pop(stack); 
		RPNInt *i1 = dynamic_cast<RPNInt*>(op1);
		if (!i1) throw Error("Error in RPNFunMoney");
		res = G->Money(i1->Get());
		delete op1;
		return new RPNInt(res);
	}
};

class RPNFunRaw : public RPNFunction{
public:
	RPNFunRaw(){}
	virtual ~RPNFunRaw(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const 
	{
		int res; 
		RPNElem *op1 = Pop(stack); 
		RPNInt *i1 = dynamic_cast<RPNInt*>(op1);
		if (!i1) throw Error("Error in RPNFunRaw");
		res = G->Raw(i1->Get());
		delete op1;
		return new RPNInt(res);
	}
};

class RPNFunProduction : public RPNFunction{
public:
	RPNFunProduction(){}
	virtual ~RPNFunProduction(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const 
	{
		int res; 
		RPNElem *op1 = Pop(stack); 
		RPNInt *i1 = dynamic_cast<RPNInt*>(op1);
		if (!i1) throw Error("Error in RPNFunProduction");
		res = G->Production(i1->Get());
		delete op1;
		return new RPNInt(res);
	}
};

class RPNFunFactories : public RPNFunction{
public:
	RPNFunFactories(){}
	virtual ~RPNFunFactories(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const 
	{
		int res; 
		RPNElem *op1 = Pop(stack); 
		RPNInt *i1 = dynamic_cast<RPNInt*>(op1);
		if (!i1) throw Error("Error in RPNFunFactories");
		res = G->Factories(i1->Get());
		return new RPNInt(res);
	}
};

class RPNFunRRawSold : public RPNFunction{
public:
	RPNFunRRawSold(){}
	virtual ~RPNFunRRawSold(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const 
	{
		int res; 
		RPNElem *op1 = Pop(stack); 
		RPNInt *i1 = dynamic_cast<RPNInt*>(op1);
		if (!i1) throw Error("Error in RPNFunRRawSold");
		res = G->RRawSold(i1->Get());
		delete op1;
		return new RPNInt(res);
	}
};

class RPNFunRRawPrice : public RPNFunction{
public:
	RPNFunRRawPrice(){}
	virtual ~RPNFunRRawPrice(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const 
	{
		int res; 
		RPNElem *op1 = Pop(stack); 
		RPNInt *i1 = dynamic_cast<RPNInt*>(op1);
		if (!i1) throw Error("Error in RPNFunRRawPrice");
		res = G->RRawPrice(i1->Get());
		delete op1;
		return new RPNInt(res);
	}
};

class RPNFunRProdBought : public RPNFunction{
public:
	RPNFunRProdBought(){}
	virtual ~RPNFunRProdBought(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const 
	{
		int res; 
		RPNElem *op1 = Pop(stack); 
		RPNInt *i1 = dynamic_cast<RPNInt*>(op1);
		if (!i1) throw Error("Error in RPNFunRProdBought");
		res = G->RProdBought(i1->Get());
		delete op1;
		return new RPNInt(res);
	}
};

class RPNFunRProdPrice : public RPNFunction{
public:
	RPNFunRProdPrice(){}
	virtual ~RPNFunRProdPrice(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const 
	{
		int res; 
		RPNElem *op1 = Pop(stack); 
		RPNInt *i1 = dynamic_cast<RPNInt*>(op1);
		if (!i1) throw Error("Error in RPNFunRProdPrice");
		res = G->RProdPrice(i1->Get());
		delete op1;
		return new RPNInt(res);
	}
};

class RPNFunBuy : public RPNFunction{
public:
	RPNFunBuy(){}
	virtual ~RPNFunBuy(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const 
	{
		RPNElem *op1 = Pop(stack); 
		RPNInt *i1 = dynamic_cast<RPNInt*>(op1);
		if (!i1) throw Error("Error in RPNFunBuy");
		RPNElem *op2 = Pop(stack);
		RPNInt *i2 = dynamic_cast<RPNInt*>(op2);
		if (!i2) throw Error("Error in RPNFunBuy");
		G->Buy(i2->Get(), i1->Get());
		delete op1;
		delete op2;
		return 0;
	}
};

class RPNFunSell : public RPNFunction{
public:
	RPNFunSell(){}
	virtual ~RPNFunSell(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const 
	{
		RPNElem *op1 = Pop(stack); 
		RPNInt *i1 = dynamic_cast<RPNInt*>(op1);
		if (!i1) throw Error("Error in RPNFunSell");
		RPNElem *op2 = Pop(stack);
		RPNInt *i2 = dynamic_cast<RPNInt*>(op2);
		if (!i2) throw Error("Error in RPNFunSell");
		G->Sell(i2->Get(), i1->Get());
		delete op1;
		delete op2;
		return 0;
	}
};

class RPNFunProd : public RPNFunction{
public:
	RPNFunProd(){}
	virtual ~RPNFunProd(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const 
	{
		RPNElem *op1 = Pop(stack); 
		RPNInt *i1 = dynamic_cast<RPNInt*>(op1);
		if (!i1) throw Error("Error in RPNFunProd");
		G->Prod(i1->Get());
		delete op1;
		return 0;
	}
};

class RPNFunBuild : public RPNFunction{
public:
	RPNFunBuild(){}
	virtual ~RPNFunBuild(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const 
	{
		RPNElem *op1 = Pop(stack); 
		RPNInt *i1 = dynamic_cast<RPNInt*>(op1);
		if (!i1) throw Error("Error in RPNFunProd");
		G->Build(i1->Get());
		delete op1;
		return 0;
	}
};

class RPNFunPrint : public RPNFunction{
public:
	RPNFunPrint(){}
	virtual ~RPNFunPrint(){}
	RPNElem* EvaluateFun(RPNItem **stack, GameInfo *G, Info_ *T) const 
	{
		RPNElem *op1 = Pop(stack); 
		RPNString *i1 = dynamic_cast<RPNString*>(op1);
        RPNInt *i2 = dynamic_cast<RPNInt*>(op1);
		if (!i1&&!i2) throw Error("Error in RPNFunPrint");
		if (i1){
            fprintf(stderr,"%s\n",i1->Get());
        }else{
            fprintf(stderr,"%i\n",i2->Get());
        }
		delete op1;
		return 0;
	}
};

Error::
Error(const char *str)
{
	msg = dup_str(str);
	lex = 0;
}	

Error::
Error(const char *str, lex_item *q)
{
	msg = dup_str(str); 
	lex = q; 
}

Error::
~Error()
{
	if (msg){
		delete []msg;
	}
}

Error::
Error(const Error& Drop)
{
	msg = dup_str(Drop.msg);
	lex = Drop.lex;
}

void Error::
Report()
{
    fprintf(stderr,"%s\n",msg);
	if (lex){
        fprintf(stderr, "Line: %i\n", lex->line);
		fprintf(stderr, "Word: %s\n", lex->word);
	}
}

Scan::
Scan()
{
	flag = beg;
	cur_line = 1;
	check = false;
	f = 0;
	q = 0;
}

Scan::
~Scan()
{
	Cln();
}

void Scan::
Feed(char c)
{
	Step(c);
	if (check){
		check = false;
		Step(c);
	}
	if (c == '\n'){
		cur_line = cur_line + 1;
	}
}

lex_item *Scan::
Get_lex()
{
	lex_item *tmp = f;
	if (flag == error){
		while (tmp->next){
			tmp = tmp->next;
		}
		throw Error("Lex Error", tmp);
	}
	if (flag != beg){
		throw Error("Doesn't return to begin", tmp);
	}
	return f;
}

void Scan::
Step(char c)
{
	switch (flag)
	{
		case beg:
			Handl_beg(c);
			break;
		case keywd:
			Handl_keywd(c);
			break;
		case quote:
			Handl_quote(c);
			break;
		case count:
			Handl_count(c);
			break; 
		case ident:
			Handl_ident(c);
			break;
		case equal:
			Handl_equal(c);
			break;
		case negation:
			Handl_negation(c);
			break;
		case negup:
			Handl_negup(c);
			break;
		case negunder:
			Handl_negunder(c); 
			break;
		case up:
			Handl_up(c);
			break;
		case under:
			Handl_under(c);
			break;
		case error:
			flag = error;
	}
}

void Scan::
Handl_beg(char c) 
{
	switch (c)
	{
		case ('"'):
			flag = quote; 
			return;
		case ('='):
            		Append(c);
			flag = equal;
			return;
		case ('!'):
            		Append(c);
			flag = negation;
			return;  
		case ('>'):
            		Append(c);
			flag = up;
			return;  
		case ('<'):
            		Append(c);
			flag = under;
			return; 
		case (' '):
		case ('\t'):
		case ('\n'):
			return;
	}
	Append(c);
	if (c == '$'|| c == '?'){
		flag = ident;
		return;
	}
	if (Is_op(c)){
		Add_lex(operation);
		return; 
	}
	if (c == '{'|| c  == '}'|| c == ';'){
		Add_lex(punctuator);
		return; 
	}
	if (c >= '0' && c <= '9'){
		flag = count;
		return;
	}
	if (c >= 'a' && c <= 'z'){
		flag = keywd;
		return;
	}
	Add_lex(defect);
	flag = error;
}

void Scan::
Handl_keywd(char c)
{
	if (c >= 'a' && c <= 'z'){
		Append(c);
	}else{
		if (Check_keywd()){
			Add_lex(keyword);
			check = true;
			flag = beg;
		}else{
			Add_lex(defect);
			flag = error;
		}
	}
}

void Scan::
Handl_quote(char c)
{
	if (c == '"'){
		Add_lex(string);
		flag = beg; 
	}else{
		Append(c);
	}
}

void Scan::
Handl_count(char c)
{
	if (c >= '0' && c <='9'){
		Append(c);
	}else{
		Add_lex(constant);
		check = true;
		flag = beg;
	}
}

void Scan::
Handl_ident(char c)
{
	if ((c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z') || 
		(c >= '0' && c <= '9') ||
        (c == '_')){
		Append(c);
	}else{
		if (Check_ident()){
			Add_lex(identifier);
			check = true;
			flag = beg; 
		}else{
			Add_lex(defect);
			flag = error;
		}
	}
}

void Scan::
Handl_equal(char c)
{
	if (c == '?'){
		Append(c);
		Add_lex(operation);
		flag = beg; 
	}else{
		Add_lex(operation);
		check = true;
		flag = beg;
	}
}

void Scan::
Handl_negation(char c)
{
	if (c == '='){
		Append(c);
		Add_lex(operation);
		flag = beg; 
	}else{
		if (c == '>'){
			flag = negup;
		}else{
			if (c == '<'){
				flag = negunder;
			}else{
				Add_lex(operation);
				check = true;
				flag = beg;
			}
		}
	}
}

void Scan::
Handl_negup(char c)
{
	Append(c);
	Add_lex(operation); 
	flag = beg;
}

void Scan::
Handl_negunder(char c)
{
	Append(c);
	Add_lex(operation);
	flag = beg;
}

void Scan::
Handl_up(char c)
{
	if (c == '='){
		Append(c);
		Add_lex(operation);
		flag = beg;
	}else{
		Add_lex(operation);
		check = true;
		flag = beg;
	}
}

void Scan::
Handl_under(char c)
{
	if (c == '='){
		Append(c);
		Add_lex(operation);
		flag = beg;
	}else{
		Add_lex(operation);
		check = true;
		flag = beg;
	}
}

void Scan::
Add_lex(mark note)
{
	lex_item *tmp; 
	if (!q){
		return;
	}
	if (f){
		tmp = f; 
		while (tmp->next){
			tmp = tmp->next; 
		}
		tmp->next = new lex_item;
		tmp = tmp->next; 
	}else{
		f = new lex_item;
		tmp = f;
	}
	tmp->next = 0;
	tmp->line = cur_line; 
	tmp->type = note; 
	tmp->word = Get_word();
	Cln();
}

bool Scan::
Check_ident()
{
	const char *const func[] = {
		"?buy", "?sell", "?prod", "?build", "?my_id",
		"?active_players", "?supply", "?raw_price",
		"?demand", "?production_price","?production",
		 "?money",
		"?raw", "?factories", "?result_raw_sold",
		"?result_raw_price", "?result_prod_bought",
		"?result_prod_price", "?print", 0
	};	
	char *lex = Get_word();
	int i; 
	if (lex[0] != '?'){
		delete []lex;
		return true;
	}
	for (i = 0; func[i] != 0; i++){
		if (!cmp_str(lex, func[i])){
			delete []lex;
			return true;
		}
	}
	delete []lex;
	return false;
}

bool Scan::
Check_keywd()
{
	const char *const keys[] = {
		"end", "if", "while", 0
	};
	char *lex = Get_word();
	int i; 
	for (i = 0; keys[i] != 0; i++){
		if (!cmp_str(lex, keys[i])){
			delete []lex;
			return true;
		}
	}
	delete []lex;
	return false;
}

bool Scan::
Is_op(char c)
{
	int i;
	const char str[] = "+-*/|&><!?=()[],";
	for (i = 0; str[i] != '\0'; i++){
		if (str[i] == c){
			return true;
		}
	}
	return false; 
}

void Scan::
Append(char c)
{
	item *tmp;
	if (q){
		tmp = q;
		while (tmp->next){
			tmp = tmp->next;
		}
		tmp->next = new item;
		tmp = tmp->next;
	}else{
		q = new item;
		tmp = q;
	}
	tmp->symbol = c;
	tmp->next = 0;
}

void Scan::
Cln()
{
	item *tmp;
	while (q){
		tmp = q;
		q = q->next;
		delete tmp;
	}
}

int Scan::
Length()
{
	item *tmp = q;
	int len;
	for (len = 0; tmp; len++){
		tmp = tmp->next;;
	}
	return len;
}

char *Scan::
Get_word()
{
	int i, n = Length();
	item *tmp = q;
	char *str = new char [n+1];
	for (i = 0; i < n; i++){
		str[i] = tmp->symbol; 
		tmp = tmp->next;
	}
	str[n] = '\0';
	return str;
}

Parser::
Parser(lex_item *q)
{
	cur_lex = q;
	stack = 0;
	prog = 0;
}

RPNItem *Parser::
Analyze()
{
	if (cur_lex){
		S();
		fprintf(stderr, "Compilation is succes\n");
		fprintf(stderr, "%i lines compiled\n", cur_lex->line);
		return prog;
	}else{
		throw Error("Unexpected input token", cur_lex);
	}
}

void Parser::
next()
{
	if (cur_lex->next){
		cur_lex = cur_lex->next;
	}else{
		throw Error("Unexpected end of token", cur_lex);
	}
}

void Parser::
S()
{
	A();
	if (!Is_lex("end")){
		throw Error("Expected keyword end", cur_lex);
	}
}

void Parser::
A()
{
	if (Is_lex("{")){
		next();
		B();
		while(Is_lex(";")){
			next();
			B();
		}
		if (!Is_lex("}")){
			throw Error("Expected }", cur_lex);
		}
		next();
	}else{
		throw Error("Expected {", cur_lex);
	}
}

void Parser::
B()
{
    if (Is_lex("while")){
        next();
		RPNItem *tmp,*tmp2;
    	Add(new RPNFunZero);
    	tmp2 = GetLast();
    	tmp = Blank();
    	C();
    	Add(new RPNOpGoFalse);
    	A();
    	Add(new RPNLabel(tmp2));
    	Add(new RPNOpGo);
    	Add(new RPNFunZero);
    	tmp->elem = new RPNLabel(GetLast());
	}else{
        if (Is_lex("if")){
			next();
			RPNItem *tmp;
   			tmp = Blank();
    		C();
    		Add(new RPNOpGoFalse);
    		A();
    		Add(new RPNFunZero);
    		tmp->elem = new RPNLabel(GetLast());
		}else{
		    if (Is_func()){
			    Push(Create());
			    next();
			    D1();
			    Add(Pop());
		    }else{
			    if (Is_var()){
				    Add(new RPNString(cur_lex->word));
				    next();
				    D2();
			    }
            }
		}
	}
}

void Parser::
D1()
{
	if (Is_lex("(")){
		next();
		if (Is_lex(")")){
			next();
		}else{
			if (Is_str()){
                Add(new RPNString(cur_lex->word));
				next();
			}else{
				C();
				if (Is_lex(",")){
					next();
					C();
				}
			}
			if (!Is_lex(")")){
				throw Error("Expected ) after last parameter", cur_lex);
			}
			next();
		}
	}else{
        throw Error("Expected ( before first parameter", cur_lex);
	}
}

void Parser::
D2()
{
    H();
	if (Is_lex("=")){
		next();
		C();
	}else{
		throw Error("Expected =", cur_lex);
	}
	Add(new RPNFunAssign);
}

void Parser::
C()
{
	C1();
	if (Is_lex("=?") || Is_lex("!=") ||
		Is_lex(">=") || Is_lex("<=") ||
		Is_lex(">") || Is_lex("<") ||
		Is_lex("!>") || Is_lex("!<")){
		if (Is_lex("?=")){
			Push(new RPNFunEqual);
		}
		if (Is_lex("!=")){
			Push(new RPNFunNegEqual);
		}
		if (Is_lex(">=") || Is_lex("!<")){
			Push(new RPNFunNegUnder);
		}
		if (Is_lex("<=") || Is_lex("!>")){
			Push(new RPNFunNegUp);
		}
		if (Is_lex(">")){
			Push(new RPNFunUp);
		}
		if (Is_lex("<")){
			Push(new RPNFunUnder);
		}
		next();
		C1();
		Add(Pop());
	}
}

void Parser::
C1()
{
	C2();
	while (Is_lex("+") || Is_lex("-") || Is_lex("|")){
		if (Is_lex("+")){
			Push(new RPNFunPlus);
		}
		if (Is_lex("-")){
			Push(new RPNFunMinus);
		}
		if (Is_lex("|")){
			Push(new RPNFunOr);
		}
		next();
		C2();
		Add(Pop());
	}
}

void Parser::
C2()
{
	C3();
	while (Is_lex("*") || Is_lex("/") || Is_lex("&")){
		if (Is_lex("*")){
			Push(new RPNFunMultiply);
		}
		if (Is_lex("/")){
			Push(new RPNFunDivision);
		}
		if (Is_lex("&")){
			Push(new RPNFunAnd);
		}
		next();
		C3();
		Add(Pop());
	}
}

void Parser::
C3()
{
	if (Is_var()){
		Add(new RPNString(cur_lex->word));
		next();
		H();
		Add(new RPNFunVar);
	}else{
		if (Is_func()){
			Push(Create());
			next();
			D1();
			Add(Pop());
		}else{
			if (Is_const()){
				Add(new RPNInt(strtoi(cur_lex->word)));
				next();
			}else{
				if (Is_lex("(")){
					next();
					C();
				if (!Is_lex(")")){
					throw Error("Expected close bracket", cur_lex);
				}
				next();
				}else{
					if (Is_lex("!")){
						Push(new RPNFunNegation);
						next();
						C3();
						Add(Pop());
					}else{
							if(Is_lex("-")){
								Push(new RPNFunUnMinus);
								next();
								C3();
								Add(Pop());
							}else{
							    throw Error("Expected operand", cur_lex);
							}
						}
                    }
				}
			}
		}
}

void Parser::
H()
{
	if (Is_lex("[")){
		next();
		C();
		if (!Is_lex("]")){
			throw Error("Expected ] after index", cur_lex);
		}
		next();
	}else{
		Add(new RPNInt(0));
	}
}

bool Parser::
Is_lex(const char *str)
{
	return (!cmp_str(cur_lex->word, str));
}

bool Parser::
Is_var()
{
	return (cur_lex->type == identifier && cur_lex->word[0] == '$');
}

bool Parser::
Is_func()
{
	return (cur_lex->type == identifier && cur_lex->word[0] == '?');
}

bool Parser::
Is_const()
{
	return cur_lex->type == constant;
}

bool Parser::
Is_str()
{
	return cur_lex->type == string;
}

void Parser::
Add(RPNElem *elem)
{
    	RPNItem *tmp;
    	tmp = Blank();
    	tmp->elem = elem;
}

void Parser::
Push(RPNElem *elem)
{
    	RPNItem *tmp;
    	tmp = new RPNItem;
    	tmp->next = stack;
    	tmp->elem = elem;
    	stack = tmp;
}

RPNElem *Parser::
Pop()
{
    	RPNItem *tmp;
    	RPNElem *elem;
    	elem = stack->elem;
    	tmp = stack;
    	stack = stack->next;
    	delete tmp;
    	return elem;
}

RPNItem *Parser::
Blank()
{
    	RPNItem *tmp;
    	if (prog){
        	tmp = GetLast();
        	tmp->next = new RPNItem;
        	tmp = tmp->next;
    	}else{
        	prog = new RPNItem;
        	tmp = prog;
    	}
    	tmp->next = 0;
    	return tmp;
}

RPNItem *Parser::
GetLast()
{
    	RPNItem *tmp = prog;
    	while (tmp->next){
        	tmp = tmp->next;
    	}
    	return tmp;
}

RPNElem *Parser::
Create()
{
    	if (Is_lex("?sell")){
        	return new RPNFunSell;
	}
    	if (Is_lex("?buy")){
        	return new RPNFunBuy;
	}
    	if (Is_lex("?prod")){
        	return new RPNFunProd;
	}
    	if (Is_lex("?build")){
        	return new RPNFunBuild;
	}
    	if (Is_lex("?my_id")){
        	return new RPNFunMyId;
	}
    	if (Is_lex("?money")){
        	return new RPNFunMoney;
	}
    	if (Is_lex("?raw")){
        	return new RPNFunRaw;
	}
	if (Is_lex("?production")){
		return new RPNFunProduction;
	}
    	if (Is_lex("?production_price")){
        	return new RPNFunProdPrice;
	}
    	if (Is_lex("?factories")){
        	return new RPNFunFactories;
	}
    	if (Is_lex("?active_players")){
        	return new RPNFunActivePlayers;
	}
    	if (Is_lex("?supply")){
        	return new RPNFunSupply;
	}
    	if (Is_lex("?demand")){
        	return new RPNFunDemand;
	}
    	if (Is_lex("?raw_price")){
        	return new RPNFunRawPrice;
	}
    	if (Is_lex("?rprod_price")){
        	return new RPNFunRProdPrice;
	}
    	if (Is_lex("?rprod_bought")){
        	return new RPNFunRProdBought;
	}
    	if (Is_lex("?rraw_price")){
        	return new RPNFunRRawPrice;
	}
    	if (Is_lex("?rraw_sold")){
        	return new RPNFunRRawSold;
	}
    	if (Is_lex("?print")){
        	return new RPNFunPrint;
    	}
    	return new RPNFunZero;
}

Info_::
Info_()
{
	data = 0;
}

Info_::
~Info_()
{
	struct list_var *tmp; 
	while (data){
		tmp = data; 
		data = data->next;  
		delete tmp; 
	}
}

void Info_::
AddVar(const char *word, int index, int value)
{
    list_var *tmp = new list_var;
    tmp->next=data;
    tmp->word=word;
    tmp->index=index;
    tmp->value=value;
    data=tmp;
}

int Info_::
FindValue(const char *word, int index)
{
	struct list_var *tmp = data; 
	while (tmp){
		if (!cmp_str(tmp->word, word) && tmp->index == index){
			return tmp->value;
		}
        tmp=tmp->next;
	}
	AddVar(word,index,0);
	return 0;
}

void Info_::
AssignValue(const char *word, int value, int index)
{
	struct list_var *tmp = data; 
	while (tmp){
		if (!cmp_str(tmp->word, word) && tmp->index == index){
            tmp->value=value;
            return;
		}
        tmp=tmp->next;
	}
	AddVar(word, index, value);
}

int
len_str(const char *str)
{
	int i, len = 0;
	if (str){
		for (i = 0; str[i] != '\0'; i++, len++){
		}
	}
	return len;
}

bool
cmp_str(const char *str1, const char *str2)
{
	int i;
	if (str1 && str2){
		if (len_str(str1) == len_str(str2)){
			for (i = 0; i <= len_str(str1); i++){
				if (str1[i] != str2[i]){
					return true;
				}
			}
		}else{
			return true;
		}
	}else{
		return true;
	}
	return false;
}

char *
dup_str(const char *str)
{
	int i, len = len_str(str);
	char *tmp = new char[len+1];
	for (i = 0; i < len; i++){
		tmp[i] = str[i];
	}
	tmp[i] = '\0';
	return tmp;
} 

void
del_lex(lex_item *f)
{
	lex_item *tmp; 
	while (f){
		tmp = f; 
		f = f->next; 
		delete[] tmp->word;
		delete tmp;
	}
}

int
strtoi(char *s)
{
	int n = 0;
	while (*s >= '0' && *s <= '9'){
		n *= 10;
		n += *s++;
		n -= '0';
	}
	return n;
}

char *
reverse_str(char *s)
{
	int i, j; 
	char c; 
	for (i = 0, j = len_str(s) - 1; i < j; i++, j--){
		c = s[i];
		s[i] = s[j]; 
		s[j] = c;
	}
    	return s;
}

char *
itost(int n)
{
	char *s = new char[16]; 
	int i, sign; 
	if ((sign = n) < 0){
		n = -n;
	}
	i = 0;
	do{
		s[i++] = n % 10 + '0';
	} while ((n /= 10) > 0);
	if (sign < 0){
		s[i++] = '-';
	}
	s[i] = '\0';
	s = reverse_str(s);
	return s;
}

void 
ClnAll(lex_item *token, RPNItem *prog)
{
    	lex_item *tmp;
        RPNItem *tmp2;
    	while (token){
        	tmp = token;
        	token = token->next;
        	delete []tmp->word;
       	 	delete tmp;
    	}
        while (prog){
                tmp2 = prog;
                prog = prog->next;
                delete tmp2;
        }
}

void 
send_msg(int sd, const char *msg)
{
    	write(sd, msg, len_str(msg));
}

void 
send_cnt(int sd, int count)
{
    	char *msg = itost(count);
    	send_msg(sd, msg);
    	delete []msg;
}

void 
send_cmd(int sd, const char *cmd, int arg1, int arg2)
{
    	send_msg(sd, cmd);
    	if (arg1 != -1){
        	send_msg(sd, " ");
        	send_cnt(sd, arg1);
    	}
    	if (arg2 != -1){
        	send_msg(sd, " ");
        	send_cnt(sd, arg2);
    	}
    	send_msg(sd, "\n");
}

item *
add_elem(struct item *first, int elem)
{
    	struct item *tmp;
    	if (first){
        	tmp = first;
        	while (tmp->next){
            		tmp = tmp->next;
        	}
        	tmp->next = new struct item;
        	tmp = tmp->next;
    	}else{
        	first = new struct item;
        	tmp = first;
    	}
    	tmp->next = 0;
    	tmp->count = elem;
    	return first;
}

int
length_item(struct item *tmp)
{
    	int length = 0;
    	for (length = 0; tmp; length++){
        	tmp = tmp->next;
    	}
    	return length;
}

int 
*item2arr(struct item *tmp)
{
    	int i, N = length_item(tmp);
    	int *arr = new int[N];
    	for(i = 0; i < N; i++){
        	arr[i] = tmp->count;
        	tmp = tmp->next;
    	}
    	return arr;
}

void 
clean_item(struct item *first)
{
    	struct item *tmp;
    	while (first){
        	tmp = first;
        	first = first->next;
        	delete tmp;
    	}
}

void 
copy(char *str, const struct buffer *data, int pos1, int pos2)
{
    	int i, j = 0;
    	for(i = pos1; i < pos2; i++){
        	str[j] = data->buf[i];
        	j++;
    	}
    	str[j] = '\0';
}

item *
get_counts(const char *str, struct item *first)
{
    	int count, digit, i = 1;
    	if (str[0] != '%'){
        	return first;
    	}
    	while (str[i] != '\0'){
        	count = 0;
        	digit = -1;
        	for (; str[i] >= '0' && str[i] <= '9'; i++){
            		digit = str[i] - '0';
            		count *= 10;
            		count += digit;
        	}
        	if (digit != -1){
            		first = add_elem(first, count);
        	}
        	for (; str[i] == ' '; i++){
        		;
        	}
    	}
    	return first;
}

int *
analyze(const struct buffer *msg, int *size)
{
    	int i, j = 0;
	int *arr;
    	char *str;
    	struct item *first = 0;
    	for(i = 0; i < msg->buf_used; i++){
        	if (msg->buf[i] == '\n'){
            	str = new char[i-j+1];
            	copy(str, msg, j, i);
            	first = get_counts(str, first);
            	delete []str;
            	j = i+1;
        	}
    	}
    	arr = item2arr(first);
    	*size = length_item(first);
    	clean_item(first);
    	return arr;
}

bool 
read_msg(int sd, struct buffer *msg)
{
    	int busy = 0, rc;
    	do{
        	rc = read(sd, msg->buf+busy, INBUFSIZE-busy);
        	if (rc == 0){
            	return false;
        	}
        	busy += rc;
        	if (busy >= INBUFSIZE){
            	busy = 0;
        	}
    	}while (msg->buf[busy-1] != '\r');
    	msg->buf_used = busy-1;
    	return true;
}

int 
GetId(int sd)
{
	int id;
    	buffer msg;
    	int *arr, size;
    	read_msg(sd, &msg);
    	arr = analyze(&msg, &size);
	id = arr[0];
    	delete []arr;
	return id; 
}

void 
GetMarket(int sd, GameInfo *G)
{
    	buffer msg;
    	int *arr, size;
    	send_cmd(sd, "market", -1, -1);
    	read_msg(sd, &msg);
    	arr = analyze(&msg, &size);
    	G->SetMarket(arr, size);
    	delete []arr;
}

void 
GetPlayers(int sd, GameInfo *G)
{
    	buffer msg;
    	int *arr;
	int size;
    	send_cmd(sd, "info", -1, -1);
    	read_msg(sd, &msg);
    	arr = analyze(&msg, &size);
    	G->SetPlayer(arr, size);
    	delete []arr;
}

void
GetAuction(int sd, GameInfo *G)
{
    	buffer msg;
    	int *arr;
	int size;
    	if (!read_msg(sd, &msg)){
        	exit(1);
    	}
    	arr = analyze(&msg, &size);
    	G->SetResult(arr, size);
    	delete []arr;
}

void 
SendCommands(int sd, GameInfo *G)
{
    	if (G->SSellCount()){
        	send_cmd(sd, "sell", G->SSellCount(), G->SSellPrice());
    	}
    	if (G->SBuyCount()){
        	send_cmd(sd, "buy", G->SBuyCount(), G->SBuyPrice());
    	}
    	if (G->SProd()){
        	send_cmd(sd, "prod", G->SProd(), -1);
    	}
    	if (G->SBuild()){
        	send_cmd(sd, "build", G->SBuild(), -1);
    	}
    	send_cmd(sd, "turn", -1, -1);
}

lex_item *
scan_proc()
{
	int c;
	Scan S; 
	lex_item *token;
	while ((c = getchar()) != EOF){
		S.Feed(c);
	}
	try{
		token = S.Get_lex();
	}
	catch(Error Drop){
		fprintf(stderr, "Scan : ");
		Drop.Report();
		exit(1);
	}
	return token;
}

RPNItem *
parser_proc(lex_item *token)
{
	Parser P(token);
	RPNItem *prog;
	try{
		prog = P.Analyze();
	}
	catch(Error Drop){
		fprintf(stderr, "Parser: ");
		Drop.Report();
		exit(1);
	}
	return prog;
}

void
exec_proc(RPNItem *prog, GameInfo *G, Info_ *T)
{
    RPNItem *stack=0;
    try{
        while (prog){
            (prog->elem)->Evaluate(&stack, &prog, G, T);
        }
    }
	catch(Error Drop){
		fprintf(stderr, "Execter: ");
		Drop.Report();
		exit(1);
	}
}

void 
initialize(const char *ip, int port)
{
	    int sd;
    	struct sockaddr_in addr;
    	addr.sin_family = AF_INET;
    	addr.sin_port = htons(port);
    	if (!inet_aton(ip, &(addr.sin_addr))){
        	fprintf(stderr, "Invalid IP\n");
        	exit(1);
    	}
    	lex_item *token = scan_proc();
    	RPNItem *prog = parser_proc(token);
    	sd = socket(AF_INET, SOCK_STREAM,0);
    	if (sd == -1){
        	perror("socket");
        	exit(1);
    	}
    	if (0 != connect(sd, (struct sockaddr*)&addr, sizeof(addr))){
        	perror("connect");
        	exit(1);
    	}
    	try{
    		Info_ T;
    		GameInfo G(GetId(sd));
    		for (;;){
        		GetMarket(sd, &G);
        		GetPlayers(sd, &G); 
        		exec_proc(prog, &G, &T);
        		SendCommands(sd, &G);
        		GetAuction(sd, &G);
    		}
    	}
    	catch (Error Drop){
        	Drop.Report();
    	}
    	shutdown(sd, 2);
    	close(sd);
    	ClnAll(token, prog);
}

int main(int argc, char **argv)
{
	int fd, port;
	if (argc != 4){
        	fprintf(stderr, "Wrong number of arguments\n");
        	exit(1);
    	}
    	port = strtoi(argv[2]);
    	if (port == -1){
        	fprintf(stderr, "Wrong port\n");
        	exit(1);
    	}
    	fd = open(argv[3], O_RDONLY);
    	if (fd == -1){
        	perror(argv[3]);
        	exit(1);
    	}
    	dup2(fd, 0);
    	close(fd);
    	initialize(argv[1], port);
    	return 0;
}

