/*
 * TaskTaskWatchdog.cpp
 *
 *  Versión: Dic 2019
 *  Author: raulMrello
 *
 *	Componente que utiliza publicaciones en MQLib para recibir los keep-alive de los componentes que se hayan
 *	registrado. Cuando un componente no actualiza su keep-alive en el plazo establecido se notificará el error
 *	como una publicación en el topic stat/timeout/$TWD indicando el nombre del componente que ha fallado, además
 *	se invocará a la callback instalada en caso de timeout en el plazo establecido.
 */

#include "TaskWatchdog.h"


//------------------------------------------------------------------------------------
//-- PRIVATE TYPEDEFS ----------------------------------------------------------------
//------------------------------------------------------------------------------------

#define _MODULE_ 	"[TaskWdog]......"
#define _EXPR_		(!IS_ISR())



//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
TaskWatchdog::TaskWatchdog(const char* name, const char* keepalive_text, esp_log_level_t level) : _name(name), _ka_text(keepalive_text){
	esp_log_level_set(_MODULE_, level);
	_createTimer();
	_err_pre_callback = 0;
	_err_cb = (Callback<void()>) NULL;
	_publicationCb = callback(this, &TaskWatchdog::_onPublished);
	_subscriptionCb = callback(this, &TaskWatchdog::_onSubscription);
}


//------------------------------------------------------------------------------------
TaskWatchdog::~TaskWatchdog(){
	stop();
}


//------------------------------------------------------------------------------------
void TaskWatchdog::start(uint32_t timeout){
	if(_tmr == NULL){
		_createTimer();
	}
	_timeout = timeout;
	_eraseRegisteredComponents();
	if(MQ::MQClient::subscribe(_ka_text, &_subscriptionCb) == MQ::SUCCESS){
		DEBUG_TRACE_I(_EXPR_, _MODULE_, "Iniciando TWD con timeout=%d ms", _timeout);
		_tmr->start(_timeout);
	}
	else{
		DEBUG_TRACE_E(_EXPR_, _MODULE_, "ERROR iniciando TWD");
	}
}


//------------------------------------------------------------------------------------
void TaskWatchdog::stop(){
	if(_tmr){
		DEBUG_TRACE_I(_EXPR_, _MODULE_, "Deteniendo TWD");
		delete(_tmr);
		_tmr = NULL;
	}
	_eraseRegisteredComponents();
}


//------------------------------------------------------------------------------------
//-- PROTECTED METHODS IMPLEMENTATION ------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
void TaskWatchdog::_eraseRegisteredComponents(){
	while(_components.size() > 0){
		auto it = _components.begin();
		DEBUG_TRACE_D(_EXPR_, _MODULE_, "Eliminando componente: %s", it->first.c_str());
		_components.erase(it);
	}
}


//------------------------------------------------------------------------------------
void TaskWatchdog::_createTimer(){
	#if __MBED__==1
	_tmr = new RtosTimer(callback(this, &TaskWatchdog::_onTick), osTimerPeriodic);
	#elif ESP_PLATFORM==1
	_tmr = new RtosTimer(callback(this, &TaskWatchdog::_onTick), osTimerPeriodic, "TWD");
	#endif
	MBED_ASSERT(_tmr);
}


//------------------------------------------------------------------------------------
void TaskWatchdog::_onTick(){
	std::string result = "";
	// recorre el mapa en busca de algún componente que no haya incrementado su contador de notificaciones
	for(auto it=_components.begin(); it!=_components.end();++it){
		if(it->second == 0){
			result += it->first + ",";
		}
		else{
			it->second = 0;
		}
	}
	const char* txt = result.c_str();
	// en caso de haber algún componente que no ha notificado en el plazo...
	if(strlen(txt) > 0){
		DEBUG_TRACE_W(_EXPR_, _MODULE_, "TWD_Timeout en: %s", txt);
		// publica el mensaje de error
		char* topic = new char[MQ::DefaultMaxTopicNameLength]();
		MBED_ASSERT(topic);
		sprintf(topic, "stat/timeout/%s", _name);
		MQ::MQClient::publish(topic, txt, strlen(txt)+1, &_publicationCb);
		// espera la temporización que esté pactada y posteriormente ejecuta callback
		if(_err_pre_callback){
			Thread::wait(_err_pre_callback);
			if(_err_cb){
				_err_cb();
			}
		}
	}
}


//------------------------------------------------------------------------------------
void TaskWatchdog::_onSubscription(const char* name, void* msg, uint16_t msg_len){
	// comprueba si el texto coincide con el tamaño recibido
	if(strlen((char*)msg)+1 == msg_len){
		// chequea si existe el componente en el mapa para incrementar su contador
		std::map<std::string, uint32_t>::iterator it = _components.find((char*)msg);
		if(it!=_components.end()){
			// incrementa el contador de notificaciones
			it->second++;
			DEBUG_TRACE_I(_EXPR_, _MODULE_, "Notificacion TWD recibida del Componente <%s>", it->first.c_str());
			return;
		}
		// si no hay ningún elemento en el mapa, con ese topic, lo crea
		char* c = new char[msg_len]();
		MBED_ASSERT(c);
		strcpy(c, (char*)msg);
		_components.insert(std::pair<std::string, uint32_t>(c, 1));
		DEBUG_TRACE_I(_EXPR_, _MODULE_, "Componente <%s> registrado en TWD", c);
		return;
	}
}
