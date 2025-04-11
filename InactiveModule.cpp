#include "InactiveModule.h"


//------------------------------------------------------------------------------------
//-- PRIVATE TYPEDEFS ----------------------------------------------------------------
//------------------------------------------------------------------------------------
#define _MODULE_ 	_name
#define _EXPR_		(_defdbg && !IS_ISR())
//int32_t InactiveModule::_max_queue_count = 0;

static int moduleId = 0;


//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
InactiveModule::InactiveModule(const char* name, FSManager* fs, bool defdbg, bool logActive, const char* logName) : StateMachine(), GlobalActiveModule(logActive, logName){
	//_queue_count = 0;
	// Inicializa flag de estado, propiedades internas y thread
	_ready = false;
	_defdbg = defdbg;
	strcpy((char*)_name, "[");
	strncat((char*)_name, name, MaxNameLength-2);
	strcat((char*)_name, "]");
	memset(&_name[strlen(_name)], '.', MaxNameLength - strlen(_name) + 1);
	_name[MaxNameLength] = 0;
	_fs = fs;
	_pub_topic_base = NULL;
	_sub_topic_base = NULL;
    _activeModule = NULL;
    _moduleId = moduleId++;
	
	//_wdt_handled = false;
	//_wdt_millis = osWaitForever;

    // Asigno manejador de mensajes en el Mailbox
    //StateMachine::attachMessageHandler(new Callback<osStatus(State::Msg*)>(this, &InactiveModule::putMessage));

    // creo m�quinas de estado inicial
    _stInit.setHandler(callback(this, &InactiveModule::Init_EventHandler));
    handlersList.push_back(callback(this, &InactiveModule::Init_EventHandler));
    

    // Inicia thread
	//_th->start(callback(this, &InactiveModule::task));
	//_sem_th.wait();
}



//------------------------------------------------------------------------------------
void InactiveModule::attachToTaskWatchdog(uint32_t millis, const char* wdog_topic, const char* wdog_name) {
	/*_wdt_topic = new char[strlen(wdog_topic)+1]();
	MBED_ASSERT(_wdt_topic);
	strcpy(_wdt_topic, wdog_topic);
	_wdt_name = new char[strlen(wdog_name)+1]();
	MBED_ASSERT(_wdt_name);
	strcpy(_wdt_name, wdog_name);

	if(MQ::MQClient::publish(_wdt_topic, _wdt_name, strlen(_wdt_name)+1, &_publicationCb) == MQ::SUCCESS){
		_wdt_handled = true;
		_wdt_millis = millis;
		DEBUG_TRACE_D(_EXPR_, _MODULE_, "Registrado componente %s en TaskWatchdog", _wdt_name);
	}
	else{
		_wdt_handled = false;
		_wdt_millis = osWaitForever;
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERROR: Registrando componente %s en TaskWatchdog", _wdt_name);
	}*/
    DEBUG_TRACE_E(_EXPR_, _MODULE_, "TaskWatchdog no disponible para Inactive Module");
}


//------------------------------------------------------------------------------------
osStatus InactiveModule::putMessage(State::Msg *msg){
    msg->moduleId = _moduleId;
    return _activeModule->putMessage(msg);
}


//------------------------------------------------------------------------------------
//-- PROTECTED METHODS IMPLEMENTATION ------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
void InactiveModule::task() {
	/*_sem_th.release();

    // espera a que se asigne un topic base
    while(!_pub_topic_base || !_sub_topic_base){
    	Thread::wait(100);
    }

    // asigna m�quina de estados por defecto  y la inicia
    initState(&_stInit);

    // Ejecuta m�quinas de estados y espera mensajes que son delegados a la m�quina de estados
    // de la clase heredera
    for(;;){
        osEvent oe = getOsEvent();
        run(&oe);
    }*/
}



//------------------------------------------------------------------------------------
osEvent InactiveModule::getOsEvent(){
	//return _parent->getOsEvent();
	osEvent oe = {0}; 
	return oe;
}

//------------------------------------------------------------------------------------
bool InactiveModule::saveParameter(const char* param_id, void* data, size_t size, NVSInterface::KeyValueType type){
	int err;
	if(!_fs->open()){
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_NVS No se puede abrir el sistema NVS");
		return false;
	}
	if((err = _fs->save(param_id, data, size, type)) != osOK){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS [0x%x] grabando %s", (int)err, param_id);
	}
	else{
		DEBUG_TRACE_D(_EXPR_, _MODULE_, "Parm %s guardados en memoria NV", param_id);
	}
	_fs->close();
	return ((err == osOK)? true : false);
}


//------------------------------------------------------------------------------------
bool InactiveModule::restoreParameter(const char* param_id, void* data, size_t size, NVSInterface::KeyValueType type){
	int err;
	if(!_fs->open()){
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_NVS No se puede abrir el sistema NVS");
		return false;
	}
	if((err = _fs->restore(param_id, data, size, type)) != osOK){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS [0x%x] recuperando %s", (int)err, param_id);
	}
	else{
		DEBUG_TRACE_D(_EXPR_, _MODULE_, "Parm %s recuperado de memoria NV", param_id);
	}
	_fs->close();
	return ((err == osOK)? true : false);
}

//------------------------------------------------------------------------------------
bool InactiveModule::removeParameter(const char* param_id){
	int err;
	if(!_fs->open()){
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERR_NVS No se puede abrir el sistema NVS");
		return false;
	}
	if((err = _fs->removeKey(param_id) != osOK)){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "ERR_NVS [0x%x] eliminando %s", (int)err, param_id);
	}
	else{
		DEBUG_TRACE_D(_EXPR_, _MODULE_, "Parm %s recuperados de memoria NV", param_id);
	}
	_fs->close();
	return ((err == osOK)? true : false);
}

bool InactiveModule::ready(){
    if(_pub_topic_base && _sub_topic_base){
		State::StateEvent se;
		se.evt = (State::Event_type)State::EV_ENTRY;
        Init_EventHandler(&se);
        return _ready;
    }
    return false;
}

void InactiveModule::setActiveModule(ActiveModule* parent){
    _activeModule = parent;
	parent->addInactiveModule(this);
}

void InactiveModule::checkActiveHandlers(State::StateEvent* se){
    if(((State::Msg*)se->oe->value.p)->moduleId == _moduleId){
        for(State::EventHandler it : handlersList){
            it.call(se);
        }
	}
}

bool InactiveModule::checkActiveHandlers(osEvent oe){
    if(((State::Msg*)(oe.value.p))->moduleId == _moduleId){
		run(&oe);
		return true;
	}
	return false;
}

void InactiveModule::checkInactiveModules(State::StateEvent* se){
	DEBUG_TRACE_W(_EXPR_, _MODULE_, "No es posible gestionar modulos inactivos desde un modulo inactivo");
}

bool InactiveModule::nextState(){
	return false;
}