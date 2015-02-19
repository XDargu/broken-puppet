#ifndef INC_MSGS_H_
#define INC_MSGS_H_

typedef unsigned TMsgID;
TMsgID generateUniqueMsgID();
  
#define DECLARE_MSG_ID() \
	static TMsgID getID() {  \
		static TMsgID unique_id = generateUniqueMsgID();  \
		return unique_id;  \
	}

struct TMsgExplosion {
	XMVECTOR source;
	float    radius;
	float    damage;
	DECLARE_MSG_ID();
};

struct TMsgDied {
	int     who;
	TMsgDied( int awho ) : who (awho ) { }
	DECLARE_MSG_ID();
};

// An interface to call something on a handle
struct IFunctorBase {
  virtual ~IFunctorBase() {}
  virtual void execute(CHandle handle, const void* msg) = 0;
};

template< class TObj, class TMsgData > 
struct TFunctor : public IFunctorBase {

  // TMemberFn = Un typedef a un puntero a member de la clase TObj 
  // que recibe un handle y const TObj&
  typedef void (TObj::*TMemberFn)(const TMsgData& msg);
  TMemberFn member;

  // En el constructor, me tienen que dar el método de la clase
  // TObj que tengo que llamar
  TFunctor(TMemberFn amember) : member(amember){}
  
  // 
  void execute(CHandle handle, const void* msg) {
	assert( msg );
    // Convertir el handle en obj real
    TObj* obj = handle;

    // El objecto deberia de ser valido?
    assert(obj);

    // Si tenemos obj, llamar al metodo que me dijeron
    // pasando como argumento el msg que me han dado
    if (obj)
      (obj->*member)(* (const TMsgData*) msg);
  }

};

// El registro se compone del tipo de objetos y a que metodo
// del objeto hay que llamar
struct TComponentMsgHandler {
  uint32_t         comp_type;
  IFunctorBase*    method;
};

#include <map>
typedef std::multimap< TMsgID, TComponentMsgHandler > MMsgSubscriptions;

// The global register of subscriptions
extern MMsgSubscriptions msg_subscriptions;

// To subscribe, we give the msg_id and the method to call
template< class TObj >
void subscribe(TMsgID msg_id, IFunctorBase* method) {
  std::pair<TMsgID, TComponentMsgHandler> e;
  e.first = msg_id;
  // Deduce the comp_type from TObj
  e.second.comp_type = getObjManager<TObj>()->getType();
  e.second.method = method;
  msg_subscriptions.insert(e);
}

#define SUBSCRIBE(acomp,amsg_arg,amethod) \
  subscribe<acomp>(amsg_arg::getID(), new TFunctor<acomp,amsg_arg>(&acomp::amethod));



#endif
