/*
 * ActiveModule.cpp
 *
 *  Versión: 7 Mar 2018
 *  Author: raulMrello
 */

#include "ActiveModule.h"


//------------------------------------------------------------------------------------
//-- PRIVATE TYPEDEFS ----------------------------------------------------------------
//------------------------------------------------------------------------------------
#define _MODULE_ 	_name
#define _EXPR_		(_defdbg && !IS_ISR())



//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
ActiveModule::ActiveModule(const char* name, osPriority priority, uint32_t stack_size, FSManager* fs, bool defdbg) : StateMachine(){

	// Inicializa flag de estado, propiedades internas y thread
	_ready = false;
	_defdbg = defdbg;
	strcpy((char*)_name, "[");
	strncat((char*)_name, name, MaxNameLength-2);
	strcat((char*)_name, "]");
	memset(&_name[strlen(_name)], '.', MaxNameLength - strlen(_name) + 1);
	_name[MaxNameLength] = 0;
	_fs = fs;
	_th = new Thread(priority, stack_size, NULL, name);
	_pub_topic_base = NULL;
	_sub_topic_base = NULL;
	_wdt_handled = false;
	_wdt_millis = osWaitForever;

    // Asigno manejador de mensajes en el Mailbox
    StateMachine::attachMessageHandler(new Callback<osStatus(State::Msg*)>(this, &ActiveModule::putMessage));

    // creo máquinas de estado inicial
    _stInit.setHandler(callback(this, &ActiveModule::Init_EventHandler));

    // Inicia thread
	_th->start(callback(this, &ActiveModule::task));
	_sem_th.wait();
}



//------------------------------------------------------------------------------------
#if ESP_PLATFORM == 1
void ActiveModule::attachToTaskWatchdog(uint32_t millis) {
	if(esp_task_wdt_add(_th->get_id()) != ESP_OK){
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "Error registrando tarea %x en TaskWatchdog", (uint32_t)_th->get_id());
	}
	else{
		_wdt_handled = true;
		_wdt_millis = millis;
		DEBUG_TRACE_D(_EXPR_, _MODULE_, "Registrada tarea %x en TaskWatchdog", (uint32_t)_th->get_id());
	}
}
#endif



//------------------------------------------------------------------------------------
#if ESP_PLATFORM == 1
void ActiveModule::detachFromTaskWatchdog() {
	if(esp_task_wdt_delete(_th->get_id()) != ESP_OK){
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "Error des-registrando tarea %x de TaskWatchdog", (uint32_t)_th->get_id());
	}
	else{
		_wdt_handled = false;
		_wdt_millis = osWaitForever;
		DEBUG_TRACE_D(_EXPR_, _MODULE_, "Desregistrada tarea %x de TaskWatchdog", (uint32_t)_th->get_id());
	}
}
#endif


//------------------------------------------------------------------------------------
osStatus ActiveModule::putMessage(State::Msg *msg){
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

    // asigna máquina de estados por defecto  y la inicia
    initState(&_stInit);

    // Ejecuta máquinas de estados y espera mensajes que son delegados a la máquina de estados
    // de la clase heredera
    for(;;){
        osEvent oe = getOsEvent();
        run(&oe);
    }
}



//------------------------------------------------------------------------------------
osEvent ActiveModule::getOsEvent(){
	uint32_t millis = (_wdt_handled)? _wdt_millis : osWaitForever;
	osEvent oe;
	do{
		oe = _queue.get(millis);
		if(_wdt_handled){
			_wdogKick();
		}
	}while (oe.status == osEventTimeout);
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
		DEBUG_TRACE_D(_EXPR_, _MODULE_, "Parm %s recuperados de memoria NV", param_id);
	}
	_fs->close();
	return ((err == osOK)? true : false);
}
