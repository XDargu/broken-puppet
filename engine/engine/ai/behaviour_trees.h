#ifndef _BEHAVIOUR_TREES_H_
#define _BEHAVIOUR_TREES_H_

//#include <string>
//#include <map>
#include "mcv_platform.h"
#include "../handle/handle.h"
#include "bt_node.h"
#include "aicontroller.h"
#include <assert.h>

using namespace std;

typedef int (bt::*btaction)();
typedef bool (bt::*btcondition)();

// Implementation of the behavior tree
// uses the BTnode so both work as a system
// tree implemented as a map of btnodes for easy traversal
// behaviours are a map to member function pointers, to 
// be defined on a derived class. 
// BT is thus designed as a pure abstract class, so no 
// instances or modifications to bt / btnode are needed...


class bt:public aicontroller
{
	// the nodes
	map<string, btnode *>*tree;
	// the C++ functions that implement node actions, hence, the behaviours
	map<string, btaction>* actions;
	// the C++ functions that implement conditions
	map<string, btcondition>* conditions;

	btnode *root;	
	btnode *previous;

	// moved to private as really the derived classes do not need to see this
	btnode *createNode(string);
protected:
	btnode *current;
	bool state_changed;

public:
	// Time in the state
	float state_time;
	// Just enter to a new state
	bool on_enter;

	string name;
	// use a derived create to declare BT nodes for your specific BTs
	virtual void create(string);
	// use this two calls to declare the root and the children. 
	// use NULL when you don't want a btcondition or btaction (inner nodes)
	btnode *createRoot(string, int, btcondition, btaction);
	btnode *addChild(string, string, int, typeInterrupAllowed, btcondition, btaction);
	btnode *addChild(string, string, int, btcondition, btaction);

	btnode *addChild(string, string, int, typeInterrupAllowed, subType, btcondition, btaction);
	btnode *addChild(string, string, int, subType, btcondition, btaction);

	//Sobrecargar del addChild para priorizar por pesos los hijos de los nodos random
	btnode *addChild(string parent, string son, int type, typeInterrupAllowed kind, btcondition btc, btaction bta, int weight);

	btnode *findNode(string);

	// internals used by btnode and other bt calls
	void addAction(string, btaction);
	int execAction(string, btnode* th);
	void addCondition(string, btcondition);
	bool testCondition(string);
	void setCurrent(btnode *);

	// call this once per frame to compute the AI. No need to derive this one, 
	// as the behaviours are derived via btactions and the tree is declared on create
	void recalc(float deltaTime);

	//--------------------------------------------------------------------------------------------
	CHandle entity;
	void SetEntity(CHandle the_entity);
	CHandle GetEntity();
	virtual void setId(unsigned int id);
	virtual unsigned int getInt();
	virtual std::string getCurrentNode();
	virtual void update(float elapsed);
	virtual CHandle getTransform();
	bool validateNode(string, int);
	bool validateNode(string, int, subType);
	void setDecoratorCondition(string name, bool t);
	bool getDecoratorCondition(string name);
	void initDecoratorCondition(string name, float t);
	void checkIfStateChanged(btnode* nd);

	float timer;
	float last_time;

	bt();
	~bt();
	//--------------------------------------------------------------------------------------------

};


#endif

