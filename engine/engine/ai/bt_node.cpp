#include "mcv_platform.h"
#include "bt_node.h"
#include "behaviour_trees.h"


btnode::btnode(string s)
{
	name = s;
}


void btnode::create(string s)
{
	name = s;
}


bool btnode::isRoot()
{
	return (parent == NULL);
}


void btnode::setParent(btnode *p)
{
	parent = p;
}


void btnode::setRight(btnode *p)
{
	right = p;
}


void btnode::setType(int t)
{
	type = t;
}

void btnode::setDecSubType(subType t){
	if (t == DEC_LIMIT_TIMES){
		decSubType = t;
		decorator_node=new decorator_limiter;
		decorator_node->init(t);
	}else if (t == CONDITIONAL_NODE){
		decSubType = t;
		decorator_node = new decorator_conditional;
		decorator_node->init(t);
	}else if (t == TIMER_NODE){
		decSubType = t;
		decorator_node = new decorator_timer;
		decorator_node->init(t);
	}else if (t == LOOP_UNTIL){
		decSubType = t;
		decorator_node = new decorator_loop;
		decorator_node->init(t);
	}
}

void btnode::setTypeInter(typeInterrupAllowed t){
	typeInter = t;
}

typeInterrupAllowed btnode::getTypeInter(){
	return typeInter;
}

string btnode::getName()
{
	return name;
}


void btnode::addChild(btnode *c)
{
	if (!children.empty()) // if this node already had children, connect the last one to this
		children.back()->setRight(c);  // new one so the new one is to the RIGHT of the last one
	children.push_back(c); // in any case, insert it
	c->right = NULL; // as we're adding from the right make sure right points to NULL
}


void btnode::recalc(bt *tree)
{
	// activate the next line to debug
	printf("recalcing node %s\n", name.c_str());
	switch (type)
	{
	case ACTION:
	{
		// run the controller of this node
		int res = tree->execAction(name);
		// now, the next lines compute what's the NEXT node to use in the next frame...
		if (res == STAY) { tree->setCurrent(this); return; }// looping vs. on-shot actions
		// climb tree iteratively, look for the next unfinished sequence to complete
		btnode *cand = this;
		while (cand->parent != NULL)
		{
			btnode *daddy = cand->parent;
			if (daddy->type == SEQUENCE)
				// oh we were doing a sequence. make sure we finished it!!!
			{
				if (cand->right != NULL)
				{
					tree->setCurrent(cand->right);
					break;
				}
				// sequence was finished (there is nobody on right). Go up to daddy.
				else cand = daddy;
			}
			// i'm not on a sequence, so keep moving up to the root of the BT
			else cand = daddy;
		}
		// if we've reached the root, means we can reset the traversal for next frame.
		if (cand->parent == NULL) tree->setCurrent(NULL);
		break;
	}
	case RANDOM:
	{
		int r = rand() % children.size();
		children[r]->recalc(tree);
		break;
	}
	case PRIORITY:
	{
		for (int i = 0; i<children.size(); i++)
		{
			if (tree->testCondition(children[i]->getName()))
			{
				children[i]->recalc(tree);
				break;
			}
		}
		break;
	}
	case DECORATOR:
	{
		if (decSubType == DEC_LIMIT_TIMES){
				if (!decorator_node->getCondition()){
				for (int i = 0; i<children.size(); i++)
				{
					if (tree->testCondition(children[i]->getName()))
					{
						children[i]->recalc(tree);
						decorator_node->updateCondition(true);
						break;
					}
				}
			}
		}
		else if (decSubType == CONDITIONAL_NODE){
			btnode *cand = this;
			if (!decorator_node->getCondition()){
				//la condicion es false, hacemos recalc de los hijos
				for (int i = 0; i < children.size(); i++)
				{
					children[i]->recalc(tree);
					break;
				}
			}else{
				if (cand->right != NULL){
					//el nodo ya ha sido ejecutado y tiene hermano. Nos situamos en él
					tree->setCurrent(right);
					break;
				}else if (cand->parent != NULL){
					//el nodo ya ha sido ejecutado y tiene padre. Nos situamos en él
					tree->setCurrent(parent);
					break;
				}else{
					// el nodo ya ha sido ejecutado y no tiene padre. Volvemos a la raiz
					tree->setCurrent(NULL);
					break;
				}
			}
		}else if (decSubType == TIMER_NODE){
			btnode *cand = this;
			decorator_node->updateCondition(true);
			if (decorator_node->getCondition()){
				//la condicion es cierta, ejecutamos la action
				for (int i = 0; i < children.size(); i++)
				{
					children[i]->recalc(tree);
					break;
				}
			}else if (cand->right != NULL){
				//el nodo ya ha sido ejecutado y tiene hermano. Nos situamos en él
				tree->setCurrent(right);
				break;
			}else if (cand->parent != NULL){
				//la cond es false y tiene padre -> backtracking al padre
				tree->setCurrent(parent);
				break;
			}else{
				//No tiene padre, volvemos al root
				tree->setCurrent(NULL);
				break;
			}
			break;
		}
		else if (decSubType == LOOP_UNTIL){
			btnode *cand = this;
			if (decorator_node->getCondition()){
				//la condicion es false, hacemos recalc de los hijos
				for (int i = 0; i < children.size(); i++)
				{
					children[i]->recalc(tree);
					break;
				}
			}
			else{
				if (cand->right != NULL){
					//el nodo ya ha sido ejecutado y tiene hermano. Nos situamos en él
					tree->setCurrent(right);
					break;
				}
				else if (cand->parent != NULL){
					//el nodo ya ha sido ejecutado y tiene padre. Nos situamos en él
					tree->setCurrent(parent);
					break;
				}
				else{
					// el nodo ya ha sido ejecutado y no tiene padre. Volvemos a la raiz
					tree->setCurrent(NULL);
					break;
				}
			}
		}
		break;
	}
	case SEQUENCE:
	{
		// begin the sequence...the inner node (action) will take care of the sequence
		// via the "setCurrent" mechanism
		children[0]->recalc(tree);
		break;
	}
	}
}
