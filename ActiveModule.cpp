/*
 * ActiveModule.cpp
 *
 *  Versión: 13 Feb 2018
 *  Author: raulMrello
 */

#include "ActiveModule.h"


//------------------------------------------------------------------------------------
//-- PRIVATE TYPEDEFS ----------------------------------------------------------------
//------------------------------------------------------------------------------------

/** Macro para imprimir trazas de depuración, siempre que se haya configurado un objeto
 *	Logger válido (ej: _debug)
 */

#define DEBUG_TRACE(format, ...)			\
if(_defdbg){								\
	printf(format, ##__VA_ARGS__);			\
}											\


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

    // Asigno manejador de mensajes en el Mailbox
    StateMachine::attachMessageHandler(new Callback<void(State::Msg*)>(this, &ActiveModule::putMessage));

    // creo máquinas de estado inicial
    _stInit.setHandler(callback(this, &ActiveModule::Init_EventHandler));

    // Inicia thread
	_th->start(callback(this, &ActiveModule::task));
}


//------------------------------------------------------------------------------------
//-- PROTECTED METHODS IMPLEMENTATION ------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
void ActiveModule::task() {

    // espera a que se asigne un topic base
    do{
    	Thread::wait(1);
    }while(!_pub_topic_base || !_sub_topic_base);

    // asigna máquina de estados por defecto  y la inicia
    initState(&_stInit);

    // marca como listo para ejecución
    _ready = true;

    // Ejecuta máquinas de estados y espera mensajes que son delegados a la máquina de estados
    // de la clase heredera
    for(;;){
        osEvent oe = getOsEvent();
        run(&oe);
    }
}


//------------------------------------------------------------------------------------
bool ActiveModule::saveParameter(const char* param_id, void* data, size_t size, NVSInterface::KeyValueType type){
	int err;
	if(!_fs->open()){
		DEBUG_TRACE("\r\n%s ERR_NVS No se puede abrir el sistema NVS", (char*)_name);
		return false;
	}
	if((err = _fs->save(param_id, data, size, type)) != osOK){
		DEBUG_TRACE("\r\n%s ERR_NVS [0x%x] grabando %s", (char*)_name, (int)err, param_id);
	}
	else{
		DEBUG_TRACE("\r\n%s Parm %s guardados en memoria NV", (char*)_name, param_id);
	}
	_fs->close();
	return ((err == osOK)? true : false);
}


//------------------------------------------------------------------------------------
bool ActiveModule::restoreParameter(const char* param_id, void* data, size_t size, NVSInterface::KeyValueType type){
	int err;
	if(!_fs->open()){
		DEBUG_TRACE("\r\n%s ERR_NVS No se puede abrir el sistema NVS", _name);
		return false;
	}
	if((err = _fs->restore(param_id, data, size, type)) != osOK){
		DEBUG_TRACE("\r\n%s ERR_NVS [0x%x] recuperando %s", (char*)_name, (int)err, param_id);
	}
	else{
		DEBUG_TRACE("\r\n%s Parm %s recuperados de memoria NV", (char*)_name, param_id);
	}
	_fs->close();
	return ((err == osOK)? true : false);
}
