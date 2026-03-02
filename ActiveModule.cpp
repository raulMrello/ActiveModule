/*
 * ActiveModule.cpp
 *
 *  Versi�n: 7 Mar 2018
 *  Author: raulMrello
 */

#include "ActiveModule.h"

#if ESP_PLATFORM == 1
#include "esp_heap_caps.h"
#include "sdkconfig.h"
#include "soc/soc_memory_types.h"
#endif


//------------------------------------------------------------------------------------
//-- PRIVATE TYPEDEFS ----------------------------------------------------------------
//------------------------------------------------------------------------------------
#define _MODULE_ 	_name
#define _EXPR_		(_defdbg && !IS_ISR())
int32_t ActiveModule::_max_queue_count = 0;


//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
ActiveModule::ActiveModule(const char* name, osPriority priority, uint32_t stack_size, FSManager* fs, bool defdbg, bool logActive, const char* logName, bool stack_in_external_memory) : StateMachine(), GlobalActiveModule(logActive, logName){
	_queue_count = 0;
	// Inicializa flag de estado, propiedades internas y thread
	_ready = false;
	_defdbg = defdbg;
	_wdt_topic = NULL;
	_wdt_name = NULL;
	strcpy((char*)_name, "[");
	strncat((char*)_name, name, MaxNameLength-2);
	strcat((char*)_name, "]");
	memset(&_name[strlen(_name)], '.', MaxNameLength - strlen(_name) + 1);
	_name[MaxNameLength] = 0;
	_fs = fs;
	_th_stack_mem = NULL;
	if(stack_in_external_memory){
		#if ESP_PLATFORM == 1
		#ifdef CONFIG_SPIRAM_ALLOW_STACK_EXTERNAL_MEMORY
		_th_stack_mem = (unsigned char*)heap_caps_malloc(stack_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
		if(_th_stack_mem == NULL){
			DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERROR reservando stack %s en PSRAM (%d bytes). Usando stack interno", name, stack_size);
		}
		else{
			DEBUG_TRACE_E(_EXPR_, _MODULE_, "Stack %s reservado en %s: %p (%d bytes)", name, esp_ptr_external_ram(_th_stack_mem) ? "PSRAM" : "RAM", _th_stack_mem, stack_size);
		}
		#else
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "CONFIG_SPIRAM_ALLOW_STACK_EXTERNAL_MEMORY no activo en este build. Usando stack interno");
		#endif
		#else
		(void)name;
		(void)stack_size;
		#endif
	}
	_th = new Thread(priority, stack_size, _th_stack_mem, name);
	_pub_topic_base = NULL;
	_sub_topic_base = NULL;
	_wdt_handled = false;
	_wdt_millis = osWaitForever;

	_moduleId = getNewModuleId();

	

    // Asigno manejador de mensajes en el Mailbox
    StateMachine::attachMessageHandler(new Callback<osStatus(State::Msg*)>(this, &ActiveModule::putMessage));

    // creo m�quinas de estado inicial
    _stInit.setHandler(callback(this, &ActiveModule::Init_EventHandler));

    // Inicia thread
	_th->start(callback(this, &ActiveModule::task));
	_sem_th.wait();
}

ActiveModule::~ActiveModule(){
	if (_wdt_topic != NULL) {
	  delete _wdt_topic;
	  _wdt_topic = NULL;
	}
	
	if (_wdt_name != NULL) {
	  delete _wdt_name;
	  _wdt_name = NULL;
	}
	if(_th != NULL){
	  delete(_th);
	  _th = NULL;
	}
	#if ESP_PLATFORM == 1
	if(_th_stack_mem){
		heap_caps_free(_th_stack_mem);
		_th_stack_mem = NULL;
	}
	#endif
	return;
  }



//------------------------------------------------------------------------------------
void ActiveModule::attachToTaskWatchdog(uint32_t millis, const char* wdog_topic, const char* wdog_name) {
	_wdt_topic = new char[strlen(wdog_topic)+1]();
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
	}
}


//------------------------------------------------------------------------------------
osStatus ActiveModule::putMessage(State::Msg *msg){
	msg->moduleId = (msg->moduleId == -1)? _moduleId : msg->moduleId;
	if(++_queue_count > _max_queue_count){
		_max_queue_count = _queue_count;
		DEBUG_TRACE_V(_EXPR_, _MODULE_, "QUEUE_COUNT = %d", _queue_count);
	}
    osStatus ost = _queue.put(msg, ActiveModule::DefaultPutTimeout);
    if(ost != osOK){
        DEBUG_TRACE_E(_EXPR_, _MODULE_, "QUEUE_PUT_ERROR %d", ost);
    }
    return ost;
}


//------------------------------------------------------------------------------------
//-- PROTECTED METHODS IMPLEMENTATION ------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
void ActiveModule::task() {
	_sem_th.release();

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

		if(checkInactiveModules(oe) != true){
        	run(&oe);
		}
    }
}



//------------------------------------------------------------------------------------
osEvent ActiveModule::getOsEvent(){
	uint32_t millis = (_wdt_handled)? _wdt_millis : osWaitForever;
	osEvent oe;
	do{
		oe = _queue.get(millis);
		// si est� habilitada la notificaci�n al task_watchdog...
		if(_wdt_handled){
			// publica keepalive
			int32_t err = MQ::SUCCESS;
			if((err = MQ::MQClient::publish(_wdt_topic, _wdt_name, strlen(_wdt_name)+1, &_publicationCb)) != MQ::SUCCESS){
				DEBUG_TRACE_E(_EXPR_, _MODULE_, "Error publicando %s desde %s", _wdt_topic, _wdt_name);
			}
		}
	}while (oe.status == osEventTimeout);
	_queue_count--;
	if(_queue_count < 0){
		_queue_count = 0;
	}
	return oe;
}

//------------------------------------------------------------------------------------
bool ActiveModule::saveParameter(const char* param_id, void* data, size_t size, NVSInterface::KeyValueType type){
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
bool ActiveModule::restoreParameter(const char* param_id, void* data, size_t size, NVSInterface::KeyValueType type){
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
bool ActiveModule::removeParameter(const char* param_id){
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

void ActiveModule::addInactiveModule(InactiveModule* module) {
	_inactiveModulesList.push_back(module);
}

void ActiveModule::checkInactiveModules(State::StateEvent* se){
	for(InactiveModule* module : _inactiveModulesList){
		module->checkActiveHandlers(se);
	}
}

bool ActiveModule::checkInactiveModules(osEvent oe){
	bool res = false;
	for(InactiveModule* module : _inactiveModulesList){
		res = module->checkActiveHandlers(oe);		
		if(res == true){
			break;
		}
	}
	return res;
}
