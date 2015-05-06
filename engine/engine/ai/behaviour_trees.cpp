#include "mcv_platform.h"
#include "behaviour_trees.h"

bt::bt()
{
	tree = new map<string, btnode *>();
	actions= new map<string, btaction>();
	conditions = new map<string, btcondition>();
	state_time = 0.f;
	on_enter = true;
	previous = nullptr;
}

bt::~bt()
{
	delete tree;
	tree = nullptr;

	delete actions;
	actions = nullptr;

	delete conditions;
	conditions = nullptr;
}

void bt::create(string s)
{
	name = s;
}


btnode *bt::createNode(string s)
{
	if (findNode(s) != NULL)
	{
		printf("Error: node %s already exists\n", s.c_str());
		return NULL;	// error: node already exists
	}
	btnode *btn = new btnode(s);
	tree->operator[](s)=btn;
	//tree[s] = btn;
	return btn;
}


btnode *bt::findNode(string s)
{
	if (tree->find(s) == tree->end()) return NULL;
	//else return tree[s];
	else return tree->operator[](s);
}


btnode *bt::createRoot(string s, int type, btcondition btc, btaction bta)
{
	btnode *r = createNode(s);
	r->setParent(NULL);
	root = r;
	r->setType(type);
	if (btc != NULL) addCondition(s, btc);
	if (bta != NULL) addAction(s, bta);

	current = NULL;
	return r;
}


btnode *bt::addChild(string parent, string son, int type, typeInterrupAllowed kind, btcondition btc, btaction bta)
{
	if (validateNode(parent, type)){
		btnode *p = findNode(parent);
		btnode *s = createNode(son);
		p->addChild(s);
		s->setParent(p);
		s->setType(type);
		s->setTypeInter(kind);
		s->setNodeWeight(0);
		if (btc != NULL) addCondition(son, btc);
		if (bta != NULL) addAction(son, bta);
		return s;
	}else{
		//METER UN xassert
		CErrorContext ec("Adding child", "Behaviour trees");
		//fatal("node not valid\n");
		XASSERT(validateNode(parent, type), "Child not valid");
		return nullptr;
	}
}

btnode *bt::addChild(string parent, string son, int type, btcondition btc, btaction bta)
{
	if (validateNode(parent, type)){
		btnode *p = findNode(parent);
		btnode *s = createNode(son);
		p->addChild(s);
		s->setParent(p);
		s->setType(type);
		s->setTypeInter(BOTH);
		s->setNodeWeight(0);
		if (btc != NULL) addCondition(son, btc);
		if (bta != NULL) addAction(son, bta);
		return s;
	}
	else{
		//METER UN xassert
		//fatal("node not valid\n");
		CErrorContext ec("Adding child", "Behaviour trees");
		XASSERT(validateNode(parent, type), "Child not valid");
		return nullptr;
	}
}

btnode *bt::addChild(string parent, string son, int type, typeInterrupAllowed kind ,subType subType, btcondition btc, btaction bta)
{
	if (validateNode(parent, type, subType)){
		btnode *p = findNode(parent);
		btnode *s = createNode(son);
		p->addChild(s);
		s->setParent(p);
		s->setType(type);
		s->setDecSubType(subType);
		s->setTypeInter(kind);
		s->setNodeWeight(0);
		if (btc != NULL) addCondition(son, btc);
		if (bta != NULL) addAction(son, bta);
		return s;
	}else{
		//METER UN xassert
		//fatal("DECORATION node not valid\n");
		CErrorContext ec("Adding child", "Behaviour trees");
		XASSERT(validateNode(parent, type, subType), "Child not valid. Decorator type invalid");
		return nullptr;
	}
}

btnode *bt::addChild(string parent, string son, int type, subType subType, btcondition btc, btaction bta)
{
	if (validateNode(parent, type, subType)){
		btnode *p = findNode(parent);
		btnode *s = createNode(son);
		p->addChild(s);
		s->setParent(p);
		s->setType(type);
		s->setDecSubType(subType);
		s->setTypeInter(BOTH);
		s->setNodeWeight(0);
		if (btc != NULL) addCondition(son, btc);
		if (bta != NULL) addAction(son, bta);
		return s;
	}
	else{
		//METER UN xassert
		//fatal("DECORATION node not valid\n");
		CErrorContext ec("Adding child", "Behaviour trees");
		XASSERT(validateNode(parent, type, subType), "Child not valid. Decorator type invalid");
		return nullptr;
	}
}

btnode *bt::addChild(string parent, string son, int type, typeInterrupAllowed kind, btcondition btc, btaction bta, int weight)
{
	if (validateNode(parent, type)){
		btnode *p = findNode(parent);
		btnode *s = createNode(son);
		p->addChild(s);
		s->setParent(p);
		s->setType(type);
		s->setTypeInter(kind);
		if (btc != NULL) addCondition(son, btc);
		if (bta != NULL) addAction(son, bta);
		s->setNodeWeight(weight);
		return s;
	}
	else{
		//METER UN xassert
		CErrorContext ec("Adding child", "Behaviour trees");
		//fatal("node not valid\n");
		XASSERT(validateNode(parent, type), "Child not valid");
		return nullptr;
	}
}

bool bt::validateNode(string parent, int type){
	btnode *p = findNode(parent);
	if ((p != nullptr) && (type != 0) && (type != DECORATOR)){
		return true;
	}else{
		return false;
	}
}
bool bt::validateNode(string parent, int type, subType subType){
	btnode *p = findNode(parent);
	if ((p != nullptr) && (type == DECORATOR)){// && (subType != NULL)){
		return true;
	}else{ 
		return false; 
	}
}


void bt::recalc(float deltaTime)
{
	state_changed = false;
	state_time += deltaTime;
	if (current == NULL) root->recalc(this);	// I'm not in a sequence, start from the root
	else current->recalc(this);				    // I'm in a sequence. Continue where I left

}

void bt::setCurrent(btnode *nc)
{
	current = nc;
}


void bt::addAction(string s, btaction act)
{
	if (actions->find(s) != actions->end())
	{
		printf("Error: node %s already has an action\n", s.c_str());
		return;	// if action already exists don't insert again...
	}
	//actions[s] = act;
	actions->operator[](s) = act;
}


int bt::execAction(string s, btnode* th)
{
	if (actions->find(s) == actions->end())
	{
		printf("ERROR: Missing node action for node %s\n", s.c_str());
		return LEAVE; // error: action does not exist
	}
	checkIfStateChanged(th);
	return (this->*actions->operator[](s))();
}


void bt::addCondition(string s, btcondition cond)
{
	if (conditions->find(s) != conditions->end())
	{
		printf("Error: node %s already has a condition\n", s.c_str());
		return;	// if condition already exists don't insert again...
	}
	//conditions[s] = cond;
	conditions->operator[](s) = cond;
}


bool bt::testCondition(string s)
{
	if (conditions->find(s) == conditions->end())
	{
		return true;	// error: no condition defined, we assume TRUE
	}
		/*if (current!=previous){
			on_enter = true;
		}else{
			on_enter = false;
		}
		previous = current;*/
	//return (this->*conditions[s])();
	return (this->*conditions->operator[](s))();
}

//------------------------------------------------------------------------------------------------------

void bt::update(float elapsed){
	//Debería hacer el update de la clase derivada
}

std::string bt::getCurrentNode(){
	//Debería hacer el currentNode de la clase derivada
	return "derivada";
}

CHandle bt::GetEntity(){
	return entity;
}

void bt::SetEntity(CHandle the_entity)
{
	entity = the_entity;
}

void bt::initDecoratorCondition(string name, float t){
	btnode *p = findNode(name);
	if (p != nullptr){
		p->decorator_node->initCondition(t);
	};
	//xassert si no existe nodo
	CErrorContext ec("Initializing decorator node", "Behaviour trees");
	XASSERT(p == nullptr, "node doesn't exists");
}

void bt::setDecoratorCondition(string name, bool t){
	btnode *p = findNode(name);
	if (p != nullptr){
		p->decorator_node->updateCondition(t);
	}
	//xassert si no existe nodo
	CErrorContext ec("Setting condition decorator node", "Behaviour trees");
	XASSERT(p == nullptr, "node doesn't exists");
}

bool bt::getDecoratorCondition(string name){
	btnode *p = findNode(name);
	if (p != nullptr){
		return p->decorator_node->getCondition();
	}
	//xassert si no existe nodo
	CErrorContext ec("Getting condition decorator node", "Behaviour trees");
	XASSERT(p == nullptr, "node doesn't exists");
	return NULL;
}

void bt::setId(unsigned int id){
	//Debería delegar en la clase hija
}

unsigned int bt::getInt(){
	//Debería delegar en la clase hija
	return 0;
}

CHandle bt::getTransform(){
	//Debería delegar en la clase hija
	CHandle aux;
	return aux;
}

void bt::checkIfStateChanged(btnode* nd){
	if (nd != previous){
		on_enter = true;
		state_time = 0;
	}
	else{
		on_enter = false;
	}
	previous = nd;
}
//------------------------------------------------------------------------------------------------------